#pragma once
#include "NativeTypes.h"
#define MAX_LEN_ACCOUNT 40   // can not big then unsigned char max = 255
#define  MAX_LEN_PASSWORD 25 // can not big then unsigned char max = 255
#define MAX_LEN_CHARACTER_NAME 25 // can not big then unsigned char  max = 255
#define MAX_LEN_SIGURE 200   // can not big then unsigned char  max = 255
#define MAX_LEN_ROOM_NAME 25
#define MAX_LEN_ROOM_DESC 60  // can not big then unsigned char max = 255
#define MAX_LEN_ROOM_INFORM 500  
#define MAX_LEN_EMAIL 50
#define MAX_LEN_SPEAK_WORDS 200  
//#define MAX_MSG_BUFFER_LEN 2048*3
#define MAX_MSG_BUFFER_LEN (4644*20)

#define PEER_CARD_COUNT 3
#define GOLDEN_PEER_CARD 3
#define TAXAS_PEER_CARD 2
#define TAXAS_PUBLIC_CARD 5
#define MAX_ROOM_PEER 5
#define MAX_TAXAS_HOLD_CARD 5
#define MAX_UPLOAD_PIC 4
#define MAX_JOINED_CLUB_CNT 10

#define MATCH_MGR_UID 1349

#define NIUNIU_HOLD_CARD_COUNT 5


#define COIN_CONDITION_TO_GET_CHARITY 500
#define TIMES_GET_CHARITY_PER_DAY 3   // 2 HOURE
#define COIN_FOR_CHARITY 800
#define GOLDEN_ROOM_COIN_LEVEL_CNT 4
#define GOLDEN_PK_ROUND 2

#ifndef SERVER
#define PIEXL_TO_POINT(px) (px)/CC_CONTENT_SCALE_FACTOR()
#define FOINT_NAME "Helvetica"
#endif

#define MAX_IP_STRING_LEN 17

#define CIRCLE_TOPIC_CNT_PER_PAGE 7
#define MAX_CIRCLE_CONTENT_LEN 700
enum ePayChannel
{
	ePay_AppStore,
	ePay_WeChat,
	ePay_WeChat_365Niu = ePay_WeChat,
	ePay_ZhiFuBao,
	ePay_XiaoMi,
	ePay_WeChat_365Golden,
	ePay_Max,
};

enum eRoomType
{
	eRoom_None = -5,
	eRoom_TexasPoker = eRoom_None,
	eRoom_NiuNiu,
	eRoom_Golden,
	eRoom_MJ_Blood_River,// 血流成河
	eRoom_MJ_Blood_End, // 血战到底 
	eRoom_MJ_NanJing = 0, 
	//eRoom_MJ_SuZhou,
	eRoom_MJ_BeiPiao,
	eRoom_MJ_CaoYang,
	eRoom_MJ_HongHe,
	eRoom_MJ_YangZhou,
	eRoom_MJ_JianShui,
	eRoom_Max ,
};

enum eMJGameType
{
	eMJ_None,
	eMJ_BloodRiver = eRoom_MJ_Blood_River,
	eMJ_BloodTheEnd,
	eMJ_NanJing = 0,
	eMJ_SuZhou,
	eMJ_COMMON,
	eMJ_Max,
};

enum eVipCardType
{
	eCard_None,
	eCard_LV1,
	eCard_LV2,
	eCard_LV3,
	eCard_Max,
};

enum ePlayerType
{
	ePlayer_Normal,
	ePlayer_Robot,
	ePlayer_Mgr,
	ePlayer_Max,
};

