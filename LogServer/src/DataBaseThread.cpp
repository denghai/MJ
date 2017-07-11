#include "DBRequest.h"
#include "DataBaseThread.h"
#include "log4z.h"
#define  MYSQL_PING_TIME (3600*8 + 30)
//CDataBaseThread* CDataBaseThread::SharedDBThread()
//{
//	static CDataBaseThread g_sDBThread ;
//	return &g_sDBThread;
//}

bool CDataBaseThread::InitDataBase( const char* pIP,unsigned pPort , const char* pUserName,const char* pPassword, const char* pDBName )
{
	// connect to data base ;
	// init my_sql ;
	m_pMySql = mysql_init(NULL);
	char bReconnect = 1 ;
	mysql_options(m_pMySql,MYSQL_OPT_RECONNECT,&bReconnect);
	if ( !mysql_real_connect(m_pMySql,pIP,pUserName,pPassword,pDBName,pPort,NULL,CLIENT_MULTI_STATEMENTS) )
	{
		fprintf(stderr, "Failed to connect to database: Error: %s\\n",  mysql_error(m_pMySql));
		m_bRunning = false ;
		mysql_close(m_pMySql) ;
		m_pMySql = NULL ;
		return false ;
	}

	if (!mysql_set_character_set(m_pMySql, "utf8"))
	{
		printf("New client character set: %s\n",
			mysql_character_set_name(m_pMySql));
	}

	m_bRunning = true ;
	//mysql_set_server_option( m_pMySql, MYSQL_OPTION_MULTI_STATEMENTS_ON ); s
	m_tNextMysqlPingTime = time(NULL) + MYSQL_PING_TIME;
	return true ;
}

void CDataBaseThread::StopWork()
{
	m_bRunning = false ;
}

void CDataBaseThread::__run()
{
	while ( true )
	{
		if ( ProcessRequest() )
			break ;
		if ( !m_bRunning )
		{
			ProcessRequest(); 
			break;
		}

		Sleep(1);

		if ( time(NULL) >= m_tNextMysqlPingTime )
		{
			unsigned long id = mysql_thread_id(m_pMySql);
			mysql_ping(m_pMySql);
			if ( id != mysql_thread_id(m_pMySql) )
			{
				// reconnected ;
				if (!mysql_set_character_set(m_pMySql, "utf8"))
				{
					printf("Reconnect !!! New client character set: %s\n",
						mysql_character_set_name(m_pMySql));
				}
			}
			m_tNextMysqlPingTime = time(NULL) + MYSQL_PING_TIME;
		}
	}

	if ( m_pMySql )
	{
		mysql_close(m_pMySql) ;
	}
}

int CDataBaseThread::EscapeString(char *to, const char *from, unsigned long length )
{
	if ( m_pMySql == NULL )
		return 0 ;
	return mysql_real_escape_string(m_pMySql, to,from,length );
}

bool CDataBaseThread::ProcessRequest()
{
	// process request here ;
	CDBRequestQueue* pRequestQueue = CDBRequestQueue::SharedDBRequestQueue();
	CDBRequestQueue::VEC_DBREQUEST vRequestOut ;
	CDBRequestQueue::VEC_DBRESULT vProcessedResult ;
	pRequestQueue->GetAllRequest(vRequestOut);
	CDBRequestQueue::VEC_DBREQUEST::iterator iter = vRequestOut.begin() ;
	stDBRequest* pRequest = NULL ;
	stDBResult* pResult = NULL ;
	MYSQL_RES *msqlResult = NULL ;
	MYSQL_ROW msqlrow;
	MYSQL_FIELD *msqlfield;
	for ( ; iter != vRequestOut.end(); ++iter )
	{
		pRequest = *iter ;
		pResult = new stDBResult;  // will be deleted after processed in the main thread .
		vProcessedResult.push_back(pResult);
		pResult->nRequestUID = pRequest->nRequestUID ;
		pResult->pUserData = pRequest->pUserData ;
		pResult->nAffectRow = 0 ;
		if ( mysql_real_query(m_pMySql,pRequest->pSqlBuffer,pRequest->nSqlBufferLen) )
		{
			LOGFMTE("query DB Error Info , Operate UID = %d : %s . sql: = %s\n", pRequest->nRequestUID, mysql_error(m_pMySql), pRequest->pSqlBuffer);
			pResult->nAffectRow = 0 ;
			continue; 
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
							case MYSQL_TYPE_FLOAT: // float 
							case MYSQL_TYPE_DOUBLE: // double 
							case MYSQL_TYPE_BLOB: // binary 
							case MYSQL_TYPE_VAR_STRING:  // string 
							case MYSQL_TYPE_INT24:
								{
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
	}

	if ( vRequestOut.empty() == false )  // means mysql connection is not idle .
	{
		m_tNextMysqlPingTime = time(NULL) + MYSQL_PING_TIME;  // do a mysql_ping() on the connection if there has been a long time [since the last query ]
	}

	pRequestQueue->PushReserveRequest(vRequestOut);
	pRequestQueue->PushResult(vProcessedResult);
	return false ;
}
