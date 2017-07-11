#pragma once
#include "RoomPeer.h"
#include "NativeTypes.h"
#include "CommonDefine.h"
#include "TaxasPokerPeerCard.h"
class CTaxasPokerPeer
	:public CRoomPeer
{
public:
	CTaxasPokerPeer(CPlayer* pPlayer);
	~CTaxasPokerPeer();
	virtual bool OnMessage(stMsg* pMsg ) ;
	virtual void OnPlayerDisconnect();
	virtual void Reset();
	virtual void OnWinCoin(int64_t nWinCoin );
	virtual void TryToLeaveRoom();
	virtual void OnGameEnd();

	void OnStartNewRound();
	void OnStartNewGame();
	void SetMyAction(eRoomPeerAction eAct ){ m_eMyAction = eAct ; m_nContinueNoneAct = 0 ;}
	eRoomPeerAction GetMyAction(){ return m_eMyAction ; }
	uint64_t GetBetCoinThisRound(){ return m_nBetThisRound ;}
	void AddBetCoinThisRound(int64_t nCoin );
	void DecreasBetCoinThisRound( int64_t nCoin );
	char PK(CTaxasPokerPeer* pPkTarget ){ return m_PeerCard.PK(&pPkTarget->m_PeerCard) ; } // -1 failed 0 same , 1 win ;
	void OnWinCoinThisPool(uint64_t nCoin );
	CTaxasPokerPeerCard& GetPeerCard(){ return m_PeerCard ;}
	void OnWaitActionTimeOut();
	uint64_t GetAllBetCoin(){ return m_nAllBetCoin ; }
	uint64_t GetAllWinCoin(){ return m_nAllWinCoin ;}
	int64_t GetCoinOffsetThisGame(){ return GetAllWinCoin() - GetAllBetCoin() ; }
	bool AutoTakein();
protected:
	uint64_t m_nAllBetCoin ;
	uint64_t m_nBetThisRound ;
	uint64_t m_nAllWinCoin ;
	eRoomPeerAction m_eMyAction ;
	CTaxasPokerPeerCard m_PeerCard ;
	unsigned char m_nContinueNoneAct ;
};