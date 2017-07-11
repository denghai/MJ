#include <windows.h>
#include "CenterServer.h"
#include "log4z.h"
#include "ServerMessageDefine.h"
#include <synchapi.h>
CCenterServerApp* CCenterServerApp::s_GateServer = NULL ;
CCenterServerApp* CCenterServerApp::SharedCenterServer()
{
	return s_GateServer;
}

CCenterServerApp::CCenterServerApp()
{
	m_pNetwork = NULL ;
	m_vGateInfos = NULL ;
}

CCenterServerApp::~CCenterServerApp()
{
	if ( m_pNetwork )
	{
		m_pNetwork->ShutDown();
		delete m_pNetwork ;
	}
	m_pNetwork = NULL ;
	if ( m_vGateInfos )
	{
		delete[] m_vGateInfos ;
		m_vGateInfos = NULL ;
	}
	s_GateServer = NULL ;
}

bool CCenterServerApp:: Init()
{
	if ( s_GateServer )
	{
		LOGFMTE("can not have too instance of CenterServerApp") ;
		return false ;
	}
	s_GateServer = this ;
	m_bRunning = true ;
	m_stSvrConfigMgr.LoadFile("../configFile/serverConfig.txt");

	stServerConfig* pSvrConfig = m_stSvrConfigMgr.GetServerConfig(eSvrType_Center); 
	if ( pSvrConfig == NULL )
	{
		LOGFMTE("can not find center server config so start up failed ") ;
		return  false ;
	}
	m_pNetwork = new CServerNetwork ;
	m_pNetwork->StartupNetwork( pSvrConfig->nPort,20,pSvrConfig->strPassword);
	m_pNetwork->AddDelegate(this);

	for ( uint16_t ndx = 0 ; ndx < eSvrType_Max ; ++ndx )
	{
		m_vTargetServers[ndx] = INVALID_CONNECT_ID ;
	}

	m_uGateCounts = m_stSvrConfigMgr.GetServerConfigCnt(eSvrType_Gate);
	if ( m_uGateCounts == 0 )
	{
		LOGFMTE("gate server config count = 0 , start up failed ") ;
		return false ;
	}

	m_vGateInfos = new stGateInfo[m_uGateCounts];
	for ( uint16_t nIdx = 0 ; nIdx < m_uGateCounts ; ++nIdx )
	{
		m_vGateInfos[nIdx].Reset();
	}
	LOGFMTI("start center server !");
	return true ;
}

void  CCenterServerApp::RunLoop()
{
	while ( m_bRunning )
	{
		if ( m_pNetwork )
		{
			m_pNetwork->RecieveMsg();
		}
		Sleep(1);
	}

	if ( m_pNetwork )
	{
		m_pNetwork->ShutDown() ;
	}
}

void  CCenterServerApp::Stop()
{
	m_bRunning = false ;
}

