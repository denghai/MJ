#pragma once
#include <map>
#include <list>
#include "ServerNetwork.h"
#include "CommonDefine.h"
struct stGateClient ;
class CGateClientMgr
	:public CServerNetworkDelegate
{
public:
	typedef std::map<unsigned int ,stGateClient*> MAP_SESSIONID_GATE_CLIENT ;
	typedef std::map<CONNECT_ID, stGateClient*> MAP_NETWORKID_GATE_CLIENT ;
	typedef std::list<stGateClient*> LIST_GATE_CLIENT ;
public:
	CGateClientMgr();
	~CGateClientMgr();
	virtual bool OnMessage( Packet* pData ) ;
	void OnServerMsg(const char* pRealMsgData, uint16_t nDataLen,uint32_t uTargetSessionID );
	virtual void OnNewPeerConnected( CONNECT_ID nNewPeer, ConnectInfo* IpInfo);
	virtual void OnPeerDisconnected( CONNECT_ID nPeerDisconnected, ConnectInfo* IpInfo );
	stGateClient* GetGateClientBySessionID(uint32_t nSessionID);
	void UpdateReconectClientLife();
	void closeAllClient();
protected:
	void AddClientGate(stGateClient* pGateClient );
	void RemoveClientGate(stGateClient* pGateClient );
	stGateClient* GetReserverGateClient();
	stGateClient* GetGateClientByNetWorkID(CONNECT_ID& nNetWorkID );
	bool CheckServerStateOk( stGateClient* pClient);
protected:
	friend struct stGateClient ;
protected:
	MAP_NETWORKID_GATE_CLIENT m_vNetWorkIDGateClientIdx ;
	MAP_SESSIONID_GATE_CLIENT m_vSessionGateClient ;
	MAP_SESSIONID_GATE_CLIENT m_vWaitToReconnect;

	LIST_GATE_CLIENT m_vGateClientReserver ;
	char m_pMsgBuffer[MAX_MSG_BUFFER_LEN] ;
};