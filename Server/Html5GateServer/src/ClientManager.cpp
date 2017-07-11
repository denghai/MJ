#include "ClientManager.h"
#include "GateClient.h"
#include "MessageDefine.h"
#include "log4z.h"
#include "CommonDefine.h"
#include "ServerNetwork.h"
#include "GateServer.h"
#include "ServerMessageDefine.h"
#include "ServerNetwork.h"
#include <time.h>
#define TIME_WAIT_FOR_RECONNECTE 40
CGateClientMgr::CGateClientMgr()
{
	m_vNetWorkIDGateClientIdx.clear();
	m_vSessionGateClient.clear() ;
	m_vWaitToReconnect.clear();
	m_vGateClientReserver.clear();
	memset(m_pMsgBuffer,0,MAX_MSG_BUFFER_LEN) ;
}

CGateClientMgr::~CGateClientMgr()
{
	MAP_SESSIONID_GATE_CLIENT::iterator iterS = m_vSessionGateClient.begin();
	for ( ; iterS != m_vSessionGateClient.end(); ++iterS )
	{
		delete  iterS->second ;
	}
	m_vSessionGateClient.clear() ;

	// just clear ; object deleted in session Gate ;
	m_vNetWorkIDGateClientIdx.clear() ;

	LIST_GATE_CLIENT::iterator iter = m_vGateClientReserver.begin() ;
	for ( ; iter != m_vGateClientReserver.end(); ++iter )
	{
		delete *iter ;
	}
	m_vGateClientReserver.clear() ;

	m_vWaitToReconnect.clear();
}

bool CGateClientMgr::onMsg(CONNECT_ID id , const char* pmsg )
{
	LOGFMTD("recived: %s",pmsg) ;
	CGateServer::SharedGateServer()->SendMsgToClient("hello client",id) ;
	Packet packet ;
	onTranlatedWebMsgToOtherSvr(&packet);
	return true ;
}

bool CGateClientMgr::onTranlatedWebMsgToOtherSvr( Packet* pData )
{
	// verify identify 
	stMsg* pMsg = (stMsg*)pData->_orgdata ;
	CHECK_MSG_SIZE(stMsg,pData->_len);
	if ( MSG_VERIFY_CLIENT == pMsg->usMsgType )
	{
		stGateClient* pGateClient = GetReserverGateClient();
		if ( !pGateClient )
		{
			pGateClient = new stGateClient ;
		}

		pGateClient->Reset(CGateServer::SharedGateServer()->GenerateSessionID(),pData->_connectID,"127.0.0.1") ;
		AddClientGate(pGateClient);
		LOGFMTI("a Client connected ip = %s Session id = %d",pGateClient->strIPAddress.c_str(),pGateClient->nSessionId ) ;
		LOGFMTI("current online cnt = %d", m_vSessionGateClient.size() - m_vWaitToReconnect.size() ) ;
		return true;
	}

	// transfer to center server 
	stGateClient* pDstClient = GetGateClientByNetWorkID(pData->_connectID) ;
	if ( pDstClient == NULL )
	{
		LOGFMTE("can not send message to Center Server , client is NULL or not verified, so close the unknown connect") ;
		//CGateServer::SharedGateServer()->GetNetWorkForClients()->ClosePeerConnection(pData->_connectID) ;
		return true ;
	}

	if ( CheckServerStateOk(pDstClient) == false )
	{
		LOGFMTE("center server is disconnected so can not send msg to it ");
		return true ;
	}

	stMsgTransferData msgTransData ;
	msgTransData.nSenderPort = ID_MSG_PORT_CLIENT ;
	msgTransData.bBroadCast = false ;
	msgTransData.nSessionID = pDstClient->nSessionId ;
	int nLne = sizeof(msgTransData) ;
	if ( nLne + pData->_len >= MAX_MSG_BUFFER_LEN )
	{
		stMsg* pmsg = (stMsg*)pData->_orgdata ;
		LOGFMTE("msg from session id = %d , is too big , cannot send , msg id = %d ",pDstClient->nSessionId,pmsg->usMsgType) ;
		return true ;
	}
	memcpy(m_pMsgBuffer,&msgTransData,nLne);
	memcpy(m_pMsgBuffer + nLne , pData->_orgdata,pData->_len );
	nLne += pData->_len ;
	CGateServer::SharedGateServer()->SendMsgToCenterServer(m_pMsgBuffer,nLne);
	return true ;
}

