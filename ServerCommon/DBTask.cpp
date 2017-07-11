#include "DBTask.h"
#include "DBRequest.h"
#include "log4z.h"
#define  MYSQL_PING_TIME (3600*8 + 30)
CDBTask::CDBTask( uint32_t nTaskID,const char* pIP,unsigned pPort , const char* pUserName,const char* pPassword, const char* pDBName ) 
	:ITask(nTaskID),m_pRequest( new stDBRequest() ),m_pResult(new stDBResult() ),m_pMySql(nullptr),m_tNextMysqlPingTime(0)
{
	m_strIP = pIP ;
	m_nPort = pPort ;
	m_strUserName = pUserName ;
	m_strPassword = pPassword ;
	m_strDBName = pDBName ;
}

CDBTask::~CDBTask()
{
	if ( m_pMySql )
	{
		mysql_close(m_pMySql) ;
		m_pMySql = nullptr ;
	}
}

bool CDBTask::setupMysqlConnection()
{
	if ( m_pMySql )
	{
		printf("already have my sql object \n") ;
		return true;
	}

	m_pMySql = mysql_init(NULL);
	char bReconnect = 1 ;
	mysql_options(m_pMySql,MYSQL_OPT_RECONNECT,&bReconnect);
	if ( !mysql_real_connect(m_pMySql,m_strIP.c_str(),m_strUserName.c_str(),m_strPassword.c_str(),m_strDBName.c_str(),m_nPort,NULL,CLIENT_MULTI_STATEMENTS) )
	{
		fprintf(stderr, "Failed to connect to database: Error: %s\\n",  mysql_error(m_pMySql));
		mysql_close(m_pMySql) ;
		m_pMySql = NULL ;
		return  false;
	}

	if (!mysql_set_character_set(m_pMySql, "utf8mb4"))
	{
		printf("New client character set: %s\n",
			mysql_character_set_name(m_pMySql));
	}
	else
	{
		fprintf(stderr, "Failed to connect to database: Error: %s\n",
			mysql_error(m_pMySql));
	}

	uint32_t nVer = mysql_get_client_version();
	printf("mysql client ver : %u \n",nVer ) ;
	nVer = mysql_get_server_version(m_pMySql);
	printf("mysql server ver : %u \n",nVer ) ;

	MY_CHARSET_INFO tep ;
	mysql_get_character_set_info(m_pMySql,&tep);    

	//mysql_set_server_option( m_pMySql, MYSQL_OPTION_MULTI_STATEMENTS_ON ); s
	m_tNextMysqlPingTime = time(NULL) + MYSQL_PING_TIME;
	return true ;
}

uint8_t CDBTask::performTask()
{
	if ( m_pMySql == nullptr )
	{
		auto ret = setupMysqlConnection();
		if ( ret == false )
		{
			printf("connect data base failed") ;
			getDBResult()->reset();
			return 1 ;
		}
	}

	// check state
	if ( time(NULL) >= m_tNextMysqlPingTime )
	{
		unsigned long id = mysql_thread_id(m_pMySql);
		mysql_ping(m_pMySql);
		if ( id != mysql_thread_id(m_pMySql) )
		{
			// reconnected ;
			if (!mysql_set_character_set(m_pMySql, "utf8mb4"))
			{
				printf("Reconnect !!! New client character set: %s\n",
					mysql_character_set_name(m_pMySql));
			}
		}
		m_tNextMysqlPingTime = time(NULL) + MYSQL_PING_TIME;
	}

	// do request 
	return doRequest(getDBRequest());
}

