#pragma warning(disable:4800)
#include "DBManager.h"
#include "DBRequest.h"
#include "ServerMessageDefine.h"
#include "DataBaseThread.h"
#include "log4z.h"
CDBManager::CDBManager( )
{
	m_vReserverArgData.clear();
}

CDBManager::~CDBManager()
{
	LIST_ARG_DATA::iterator iter = m_vReserverArgData.begin() ;
	for ( ; iter != m_vReserverArgData.end() ; ++iter )
	{
		if ( *iter )
		{
			delete *iter ;
			*iter = NULL ;
		}
	}
	m_vReserverArgData.clear() ;
}

void CDBManager::Init()
{
	// register funcs here ;
	//stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
	//pRequest->cOrder = eReq_Order_High ;
	//pRequest->eType = eRequestType_Select ;
	//pRequest->nRequestUID = -1;
	//pRequest->pUserData = NULL;
	//pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"(select max(Account.UserUID) FROM Account)") ;
	//CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
}

void CDBManager::OnMessage(stMsg* pmsg , eMsgPort eSenderPort , uint32_t nSessionID )
{
	// construct sql
	stArgData* pdata = GetReserverArgData() ;
	if ( pdata == NULL )
	{
		pdata = new stArgData ;
	}

	pdata->eFromPort = eSenderPort ;
	pdata->nSessionID = nSessionID ;

	stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
	pRequest->cOrder = eReq_Order_Normal ;
	pRequest->nRequestUID = pmsg->usMsgType ;
	pRequest->pUserData = pdata;
	pRequest->eType = eRequestType_Max ;
	pRequest->nSqlBufferLen = 0 ;

	switch( pmsg->usMsgType )
	{
	case MSG_SAVE_LOG:
		{
			stMsgSaveLog* pCreate = (stMsgSaveLog*)pmsg ;
			pdata->nExtenArg1 = pCreate->nTargetID ;
			char* pStrBuffer = new char[pCreate->nJsonExtnerLen+1];
			memset(pStrBuffer,0,pCreate->nJsonExtnerLen+1);
			char* pS = (char*)&pCreate->nJsonExtnerLen ;
			pS += sizeof(pCreate->nJsonExtnerLen);
			memcpy(pStrBuffer,pS,pCreate->nJsonExtnerLen);
			pRequest->eType = eRequestType_Select ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,
				"call saveLog(%d,'%d','%s'",pCreate->nLogType,pCreate->nTargetID,pStrBuffer) ;
			for ( uint8_t nIdx = 0 ; nIdx < LOG_ARG_CNT; ++nIdx )
			{
				pRequest->nSqlBufferLen += sprintf((char*)(pRequest->pSqlBuffer + pRequest->nSqlBufferLen),
					",'%I64d'",pCreate->vArg[nIdx]) ;
			}

			pRequest->nSqlBufferLen += sprintf((char*)(pRequest->pSqlBuffer + pRequest->nSqlBufferLen),
				")") ;
			delete[] pStrBuffer ;
			pStrBuffer = nullptr;
		}
		break;
	case MSG_SAVE_PLAYER_ADVICE:
		{
			stMsgSavePlayerAdvice* pSave = (stMsgSavePlayerAdvice*)pmsg ;
			time_t nT = time(nullptr) ;
			char* pBuff = new char[pSave->nLen+1] ;
			memset(pBuff,0,pSave->nLen+1) ;
			memcpy(pBuff,(char*)pmsg + sizeof(stMsgSavePlayerAdvice) ,pSave->nLen );
			pRequest->eType = eRequestType_Add ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"INSERT INTO log_playeradvice (userUID, cotent,time,c_time) VALUES ('%u', '%s','%u','%s')",
				pSave->nUserUID,pBuff,(uint32_t)nT,ctime(&nT)) ;
			delete[] pBuff ;
			pBuff = nullptr ;
		}
		break;
	default:
		{
			m_vReserverArgData.push_back(pdata) ;
			LOGFMTE("unknown msg type = %d",pmsg->usMsgType ) ;
		}
	}

	if ( pRequest->nSqlBufferLen == 0 || pRequest->eType == eRequestType_Max )
	{
		LOGFMTE("a request sql len = 0 , msg = %d" , pRequest->nRequestUID ) ;
		
		CDBRequestQueue::VEC_DBREQUEST v ;
		v.push_back(pRequest) ;
		CDBRequestQueue::SharedDBRequestQueue()->PushReserveRequest(v);
	}
	else
	{
		CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
	}
}

void CDBManager::OnDBResult(stDBResult* pResult)
{
	stArgData*pdata = (stArgData*)pResult->pUserData ;
	switch ( pResult->nRequestUID )
	{
	case MSG_SAVE_LOG:
	case MSG_SAVE_PLAYER_ADVICE:
		{
			if ( pResult->nAffectRow != 1 )
			{
				LOGFMTE("save log error",pdata->nExtenArg1) ;
			}
			else
			{
				LOGFMTD("save log success");
			}
		}
		break;
	default:
		{
			if ( pResult->nAffectRow <= 0 )
			{
				LOGFMTE("unprocessed db result msg id = %d , row cnt = %d  ", pResult->nRequestUID,pResult->nAffectRow );
			}
			else
			{
				LOGFMTI("unprocessed db result msg id = %d , row cnt = %d  ", pResult->nRequestUID,pResult->nAffectRow );
			}
		}
	}
	m_vReserverArgData.push_back(pdata) ;
}

CDBManager::stArgData* CDBManager::GetReserverArgData()
{
	LIST_ARG_DATA::iterator iter = m_vReserverArgData.begin() ;
	if ( iter != m_vReserverArgData.end() )
	{
		stArgData* p = *iter ;
		m_vReserverArgData.erase(iter) ;
		p->Reset();
		return p ;
	}
	return NULL ;
}
