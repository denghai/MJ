#include "PushNotificationServer.h"
#include "log4z.h"
#include "ServerMessageDefine.h"
CPushNotificationServer::~CPushNotificationServer()
{

}

bool CPushNotificationServer::init()
{
	IServerApp::init();
	//CLogMgr::SharedLogMgr()->SetOutputFile("ApnsSvr");
	m_stSvrConfigMgr.LoadFile("../configFile/serverConfig.txt");

	stServerConfig* pSvrConfigItem = m_stSvrConfigMgr.GetServerConfig(eSvrType_DataBase );
	if ( pSvrConfigItem == NULL )
	{
		LOGFMTE("Data base config is null , can not start login svr ") ;
		return false;
	}

	m_nPushThread.InitSSLContex();
	if ( m_nPushThread.ConnectToAPNs() == false )
	{
		LOGFMTE("connect to apns svr failed") ;
	}
	else
	{
		LOGFMTI("connect to apns svr success") ;
	}

	// connected to center ;
	pSvrConfigItem = m_stSvrConfigMgr.GetServerConfig(eSvrType_Center );
	if ( pSvrConfigItem == NULL )
	{
		LOGFMTE("center svr config is null so can not start svr ") ;
		return false;
	}
	setConnectServerConfig(pSvrConfigItem);

	m_nPushThread.Start();
	return true; 
}

bool CPushNotificationServer::onLogicMsg( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID )
{
	if (  IServerApp::onLogicMsg(prealMsg,eSenderPort,nSessionID) )
	{
		return true;
	}

	if ( m_tNoticePlayerMgr.onMsg(prealMsg,nSessionID) )
	{
		return true ;
	}

	return false ;
}