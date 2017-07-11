#pragma once
#pragma pack(push)
#pragma pack(1)
#include "MessageDefine.h"
struct stMsgNNLeaveRoom
	:public stMsgPlayerLeaveRoom
{
	stMsgNNLeaveRoom(){ cSysIdentifer = ID_MSG_PORT_NIU_NIU ;  }
};

struct stMsgNNLeaveRoomRet
	:public stMsg
{
	stMsgNNLeaveRoomRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT; usMsgType = MSG_PLAYER_LEAVE_ROOM ; }
};

//struct stMsgNNRequestRoomInfo
//	:public stMsgToNNRoom
//{
//	stMsgNNRequestRoomInfo(){ usMsgType = MSG_NN_REQUEST_ROOM_INFO ;}
//};

struct stNNRoomInfoPayerItem
{
	uint8_t nIdx ;
	uint32_t nUserUID ;
	uint64_t nCoin ;
	uint8_t nBetTimes ;
	uint32_t nStateFlag ;
	uint8_t vHoldChard[NIUNIU_HOLD_CARD_COUNT] ;
};

struct stMsgNNRoomPlayers
	:public stMsg
{
	stMsgNNRoomPlayers(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_NN_ROOM_PLAYERS ; }
	uint8_t nPlayerCnt ;
	PLACE_HOLDER(stNNRoomInfoPayerItem*) ;
};

struct stMsgNNPlayerSitDown
	:public stMsgPlayerSitDown
{
	stMsgNNPlayerSitDown(){ cSysIdentifer = ID_MSG_PORT_NIU_NIU ; }
};

struct stMsgNNPlayerSitDownRet
	:public stMsgPlayerSitDownRet
{

};


struct stMsgNNPlayerStandUp
	:public stMsgPlayerStandUp
{
	stMsgNNPlayerStandUp(){ cSysIdentifer = ID_MSG_PORT_NIU_NIU ;  }
};

struct stMsgNNStandUp
	:public stMsgRoomStandUp
{

};

struct stDistriuet4CardItem
{
	uint8_t nSeatIdx ;
	uint8_t vCardCompsitNum[5] ;
};

struct stMsgNNDistriute4Card
	:public stMsg
{
	stMsgNNDistriute4Card(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_NN_DISTRIBUTE_4_CARD ; }
	uint8_t nPlayerCnt ;
	PLACE_HOLDER(stDistriuet4CardItem*);
};

struct stMsgNNPlayerTryBanker
	:public stMsgToNNRoom
{
	stMsgNNPlayerTryBanker(){ usMsgType = MSG_NN_PLAYER_TRY_BANKER ; }
	uint8_t nTryBankerBetTimes ;
};

struct stMsgNNPlayerTryBankerRet
	:public stMsg
{
	stMsgNNPlayerTryBankerRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_NN_PLAYER_TRY_BANKER ;  }
	uint8_t nRet ; // 0 success , 1 state error , 2 coin not enough , 3 , you are not sit down ; 4 already grabed banker ;
};


struct stMsgNNTryBanker
	:public stMsg
{
	stMsgNNTryBanker(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_NN_TRY_BANKER ; }
	uint8_t nTryerIdx ;
	uint8_t nTryBankerBetTimes ;
};

struct stMsgNNProducedBanker
	:public stMsg
{
	stMsgNNProducedBanker(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_NN_PRODUCED_BANKER ; }
	uint8_t nBankerIdx ;
	uint8_t nBankerBetTimes ;
};

struct stMsgNNRandBanker
	:public stMsg
{
	stMsgNNRandBanker(){ cSysIdentifer  = ID_MSG_PORT_CLIENT ; usMsgType = MSG_NN_RAND_BANKER ; }
	uint8_t nBankerIdx ;
	uint8_t nBankerBetTimes ;
};

struct stMsgNNPlayerBet
	:public stMsgToNNRoom
{
	stMsgNNPlayerBet(){ usMsgType = MSG_NN_PLAYER_BET ;}
	uint8_t nBetTimes ;
};

struct stMsgNNPlayerBetRet
	:public stMsg
{
	stMsgNNPlayerBetRet(){ usMsgType = MSG_NN_PLAYER_BET ; cSysIdentifer = ID_MSG_PORT_CLIENT ; }
	uint8_t nRet ; // 0 success , 1 banker coin not enough , 2 self coin not enough , 3 state error; 4 don't bet twice ;
};