uint8_t CDBTask::doRequest(DBRequest_ptr ptr )
{
	if ( !ptr )
	{
		printf("why DB request is null ? \n") ;
		return 1;
	}


	// process request here ;
 
	stDBRequest* pRequest = getDBRequest().get() ;
	stDBResult* pResult = getDBResult().get() ;
	pResult->reset();

	MYSQL_RES *msqlResult = NULL ;
	MYSQL_ROW msqlrow;
	MYSQL_FIELD *msqlfield;
 
	pResult->nRequestUID = pRequest->nRequestUID ;
	pResult->pUserData = pRequest->pUserData ;
	pResult->nAffectRow = 0 ;
	if ( mysql_real_query(m_pMySql,pRequest->pSqlBuffer,pRequest->nSqlBufferLen) )
	{
		LOGFMTE("query DB Error Info , Operate UID = %d : %s . sql: = %s\n", pRequest->nRequestUID, mysql_error(m_pMySql), pRequest->pSqlBuffer);
		pResult->nAffectRow = 0 ;
		return 1;
	}

	do 
	{
		switch ( pRequest->eType )
		{
		case eRequestType_Add:
		case eRequestType_Delete:
		case eRequestType_Update:
			{
				pResult->nAffectRow = (unsigned int)mysql_affected_rows(m_pMySql);
				msqlResult = mysql_store_result( m_pMySql ); 
				mysql_free_result(msqlResult); 
			}
			break;
		case eRequestType_Select:
			{
				msqlResult = mysql_store_result(m_pMySql);

				if ( msqlResult == NULL )
				{
					mysql_free_result(msqlResult);
					continue;
				}

				if ( pResult->nAffectRow >= 1 )
				{
					if ( msqlResult != NULL )
					{
						printf("mysql_store_result Error Info , Operate UID = %d : %s why have more than one result type  result \n", pRequest->nRequestUID, mysql_error(m_pMySql));
					}
					//pResult->nAffectRow = 0 ;
					mysql_free_result(msqlResult);
					continue;
				}

				pResult->nAffectRow += (unsigned int)mysql_num_rows(msqlResult);
				// process row ;
				int nNumFiled = mysql_num_fields(msqlResult);
				while ( msqlrow = mysql_fetch_row(msqlResult))
				{
					mysql_field_seek(msqlResult,0); // reset o begin ;
					CMysqlRow* rowData = new CMysqlRow ;
					unsigned long* pLengths = mysql_fetch_lengths(msqlResult);
					for ( int i = 0 ; i < nNumFiled ; ++i )
					{
						msqlfield = mysql_fetch_field(msqlResult);
						if ( !msqlfield )
						{
							continue;
						}
						stMysqlField* pField = new stMysqlField(msqlfield->name,msqlfield->name_length) ;
						pField->nBufferLen = pLengths[i] ;
						bool bValide = true ;
						switch (msqlfield->type)
						{
						case MYSQL_TYPE_TINY: // char
						case MYSQL_TYPE_SHORT: // short 
						case MYSQL_TYPE_LONG: // int
						case MYSQL_TYPE_LONGLONG: // 64 bit int 
						case MYSQL_TYPE_INT24:
							{
								pField->nValueType = eValue_Int ;
								bValide = true;
							}
							break;
						case MYSQL_TYPE_FLOAT: // float 
						case MYSQL_TYPE_DOUBLE: // double 
							{
								pField->nValueType = eValue_Float ;
								bValide = true;
							}
							break;
						case MYSQL_TYPE_BLOB: // binary 
						case MYSQL_TYPE_DATETIME:
						case MYSQL_TYPE_VAR_STRING:  // string 
							{
								pField->nValueType = eValue_String ;
								bValide = true;
							}
							break;
						default:
							{
								bValide = false ;
								printf("error DB request unsupport field Type : Type = %d : field Name: %s \n",msqlfield->type, pField->strFieldName.c_str()) ;
							}
						}

						if ( bValide )
						{
							pField->pBuffer = new char [pField->nBufferLen + 1 ] ;
							memset(pField->pBuffer,0,pField->nBufferLen + 1 );
							memcpy(pField->pBuffer,msqlrow[i],pLengths[i]);
						}
						else
						{
							delete pField ;
							printf("not support data type from db") ;
							assert(0&&"why support type not !");
							continue; 
						}
						rowData->PushFiled(pField);
					}
					pResult->vResultRows.push_back(rowData);
				}
				mysql_free_result(msqlResult);
			}
			break; 
		default:
			{
				printf("error DB request type, DB request UID = %d , Type = %d\n",pRequest->nRequestUID,pRequest->eType) ;
				continue; ;
			}
		}
	} while ( !mysql_next_result(m_pMySql) );

	m_tNextMysqlPingTime = time(NULL) + MYSQL_PING_TIME;  // do a mysql_ping() on the connection if there has been a long time [since the last query ]
	return 0 ;
}

void CDBTask::setDBRequest( DBRequest_ptr ptr )
{
	if ( !ptr )
	{
		printf("why db request is null ? \n") ;
		return ;
	}

	m_pRequest = ptr ;
}

CDBTask::DBRequest_ptr CDBTask::getDBRequest()
{
	return m_pRequest ;
}

CDBTask::DBResult_ptr CDBTask::getDBResult()
{
	return m_pResult ;
}