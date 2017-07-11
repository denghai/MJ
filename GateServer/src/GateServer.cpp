#include "GateServer.h"
#include "CommonDefine.h"
#include "ServerMessageDefine.h"
#include "log4z.h"
#include "GateClient.h"
#include <assert.h>
#define MAX_INCOME_PLAYER 5000
CGateServer* CGateServer::s_GateServer = NULL ;
CGateServer* CGateServer::SharedGateServer()
{
	return s_GateServer ;
}

CGateServer::CGateServer()
{
	m_pNetWorkForClients = NULL ;
	m_pGateManager = NULL ;
	m_nSvrIdx = m_nAllGeteCount = 0 ;
	m_nCurMaxSessionID = 0 ;
	if ( s_GateServer )
	{
		assert(0&&"only once should");
	}
}

CGateServer::~CGateServer()
{
	if ( m_pNetWorkForClients )
	{
		m_pNetWorkForClients->ShutDown() ;
	}
	delete m_pNetWorkForClients ;
	delete m_pGateManager ;
	s_GateServer = NULL ;
}

bool CGateServer::OnLostSever(Packet* pMsg)
{
	IServerApp::OnLostSever(pMsg);

	m_pGateManager->closeAllClient() ;

	return false ;
}

bool CGateServer::init()
{
	IServerApp::init();
	if ( s_GateServer )
	{
		assert(0&&"only once should");
	}
	s_GateServer = this ;
	// client player mgr ;
	m_pGateManager = new CGateClientMgr ;
	
	m_stSvrConfigMgr.LoadFile("../configFile/serverConfig.txt");

	stServerConfig* pSvrConfig = m_stSvrConfigMgr.GetServerConfig(eSvrType_Center) ;
	if ( !pSvrConfig )
	{
		return false ;
	}
	setConnectServerConfig(pSvrConfig);
	return true ;
}

void CGateServer::update(float fDeta )
{
	IServerApp::update(fDeta);  
	if ( m_pNetWorkForClients )
	{
		m_pNetWorkForClients->RecieveMsg() ;
	}

	if ( m_pGateManager )
	{
		m_pGateManager->UpdateReconectClientLife();
	}
}

void CGateServer::onExit()
{
	m_pNetWorkForClients->ShutDown() ;
	m_pGateManager->closeAllClient();
}

void CGateServer::SendMsgToClient(const char* pData , int nLength , CONNECT_ID& nSendToOrExcpet ,bool bBroadcast )
{
	if ( m_pNetWorkForClients )
	{
		m_pNetWorkForClients->SendMsg(pData,nLength,nSendToOrExcpet,bBroadcast) ;
	}
}

void CGateServer::SendMsgToCenterServer( const char* pmsg, uint16_t nLen )
{
	sendMsg(pmsg,nLen);
}

bool CGateServer::OnMessage( Packet* pPacket )
{
	//if ( IServerApp::OnMessage(pPacket) )
	//{
	//	return true ;
	//}

	stMsg* pMsg = (stMsg*)pPacket->_orgdata ;
	if ( MSG_TRANSER_DATA == pMsg->usMsgType )
	{
		stMsgTransferData* pData = (stMsgTransferData*)pMsg;
		stMsg* pReal = (stMsg*)(pPacket->_orgdata + sizeof(stMsgTransferData));
		if ( pReal->cSysIdentifer == ID_MSG_PORT_CLIENT )
		{
			m_pGateManager->OnServerMsg((char*)pReal,pPacket->_len - sizeof(stMsgTransferData),pData->nSessionID ) ;
		}
		else if ( ID_MSG_PORT_GATE == pReal->cSysIdentifer )
		{
			OnMsgFromOtherSrvToGate(pReal,pData->nSenderPort,pData->nSessionID);
		}
		else
		{
			LOGFMTE("wrong msg send to gate, target port = %d, msgType = %d",pReal->cSysIdentifer, pReal->usMsgType ) ;
		}
	}
	else if ( MSG_GATESERVER_INFO == pMsg->usMsgType )
	{
		stMsgGateServerInfo* pInfo = (stMsgGateServerInfo*)pMsg ;
		if ( pInfo->bIsGateFull )
		{
			LOGFMTE("gate is full , can not setup more gate, plase colse this exe");
			m_pNetWorkForClients = NULL ;
			return true ;
		}
		m_nSvrIdx = pInfo->uIdx;
		m_nAllGeteCount = pInfo->uMaxGateSvrCount ;
		m_nCurMaxSessionID = m_nSvrIdx ;
		// start gate svr for client to connected 
		stServerConfig* pGateConfig = m_stSvrConfigMgr.GetGateServerConfig(m_nSvrIdx) ;
		if ( nullptr == m_pNetWorkForClients )
		{
			m_pNetWorkForClients = new CServerNetwork ;
			m_pNetWorkForClients->StartupNetwork(pGateConfig->nPort,MAX_INCOME_PLAYER,pGateConfig->strPassword);
			m_pNetWorkForClients->AddDelegate(m_pGateManager) ;
		}

		LOGFMTI("setup network for clients to client ok " );
		LOGFMTI("Gate Server Start ok idx = %d port = %u!",m_nSvrIdx,pGateConfig->nPort ) ;
	}
	else
	{
		LOGFMTE("unknown msg = %d , from = %d",pMsg->usMsgType, pMsg->cSysIdentifer ) ;
	}

	return true ;
}