enum eRoomState
{
	eRoomState_None,
	eRoomState_WaitOpen,
	eRoomState_Opening,
	eRoomState_Pasue,
	eRoomState_Dead,
	eRoomState_WillDead,
	eRoomState_TP_Dead = eRoomState_Dead,
	eRoomState_WaitJoin,
	eRoomSate_WaitReady = eRoomState_WaitJoin,
	eRoomState_NN_WaitJoin = eRoomState_WaitJoin ,
	eRoomState_TP_WaitJoin = eRoomState_WaitJoin,
	eRoomState_WillClose,
	eRoomState_Close,
	eRoomState_DidGameOver,
	eRoomState_StartGame,
	eRoomState_NN_Disribute4Card = eRoomState_StartGame,
	// state for texas poker
	eRoomState_TP_BetBlind = eRoomState_StartGame,
	eRoomState_TP_PrivateCard = eRoomState_StartGame,
	eRoomState_TP_Beting,
	eRoomState_TP_OneRoundBetEndResult,
	eRoomState_TP_PublicCard,
	eRoomState_TP_GameResult,
	eRoomState_TP_MAX,
	eRoomState_TP_Insurance = eRoomState_TP_MAX,  // in case of changing below enum value ; i want add a insurance value ;

	// state for Baccarat ;
	eRoomState_BC_Shuffle,
	eRoomState_BC_WaitBet,
	eRoomState_BC_Distribute,
	eRoomState_BC_AddIdleCard,
	eRoomState_BC_AddBankerCard,
	eRoomState_BC_Caculate,

	// state for NiuNiu
	eRoomState_NN_TryBanker,
	eRoomState_NN_GrabBanker = eRoomState_NN_TryBanker,
	eRoomState_NN_RandBanker,
	eRoomState_NN_StartBet,
	eRoomState_NN_FinalCard,
	eRoomState_NN_CaculateCard,
	eRoomState_NN_GameResult,

	// state for golden
	eRoomState_Golden_Bet,
	eRoomState_Golden_PK,
	eRoomState_Golden_GameResult,

	// state for si chuan ma jiang 
	eRoomState_WaitExchangeCards, //  等待玩家换牌
	eRoomState_DoExchangeCards, // 玩家换牌
	eRoomState_WaitDecideQue,  // 等待玩家定缺
	eRoomState_DoDecideQue,  //  玩家定缺
	eRoomState_DoFetchCard, // 玩家摸牌
	eRoomState_WaitPlayerAct,  // 等待玩家操作 { idx : 0 , huaCard : 23 }
	eRoomState_WaitPlayerChu, // 等待玩家出牌 { idx : 2 }
	eRoomState_DoPlayerAct,  // 玩家操作 // { idx : 0 ,huIdxs : [1,3,2,], act : eMJAct_Chi , card : 23, invokeIdx : 23, eatWithA : 23 , eatWithB : 22 }
	eRoomState_WaitOtherPlayerAct,  // 等待玩家操作，有人出牌了 { invokerIdx : 0 , card : 0 ,cardFrom : eMJActType , arrNeedIdxs : [2,0,1] } 
	eRoomState_DoOtherPlayerAct,  // 其他玩家操作了。
	eRoomState_AskForRobotGang, // 询问玩家抢杠胡， { invokeIdx : 2 , card : 23 }
	eRoomState_AskForHuAndPeng, // 询问玩家碰或者胡  { invokeIdx : 2 , card : 23 }
	eRoomState_WaitSupplyCoin, // 等待玩家补充金币  {nextState: 234 , transData : { ... } }
	eRoomState_WaitPlayerRecharge = eRoomState_WaitSupplyCoin,  //  等待玩家充值
	eRoomState_NJ_Auto_Buhua, // 南京麻将自动不花 
	eRoomState_GameEnd, // 游戏结束
	eRoomState_Max,
};


enum eMJActType
{
	eMJAct_None,
	eMJAct_Mo = eMJAct_None, // 摸牌
	eMJAct_Chi, // 吃
	eMJAct_Peng,  // 碰牌
	eMJAct_MingGang,  // 明杠
	eMJAct_AnGang, // 暗杠
	eMJAct_BuGang,  // 补杠 
	eMJAct_BuGang_Pre, // 补杠第一阶段
	eMJAct_BuGang_Declare = eMJAct_BuGang_Pre, // 声称要补杠 
	eMJAct_BuGang_Done, //  补杠第二阶段，执行杠牌
	eMJAct_Hu,  //  胡牌
	eMJAct_Chu, // 出牌
	eMJAct_Pass, //  过 
	eMJAct_BuHua,  // 补花
	eMJAct_HuaGang, // 花杠
	eMJAct_Followed, // 连续跟了4张牌，要罚钱了
	eMJAct_4Feng, // 前4张出了4张不一样的风牌
	eMJAct_Ting, // 听牌
	eMJAct_MoBao, // 朝阳麻将第一个听牌的人摸宝牌
	eMJAct_MoBaoDice, // 朝阳麻将摸宝时投塞子
	eMJAct_Max,
};

