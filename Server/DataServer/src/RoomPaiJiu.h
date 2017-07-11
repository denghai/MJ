//#pragma once
//#include "NativeTypes.h"
//#include "RoomBase.h"
//#include <list>
//#include "CardPoker.h"
//#include "PaiJiuPeerCard.h"
//#include "PaiJiuMessageDefine.h"
//class CRoomPeerPaiJiu ;
//class CPoker;
//class CRoomPaiJiu
//	:public CRoomBase
//{
//public:
//	typedef std::list<CRoomPeer*> LIST_ROOM_PERR ;
//	typedef std::list<bool> LIST_BOOL_VALUE ;
//public:
//	virtual void Init( unsigned int nRoomID , unsigned char nMaxSeat );
//	unsigned char CanJoin(CPlayer* peer );
//	virtual void AddPeer(CRoomPeer* peer ) ;
//	virtual void OnPeerLeave( CRoomPeer* peer );
//	virtual void Update(float fTimeElpas, unsigned int nTimerID );
//	virtual bool OnPeerMsg(CRoomPeer* pPeer, stMsg* pmsg );
//	virtual bool SendCurRoomInfoToPlayer(CRoomPeer* pPeer ) ;
//	uint64_t GetCurrentAllBetCoin();
//	uint64_t GetAllowBetCoin();
//	void SetBankerEquairedCoin(uint64_t nCoinRequair ){ m_nBankerCoinRequair = nCoinRequair ; } 
//	CRoomPeerPaiJiu* GetBanker(){ return m_pBanker ;}
//protected:
//	void ClearHistory();
//	void GoToState(eRoomState eTargetState );
//	void SettlementGame();
//	void SetNewBanker();
//	bool IsInApplyList(CRoomPeer* peer );
//	void RemoveFromApplyList( CRoomPeer* peer );
//	void RefreshHistory(bool bShunWin, bool bDaoWin , bool bTianWin );
//	void RefreshApplyBankList();
//protected:
//	uint64_t m_nBetCoin[ePJ_BetPort_Max] ;
//	CRoomPeerPaiJiu* m_pBanker ;
//	LIST_ROOM_PERR m_vApplyBankerList ;
//	stHistroyRecorder m_vWinHistory[ePJ_BetPort_Normal_Max][MAX_PAIJIU_HISTROY_RECORDER]; // 0 default , 1 win ,2 failed ;
//	CPaiJiuPeerCard m_vPeerCard[ePJ_BetPort_Normal_Max] ;
//	CPoker m_tPoker ;
//
//	// wait timer // will come from config file ;
//	uint64_t m_nBankerCoinRequair ;
//
//	float m_fStateTick ;
//	bool m_bChangeBanker ;
//	unsigned char m_nCurBankerRound ;   // more than 20 round will change another player ;
//
//	// Dice
//	unsigned char m_nDiceIdx ;
//
//	unsigned int m_nPokerShuffleRound ; 
//};