void CGateServer::OnMsgFromOtherSrvToGate( stMsg* pmsg , uint16_t eSendPort , uint32_t uTargetSessionID )
{
	if ( pmsg->usMsgType == MSG_LOGIN_INFORM_GATE_SAVE_LOG )
	{
		stGateClient* pClient = m_pGateManager->GetGateClientBySessionID(uTargetSessionID) ;
		stMsgLoginSvrInformGateSaveLog* preal = (stMsgLoginSvrInformGateSaveLog*)pmsg ;
		if ( NULL == pClient )
		{
			LOGFMTE("big error !!!! can not send msg to session id = %d , client is null , msg = %d",uTargetSessionID,preal->usMsgType  ) ;
			return  ;
		}

		stMsgSaveLog msg ;
		memset(msg.vArg,0,sizeof(msg.vArg));
		msg.nLogType = preal->nlogType ;
		msg.nTargetID = preal->nUserUID ;
		msg.nJsonExtnerLen = strlen(pClient->strIPAddress.c_str());

		uint16_t nLen = sizeof(stMsgSaveLog) + msg.nJsonExtnerLen ;
		char* pBuffer = new char[nLen];
		memset(pBuffer,0,nLen);
		memcpy(pBuffer,&msg,sizeof(msg));
		memcpy(pBuffer + sizeof(msg),pClient->strIPAddress.c_str(),msg.nJsonExtnerLen);
		sendMsg(pClient->nSessionId,pBuffer,nLen);
		delete[] pBuffer ;
	}

	if ( MSG_REQUEST_CLIENT_IP == pmsg->usMsgType )
	{
		stGateClient* pClient = m_pGateManager->GetGateClientBySessionID(uTargetSessionID) ;
		stMsgRequestClientIpRet msgRet ;
		msgRet.nRet = 0 ;
		memset(msgRet.vIP,0,sizeof(msgRet.vIP)) ;
		if ( pClient == nullptr || pClient->tTimeForRemove  )
		{
			msgRet.nRet = 1 ;	
		}
		else
		{
			sprintf_s(msgRet.vIP,sizeof(msgRet.vIP),"%s",pClient->strIPAddress.c_str()) ;
		}

		// transer dat to center svr  ;
		LOGFMTD("tell data svr disconnected event ") ;
		Packet tPacket ;
		tPacket._brocast = false ;
		if ( pClient )
		{
			tPacket._connectID = pClient->nNetWorkID ;
		}
		else
		{
			return ;
		}
		tPacket._len = sizeof(msgRet);
		memcpy(tPacket._orgdata,(char*)&msgRet,sizeof(msgRet)) ;
		m_pGateManager->OnMessage(&tPacket) ;
		return ;
	}
}

uint32_t CGateServer::GenerateSessionID()
{
	return (m_nCurMaxSessionID += m_nAllGeteCount);
}