enum eFanxingType
{
	eFanxing_PingHu, // 平胡

	eFanxing_DuiDuiHu, //  对对胡

	eFanxing_QingYiSe, // 清一色
	eFanxing_DaiYaoJiu, //  带幺九
	eFanxing_QiDui, //  七对
	eFanxing_JinGouDiao, //  金钩钓

	eFanxing_QingDuiDuiHu, // 清对对胡

	eFanxing_QingDui, //  清七对
	eFanxing_LongQiDui, //  龙七对
	eFanxing_QingDaiYaoJiu, //  清 带幺九
	eFanxing_JiangJinGouDiao, // 将金钩钓
	eFanxing_QingJinGouDiao, // 清金钩钓

	eFanxing_QingLongQiDui, //  清龙七对
	eFanxing_ShiBaLuoHan, //  十八罗汉

	eFanxing_MengQing, // 门清
	eFanxing_ShuangQiDui, // 双七对
	eFanxing_QuanQiuDuDiao, // 全球独钓
	eFanxing_YaJue, // 压绝 
	eFanxing_HunYiSe, // 混一色
	eFanxing_WuHuaGuo, // 无花果
	eFanxing_TianHu , //天胡
	eFanxing_DiHu,//地胡

	eFanxing_HaiDiLaoYue, // 海底捞月
	eFanxing_DaMenQing, // 大门清
	eFanxing_XiaoMenQing, // 小门清
	eFanxing_HaoHuaQiDui, // 豪华七对
	eFanxing_DaDiaoChe, // 大吊车

	//朝阳麻将
	eFanxing_MingPiao, // 明飘：对对胡单调将
	eFanxing_LiuLei, // 流泪：1番
	eFanxing_LouBao, // 搂宝：1番
	eFanxing_JiaHu, // 夹胡

	//红河麻将
	eFanxing_TZ_6Pair, // 塘子小七对
	eFanxing_TZ_10Old, // 塘子十老头1
	eFanxing_TZ_BadCard, // 塘子烂牌
	eFanxing_TZ_13, // 塘子十三幺
	eFanxing_TZ_7Stair, // 塘子七星
	eFanxing_TZ_YiBanGao, // 塘子一般高
	eFanxing_TZ_7StairYiBanGao, // 塘子七星一般高
	eFanxing_TZ_Zi6Pair, // 塘子字小七对

	eFanxing_7StairYiBanGao, // 七星一般高
	eFanxing_7Stair, // 七星
	eFanxing_YiBanGao, // 一般高
	eFanxing_ZhengGuiLan, // 正规烂
	eFanxing_RuanLan, // 软烂
	eFanxing_LuoDiLong, // 落地龙
	eFanxing_ShouDaiLong, // 手逮龙
	eFanxing_3LongJia, // 三龙夹背
	eFanxing_2LongJia, // 双龙夹背
	eFanxing_LongJia, // 龙夹背
	eFanxing_Qiang2Gang, // 抢双杠胡（1番 / 2番 / x10）
	eFanxing_QiangGang, // 抢杠胡（1番 / 2番 / x5）
	eFanxing_2GangPao, // 双杠上炮（1番 / 2番 / x10）
	eFanxing_GangPao, // 杠上炮（1番 / 2番 / x5）
	eFanxing_4GangKai, // 4杠开
	eFanxing_3GangKai, // 3杠开
	eFanxing_2GangKai, // 2杠开
	eFanxing_GangKai, // 杠开
	eFanxing_ZiDaDui, // 字大对
	eFanxing_Zi7Pair, // 字7对
	eFanxing_ZiLongJia, // 字龙夹背
	eFanxing_Zi2LongJia, // 字双龙夹背
	eFanxing_Zi3LongJia, // 字三龙夹背

	//扬州麻将
	eFanxing_YiTiaoLong, // 一条龙
	eFanxing_FengQing, // 风清