bool  CCenterServerApp::OnMessage( Packet* pData )
{
	stMsg* pMsg =(stMsg*)pData->_orgdata ;
	std::string strIP = m_pNetwork->GetIPInfoByConnectID(pData->_connectID) ;
	switch ( pMsg->usMsgType )
	{
	case MSG_VERIFY_APNS:
		{
			if ( m_vTargetServers[eSvrType_APNS] != INVALID_CONNECT_ID )
			{
				LOGFMTE("eSvrType_APNS close pre connect ") ;
				m_pNetwork->ClosePeerConnection(m_vTargetServers[eSvrType_APNS]);
			}

			m_vTargetServers[eSvrType_APNS] = pData->_connectID;
			LOGFMTI("apns server connected ip = %s",strIP.c_str()) ;
		}
		break;
	case MSG_VERIFY_VERIYF:
		{
			if ( m_vTargetServers[eSvrType_Verify] != INVALID_CONNECT_ID )
			{
				LOGFMTE("eSvrType_Verify close pre connect ") ;
				m_pNetwork->ClosePeerConnection(m_vTargetServers[eSvrType_APNS]);
			}

			m_vTargetServers[eSvrType_Verify] = pData->_connectID;
			LOGFMTI("verify server connected ip = %s",strIP.c_str()) ;
		}
		break;
	case MSG_VERIFY_LOGIN:
		{
			if ( m_vTargetServers[eSvrType_Login] != INVALID_CONNECT_ID )
			{
				LOGFMTE("eSvrType_Login close pre connect ") ;
				m_pNetwork->ClosePeerConnection(m_vTargetServers[eSvrType_Login]);
			}
			m_vTargetServers[eSvrType_Login] = pData->_connectID;
			LOGFMTI("login server connected ip = %s",strIP.c_str()) ;
		}
		break;
	case MSG_VERIFY_LOG:
		{
			if ( m_vTargetServers[eSvrType_Log] != INVALID_CONNECT_ID )
			{
				LOGFMTE("eSvrType_Log close pre connect ") ;
				m_pNetwork->ClosePeerConnection(m_vTargetServers[eSvrType_Log]);
			}

			m_vTargetServers[eSvrType_Log] = pData->_connectID;
			LOGFMTI("log server connected ip = %s",strIP.c_str()) ;
		}
		break;
	case MSG_VERIFY_DB:
		{
			if ( m_vTargetServers[eSvrType_DB] != INVALID_CONNECT_ID )
			{
				LOGFMTE("eSvrType_DB close pre connect ") ;
				m_pNetwork->ClosePeerConnection(m_vTargetServers[eSvrType_DB]);
			}

			m_vTargetServers[eSvrType_DB] = pData->_connectID;
			LOGFMTI("DB server connected ip = %s",strIP.c_str()) ;
		}
		break;
	case MSG_VERIFY_DATA:
		{
			if ( m_vTargetServers[eSvrType_Data] != INVALID_CONNECT_ID )
			{
				LOGFMTE("eSvrType_Data close pre connect ") ;
				m_pNetwork->ClosePeerConnection(m_vTargetServers[eSvrType_Data]);
			}

			m_vTargetServers[eSvrType_Data] = pData->_connectID;
			LOGFMTI("Data server connected ip = %s",strIP.c_str()) ;
		}
		break;
	case MSG_VERIFY_GOLDEN:
		{
			if ( m_vTargetServers[eSvrType_Golden] != INVALID_CONNECT_ID )
			{
				LOGFMTE("eSvrType_Taxas close pre connect ") ;
				m_pNetwork->ClosePeerConnection(m_vTargetServers[eSvrType_Golden]);
			}
			m_vTargetServers[eSvrType_Golden] = pData->_connectID;
			LOGFMTI("Golden server connected ip = %s",strIP.c_str()) ;
		}
		break;
	case MSG_VERIFY_TAXAS:
		{
			if ( m_vTargetServers[eSvrType_Taxas] != INVALID_CONNECT_ID )
			{
				LOGFMTE("eSvrType_Taxas close pre connect ") ;
				m_pNetwork->ClosePeerConnection(m_vTargetServers[eSvrType_Taxas]);
			}
			m_vTargetServers[eSvrType_Taxas] = pData->_connectID;
			LOGFMTI("Taxas server connected ip = %s",strIP.c_str()) ;
		}
		break;
	case MSG_VERIFY_NIU_NIU:
		{
			if ( m_vTargetServers[eSvrType_NiuNiu] != INVALID_CONNECT_ID )
			{
				LOGFMTE("eSvrType_NiuNiu close pre connect ") ;
				m_pNetwork->ClosePeerConnection(m_vTargetServers[eSvrType_Taxas]);
			}
			m_vTargetServers[eSvrType_NiuNiu] = pData->_connectID;
			LOGFMTI("Niu Niu server connected ip = %s",strIP.c_str()) ;
		}
		break;
	case MSG_VERIFY_GATE:
		{			
			stMsgGateServerInfo msg ;
			msg.uMaxGateSvrCount = m_uGateCounts ;
			msg.bIsGateFull = true ;
			for ( uint16_t nIdx = 0 ; nIdx < m_uGateCounts ; ++nIdx )
			{
				if ( m_vGateInfos[nIdx].IsGateWorking() == false )
				{
					msg.bIsGateFull = false ;
					msg.uIdx = nIdx;
					m_vGateInfos[nIdx].nIdx = nIdx ;
					m_vGateInfos[nIdx].nNetworkID = pData->_connectID ; 
					LOGFMTI("Gate server started idx = %d connected ip = %s",nIdx,strIP.c_str()) ;
					break;
				}
			}

			if ( msg.bIsGateFull )
			{
				LOGFMTE("gate is full can not accept more gate ip = %s",strIP.c_str()) ;
			}

			m_pNetwork->SendMsg((char*)&msg,sizeof(msg),pData->_connectID) ;

			if ( msg.bIsGateFull )
			{
				m_pNetwork->ClosePeerConnection(pData->_connectID);
			}
		}
		break;
	case MSG_CONNECT_NEW_CLIENT:
		{
			stMsgNewClientConnected* pC = (stMsgNewClientConnected*)pMsg;
			stGateInfo* pGateInfo = GetGateInfoByNetworkID(pData->_connectID);
			if ( pGateInfo == NULL )
			{
				LOGFMTE("why no gate , this peer is not gate ip = %s",strIP.c_str()) ;
				return true ;
			}
			pGateInfo->AddSessionID(pC->nNewSessionID) ;
		}
		break;
	case MSG_DISCONNECT_CLIENT:
		{
			stMsgClientDisconnect* pRet = (stMsgClientDisconnect*)pMsg;
			SendClientDisconnectMsg(pRet->nSeesionID) ;

			stGateInfo* pGateInfo = GetGateInfoByNetworkID(pData->_connectID);
			if ( pGateInfo == NULL )
			{
				LOGFMTE("why no gate , this peer is not gate ip = %s",strIP.c_str()) ;
				return true ;
			}
			pGateInfo->RemoveSessionID(pRet->nSeesionID) ;
		}
		break;
	case MSG_TRANSER_DATA:
		{
			stMsgTransferData* pTransfer = (stMsgTransferData*)pData->_orgdata ;
			stMsg* pReal = (stMsg*)(pData->_orgdata + sizeof(stMsgTransferData)) ;
			if ( pReal->cSysIdentifer == ID_MSG_PORT_CLIENT || pReal->cSysIdentifer == ID_MSG_PORT_GATE )
			{
				stGateInfo* pInfo = GetOwerGateInfoBySessionID(pTransfer->nSessionID) ;
				if ( pInfo && pInfo->IsGateWorking() )
				{
					m_pNetwork->SendMsg(pData->_orgdata,pData->_len,pInfo->nNetworkID ) ;
				}
				else
				{
					LOGFMTE("send msg = %d to client but nSession id = %d have no gate, inform this session id disconnect ",pReal->usMsgType,pTransfer->nSessionID) ;
					SendClientDisconnectMsg(pTransfer->nSessionID) ;
					if ( pInfo )
					{
						pInfo->RemoveSessionID(pTransfer->nSessionID);
					}
				}
			}
			else
			{
				eServerType svr = GetServerTypeByMsgTarget(pReal->cSysIdentifer);
				if ( eSvrType_Center == svr )
				{
					LOGFMTE("why msg = %d process here should process above send port = %d ",pReal->usMsgType,pTransfer->nSenderPort ) ;
				}
				else if ( eSvrType_Max == svr )
				{
					LOGFMTE("unknown msg target = %d , msg = %d",pReal->cSysIdentifer ,pReal->usMsgType) ;
				}
				else
				{
					if ( m_vTargetServers[svr] == INVALID_CONNECT_ID )
					{
						LOGFMTE("%s is not connecting,so can not transfer msg to it  ",GetServerDescByType(svr)) ;
					}
					else
					{
						m_pNetwork->SendMsg(pData->_orgdata,pData->_len,m_vTargetServers[svr]) ;
					}
				}
			}
		}
		break;
	default:
		{
			LOGFMTE("unknown msg id = %d , csysIdentifer = %d, ip = %s",pMsg->usMsgType,pMsg->cSysIdentifer,strIP.c_str()) ;
		}
	}
	return true ;
}