struct stMsgNNBet
	:public stMsg
{
	stMsgNNBet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_NN_BET ; }
	uint8_t nPlayerIdx ;
	uint8_t nBetTimes ;
};

struct stDistributeFinalCardItem  
{
	uint8_t nPlayerIdx ;
	uint8_t nCardCompsitNum ;
};

struct stMsgNNDistributeFinalCard
	:public stMsg
{
	stMsgNNDistributeFinalCard(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_NN_DISTRIBUTE_FINAL_CARD ; }
	uint8_t nPlayerCnt ;
	PLACE_HOLDER(stDistributeFinalCardItem*) ;
};

struct stNNGameResultItem
{
	uint8_t nPlayerIdx ;
	int32_t nOffsetCoin ;
	int32_t nFinalCoin ;
};
struct stMsgNNGameResult
	:public stMsg
{
	stMsgNNGameResult(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_NN_GAME_RESULT ; }
	uint8_t nPlayerCnt ;
	PLACE_HOLDER(stNNGameResultItem*);
};

struct stMsgNNPlayerCaculateCardOk
	:public stMsgToNNRoom
{
public:
	stMsgNNPlayerCaculateCardOk(){ cSysIdentifer = ID_MSG_PORT_NIU_NIU ; usMsgType = MSG_NN_PLAYER_CACULATE_CARD_OK ; }
};

struct stMsgNNCaculateCardOk
	:public stMsg
{
public: 
	stMsgNNCaculateCardOk(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_NN_CACULATE_CARD_OK ; }
	uint8_t nPlayerIdx ;
};

struct stMsgNNRequestRoomRank
	:public stMsgToNNRoom
{
	stMsgNNRequestRoomRank(){ usMsgType = MSG_REQUEST_ROOM_RANK ;}
};

struct stMsgNNRequestLastTermRoomRank
	:public stMsgToNNRoom
{
	stMsgNNRequestLastTermRoomRank(){ usMsgType = MSG_REQUEST_LAST_TERM_ROOM_RANK ;}
};

struct stMsgNNRequestRoomRankRet
	:public stMsgRequestRoomRankRet
{
    
};

//struct stMsgRequestMyOwnNiuNiuRoomDetail
//	:public stMsgToNNRoom
//{
//	stMsgRequestMyOwnNiuNiuRoomDetail(){ usMsgType = MSG_REQUEST_MY_OWN_ROOM_DETAIL ; }
//};

struct stMsgRequestNiuNiuRoomList
	:public stMsgToNNRoom
{
	stMsgRequestNiuNiuRoomList(){ usMsgType = MSG_REQUEST_ROOM_LIST; }
};

struct stMsgModifyNiuNiuRoomName
	:public stMsgToRoom
{
	stMsgModifyNiuNiuRoomName(){ cSysIdentifer = ID_MSG_PORT_NIU_NIU; usMsgType = MSG_NN_MODIFY_ROOM_NAME ; }
	char vNewRoomName[MAX_LEN_ROOM_NAME] ;
};

struct stMsgModifyNiuNiuRoomNameRet
	:public stMsg
{
	stMsgModifyNiuNiuRoomNameRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT; usMsgType = MSG_NN_MODIFY_ROOM_NAME ; }
	uint8_t nRet ; // 0 sucess , 1 you are not creator , 2 room is dead , please pay rent fee 3 , you are not in room;
};

// room inform ;
struct stMsgRemindNiuNiuRoomNewInform
	:public stMsg
{
	stMsgRemindNiuNiuRoomNewInform(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_REMIND_NEW_ROOM_INFORM ; }
};

//struct stMsgRequestNiuNiuRoomInform
//	:public stMsgToRoom
//{
//	stMsgRequestNiuNiuRoomInform(){ cSysIdentifer = ID_MSG_PORT_NIU_NIU ; usMsgType = MSG_REQUEST_ROOM_INFORM ; }
//};
//
//struct  stMsgRequestNiuNiuRoomInformRet
//	:public stMsg
//{
//	stMsgRequestNiuNiuRoomInformRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_REQUEST_ROOM_INFORM ; }
//	uint16_t nLen ;
//	PLACE_HOLDER(char* pInform);
//};

#pragma pack(pop)

