//#pragma once
//#include "RoomBase.h"
//#include "CardPoker.h"
//class CBaccaratRoom
//	:public CRoomBase
//{
//public:
//	CBaccaratRoom();
//	~CBaccaratRoom();
//	virtual void Init( unsigned int nRoomID , unsigned char nMaxSeat );
//	virtual void OnPeerLeave( CRoomPeer* peer );
//	virtual unsigned char CanJoin(CPlayer* peer );  // 0 means ok , 1 room is full , 2 money is not engough ;
//	virtual void Update(float fTimeElpas, unsigned int nTimerID );
//	virtual bool OnPeerMsg(CRoomPeer* pPeer, stMsg* pmsg );
//	virtual bool SendCurRoomInfoToPlayer(CRoomPeer* pPeer );
//	void GoToState(eRoomState eState );
//	virtual void AddPeer(CRoomPeer* peer );
//protected:
//	unsigned char GetPeerPoint(bool bBanker );
//	void OnGameEnd();
//protected:
//	CCard m_vIdleCard[MAX_BACCART_CARD_COUNT];
//	CCard m_vBankerCard[MAX_BACCART_CARD_COUNT] ;
//	float m_fTimeTicket ;
//	uint64_t m_vBetPort[eBC_BetPort_Max];
//	CPoker m_tPoker ;
//	bool m_vRecorder[eBC_BetPort_Max][MAX_BACCARAT_RECORD] ; 
//	unsigned int m_nRound ;
//};