	eFanxing_Max, // 没有胡
};

enum eSettleType    // 这个枚举定义的只是一个中立的事件，对于发生事件的双方，叫法不一样，例如： 赢的人叫被点炮，输的人 叫 点炮。
{
	eSettle_DianPao,  // 点炮
	eSettle_MingGang, // 明杠
	eSettle_AnGang, //  暗杠
	eSettle_BuGang,  //  补杠
	eSettle_ZiMo,  // 自摸
	eSettle_HuaZhu,  //   查花猪
	eSettle_DaJiao,  //  查大叫
	eSettle_Max,
};

enum eTime
{
	eTime_ExeGameStart = 4,			// 执行游戏开始 的时间
	eTime_WaitChoseExchangeCard = 5, //  等待玩家选择换牌的时间
	eTime_DoExchangeCard = 3, //   执行换牌的时间
	eTime_WaitDecideQue = 10, // 等待玩家定缺
	eTime_DoDecideQue = 2, // 定缺时间
	eTime_WaitPlayerAct = 10,  // 等待玩家操作的时间
	eTime_WaitPlayerChoseAct = eTime_WaitPlayerAct,
	eTime_DoPlayerMoPai = 0,  //  玩家摸牌时间
	eTime_DoPlayerActChuPai = 1,  // 玩家出牌的时间
	eTime_DoPlayerAct_Gang = 0, // 玩家杠牌时间
	eTime_DoPlayerAct_Hu = 1,  // 玩家胡牌的时间
	eTime_DoPlayerAct_Peng = 0, // 玩家碰牌时间
	eTime_DoPlayerAct_Ting = 0, // 玩家听牌时间
	eTime_GameOver = 1, // 游戏结束状态持续时间
	eTime_TingedAutoChu = 1, // 听过牌，自动打，从摸到打的间隔
	eTime_MoBaoDice = 2, // 摸宝投塞子的时间
	eTime_ChangePlayerIdx = 2, // 给玩家换座位
};


// ROOM TIME BY SECOND 
#define TIME_ROOM_WAIT_READY 5
#define TIME_ROOM_DISTRIBUTE 5
#define TIME_ROOM_WAIT_PEER_ACTION 30
#define TIME_ROOM_PK_DURATION 5
#define TIME_ROOM_SHOW_RESULT 5

// Golden room time 
#define TIME_GOLDEN_ROOM_WAIT_READY 10
#define TIME_GOLDEN_ROOM_DISTRIBUTY 3
#define TIME_GOLDEN_ROOM_WAIT_ACT 25
#define TIME_GOLDEN_ROOM_PK 4
#define TIME_GOLDEN_ROOM_RESULT 2

static unsigned char s_vChangeCardDimonedNeed[GOLDEN_PEER_CARD] = {0,4,8} ;


#define JS_KEY_MSG_TYPE "msgID"

//enum eSpeed
//{
//	eSpeed_Normal,
//	eSpeed_Quick,
//	eSpeed_Max,
//};

enum eNoticeType
{
	eNotice_Text,
	eNotice_BeInvite, // { targetUID : 2345 , addCoin : 34556 }
	eNotice_InvitePrize, // { targetUID : 2345 addCoin : 3555 }
	eNotice_ApplyTakeIn, // { roomID : 235 ,roomName : "hello", applyUID : 234 , takeIn : 23423 }
	eNotice_ReplyTakeIn, // { roomID : 2345,roomName : "hello" , coin : 235 , isAgree : 0 }  // isAgree : 1 agree , 0 refuse ;
	eNotice_BeAddedToClub, // { clubID : 2356 }
	eNotice_BeRemoveFromClub, // { clubID : 2345 }
	eNotice_RecivedApplyToJoinClub, // { applicantUID : 2345 , clubID : 2345 , text : "join me" }
	eNotice_RecivedReplyForApplyForJoinClub, // { clubID : 23455 , isAgree : 1 , text : "text" }
	eNotice_ShopResult, //{ finalDiamond : 234, addDiamond : 2345, nRet : 0 , itemID : 23 }  // nRet : 0 success , 1 failed ; 
};

