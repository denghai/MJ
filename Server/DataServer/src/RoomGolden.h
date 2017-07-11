#pragma once
#include "CommonDefine.h"
#include <string>
#include "MessageDefine.h"
#include "CardPoker.h"
#include "RoomBase.h"
class CRoomPeer ;
class CRoom
	:public CRoomBase
{
public:
	CRoom();
	~CRoom();
	virtual void Init( unsigned int nRoomID , unsigned char nMaxSeat );
	virtual unsigned char GetEmptySeatCount();
	virtual unsigned char CanJoin(CPlayer* peer );
	virtual void AddPeer(CRoomPeer* peer ) ;
	virtual void Update(float fTimeElpas, unsigned int nTimerID );
	virtual bool OnPeerMsg(CRoomPeer* pPeer, stMsg* pmsg );
	void Rest();
	void SendCurRoomToPeer(CRoomPeer* peer );
	virtual void OnPeerLeave( CRoomPeer* peer );
protected:
	void SwitchToRoomSate( eRoomState eFrom, eRoomState eToDest );
	void NextPlayerAction();
	char GetReadyPeerCount();
	bool CheckFinish();  // when finished return true ;
	void DebugRoomInfo();  // print room id player count , total bet coin , room state , single bet coin , round , current idx , main idx .
	void DecideMainPeerIdx();
protected:
	float m_fRoomSateTick[eRoomState_Max] ;
	char m_nCurMainPeerIdx ;
	char m_nCurWaitPeerIdx ;
	unsigned int m_nSingleBetCoin ;
	unsigned int m_nTotalBetCoin ;
	short m_nRound ;
	CPoker m_Poker;
};