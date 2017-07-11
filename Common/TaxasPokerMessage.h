#pragma once
#pragma pack(push)
#pragma pack(1)
#include "MessageDefine.h"
struct stTPRoomPeerBrifInfo
	:public stPlayerBrifData
{
	uint64_t nTakeInCoin ;
	uint64_t nCurBetCoin;
	unsigned char nIdxInRoom ;
	unsigned int nSessionID ;
	unsigned int eCurState ;  // eRoomPeerState 
	unsigned char vCards[2] ;
};

struct stMsgTaxasPokerTypeEnter
:public stMsg
{
    stMsgTaxasPokerTypeEnter(){ usMsgType = MSG_TP_ENTER_ROOM ; cSysIdentifer = ID_MSG_C2GAME ; }
    unsigned char cSpeedType ;
    unsigned char cSeatType ;
    unsigned int nBigBlind ;
};

struct stVicePool
{
	unsigned char nIdx ;
	uint64_t nViceCoin ;
};

struct stMsgTaxasPokerRoomInfo
	:public stMsg
{
	stMsgTaxasPokerRoomInfo()
	{
		cSysIdentifer = ID_MSG_GAME2C; usMsgType = MSG_TP_ROOM_CUR_INFO;
	}
    unsigned char vCardsNum[MAX_TAXAS_HOLD_CARD] ;  // public cards ;
	uint64_t nMainBetPool ;
	unsigned char nMaxSeat ;
	uint64_t nBigBlindBetCoin ;
	unsigned char eCurRoomState ;//eRoomState
	unsigned char nBankerIdx ;
	unsigned char nLittleBlindIdx ;
	unsigned char nBigBlindIdx ;
	float fTick ;
	unsigned char nWaitActionPeerIdx ;
	unsigned char nViceBetPoolCnt ;
	stVicePool* vVicePools ;
	unsigned char nRoomLevel ;
	int nRoomID ;
	int nMinTakeIn ;
	uint64_t nMaxTakeIn ;
	unsigned char nCurPlayerCount ;
	unsigned char nCurBetRound ;
	stTPRoomPeerBrifInfo* pPlayers ;
};

struct stPTPeerCard
{
	unsigned char nPlayerIdx ;
	unsigned char vCardNum[2] ;
};

struct stMsgTaxasPokerPrivateDistribute
	:public stMsg
{
	stMsgTaxasPokerPrivateDistribute(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_TP_DISTRIBUTE ;}
	unsigned char nBankerIdx ;
	unsigned char nLittleBlindIdx ;
	unsigned char nBigBlindIdx ;
	unsigned char vPublicCard[5];
	unsigned char nPeerCount ;
	stPTPeerCard* vPeers ;
};

struct stMsgTaxasPokerDistribute
	:public stMsg
{
	stMsgTaxasPokerDistribute(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_TP_PUBLIC_DISTRIBUTE ; }
	unsigned char nBetRound ; // private after priate card nBetRound = 1 , after 3 cards , nBetRound = 2 , this means , if this msg.nBetRound = 1 , client should send 3 card  ; 
	//, after this distribute animate bet round will + 1 ;
};

struct stMsgTaxasPokerPeerAction
	:public stMsg
{
	stMsgTaxasPokerPeerAction() { cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_TP_PEER_ACTION ; }
	unsigned char ePeerAct ;  // eRoomPeerAction 
	uint64_t nActionValue ; 
	uint64_t nTakeInCoin ; // used when sit down ;
};

enum eTaxasPokerActRet
{
	eTPActRet_Successs,
	eTPActRet_NotYourTurn,
	eTPActRet_NotEnoughCoin,
	eTPActRet_BetAddNeedBigerThanBlindBet,
	eTPActRet_BetAddMustTimesOffset,
	eTPActRet_PosNotEmpty,
	eTPActRet_Already_SitDown,
	eTPActRet_Alread_StandUp,
	eTPActRet_StateError,
	eTPActRet_RoomStateError,
	eTPActRet_TakeInCoinError,
	eTPActRet_UnknownError,
	eTPActRet_Max,
};

struct stMsgTaxasPokerPeerActionRet
	:public stMsg 
{
	stMsgTaxasPokerPeerActionRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_TP_PEER_ACTION_RET ; }
	unsigned char ePeerAct ; // eRoomPeerAction 
	unsigned char nRet ; // eTaxasPokerActRet
	uint64_t nTakeInCoin ; // used when sit down ;
};

struct stMsgTaxasPokerOtherPeerAction
	:public stMsgTaxasPokerPeerAction
{
	stMsgTaxasPokerOtherPeerAction(){ usMsgType = MSG_TP_OTHER_PEER_ACTION ; cSysIdentifer = ID_MSG_GAME2C ; }
	unsigned char nPlayerIdxInRoom ;
	// nActionValue  Warning: this is final value after action ;
};

struct stMsgTaxasPokerOtherPeerSitDown
	:public stMsgTaxasPokerPeerAction
{
	stMsgTaxasPokerOtherPeerSitDown(){ ePeerAct = eRoomPeerAction_SitDown ; cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_TP_OTHER_PEER_ACTION; }
	stTPRoomPeerBrifInfo tPlayerInfo ;
};

struct stPoolResultInfo
{
	unsigned char nPoolIdx ;  // when = 100, means main pool 
	uint64_t nPerPeerWinCoin ;
	unsigned char nWinnerCnt ;
	// the below two , paired exist in msg Buffer ;
	unsigned char* nPeerRoomIdx ; // used to decide where the win coin animte to go ; WARNING: just animate target ;
	unsigned int* nSessionID ;  // to deacide real peer to add real coin ;
};

struct stMsgTaxasPokerGameRessult
	:public stMsg
{
	stMsgTaxasPokerGameRessult(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_TP_GAME_RESULT ;}
	unsigned char nResultPoolCount;
	stPoolResultInfo* pResultPool ;
};

struct stMsgTaxasPokerWaitPeerAction
	:public stMsg
{
	stMsgTaxasPokerWaitPeerAction(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_TP_WAIT_PEER_ACTION ; }
	unsigned int nWaitPlayerSessionID ;
	unsigned char nWaitPlayerIdx ;
	uint64_t nCurMaxBetCoin ;
	unsigned char ePriePlayerAction ;  // eRoomPeerAction 
};


struct stMsgTaxasPokerRoundEnd
	:public stMsg
{
	stMsgTaxasPokerRoundEnd(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_TP_ROUND_END ; }
	uint64_t nMainPool ;
	unsigned char nNewPoolCount ;
	stVicePool* nVicePool ;
};

struct stGameEndAutoAction
{

};

struct stMsgTaxasPeerAutoAction
	:public stMsg
{
	stMsgTaxasPeerAutoAction(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_TP_GAME_END_PEER_AUTO_ACTION ; }
	unsigned char nIdxInRoom ;
	unsigned int nSessionID ;
	unsigned char nAutoActionType ; // 0 auto take in , 1 auto stand up ; 
	uint64_t nFinalCoin ;
	uint64_t nTakeInCoin ; 
};
#pragma pack(pop)