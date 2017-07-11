#include "LoginPeer.h"
#include "LoginPeerMgr.h"
CLoginPeer::CLoginPeer(CLoginPeerMgr* pPeerMgr )
{
	m_nSessionID = 0 ;
	m_pPeerMgr = pPeerMgr ;
}

CLoginPeer::~CLoginPeer()
{
	m_pPeerMgr = NULL ;
}

void CLoginPeer::OnMessage(Packet* packet)
{

}

void CLoginPeer::Reset(unsigned int nSession )
{
	m_nSessionID = nSession ;
}

void CLoginPeer::SendMsgToDB(const char* pBuffer , unsigned int nLen )
{
	bool b = m_pPeerMgr->SendMsgToDB(pBuffer,nLen) ;
	if ( !b )
	{
		// send error message to client ;
	}
}

void CLoginPeer::SendMsgToGate(const char* pBuffer , unsigned int nLen )
{
	bool b = m_pPeerMgr->SendMsgToGate(pBuffer,nLen) ;
	if ( !b )
	{
		// send error message to client ;
	}
}