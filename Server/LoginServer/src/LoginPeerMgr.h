#pragma once
#include <map>
#include <list>
#include "NetWorkManager.h"
class CLoginApp ;
class CLoginPeer ;
class CLoginPeerMgr
{
public:
	typedef std::map<unsigned int , CLoginPeer* > MAP_LOGIN_PEER ;
	typedef std::list<CLoginPeer*> LIST_LOGIN_PEER ;
public:
	CLoginPeerMgr(CLoginApp* app );
	~CLoginPeerMgr();
	void OnMessage( Packet* pMsg ) ;
	CLoginPeer* GetPeerBySessionID(unsigned int nSessionID );
	CLoginPeer* GetReserverPeer();
	void OnGateMessage(stMsg* pmsg ,unsigned int nSessionID );
	void OnDBMessage(stMsg* pmsg);
	bool SendMsgToDB(const char* pBuffer , int nLen );
	bool SendMsgToGate(const char* pBuffer , int nLen );
protected:
	void ClearAll();
protected:
	MAP_LOGIN_PEER m_vAllPeers ;
	LIST_LOGIN_PEER m_vReseverLoginPeers ; 
	CLoginApp* m_pApp ;
	char m_pMaxBuffer[1024];
};