void CGateClientMgr::closeAllClient()
{
	LOGFMTI("close all client peers");
	// remove all connecting ;
	auto iter = m_vSessionGateClient.begin() ;
	for ( ; iter != m_vSessionGateClient.end() ;  )
	{
		// tell other server the peer disconnect
		{
			stMsgClientDisconnect msgdis ;
			msgdis.nSeesionID = iter->second->nSessionId ;
			CGateServer::SharedGateServer()->SendMsgToCenterServer((char*)&msgdis,sizeof(msgdis));
		}

		//CGateServer::SharedGateServer()->GetNetWorkForClients()->ClosePeerConnection(iter->second->nNetWorkID) ;
		RemoveClientGate(iter->second) ;
		iter = m_vSessionGateClient.begin() ;
	}
}

void CGateClientMgr::OnServerMsg( const char* pRealMsgData, uint16_t nDataLen,uint32_t uTargetSessionID )
{
	stGateClient* pClient = GetGateClientBySessionID(uTargetSessionID) ;
	stMsg* pReal = (stMsg*)pRealMsgData ;
	if ( NULL == pClient )
	{
		LOGFMTE("big error !!!! can not send msg to session id = %d , client is null , msg = %d",uTargetSessionID,pReal->usMsgType  ) ;
		return  ;
	}

	if ( pClient->tTimeForRemove )
	{
		LOGFMTD("client is waiting for reconnected session id = %d, msg = %d",uTargetSessionID,pReal->usMsgType) ;
		return ;
	}

	std::string strOut = "" ;
	if ( tranlateMsgToWebClient(pReal,strOut) )
	{
		CGateServer::SharedGateServer()->SendMsgToClient(strOut.c_str(),pClient->nNetWorkID ) ;
	}
	else
	{
		LOGFMTE("msg = %u not translate to client",pReal->usMsgType) ;
	}
}

bool CGateClientMgr::tranlateMsgToWebClient(stMsg* pmsg,std::string& strOut)
{
	return true ;
}

bool CGateClientMgr::onPeerConnected(const char* pIPAddress , CONNECT_ID nConnectID)
{
	if ( pIPAddress )
	{
		LOGFMTD("a peer connected ip = %s ,",pIPAddress ) ;
	}
	else
	{
		LOGFMTD("a peer connected ip = NULL" ) ;
	}
	return true ;
	
	//stMsg msg ;
	//msg.cSysIdentifer = ID_MSG_VERIFY ;
	//msg.usMsgType = MSG_VERIFY_GATE ;
	//// send msg to peer ;
	//CGateServer::SharedGateServer()->GetNetWork()->SendMsg((char*)&msg,sizeof(msg),pData->guid,false) ;
}

bool CGateClientMgr::onPeerClosed(CONNECT_ID nPeerDisconnected )
{
	// client disconnected ;
	stGateClient* pDstClient = GetGateClientByNetWorkID(nPeerDisconnected) ;
	if ( pDstClient )
	{
		if ( pDstClient->tTimeForRemove )
		{
			LOGFMTE("already wait to reconnected");
			return true;
		}

		pDstClient->tTimeForRemove = time(NULL) + TIME_WAIT_FOR_RECONNECTE ;
		m_vWaitToReconnect[pDstClient->nSessionId] = pDstClient;
		LOGFMTD("a peer disconnected") ;
		return true;
	}
	return true ;
}

void CGateClientMgr::AddClientGate(stGateClient* pGateClient )
{
	if ( m_vNetWorkIDGateClientIdx.find(pGateClient->nNetWorkID) != m_vNetWorkIDGateClientIdx.end() )
	{
		LOGFMTE("why this pos already have data client") ;
		m_vNetWorkIDGateClientIdx.erase(m_vNetWorkIDGateClientIdx.find(pGateClient->nNetWorkID));
	}

	if ( m_vSessionGateClient.find(pGateClient->nSessionId) != m_vSessionGateClient.end() )
	{
		LOGFMTE("why this pos session id = %d had client data",pGateClient->nSessionId) ;
		m_vSessionGateClient.erase(m_vSessionGateClient.find(pGateClient->nSessionId));
	}

	m_vNetWorkIDGateClientIdx[pGateClient->nNetWorkID] = pGateClient ;
	m_vSessionGateClient[pGateClient->nSessionId] = pGateClient ;
}

