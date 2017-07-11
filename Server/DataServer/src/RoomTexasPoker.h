#pragma once
#include "RoomBase.h"
#include "CommonDefine.h"
#include <list>
#include <set>
#include "CardPoker.h"
class CTaxasPokerPeer;
class CRoomTexasPoker
	:public CRoomBase
{

public:
	typedef std::set<CRoomPeer*> LIST_PEERS ;
	typedef std::vector<uint64_t> VEC_TEMP_BET_COIN_POOL;
public:
	struct stBetPool
	{
		unsigned char nIdx ;
		uint64_t nBetCoinInThisPool ;
		LIST_PEERS vPeersInThisPool ;
	};
	typedef std::list<stBetPool*> LIST_BET_POOLS ;
public:
	CRoomTexasPoker();
	~CRoomTexasPoker();
	virtual void Init( unsigned int nRoomID , unsigned char nMaxSeat );
	virtual void OnPeerLeave( CRoomPeer* peer );
	unsigned char CanJoin(CPlayer* peer );
	virtual void AddPeer(CRoomPeer* peer ) ;
	virtual bool AddBeInvitedPlayer(CPlayer* peer,unsigned char nRoomIdx );
	virtual void Update(float fTimeElpas, unsigned int nTimerID );
	virtual bool OnPeerMsg(CRoomPeer* pPeer, stMsg* pmsg );
	virtual bool SendCurRoomInfoToPlayer(CRoomPeer* pPeer );
	void SendMsgRoomPeers(stMsg*pMsg ,unsigned short nLen ,CRoomPeer* pExcpetPeer = NULL );
	eRoomPeerAction GetCurPeerAction(){ return m_eCurPeerAction ;}
	uint64_t GetCurMostBetCoinThisRound(){ return m_nMostBetCoinThisRound ; }
	uint64_t GetBigBlindBet(){ return m_nBigBlindBet ;}
	void SetBigBlindBet(uint64_t nBigBlindRet ){  m_nBigBlindBet = nBigBlindRet;}
	bool IsBiggestCardPeer(CTaxasPokerPeer* pTestPlayer);
	static bool SortBetPeers(CTaxasPokerPeer*pLeft,CTaxasPokerPeer*pRight);
	bool CanPeerSitDown(CTaxasPokerPeer* peer );
protected:
	bool CheckRoundEnd();
	bool CheckGameEnd();
	void CaculateRound();
	void CaculateGameResult();

	void GoToState( eRoomState eTargetState );
	void WaitActivePeerBet();
	CRoomPeer* GetFirstActivePeerFromIdx(unsigned char nIdx );
	unsigned char GetCanActPeerCount();
	void OnStartNewGame();
	void OnStartNewRound() ;
	void OnGameEnd();
	void RemovePeerFromBetPool(CRoomPeer* peerToGiveUp );
	bool CaculateRoundEnd();
	void LogPlayersInfo();
	void LogRoomStateInfo();
	void OnPeerStandUp(CTaxasPokerPeer* pPeerStandUp);
	uint64_t GetAllowMaxAllInCoin();
protected:
	LIST_PEERS m_vAudiencers ;
	float m_fTick ;

	char m_nBankerIdx ;
	unsigned char m_nLittleBlindIdx ;
	unsigned char m_nBigBlindIdx ;
	unsigned char m_nCurWaitPlayerActionIdx ;

	uint64_t m_nCurMainBetPool ;
	uint64_t m_nMostBetCoinThisRound;
	LIST_BET_POOLS m_vAllBetPools ;
	CPoker m_ptPoker ;

	unsigned char m_vPublicCardNums[5] ; 
	eRoomPeerAction m_eCurPeerAction ; 
	unsigned char m_nBetRound ; //valid value , 1 , 2 , 3 ,4 

	uint64_t m_nBigBlindBet ;
	float m_fCaluateRoundTime;
	float m_fCaluateGameResultTime ;

	//VEC_TEMP_BET_COIN_POOL m_vCurRoundTempBetPool; // when peer gived up ,its bet coin will be put in this pool , after cacluate roundEnd , then go to main pool , this pool clean , when start new round; 
};