enum eRoomSeat
{
	eSeatCount_5,
	eSeatCount_9,
	eSeatCount_Max,
};
// player State 
enum eRoomPeerState
{
	eRoomPeer_None,
	// peer state for taxas poker peer
	eRoomPeer_SitDown = 1,
	eRoomPeer_StandUp = 1 << 1,
	eRoomPeer_Ready =  (1<<12)|eRoomPeer_SitDown ,
	eRoomPeer_StayThisRound = ((1 << 2)|eRoomPeer_SitDown)| eRoomPeer_Ready ,
	eRoomPeer_WaitCaculate = ((1 << 7)|eRoomPeer_StayThisRound ),
	eRoomPeer_AllIn = ((1 << 3)|eRoomPeer_WaitCaculate) ,
	eRoomPeer_GiveUp = ((1 << 4)|eRoomPeer_StayThisRound),
	eRoomPeer_CanAct = ((1 << 5)|eRoomPeer_WaitCaculate),
	eRoomPeer_WaitNextGame = ((1 << 6)|eRoomPeer_SitDown ),
	eRoomPeer_WithdrawingCoin = (1 << 8),  // when invoke drawing coin , must be sitdown , but when staup up , maybe in drawingCoin state 
	eRoomPeer_LackOfCoin = (1<<9)|eRoomPeer_SitDown,
	eRoomPeer_WillLeave = (1<<10)|eRoomPeer_StandUp ,
	eRoomPeer_Looked =  (1<<13)|eRoomPeer_CanAct ,
	eRoomPeer_PK_Failed = (1<<14)|eRoomPeer_StayThisRound ,

	eRoomPeer_AlreadyHu = ((1 << 15) | eRoomPeer_CanAct),  //  已经胡牌的状态
	eRoomPeer_DelayLeave = (1 << 17),  //  牌局结束后才离开
	eRoomPeer_Max,
};


enum eSex
{
	eSex_Unknown,
	eSex_Male,
	eSex_Female,
	eSex_Max,
};

enum eRoomPeerAction
{
	eRoomPeerAction_None,
	eRoomPeerAction_EnterRoom,
	eRoomPeerAction_Ready,
	eRoomPeerAction_Follow,
	eRoomPeerAction_Add,
	eRoomPeerAction_PK,
	eRoomPeerAction_GiveUp,
	eRoomPeerAction_ShowCard,
	eRoomPeerAction_ViewCard,
	eRoomPeerAction_TimesMoneyPk,
	eRoomPeerAction_LeaveRoom,
	eRoomPeerAction_Speak_Default,
	eRoomPeerAction_Speak_Text,
	// action for 
	eRoomPeerAction_Pass,
	eRoomPeerAction_AllIn,
	eRoomPeerAction_SitDown,
	eRoomPeerAction_StandUp,
	eRoomPeerAction_Max
};

enum eRoomFlag
{
	eRoomFlag_None ,
	eRoomFlag_ShowCard  ,
	eRoomFlag_TimesPK ,
	eRoomFlag_ChangeCard,
	eRoomFlag_Max,
};



 


// mail Module 
#define MAX_KEEP_MAIL_COUNT 50
enum eMailType
{
	eMail_SysOfflineEvent,// { event: concret type , arg:{ arg0: 0 , arg 1 = 3 } }  // processed in svr , will not send to client ;
	eMail_DlgNotice, // content will be send by , stMsgDlgNotice 
	eMail_ReadTimeTag,  // use tell time for public mail ;
	eMail_AddRoomCard, // { addCard : 235 , addCardNo : 23452345 }
	eMail_Sys_End = 499,

	eMail_RealMail_Begin = 500, // will mail will show in golden server windown ;
	eMail_PlainText,  // need not parse , just display the content ;
	eMail_InvitePrize, // { targetUID : 2345 , addCoin : 300 } // you invite player to join game ,and give prize to you 
	eMail_WinMatch, // { gameType:234,roomName:234,rankIdx:2,addCoin:345,cup : 2 , diamomd : 34 }
	eMail_Max,
};

enum eProcessMailAct
{
	ePro_Mail_None,
	ePro_Mail_Delete,
	ePro_Mail_DoYes,
	ePro_Mail_DoNo,
	ePro_Mail_Look,
	ePor_Mail_Max,
};

