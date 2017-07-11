#pragma once
#include "CommonDefine.h"
#define LOG_ARG_CNT 6
#define CROSS_SVR_REQ_ARG 4
#define RESEVER_GAME_SERVER_PLAYERS 100 
#define TIME_MATCH_PAUSE 60*30
#define MAX_NEW_PLAYER_HALO 120
#define  ROOM_CIRCLES_PER_VIP_ROOM_CARDS 4
#define ROOM_CARD_CNT_PER_CIRLE_NJMJ 2
enum  eLogType
{
	eLog_Register, // externString, {ip:"ipdizhi"}
	eLog_Login,  // externString, {ip:"ipdizhi"}
	eLog_BindAccount, // externString, {ip:"ipdizhi"}
	eLog_Logout, 
	eLog_ModifyPwd, // externString, {ip:"ipdizhi"}
	eLog_TaxasGameResult, // nTargetID = roomid , vArg[0] = creator uid ,var[1] = public0 ---var[5] = public4, externString: {[ {uid:234,idx:2,betCoin:4456,card0:23,card1:23,offset:-32,state:GIVE_UP,coin:23334 },{ ... },{ ... }] } 
	eLog_AddMoney, // nTargetID = userUID , var[0] = isCoin , var[1] = addMoneyCnt, var[2] final coin, var[3] finalDiamond ,var[4] subType, var[5] subarg ;
	eLog_DeductionMoney,  // nTargetID = userUID , var[0] = isCoin , var[1] = DeductionCnt, var[2] final coin, var[3] finalDiamond, var[4] subType, var[5] subarg ;
	eLog_ResetPassword,
	eLog_NiuNiuGameResult, // nTargetID = room id , vArg[0] = bankerUID , vArg[1] = banker Times, vArg[2] = finalBottomBet, externString: {[ {uid:234,idx:2,betTimes:4456,card0:23,card1:23,card2:23,card3:23,card4:23,offset:-32,coin:23334 },{ ... },{ ... }] } 
	eLog_MatchResult, // nTargetID = room id , var[0] = room type ,var[1] = termNumber, var[2] room profit;
	eLog_PlayerSitDown, // nTargetID = playerUID , var[0] = room type , var[1] = roomID  , var[2] = coin;
	eLog_PlayerStandUp, // nTargetID = playerUID , var[0] = room type , var[1] = roomID  , var[2] = coin; 
	eLog_GetCharity,   // nTargetID = playerUID , var[0] = final coin ;
	eLog_PlayerLogOut, // nTargetID = playerUID , var[0] = final Coin ;
	eLog_Purchase, // nTargetID = playerUID , var[0] = final Coin ; var[1] = shop item id ;
	eLog_ExchangeOrder, // nTargetID = playerUID , var[0] exchange configID {playerName : "guest1145", excDesc : "this is fee card", remark : "my phone number is xxxxx" }
	eLog_RobotAddCoin, // nTargetID = robotUID , var[0] offset coin ; < 0  means save coin to banker, > 0 means add coin to robot ; 
	eLog_Max,
};

enum eOperateStage
{
	eOperate_NotDo,
	eOperate_Doing,
	eOperate_Done,
	eOperate_Max,
};

enum eDBAct
{
	eDBAct_Add,
	eDBAct_Delete,
	eDBAct_Update,
	eDBAct_Select,
	eDBAct_Max,
};

enum eServerType
{
	eSvrType_Gate,
	eSvrType_Login,
	eSvrType_DB,
	eSvrType_Game,
	eSvrType_Verify,
	eSvrType_DataBase,
	eSvrType_APNS,
	eSvrType_MJ = eSvrType_APNS,
	eSvrType_Log,
	eSvrType_LogDataBase,
	eSvrType_Center,
	eSvrType_Data,
	eSvrType_Taxas,
	eSvrType_NiuNiu,
	eSvrType_Golden,
	eSvrType_Max,
};

enum  eReqMoneyType
{
	eReqMoney_TaxasTakeIn,// backArg[0] = roomID , backArg[1] = seatIdx ;
	eReqMoney_CreateRoom,  // backArg[0] = sessionID backArg[1] = ConfigID;
	eReqMoney_Max,
	eReqMoneyArgCnt = 3 ,
};

