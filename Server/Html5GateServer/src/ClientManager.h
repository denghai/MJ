#pragma once
#include <map>
#include <list>
#include "WebsocketNetwork.h"
#include "CommonDefine.h"
struct stGateClient ;
struct stMsg ;
class CGateClientMgr
	:public IWebSocketDelegate
{
public:
	typedef std::map<unsigned int ,stGateClient*> MAP_SESSIONID_GATE_CLIENT ;
	typedef std::map<CONNECT_ID, stGateClient*> MAP_NETWORKID_GATE_CLIENT ;
	typedef std::list<stGateClient*> LIST_GATE_CLIENT ;
public:
	CGateClientMgr();
	~CGateClientMgr();
	bool onMsg(CONNECT_ID id , const char* pmsg )override ;
	bool onTranlatedWebMsgToOtherSvr( Packet* pData );
	void OnServerMsg(const char* pRealMsgData, uint16_t nDataLen,uint32_t uTargetSessionID );
	 bool onPeerConnected(const char* pIPAddress , CONNECT_ID nConnectID )override;
	bool onPeerClosed(CONNECT_ID nNetID )override;
	stGateClient* GetGateClientBySessionID(uint32_t nSessionID);
	void UpdateReconectClientLife();
	void closeAllClient();
protected:
	void AddClientGate(stGateClient* pGateClient );
	void RemoveClientGate(stGateClient* pGateClient );
	stGateClient* GetReserverGateClient();
	stGateClient* GetGateClientByNetWorkID(CONNECT_ID& nNetWorkID );
	bool CheckServerStateOk( stGateClient* pClient);
private:
	bool tranlateMsgToWebClient(stMsg* pmsg,std::string& strOut);
protected:
	friend struct stGateClient ;
protected:
	MAP_NETWORKID_GATE_CLIENT m_vNetWorkIDGateClientIdx ;
	MAP_SESSIONID_GATE_CLIENT m_vSessionGateClient ;
	MAP_SESSIONID_GATE_CLIENT m_vWaitToReconnect;

	LIST_GATE_CLIENT m_vGateClientReserver ;
	char m_pMsgBuffer[MAX_MSG_BUFFER_LEN] ;
};