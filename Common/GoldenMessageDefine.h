#pragma once
#pragma pack(push)
#pragma pack(1)
#include "MessageDefine.h"
#include "CommonData.h"

struct stGoldenRoomInfoPayerItem
{
	uint8_t nIdx ;
	uint32_t nUserUID ;
	uint32_t nCoin ;
	uint32_t nBetCoin ;
	uint32_t nStateFlag ;
	uint8_t vHoldChard[GOLDEN_PEER_CARD] ;
};

struct stMsgGoldenRoomPlayers
	:public stMsg
{
	stMsgGoldenRoomPlayers(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_GOLDEN_ROOM_PLAYERS ; }
	uint8_t nPlayerCnt ;
	PLACE_HOLDER(stGoldenRoomInfoPayerItem*) ;
};

struct stGoldenHoldPeerCard
{
	uint8_t nIdx ;
	uint8_t vCard[GOLDEN_PEER_CARD];
};

struct stMsgGoldenDistribute
	:public stMsg
{
	stMsgGoldenDistribute(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_GOLDEN_ROOM_DISTRIBUTE ; }
	uint8_t nCnt ;
	uint8_t nBankIdx ;
	PLACE_HOLDER(stGoldenHoldPeerCard* peerCards );
};

struct stMsgGoldenRoomWaitPlayerAct
	:public stMsg
{
	stMsgGoldenRoomWaitPlayerAct(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_GOLDEN_WAIT_PLAYER_ACT ;}
	uint8_t nActPlayerIdx ; 
};

struct stMsgGoldenPlayerAct
	:public stMsgToRoom
{
    stMsgGoldenPlayerAct(){ cSysIdentifer = ID_MSG_PORT_GOLDEN; usMsgType = MSG_GOLDEN_PLAYER_ACT ; }
	uint8_t nPlayerAct ; // eRoomPeerAction ;   add, give up , follow , add , look 
	uint32_t nValue ;    // used when add act , add offset 
};

struct stMsgGoldenPlayerActRet
	:public stMsg
{
	stMsgGoldenPlayerActRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT; usMsgType = MSG_GOLDEN_PLAYER_ACT ; }
	uint8_t nRet ; // 0 success ; 1 not your turn ; 2 . you are not in this game , 3 state error , you can not act , 4 unknown act type 5 . can not do this act , 6 coin not engough
};

struct stMsgGoldenRoomAct
	:public stMsg
{
	stMsgGoldenRoomAct(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_GOLDEN_ROOM_ACT; }
	uint8_t nPlayerIdx ;
	uint8_t nPlayerAct ; // eRoomPeerAction ;   add, give up , follow , add , look 
	uint32_t nValue ;    // used when add act , add offset 
};

struct stMsgGoldenPlayerPK
	:public stMsgToRoom
{
	stMsgGoldenPlayerPK(){ cSysIdentifer = ID_MSG_PORT_GOLDEN ;usMsgType = MSG_GOLDEN_PLAYER_PK ; }
	unsigned char nPkTargetIdx ;
};

struct stMsgGoldenPlayerPKRet
	:public stMsg
{
	stMsgGoldenPlayerPKRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_GOLDEN_PLAYER_PK ; }
	unsigned char cRet ; // 0 success , 1 not youre turn , 2 target not invalid , 3 you are not in this game;
};

struct stMsgGoldenRoomPK
	:public stMsg
{
	stMsgGoldenRoomPK(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_GOLDEN_ROOM_PK ; }
	unsigned char nActPlayerIdx ;
	unsigned char nTargetIdx ;
	bool bWin ; 
};

struct stMsgGoldenResult
	:public stMsg
{
	stMsgGoldenResult(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_GOLDEN_ROOM_RESULT ; }
	unsigned char cWinnerIdx ;
	uint32_t nWinCoin ;
	uint32_t nFinalCoin ;
};

#pragma pack(pop)