void  CCenterServerApp::OnNewPeerConnected( CONNECT_ID nNewPeer, ConnectInfo* IpInfo )
{
	if ( IpInfo )
	{
		LOGFMTI("a peer connected ip = %s , port = %d connect id = %d",IpInfo->strAddress,IpInfo->nPort ,nNewPeer );
	}
	else
	{
		LOGFMTI("a peer connected ip = NULL, connect id = %d", nNewPeer) ;
	}
}

void CCenterServerApp::OnGateDisconnected(CONNECT_ID& nNetworkID )
{
	stGateInfo* pGate = GetGateInfoByNetworkID(nNetworkID);
	if ( pGate == NULL )
	{
		LOGFMTE("this is not gate , info = NULL ip = %s",m_pNetwork->GetIPInfoByConnectID(nNetworkID)) ;
		return ;
	}

	std::map<uint32_t,uint8_t>::iterator iter = pGate->vMapSessionIDInGate.begin();
	for ( ; iter != pGate->vMapSessionIDInGate.end(); ++iter )
	{
		SendClientDisconnectMsg(iter->first) ;
	}
	pGate->Reset();

	for ( uint16_t nIdx = 0 ; nIdx < m_uGateCounts ; ++nIdx )
	{
		if ( m_vGateInfos[nIdx].nNetworkID == nNetworkID )
		{
			m_vGateInfos[nIdx].nIdx = nIdx ;
			m_vGateInfos[nIdx].nNetworkID = INVALID_CONNECT_ID ; 
			LOGFMTI("Gate server disconnected idx = %d ",nIdx) ;
			break;
		}
	}
}

