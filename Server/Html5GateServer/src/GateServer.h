#pragma once
#include "WebsocketNetwork.h"
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
	IWebsocketNetwork* GetNetWorkForClients(){ return m_pNetWorkForClients ;}
	void SendMsgToClient(const char* pData , CONNECT_ID& nSendToOrExcpet);
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
	IWebsocketNetwork* m_pNetWorkForClients ;
	CGateClientMgr* m_pGateManager ;

	CSeverConfigMgr m_stSvrConfigMgr ;
	uint16_t m_nSvrIdx ;  // value from Center svr ;
	uint16_t m_nAllGeteCount ;   // value from Center svr ;
	uint32_t m_nCurMaxSessionID ;
};