// item id , here type = id ;
enum eItemType
{
	eItem_None,
	eItem_Car = eItem_None,
	eItem_Boat,
	eItem_Airplane,
	eItem_House,
	eItem_Asset, // uplow are assets ;
	// below are can be used item ;
	eItem_Props , // can be used item ;
	eItem_Gift,
	eItem_Max,
};

#define ITEM_ID_INTERACTIVE 10
#define ITEM_ID_LA_BA 12
#define ITEM_ID_KICK_CARD 11
#define ITEM_ID_CREATE_ROOM 13
// game ranker
enum eRankType
{
	eRank_AllCoin,
	eRank_SingleWinMost,
	eRank_YesterDayWin,
	eRank_Max,
};
#define RANK_SHOW_PEER_COUNT 50


#define MAX_PAIJIU_HISTROY_RECORDER 20

enum eRoomLevel
{
	eRoomLevel_None,
	eRoomLevel_Junior = eRoomLevel_None ,
	eRoomLevel_Middle,
	eRoomLevel_Advanced,
	eRoomLevel_Super,
	eRoomLevel_Max,
};

// texas poker timer measus by second
#define TIME_TAXAS_FILP_CARD 0.2f
#define TIME_PLAYER_BET_COIN_ANI 0.3f
#define TIME_BLIND_BET_STATE (TIME_PLAYER_BET_COIN_ANI + 1) 
#define TIME_TAXAS_BET 15
#define TIME_TAXAS_WAIT_COIN_GOTO_MAIN_POOL 0.6f
#define TIME_TAXAS_MAKE_VICE_POOLS 0.8f
#define TIME_TAXAS_DISTRIBUTE_ONE_HOLD_CARD 0.2f
#define TIME_TAXAS_DISTRIBUTE_HOLD_CARD_DELAY ( 0.65f * TIME_TAXAS_DISTRIBUTE_ONE_HOLD_CARD )
#define TIME_DISTRIBUTE_ONE_PUBLIC_CARD 0.5f
#define TIME_TAXAS_WIN_COIN_GOTO_PLAYER 1.2f
#define TIME_TAXAS_CACULATE_PER_BET_POOL (TIME_TAXAS_WIN_COIN_GOTO_PLAYER+1.0f)
#define TIME_TAXAS_SHOW_BEST_CARD 0.7f
#define TIME_TAXAS_WAIT_BUY_INSURANCE 20
#define TIME_TAXAS_WAIT_CALCULATE_INSURANCE 4

#define MIN_PEERS_IN_ROOM_ROBOT 6
#define MAX_PEERS_IN_TAXAS_ROOM 9
#define TIME_LOW_LIMIT_FOR_NORMAL_ROOM 10

// time for niuniu 
#define TIME_NIUNIU_DISTRIBUTE_4_CARD_PER_PLAYER 1.2f 
#define TIME_NIUNIU_TRY_BANKER 8.0f
#define TIME_NIUNIU_RAND_BANKER_PER_WILL_BANKER 0.7f
#define TIME_NIUNIU_PLAYER_BET 8.0f
#define TIME_NIUNIU_DISTRIBUTE_FINAL_CARD_PER_PLAYER 0.3f
#define TIME_NIUNIU_PLAYER_CACULATE_CARD 8.0f
#define TIME_NIUNIU_GAME_RESULT_PER_PLAYER 1.0f  //0.8f
#define TIME_NIUNIU_GAME_RESULT_EXT 8.0f

// time for golden
#define TIME_GOLDEN_DISTRIBUTE_CARD_PER_PLAYER 1.0f 

#define SETTING_MUSIC_ON "MusicOn"
#define SETTING_SOUND_ON "SoundOn"

#define LOCAL_ACCOUNT "account"
#define LOCAL_PASSWORD "password"
#define IS_AUTO_REGISTER "IsAutoLogin"
#define IS_CREATE_ROLE "IsCreateRole"

#define TEMP_NAME "tempName"
#define TEMP_ACCOUNT "tempAccount"
#define TEMP_PASSWORD "tempPassword"