enum  eCrossSvrReqType
{
	eCrossSvrReq_DeductionMoney, //  var[0] isCoin ,var[1] needMoney, var[2] at least money,; result:  var[0] isCoin ,var[1] final deductionMoney 
	eCrossSvrReq_AddMoney, //  var[0] isCoin ,var[1] addCoin
	eCrossSvrReq_CreateTaxasRoom, // var[0] room config id, var[1] rent days; json arg:"roonName", result: var[0] room config id, var[1] newCreateRoomID, var[2] rent days ;
	eCrossSvrReq_CreateRoom, // var[0] room config id, var[1] rent minites; var[2] roomType{eRoomType}json arg:"roonName", result: var[0] room config id, var[1] newCreateRoomID , var[2] roomType{eRoomType} var[3] rent days,
	eCrossSvrReq_RoomProfit, // result: var[0] isCoin , var[1] RecvMoney, var[2] roomType{eRoomType};
	eCrossSvrReq_AddRentTime, // var[0] add minites, var[1] nRoomType,var[2] comsume coin;  result var[0] add days, var[1] nRoomType,var[2] comsume coin ;
	eCrossSvrReq_SelectTakeIn, // var[0] select player uid,  result: var[0] select player uid, var[1] isCoin, var[2] money 
	eCrossSvrReq_Inform, // var[0] target player uid 
	eCrossSvrReq_EnterRoom, // var[0] playerSessionID, var[1] targetID ,var[2] coin, var[3] target id type{0 room id , 1 ConfigID }, retsult: var[0] playerSessionID , var[1] roomType , var[2] roomID, ret{ 0, success , 1 can not find room };
	eCrossSvrReq_SyncCoin, // var[0] coin var[1] room type 
	eCrossSvrReq_ApplyLeaveRoom, // var[0] nRoomID , var[1] session id , var[2] = nGame type;
	eCrossSvrReq_LeaveRoomRet, // var[0] roomType {eRoomType} ; var[1] nRoomID ;
	eCrossSvrReq_DeleteRoom, // var[0] roomType {eRoomType} ; var[1] nRoomID ;
	eCrossSvrReq_SyncNiuNiuData, // var[0] player times , var[1] win times , var[2] SingleWinMoset ;
	eCrossSvrReq_GameOver, // var[0] roomType  json arg:roomName: "chap Match", players:{{userUID: 234,rewardID : 23 },{userUID: 234,rewardID : 23 },{userUID: 234,rewardID : 23 },{userUID: 234,rewardID : 23 }} ;
	eCrossSvrReq_Max,
};

enum eAsyncReq
{
	eAsync_CreateRoom, // extern MSG_CREATE_ROOM client , addtion : { roomID : 235, createUID : 3334, serialNum : 23455, chatRoomID : 2345234 }  // result : { ret : 0 } , must success ;
	eAsync_DeleteRoom,// { roomID : 2345 }  // ret : { ret : 0 } // 0 success , 1 not find room , 2 room is running ;
	eAsync_PostDlgNotice, // { dlgType : eNoticeType , targetUID : 2345 , arg : { ....strg } }
	eAsync_OnRoomDeleted, // { roomID : 234 }
	eAsync_DB_Select,   // { sql : "select * from table where uid = 345" , order : 0 } // order [ 0 - 3 ] biger first process ,  result : { afctRow : 1 , data : [row0,row1] }/// row { tile0 : value , title 0 ;}
	eAsync_DB_Update, // { sql : "select * from table where uid = 345" , order : 0 } // order [ 0 - 3 ] biger first process ,  result : { afctRow : 1 , data : [row0,row1] }/// row { tile0 : value , title 0 ;}
	eAsync_DB_Add,	// { sql : "select * from table where uid = 345" , order : 0 } // order [ 0 - 3 ] biger first process ,  result : { afctRow : 1 , data : [row0,row1] }/// row { tile0 : value , title 0 ;}
	eAsync_Db_Delete,	// { sql : "select * from table where uid = 345" , order : 0 } // order [ 0 - 3 ] biger first process ,  result : { afctRow : 1 , data : [row0,row1] }/// row { tile0 : value , title 0 ;}
	eAsync_ReqRoomSerials, // {roomType : 2 }  // result :  { ret : 0 , serials : [{ serial : 0 , chatRoomID : 2345} , { serial : 0 , chatRoomID : 2345} ,{ serial : 0 , chatRoomID : 2345} ] }  // ret : 0 success , 1 svr is reading from db wait a moment ; 
	eAsync_Apns, // { apnsType : 0 , targets : [234,2345,23,4] , content : "hello this is" ,msgID : "fs" ,msgdesc : "shfsg" }  apnsType : 0 , group type . 1 , target persions ;
	eAsync_ComsumDiamond, // { targetUID : 2345 , diamond : 23 } // ret : { ret : 0 , diamond : 23 } // ret : 0 success , 1 not enough diamond ;
	eAsync_GiveBackDiamond, // { targetUID : 2345 , diamond : 23 } ;
	eAsync_AgentAddRoomCard, // { targetUID : 234523 , addCard : 2345 , addCardNo  : 2345 }  // ret ; always success ;
	eAsync_AgentGetPlayerInfo, // { targetUID : 2345 } , // ret { isOnline : 0 , targetUID : 2345 , name : "hello name" , leftCardCnt : 2345  }  
	eAsync_ApplyLeaveRoom, // {uid : 234 , roomID : 2345 , reason : 0 } reason : 0 , disconnect , 1 other peer login.  result : { ret : 0 , coin : 2345 } // ret : 0 leave direct, 1 delay leave room , 2 not in room , 3 not find room   ;
	eAsync_CreateGoldRoom,
	eAsync_Max,
};

