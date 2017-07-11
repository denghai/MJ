#pragma once 
#include "RakNetTypes.h"
class CPlayer ;
class CBrocaster
{
public:
	CBrocaster();
	~CBrocaster();
	void PostPlayerSayMsg(CPlayer* pPlayer , char* pContent , unsigned short nLen );
	void SendInformsToPlayer(CPlayer* pPlayer);
	bool OnMessage(RakNet::Packet* packet );
protected:
	void SendBrocastMessage(char* pBuffer , unsigned short nLen );
protected:
	// informs ;
	char* m_pInformBuffer ;
	unsigned int m_nBufferLen ;
	time_t m_nInformLoadTime ;
};