#pragma once
#pragma pack(push)
#pragma pack(1)
#include "MessageDefine.h"
struct stMsgBaccaratRoomInfo
	:public stMsg
{
	stMsgBaccaratRoomInfo(){ cSysIdentifer = ID_MSG_GAME2C; usMsgType = MSG_BC_ROOM_INFO ; }
	unsigned char nRoomType ;
	unsigned char cRoomLevel ;
	unsigned int nMinCoinNeedToEnter ;
	unsigned short nCurPeesCount ;
	unsigned short nMaxPeerCount ;
	unsigned char cRoomState ;
	float fTimeTicket ;
	unsigned char nIdleCard[MAX_BACCART_CARD_COUNT] ;
	unsigned char nBankerCard[MAX_BACCART_CARD_COUNT] ;
	uint64_t vBetPort[eBC_BetPort_Max];
	bool vRecorder[eBC_BetPort_Max][MAX_BACCARAT_RECORD] ; 
	unsigned int m_nRound ;
	unsigned short nLeftCardCount ;
};

struct stMsgBCPlayerBet
	:public stMsg
{
	stMsgBCPlayerBet(){ cSysIdentifer = ID_MSG_C2GAME ;usMsgType = MSG_BC_BET; }
	int64_t nBetCoin ;
	unsigned char cBetPort ;  // eBC_BetPort_Max
};

struct stMsgBCPlayerBetRet
	:public stMsg
{
	stMsgBCPlayerBetRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_BC_BET;}
	unsigned char nRet ; // 0 success , 1 state error , 3 coin not enough , 2 arg error ;
};

struct stMsgBCOtherPlayerBet
	:public stMsg
{
	stMsgBCOtherPlayerBet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_BC_OTHER_BET ;}
	unsigned int nSession ;  // peer session in room , who bet ;
	int64_t nBetCoin ;
	unsigned char cBetPort ;
};

struct stMsgBCDistribute
	:public stMsg
{
	stMsgBCDistribute(){ cSysIdentifer = ID_MSG_GAME2C; usMsgType = MSG_BC_DISTRIBUTE ;}
	unsigned char vIdle[2];
	unsigned char vBanker[2] ;
};

struct stMsgBCAddCard
	:public stMsg
{
	stMsgBCAddCard(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_BC_ADD_CARD ;}
	bool bIdle ;
	unsigned char nCard ;
};

struct stMsgBCCaculate
	:public stMsg
{
	stMsgBCCaculate(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_BC_CACULATE;}
	uint64_t nFinalCoin ;
	int64_t vWinCoin[eBC_BetPort_Max] ;
};

struct stMsgBCStartBet
	:public stMsg
{
	stMsgBCStartBet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_BC_START_BET;}
};

struct stMsgBCShuffle
	:public stMsg
{
	stMsgBCShuffle(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_BC_START_SHUFFLE ;}
};

// end 
#pragma pack(pop)