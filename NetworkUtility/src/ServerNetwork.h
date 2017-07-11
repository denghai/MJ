#pragma once
#include "NetworkDefine.h"
#include <list>
class  CServerNetworkImp ;
class CServerNetworkDelegate
{
public:
	CServerNetworkDelegate():m_nPriority(0){}
	virtual ~CServerNetworkDelegate(){}
	virtual bool OnMessage( Packet* pData ) = 0;
	virtual void OnNewPeerConnected( CONNECT_ID nNewPeer, ConnectInfo* IpInfo ){}
	virtual void OnPeerDisconnected( CONNECT_ID nPeerDisconnected, ConnectInfo* IpInfo ){}
	void SetPriority(unsigned int nPriority );
	unsigned int GetPriority(){ return m_nPriority ; }
protected:
	unsigned int m_nPriority ;
};

class CServerNetwork
{
public:
	typedef std::list<CServerNetworkDelegate*> LIST_DELEGATE ;
	typedef bool (CServerNetwork::*lpFunc)(CServerNetworkDelegate* pDelegate, Packet* pData);
public:
	static CServerNetwork* SharedNetwork();
	CServerNetwork();
	~CServerNetwork();
	bool StartupNetwork( unsigned short nPort , int nMaxInComming, const char* pIncomingPassword = NULL );
	void ShutDown();
	void RecieveMsg();
	void SendMsg(const char* pData , int nLength , CONNECT_ID& nSendToOrExcpet ,bool bBroadcast = false );
	void ClosePeerConnection(CONNECT_ID& nPeerToClose);
	void AddDelegate(CServerNetworkDelegate* pDelegate , unsigned int nPriority = 0 );
	void RemoveDelegate(CServerNetworkDelegate* pDelegate );
	std::string GetIPInfoByConnectID(CONNECT_ID nID);
protected:
	bool OnNewPeerConnected(CServerNetworkDelegate* pDelegate, Packet* pData  );
	bool OnPeerDisconnected(CServerNetworkDelegate* pDelegate, Packet* pData  );
	bool OnLogicMessage(CServerNetworkDelegate* pDelegate, Packet* pData);
	void EnumDelegate( lpFunc pFunc, Packet* pData );
protected:
	CServerNetworkImp* m_pNetPeer ;
	LIST_DELEGATE m_vAllDelegates ;
};