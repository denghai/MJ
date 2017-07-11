#pragma once
#include "RoomPeer.h"
class CBaccaratPeer
	:public CRoomPeer
{
public:
	CBaccaratPeer(CPlayer* pPlayer );
	~CBaccaratPeer(){}
	virtual bool OnMessage(stMsg* pMsg ) ;
	virtual void Reset();
	virtual void TryToLeaveRoom();
	bool  AddBetCoin( uint64_t naddBetCoin, eBaccaratBetPort ePort );
	uint64_t GetBetCoin();
	int64_t OnSettlement(bool bResult[eBC_BetPort_Max] );
	virtual void OnGameEnd();
protected:
	uint64_t m_vBetPort[eBC_BetPort_Max] ;
};