void CGateClientMgr::RemoveClientGate(stGateClient* pGateClient )
{
	if ( pGateClient == NULL )
	{
		LOGFMTE("why remove a null client ") ;
		return ;
	}

	MAP_NETWORKID_GATE_CLIENT::iterator iterN = m_vNetWorkIDGateClientIdx.find(pGateClient->nNetWorkID) ;
	if ( iterN != m_vNetWorkIDGateClientIdx.end() )
	{
		m_vNetWorkIDGateClientIdx.erase(iterN) ;
	}
	else
	{
		LOGFMTE("can not find net work id = %d to remove",pGateClient->nNetWorkID ) ;
	}
	
	MAP_SESSIONID_GATE_CLIENT::iterator iterS = m_vSessionGateClient.find(pGateClient->nSessionId );
	if ( iterS != m_vSessionGateClient.end() )
	{
		m_vSessionGateClient.erase(iterS) ;
	}
	else
	{
		LOGFMTD("can not find session id = %d to remove",pGateClient->nSessionId ) ;
	}

	iterS = m_vWaitToReconnect.find(pGateClient->nSessionId) ;
	if ( iterS != m_vWaitToReconnect.end() )
	{
		m_vWaitToReconnect.erase(iterS) ;
	}
	else
	{
		LOGFMTE("why can not find session id = %d to remove from vWaiReconecte",pGateClient->nSessionId) ;
	}
	
	pGateClient->Reset(0,INVALID_CONNECT_ID,NULL) ;
	m_vGateClientReserver.push_back(pGateClient) ;
}

stGateClient* CGateClientMgr::GetReserverGateClient()
{
	stGateClient* pGateClient = NULL ;
	if ( m_vGateClientReserver.empty() == false )
	{
		pGateClient = m_vGateClientReserver.front() ;
		m_vGateClientReserver.erase(m_vGateClientReserver.begin()) ;
	}
	return pGateClient ;
}

stGateClient* CGateClientMgr::GetGateClientBySessionID(unsigned int nSessionID)
{
	MAP_SESSIONID_GATE_CLIENT::iterator iter = m_vSessionGateClient.find(nSessionID) ;
	if ( iter == m_vSessionGateClient.end() )
		return NULL ;
	return iter->second ;
}

void CGateClientMgr::UpdateReconectClientLife()
{
	if ( m_vWaitToReconnect.empty() )
	{
		return ;
	}

	time_t tNow = time(NULL) ;
	LIST_GATE_CLIENT vWillRemove ;
	MAP_SESSIONID_GATE_CLIENT::iterator iter = m_vWaitToReconnect.begin();
	for ( ; iter != m_vWaitToReconnect.end(); ++iter )
	{
		if ( iter->second == NULL )
		{
			LOGFMTE("why this null client wait reconnect");
			continue;
		}

		if ( iter->second->tTimeForRemove == 0 )
		{
			LOGFMTE("big error , timeForRemove can not be 0 ") ;
		}

		if ( iter->second->tTimeForRemove <= tNow )
		{
			vWillRemove.push_back(iter->second) ;
		}
	}

	// do remove 
	LIST_GATE_CLIENT::iterator iterRemove = vWillRemove.begin() ;
	for ( ; iterRemove != vWillRemove.end(); ++iterRemove )
	{
		stGateClient* p = *iterRemove ;
		if ( p == NULL )
		{
			continue;
		}
		// tell other server the peer disconnect
		{
			stMsgClientDisconnect msgdis ;
			msgdis.nSeesionID = p->nSessionId ;
			CGateServer::SharedGateServer()->SendMsgToCenterServer((char*)&msgdis,sizeof(msgdis));
		}

		// do remove 
		LOGFMTI("session id = %d , ip = %s , wait reconnect time out ,do exit game",p->nSessionId,p->strIPAddress.c_str()) ;
		RemoveClientGate(p);
	}
	vWillRemove.clear();
}

stGateClient* CGateClientMgr::GetGateClientByNetWorkID(CONNECT_ID& nNetWorkID )
{
	MAP_NETWORKID_GATE_CLIENT::iterator iter = m_vNetWorkIDGateClientIdx.find(nNetWorkID) ;
	if ( iter != m_vNetWorkIDGateClientIdx.end() )
		return iter->second ;
	return NULL ;
}

bool CGateClientMgr::CheckServerStateOk( stGateClient* pClient)
{
	bool b = CGateServer::SharedGateServer()->isConnected() ;
	if ( b )
	{
		return true ;
	}
	stMsgServerDisconnect msg ;
	msg.nServerType = eSvrType_Center ;

	std::string strOut = "" ;
	if ( tranlateMsgToWebClient(&msg,strOut) )
	{
		CGateServer::SharedGateServer()->SendMsgToClient(strOut.c_str(),pClient->nNetWorkID ) ;
	}
	else
	{
		LOGFMTE("msg = %u not translate to client",msg.usMsgType) ;
	}
	return false ;
}