CCenterServerApp::stGateInfo* CCenterServerApp::GetGateInfoByNetworkID(CONNECT_ID nNetworkID )
{
	for ( uint16_t nIdx = 0 ; nIdx < m_uGateCounts ; ++nIdx )
	{
		if ( m_vGateInfos[nIdx].nNetworkID == nNetworkID )
		{
			return &m_vGateInfos[nIdx] ;
		}
	}
	return NULL ;
}

CCenterServerApp::stGateInfo* CCenterServerApp::GetOwerGateInfoBySessionID(uint32_t nSessionID )
{
	uint16_t nIdx = nSessionID % m_uGateCounts ;
	return &m_vGateInfos[nIdx] ;
}

void  CCenterServerApp::OnPeerDisconnected( CONNECT_ID nPeerDisconnected, ConnectInfo* IpInfo )
{
	// check server dis connect ;
	stGateInfo* pinfo = GetGateInfoByNetworkID(nPeerDisconnected);
	if ( pinfo )
	{
		if ( IpInfo )
		{
			LOGFMTE("a gate idx = %d ,disconnected , ip = %s , port = %d",pinfo->nIdx,IpInfo->strAddress,IpInfo->nPort ) ;
		}
		else
		{
			LOGFMTE("a gate idx = %d ,disconnected , ip = %s",pinfo->nIdx,m_pNetwork->GetIPInfoByConnectID(nPeerDisconnected)) ;
		}
		OnGateDisconnected(nPeerDisconnected);
		pinfo->Reset();
		return ;
	}

	// check other server ;
	for ( uint16_t nIdx = 0 ; nIdx < eSvrType_Max ; ++nIdx )
	{
		if ( m_vTargetServers[nIdx] == nPeerDisconnected )
		{
			LOGFMTE("%s disconnected ", GetServerDescByType((eServerType)nIdx) ) ;
			m_vTargetServers[nIdx] = INVALID_CONNECT_ID ;
			return ;
		}
	}

	if ( IpInfo )
	{
		LOGFMTE( "a unknown peer dis conntcted ip = %s port = %d",IpInfo->strAddress,IpInfo->nPort ) ;
	}
	else
	{
		LOGFMTE("a unknown peer disconnect ip = unknown") ;
	}
}