enum eCrossSvrReqSubType
{
	eCrossSvrReqSub_Default,
	eCrossSvrReqSub_TaxasSitDown, // ps: json arg: seatIdx , result: json arg just back 
	eCrossSvrReqSub_TaxasSitDownFailed,
	eCrossSvrReqSub_TaxasStandUp,
	eCrossSvrReqSub_SelectPlayerData,  // ps: orgid = sessionid , not uid this situation; var[1] isDeail, result: var[3] isDetail  , json: playTimes,winTimes,singleMost;
	eCrossSvrReqSub_Max,
};

#define FILL_CROSSE_REQUEST_BACK(resultBack,pRequest,eSenderPort)  \
	resultBack.cSysIdentifer = eSenderPort ; \
	resultBack.nJsonsLen = 0 ; \
	resultBack.nReqOrigID = pRequest->nTargetID ; \
	resultBack.nRequestSubType = pRequest->nRequestSubType ; \
	resultBack.nRequestType = pRequest->nRequestType ; \
	resultBack.nRet = 0 ; \
	resultBack.nTargetID = pRequest->nReqOrigID ; \
	memcpy_s(resultBack.vArg,sizeof(resultBack.vArg),pRequest->vArg,sizeof(resultBack.vArg)) ;


#define CON_REQ_MSG_JSON(msgCrossReq,jsonArg,autoBuf)  Json::StyledWriter jsWrite ;\
	std::string str = jsWrite.write(jsonArg) ; \
	msgCrossReq.nJsonsLen = strlen(str.c_str()); \
	CAutoBuffer autoBuf(sizeof(msgCrossReq) + msgCrossReq.nJsonsLen ); \
	autoBuf.addContent((char*)&msgCrossReq,sizeof(msgCrossReq)); \
	autoBuf.addContent(str.c_str(),msgCrossReq.nJsonsLen ) ;

#define CHECK_MSG_SIZE(CHECK_MSG,nLen) \
{\
	if (sizeof(CHECK_MSG) > (nLen) ) \
{\
	LOGFMTE("Msg Size Unlegal msg") ;	\
	return false; \
	}\
	}

#define CHECK_MSG_SIZE_VOID(CHECK_MSG,nLen) \
{\
	if (sizeof(CHECK_MSG) > (nLen) ) \
{\
	LOGFMTE("Msg Size Unlegal msg") ;	\
	return; \
	}\
	}

#if defined(_DEBUG)
extern bool CustomAssertFunction(bool isfalse, char* description, int line, char*filepath);
#define Assert(exp, description) \
if( CustomAssertFunction( (int) (exp),description, __LINE__, __FILE__ )) \
{ _asm { int 3 } } 
#else
#define Assert( exp, description)
#endif