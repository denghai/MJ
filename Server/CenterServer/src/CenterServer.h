#pragma once
#include "ServerNetwork.h"
#include "ServerConfig.h"
class CCenterServerApp
	:public CServerNetworkDelegate
{
public:
	struct stGateInfo
	{
		uint16_t nIdx ;
		CONNECT_ID nNetworkID ;
		std::map<uint32_t,uint8_t> vMapSessionIDInGate ; // // used for inform disconnect when a gate svr crash ;

		void Reset(){ nIdx = 0 ; nNetworkID = INVALID_CONNECT_ID ; vMapSessionIDInGate.clear(); }
		bool IsGateWorking(){ return nNetworkID != INVALID_CONNECT_ID ; }
		void AddSessionID( uint32_t nSessionID ){ vMapSessionIDInGate[nSessionID] = 1;}
		void RemoveSessionID( uint32_t nSessionID )
		{
			std::map<uint32_t,uint8_t>::iterator iter = vMapSessionIDInGate.find(nSessionID) ;
			if ( iter != vMapSessionIDInGate.end() )
			{
				vMapSessionIDInGate.erase(iter) ;
			}
		}
	};

public:
	static CCenterServerApp* SharedCenterServer();
	CCenterServerApp();
	~CCenterServerApp();
	bool Init();
	void RunLoop();
	void Stop();

	virtual bool OnMessage( Packet* pData );
	virtual void OnNewPeerConnected( CONNECT_ID nNewPeer, ConnectInfo* IpInfo );
	virtual void OnPeerDisconnected( CONNECT_ID nPeerDisconnected, ConnectInfo* IpInfo );
protected:
	void OnGateDisconnected(CONNECT_ID& nNetworkID );
	stGateInfo* GetGateInfoByNetworkID(CONNECT_ID nNetworkID );
	stGateInfo* GetOwerGateInfoBySessionID(uint32_t nSessionID );
	const char* GetServerDescByType(eServerType eType );
	void SendClientDisconnectMsg(uint32_t nSessionID );
	eServerType GetServerTypeByMsgTarget(uint16_t nTarget );
protected:
	static CCenterServerApp* s_GateServer ;
	bool m_bRunning ;
	CServerNetwork* m_pNetwork ;
	CSeverConfigMgr m_stSvrConfigMgr ;
	CONNECT_ID m_vTargetServers[eSvrType_Max];
	stGateInfo* m_vGateInfos ;
	uint16_t m_uGateCounts ;
};