const char* CCenterServerApp::GetServerDescByType(eServerType eType )
{
	static const char* vSvrString[] = 
	{ 
		"eSvrType_Gate",
		"eSvrType_Login",
		"eSvrType_DB",
		"eSvrType_Game",
		"eSvrType_Verify",
		"eSvrType_DataBase",
		"eSvrType_APNS",
		"eSvrType_Log",
		"eSvrType_LogDataBase",
		"eSvrType_Center",
		"eSvrType_Data",
		"eSvrType_Taxas",
		"eSvrType_NiuNiu",
		"eSvrType_Golden",
		"eSvrType_Max",
	} ;
	
	if ( eType > eSvrType_Max )
	{
		return "unknown server type ";
	}
	return vSvrString[eType] ;
}

void CCenterServerApp::SendClientDisconnectMsg(uint32_t nSessionID )
{
	static char* s_pBuffer = NULL ;
	if ( s_pBuffer == NULL )
	{
		s_pBuffer = new char[sizeof(stMsgTransferData) + sizeof(stMsgClientDisconnect)];
	}

	stMsgTransferData msgTransData ;
	msgTransData.bBroadCast = false ;
	msgTransData.nSessionID = nSessionID ;
	stMsgClientDisconnect msgDisc ;
	msgDisc.nSeesionID = nSessionID ;

	uint16_t nLen = 0 ;
	memcpy(s_pBuffer,&msgTransData,sizeof(stMsgTransferData) );
	nLen += sizeof(msgTransData) ;
	memcpy(s_pBuffer + nLen , &msgDisc,sizeof(msgDisc)) ;
	nLen += sizeof (msgDisc);
	
	if ( m_vTargetServers[eSvrType_Data] != INVALID_CONNECT_ID )
	{
		m_pNetwork->SendMsg(s_pBuffer,nLen,m_vTargetServers[eSvrType_Data] ) ;
	}
	else
	{
		LOGFMTE("data svr is disconnected") ;
	}
	LOGFMTD("send dis connect msg session id = %d",nSessionID);
}
eServerType CCenterServerApp::GetServerTypeByMsgTarget(uint16_t nTarget)
{
	switch ( nTarget )
	{
	case ID_MSG_PORT_CLIENT:
		{
			return eSvrType_Max ;
		}
		break;
	case ID_MSG_PORT_DATA:
		{
			return eSvrType_Data ;
		}
		break;
	case ID_MSG_PORT_GATE:
		{
			return eSvrType_Gate;
		}
		break;
	case ID_MSG_PORT_VERIFY:
		{
			return eSvrType_Verify ;
		}
		break;
	case ID_MSG_PORT_LOGIN:
		{
			return eSvrType_Login ;
		}
		break;
	case ID_MSG_PORT_DB:
		{
			return eSvrType_DB ;
		}
		break;
	case ID_MSG_PORT_CENTER:
		{
			return eSvrType_Center ;
		}
		break;
	case ID_MSG_PORT_APNS:
		{
			return eSvrType_APNS;
		}
		break;
	case ID_MSG_PORT_MJ:
		{
			return eSvrType_MJ;
		}
	case ID_MSG_PORT_LOG:
		{
			return eSvrType_Log;
		}
		break;
	case ID_MSG_PORT_TAXAS:
		{
			return eSvrType_Taxas ;
		}
		break;
	case ID_MSG_PORT_NIU_NIU:
		{
			return eSvrType_NiuNiu;
		}
		break;
	case ID_MSG_PORT_GOLDEN:
		{
			return eSvrType_Golden;
		}
		break;
	default:
		return eSvrType_Max ;
	}
}
