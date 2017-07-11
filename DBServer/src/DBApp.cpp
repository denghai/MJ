#include "DBApp.h"
#include "DBManager.h"
#include "DataBaseThread.h"
#include "DBRequest.h"
#include "ServerMessageDefine.h"
#include "CommonDefine.h"
#include "log4z.h"
#include "ConfigDefine.h"
CDBServerApp::CDBServerApp()
{
	m_pDBManager = NULL ;
	m_pDBWorkThread = NULL ;
}

CDBServerApp::~CDBServerApp()
{
	if ( m_pDBManager )
	{
		delete m_pDBManager ;
	}

	if ( m_pDBWorkThread )
	{
		delete m_pDBWorkThread ;
	}
}

bool CDBServerApp::init()
{
	IServerApp::init();
	
	m_stSvrConfigMgr.LoadFile("../configFile/serverConfig.txt");
	stServerConfig* pCenter = m_stSvrConfigMgr.GetServerConfig(eSvrType_Center);
	if ( pCenter == NULL )
	{
		LOGFMTE("center svr config is null canont start DB server") ;
		return false;
	}
	setConnectServerConfig(pCenter);
	// set up data base thread 
	stServerConfig* pDatabase = m_stSvrConfigMgr.GetServerConfig(eSvrType_DataBase);
	if ( pDatabase == NULL )
	{
		LOGFMTE("data base config is null, cant not start server") ;
		return false;
	}

	m_pDBWorkThread = new CDataBaseThread ;
	m_pDBWorkThread->InitDataBase(pDatabase->strIPAddress, pDatabase->nPort, pDatabase->strAccount, pDatabase->strPassword, Game_DB_Name);
	m_pDBWorkThread->Start();

	// dbManager ;
	m_pDBManager = new CDBManager(this) ;
	m_pDBManager->Init();

	LOGFMTI("DBServer Start!");
	return true ;
}
void CDBServerApp::update(float fDeta )
{
	IServerApp::update(fDeta);
	// process DB Result ;
	CDBRequestQueue::VEC_DBRESULT vResultOut ;
	CDBRequestQueue::SharedDBRequestQueue()->GetAllResult(vResultOut) ;
	CDBRequestQueue::VEC_DBRESULT::iterator iter = vResultOut.begin() ;
	for ( ; iter != vResultOut.end(); ++iter )
	{
		stDBResult* pRet = *iter ;
		m_pDBManager->OnDBResult(pRet) ;
		CDBRequestQueue::SharedDBRequestQueue()->pushDeleteResult(pRet) ;
	}
	vResultOut.clear();
}

// net delegate
bool CDBServerApp::onLogicMsg( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID  )
{
	m_pDBManager->OnMessage(prealMsg,eSenderPort,nSessionID ) ;
	return true ;
}

bool CDBServerApp::OnMessage( Packet* pMsg )
{
	CHECK_MSG_SIZE(stMsg,pMsg->_len) ;
	stMsg* pmsg = (stMsg*)pMsg->_orgdata ;
	if ( pmsg->cSysIdentifer == ID_MSG_VERIFY )
	{
		LOGFMTI("no need recieve verify msg") ;
		return true ;
	}

	stMsg* pRet = pmsg;
	if ( pRet->usMsgType != MSG_TRANSER_DATA )
	{
		LOGFMTE("why msg type is not transfer data , type = %d",pRet->usMsgType ) ;
		return true;
	}

	stMsgTransferData* pData = (stMsgTransferData*)pRet ;
	stMsg* preal = (stMsg*)( pMsg->_orgdata + sizeof(stMsgTransferData));

	// check async request 
	if ( preal->usMsgType == MSG_ASYNC_REQUEST )
	{
		stMsgAsyncRequest* pRet = (stMsgAsyncRequest*)preal ;
		Json::Value jsReqContent ;
		if ( pRet->nReqContentLen > 0 )
		{
			char* pBuffer = (char*)pRet ;
			pBuffer += sizeof(stMsgAsyncRequest) ;
			Json::Reader jsReader ;
			jsReader.parse(pBuffer,pBuffer + pRet->nReqContentLen,jsReqContent,false);
		}

		if ( !m_pDBManager->onAsyncRequest(pRet->nReqType,pRet->nReqSerailID,pData->nSenderPort,jsReqContent) )
		{
			LOGFMTE("async request type = %u , not process from port = %u",pRet->nReqType,pData->nSenderPort) ;
			assert(0 && "must process the req" );
		}
		return true ;
	}

	return IServerApp::OnMessage(pMsg);
}

void CDBServerApp::onExit()
{
	m_pDBWorkThread->StopWork();
	LOGFMTI("DBServer ShutDown!");
}