#pragma once
#include "ServerNetwork.h"
#include "ClientManager.h"
#include "ServerConfig.h"
#include "ISeverApp.h"
class CGateServer
	:public IServerApp
{
public:
	static CGateServer* SharedGateServer();
	CGateServer();
	~CGateServer();
	bool OnLostSever(Packet* pMsg)override;
	bool init();
	CServerNetwork* GetNetWorkForClients(){ return m_pNetWorkForClients ;}
	void SendMsgToClient(const char* pData , int nLength , CONNECT_ID& nSendToOrExcpet ,bool bBroadcast = false );
	CGateClientMgr* GetClientMgr(){ return m_pGateManager ;}
	void update(float fDeta );
	void SendMsgToCenterServer(const char* pmsg, uint16_t nLen );
	// network
	bool OnMessage( Packet* pMsg );
	uint16_t getLocalSvrMsgPortType(){ return ID_MSG_PORT_GATE ; }
	uint32_t GenerateSessionID();
	void onExit();
protected:
	void OnMsgFromOtherSrvToGate(stMsg* pmsg, uint16_t eSendPort , uint32_t nSessioniD );
protected:
	static CGateServer* s_GateServer ;
	CServerNetwork* m_pNetWorkForClients ;
	CGateClientMgr* m_pGateManager ;

	CSeverConfigMgr m_stSvrConfigMgr ;
	uint16_t m_nSvrIdx ;  // value from Center svr ;
	uint16_t m_nAllGeteCount ;   // value from Center svr ;
	uint32_t m_nCurMaxSessionID ;
};