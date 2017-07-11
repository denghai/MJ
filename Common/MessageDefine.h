#pragma once 
#pragma pack(push)
#pragma pack(1)
// define message struct , used between Server and Client ;
#include "MessageIdentifer.h"
#include "CommonData.h"
#define PLACE_HOLDER(X)
// WARNNING:±‰≥§◊÷∑˚¥Æ£¨Œ“√«≤ª∞¸¿®÷’Ω·∑˚ \0 ;           
struct stMsg
{
	unsigned char cSysIdentifer ;  // msg target eServerType
	unsigned short usMsgType ;
public:
	stMsg():cSysIdentifer( ID_MSG_PORT_NONE  ),usMsgType(MSG_NONE){}
};
//struct stMsgTransferData
//:public stMsg
//{
//    stMsgTransferData()
//    {
//        cSysIdentifer = ID_MSG_TRANSFER ;
//        usMsgType = MSG_TRANSER_DATA;
//        bBroadCast = false ;
//        nSessionID = 0 ;
//    }
//    uint16_t nSenderPort ; // who send this msg ;  eMsgPort
//    uint32_t nSessionID ;
//    bool bBroadCast ;
//    char pData[0] ;
//};
// client reconnect ;
struct stMsgReconnect
	:public stMsg
{
public:
	stMsgReconnect(){cSysIdentifer = ID_MSG_PORT_GATE ; usMsgType = MSG_RECONNECT ; }
public:
	unsigned int nSessionID ;
};

struct stMsgReconnectRet
	:public stMsg
{
public:
	stMsgReconnectRet(){cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_RECONNECT ; }
public:
	char nRet; // 0 : success , 1 failed ;
};

struct stMsgServerDisconnect
	:public stMsg
{
public:
	stMsgServerDisconnect(){ cSysIdentifer = ID_MSG_PORT_CLIENT; usMsgType = MSG_DISCONNECT_SERVER  ;}
	unsigned char nServerType ; // eServerType ;
};

struct stMsgPlayerLogout
	:public stMsg
{
public: 
	stMsgPlayerLogout(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_PLAYER_LOGOUT ;}
};

struct stMsgControlFlag
	:public stMsg
{
public:
	stMsgControlFlag(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_CONTROL_FLAG ; }
	uint32_t nFlag ;  // 0 , not in check , 1 , means in check , should hide something ;
	uint16_t nVerfion ;
};

struct stMsgJsonContent
	:public stMsg
{
	stMsgJsonContent(){ cSysIdentifer = ID_MSG_PORT_NONE ; usMsgType = MSG_JSON_CONTENT ; }
	uint16_t nJsLen ;
	PLACE_HOLDER(char* pJsMsgContent);
};

// register an login ;
struct stMsgRegister
	:public stMsg
{
	stMsgRegister(){cSysIdentifer = ID_MSG_PORT_LOGIN ; usMsgType = MSG_PLAYER_REGISTER ; }
	unsigned char cRegisterType ; // 0 ±Ì æ”ŒøÕµ«¬º£�?±Ì æ’˝≥£◊¢≤· , 2 ∞Û∂®’À∫≈ 
	char cAccount[MAX_LEN_ACCOUNT] ;
	char cPassword[MAX_LEN_PASSWORD] ;
	unsigned char nChannel; // «˛µ¿±Í æ£¨0. appstore  1. pp ÷˙ ÷£¨2.  91…Ãµ�?3. 360…Ãµ�?4.winphone store
	char cName[MAX_LEN_CHARACTER_NAME] ;
};

struct stMsgRegisterRet
	:public stMsg
{
	stMsgRegisterRet()
	{
		cSysIdentifer = ID_MSG_PORT_CLIENT ;
		usMsgType = MSG_PLAYER_REGISTER ;
	}
	char nRet ; // 0 success ;  1 . account have exsit , 2 nick name already exsit;
	unsigned char cRegisterType ; // 0 ±Ì æ”ŒøÕµ«¬º£�?±Ì æ’˝≥£◊¢≤· , 2 ∞Û∂®’À∫≈ 
	char cAccount[MAX_LEN_ACCOUNT] ;
	char cPassword[MAX_LEN_PASSWORD] ;
	unsigned int nUserID ;
};

struct stMsgLogin
	:public stMsg 
{
	stMsgLogin(){ cSysIdentifer = ID_MSG_PORT_LOGIN ; usMsgType = MSG_PLAYER_LOGIN ; }
	char cAccount[MAX_LEN_ACCOUNT] ;
	char cPassword[MAX_LEN_PASSWORD] ;
};

struct stMsgLoginRet
	:public stMsg 
{
	stMsgLoginRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT; usMsgType = MSG_PLAYER_LOGIN ; }
	unsigned char nRet ; // 0 ; success ; 1 account error , 2 password error, 3 state error  ;
	uint8_t nAccountType ; // 0 gueset , 1 registered ,2 rebinded  .
};

struct stMsgRebindAccount
	:public stMsg
{
	stMsgRebindAccount(){ cSysIdentifer = ID_MSG_PORT_LOGIN; usMsgType = MSG_PLAYER_BIND_ACCOUNT ;}
	unsigned int nCurUserUID ;
	char cAccount[MAX_LEN_ACCOUNT] ;
	char cPassword[MAX_LEN_PASSWORD] ;
};

struct stMsgRebindAccountRet
	:public stMsg
{
	stMsgRebindAccountRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ;; usMsgType = MSG_PLAYER_BIND_ACCOUNT ; }
	unsigned char nRet ; // 0 success , 1 double account , 2 uid not exsit, 3 unknown error  ;
};

struct stMsgModifyPassword
	:public stMsg
{
	stMsgModifyPassword(){ cSysIdentifer = ID_MSG_PORT_LOGIN ; usMsgType = MSG_MODIFY_PASSWORD ; }
	unsigned int nUserUID ;
	char cOldPassword[MAX_LEN_PASSWORD] ;
	char cNewPassword[MAX_LEN_PASSWORD] ;
};

struct stMsgModifyPasswordRet
	:public stMsg
{
	stMsgModifyPasswordRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ;usMsgType = MSG_MODIFY_PASSWORD ; }
	unsigned char nRet ; // 0 success , 1 uid not exsit , 2 old passworld error 
};

struct stMsgResetPassword
	:public stMsg
{
	stMsgResetPassword(){ cSysIdentifer = ID_MSG_PORT_LOGIN ; usMsgType = MSG_RESET_PASSWORD ; }
	char cAccount[MAX_LEN_ACCOUNT] ;
	char cNewPassword[MAX_LEN_PASSWORD] ;
};

struct stMsgResetPasswordRet
	:public stMsg
{
	stMsgResetPasswordRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_RESET_PASSWORD ; }
 	uint8_t nRet ; // 0 success , 1 can not find account ;
};


struct stMsgPlayerOtherLogin
	:public stMsg
{
	stMsgPlayerOtherLogin(){cSysIdentifer = ID_MSG_PORT_CLIENT; usMsgType = MSG_PLAYER_OTHER_LOGIN ;}
};



struct stMsgPlayerBaseData
	:public stMsg
{
	stMsgPlayerBaseData(){ cSysIdentifer = ID_MSG_PORT_CLIENT; usMsgType = MSG_PLAYER_BASE_DATA ; }
	stCommonBaseData stBaseData ;
	uint32_t nSessionID ;
};

struct stMsgPlayerBaseDataTaxas
	:public stMsg
{
	stMsgPlayerBaseDataTaxas(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_PLAYER_BASE_DATA_TAXAS ; }
	stPlayerGameData tTaxasData ;
};

struct stMsgPlayerBaseDataNiuNiu
	:public stMsg
{
	stMsgPlayerBaseDataNiuNiu(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_PLAYER_BASE_DATA_NIUNIU ; }
	stPlayerGameData tNiuNiuData ;
};

struct stMsgRequestPlayerData
	:public stMsg
{
	stMsgRequestPlayerData(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_REQUEST_PLAYER_DATA ; }
	bool isDetail ;
	uint32_t nPlayerUID ;
};

struct stMsgRequestPlayerDataRet
	:public stMsg
{
	stMsgRequestPlayerDataRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_REQUEST_PLAYER_DATA ;}
	uint8_t nRet ; // 0 success , 1 can not find player 
	bool isDetail ;
	PLACE_HOLDER(stPlayerBrifData* tData ;);  //or stPlayerDetailDataClient* ; 
};


struct stMsgPlayerUpdateMoney
	:public stMsg
{
	stMsgPlayerUpdateMoney(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_PLAYER_UPDATE_MONEY ; }
	uint32_t nFinalCoin ;
	uint32_t nFinalDiamoned ;
	uint32_t nCupCnt ;
};

struct stMsgDlgNotice
	:public stMsg
{
	stMsgDlgNotice(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_DLG_NOTICE ; }
	uint8_t nNoticeType ;   // eNoticeType ;
	uint16_t nJsonLen ;
	PLACE_HOLDER(char* pJson);
};

// modify name and sigure
struct stMsgPLayerModifyName
	:public stMsg
{
	stMsgPLayerModifyName(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_PLAYER_MODIFY_NAME ; }
	char pNewName[MAX_LEN_CHARACTER_NAME] ;
};

struct stMsgPlayerModifyNameRet
	:public stMsg
{
	stMsgPlayerModifyNameRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_PLAYER_MODIFY_NAME ; }
	unsigned char nRet ; // 0 ok
	char pName[MAX_LEN_CHARACTER_NAME] ;
};

struct stMsgPLayerModifySigure
	:public stMsg
{
	stMsgPLayerModifySigure(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_PLAYER_MODIFY_SIGURE ; }
	char pNewSign[MAX_LEN_SIGURE] ;
};

struct stMsgPlayerModifySigureRet
	:public stMsg
{
	stMsgPlayerModifySigureRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_PLAYER_MODIFY_SIGURE ; }
	unsigned char nRet ;        // 0 ok
};

struct stMsgPlayerModifyPhoto
	:public stMsg
{
	stMsgPlayerModifyPhoto(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_PLAYER_MODIFY_PHOTO ; }
	uint16_t nPhotoID ;
};

struct stMsgPlayerModifyPhotoRet
	:public stMsg
{
	stMsgPlayerModifyPhotoRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_PLAYER_MODIFY_PHOTO ; }
	uint8_t nRet ; // 0 means success ;
};

struct stMsgPlayerModifySex
	:public stMsg
{
	stMsgPlayerModifySex(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_PLAYER_MODIFY_SEX ; }
	uint8_t nNewSex ;
};

struct stMsgPlayerModifySexRet
	:public stMsg
{
	stMsgPlayerModifySexRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_PLAYER_MODIFY_SEX ; }
	uint8_t nRet ; // 0 means success ;
};



// friend 
struct stMsgPlayerRequestFriendList
	:public stMsg
{
	stMsgPlayerRequestFriendList(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_REQUEST_FRIEND_LIST ; }
};

struct stMsgPlayerRequestFriendListRet
	:public stMsg
{
	stMsgPlayerRequestFriendListRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_REQUEST_FRIEND_LIST ; }
	uint16_t nFriendCount ; 
	PLACE_HOLDER(uint32_t* nFriendUIDs);
};

struct stMsgPlayerAddFriend
	:public stMsg
{
	stMsgPlayerAddFriend(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_PLAYER_ADD_FRIEND ; }
	uint32_t nTargetUID ;
};

struct stMsgPlayerAddFriendRet 
	:public stMsg
{
	stMsgPlayerAddFriendRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_PLAYER_ADD_FRIEND ; }
	uint8_t nRet ; // 0 target agree, 1 target disagree , 2 your friend list is full , 3 target player friend list is full , 4 target offline , 5 , already friend ;
	uint32_t nTaregtUID;
	char pReplayerName[MAX_LEN_CHARACTER_NAME] ;
};

struct stMsgPlayerBeAddedFriend  
	:public stMsg
{
	stMsgPlayerBeAddedFriend(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_PLAYER_BE_ADDED_FRIEND ; }
	uint32_t nPlayerUserUID; // who want add you 
	char pPlayerName[MAX_LEN_CHARACTER_NAME] ; // who want add you 
};

struct stMsgPlayerBeAddedFriendReply
	:public stMsg
{
	stMsgPlayerBeAddedFriendReply(){ cSysIdentifer = ID_MSG_PORT_DATA; usMsgType = MSG_PLAYER_BE_ADDED_FRIEND_REPLY ; }
	uint32_t nReplayToPlayerUserUID ;  // who you relay to ;
	bool bAgree ; // 1 agree to make friend , 0  don't want to make friend ;
};

struct stMsgPlayerBeAddedFriendReplyRet
	:public stMsg
{
	stMsgPlayerBeAddedFriendReplyRet(){cSysIdentifer = ID_MSG_PORT_CLIENT; usMsgType = MSG_PLAYER_BE_ADDED_FRIEND_REPLY ;}
	uint8_t nRet ; // 0 success ; 1 target list full , 2 self list full  3 target offline
	uint32_t nNewFriendUserUID ;  // who you relay to ;
};

struct stMsgPlayerDelteFriend
	:public stMsg
{
	stMsgPlayerDelteFriend(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_PLAYER_DELETE_FRIEND ; }
	uint32_t nDelteFriendUserUID ;
};

struct stMsgPlayerDelteFriendRet
	:public stMsg
{
	stMsgPlayerDelteFriendRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_PLAYER_DELETE_FRIEND ; }
	unsigned char nRet ; // 0 success , 1 target player aready is not your friend ;
	uint32_t nDeleteUID ;
};

// mail
struct stMsgInformNewMail
	:public stMsg
{
	stMsgInformNewMail(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_PLAYER_INFORM_NEW_MAIL ; }
	uint8_t nUnreadMailCount ;
};

struct stMsgRequestMailList
	:public stMsg
{
	stMsgRequestMailList(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_PLAYER_REQUEST_MAIL_LIST ; }
};

struct stMail
{
	uint32_t nPostTime ;
	uint8_t eType;
	uint16_t nContentLen ;
	PLACE_HOLDER(char* pJsonContent) ;
	stMail(){ nContentLen = 0 ; }
};

struct stMsgRequestMailListRet
	:public stMsg
{
	stMsgRequestMailListRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_PLAYER_REQUEST_MAIL_LIST ; }
	bool isFinal ;
	stMail tMail;
};

struct stMsgPlayerAdvice
	:public stMsg
{
	stMsgPlayerAdvice(){ cSysIdentifer = ID_MSG_PORT_DATA; usMsgType = MSG_PLAYER_ADVICE ; }
	uint16_t nLen ;
	PLACE_HOLDER(char* pAdviceContent);
};

struct stMsgPlayerAdviceRet
	:public stMsg
{
	stMsgPlayerAdviceRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT; usMsgType = MSG_PLAYER_ADVICE ; }
	uint8_t nRet ;
};

// exchange module 
struct stMsgPlayerExchange
	:public stMsg
{
	stMsgPlayerExchange(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_PLAYER_EXCHANGE ; }
	uint16_t nExchangeID ;
	uint16_t nJsonLen ;
	PLACE_HOLDER(char* json); // {remark : "my phone name is xxxx" }
};

struct stMsgPlayerExchangeRet
	:public stMsg
{
	stMsgPlayerExchangeRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_PLAYER_EXCHANGE ;}
	uint8_t nRet ; // 0 success , 1 can not find target exchange id , 2 diamond is not engough ; 3 you are not log in ;
	uint16_t nExchageID ;
};

struct stMsgRequestExchangeList
	:public stMsg
{
	stMsgRequestExchangeList(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_REQUEST_EXCHANGE_LIST ; }
};

struct stExchangeItem
{
	uint16_t nExchangeID ;
	uint32_t nExchangedCnt ;
};

struct stMsgRequestExchangeListRet 
	:public stMsg
{
	stMsgRequestExchangeListRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_REQUEST_EXCHANGE_LIST ; }
	uint8_t nCnt ;
	PLACE_HOLDER(stExchangeItem* allExchangeIDs);
};

struct stMsgRequestExchangeDetail
	:public stMsg
{
	stMsgRequestExchangeDetail(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_REQUEST_EXCHANGE_DETAIL ; }
	uint16_t nExchangeID ;
};

struct stMsgRequestExchangeDetailRet 
	:public stMsg
{
	stMsgRequestExchangeDetailRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_REQUEST_EXCHANGE_DETAIL ;  }
	uint8_t nRet ; // 0 success , 1 can not find target exchange id ;
	uint16_t nExchangeID ;
	uint16_t nJsonLen ;
	PLACE_HOLDER(char* pJsonDetail); // {desc: "this is describle" , diamond : 25, icon : "fee.jpg" }
};

// vip card 
struct stMsgGetVipcardGift
	:public stMsg
{
	stMsgGetVipcardGift(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_GET_VIP_CARD_GIFT ; }
	uint8_t nVipCardType ; // eCardType 
};

struct stMsgGetVipcardGiftRet
	:public stMsg
{
	stMsgGetVipcardGiftRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_GET_VIP_CARD_GIFT ; }
	uint8_t nRet ; // 0 success , 1 you do not have card , 2 card expire , 3 already got today ;
	uint8_t nVipCardType ;
	uint32_t nAddCoin ;
};

// Charity module ;
struct stMsgPlayerRequestCharityState
	:public stMsg
{
	stMsgPlayerRequestCharityState(){ cSysIdentifer = ID_MSG_PORT_DATA; usMsgType = MSG_PLAYER_REQUEST_CHARITY_STATE ; }
};

struct stMsgPlayerRequestCharityStateRet
	:public stMsg
{
	stMsgPlayerRequestCharityStateRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT; usMsgType = MSG_PLAYER_REQUEST_CHARITY_STATE ; }
	uint8_t nState ; // 0 can get charity , 1 you coin is enough , do not need charity, 2 times reached ;
	uint8_t nLeftTimes ;
};

struct stMsgPlayerGetCharity
	:public stMsg
{
	stMsgPlayerGetCharity(){ cSysIdentifer = ID_MSG_PORT_DATA; usMsgType = MSG_PLAYER_GET_CHARITY; }
};

struct stMsgPlayerGetCharityRet
	:public stMsg
{
	stMsgPlayerGetCharityRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT; usMsgType = MSG_PLAYER_GET_CHARITY; }
	uint8_t nRet ; // 0 success ,  1 you coin is enough , do not need charity, 2 times limit ;
	uint32_t nGetCoin ;
	uint32_t nFinalCoin ;
	uint8_t nLeftTimes ;
};

// shop
struct stMsgPlayerBuyShopItem
	:public stMsg 
{
public:
	stMsgPlayerBuyShopItem(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_BUY_SHOP_ITEM ; }
	unsigned short nShopItemID ;
	unsigned int nBuyShopItemForUserUID ; // 0 means buy it for self , other means buy it for other player ;
	unsigned int nCount ;
	// below only used when RMB purchase 
	uint32_t nMiUserUID ;
	unsigned char nChannelID ; // ePayChannel 
	unsigned short nBufLen ;   // based64 string , for app store purchase ;, or xiao mi cporder id , other may not use ;
	PLACE_HOLDER(char* pBuffer);
};

struct stMsgPlayerBuyShopItemRet 
	:public stMsg
{
	stMsgPlayerBuyShopItemRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_BUY_SHOP_ITEM ; }
	unsigned short nShopItemID; // buyed shoped;
	unsigned int nBuyShopItemForUserUID;
	unsigned char nRet ; // 0 success , 1 money not enough , 2 verify failed , 3 buy times limit , 4 shop item out of date, 5 shopitem don't exsit , 6 unknown error;
	uint64_t nFinalyCoin ;
	unsigned int nDiamoned ;
	unsigned int nSavedMoneyForVip ;  // a vip player can buy discont shop item , this is saved money compare to normal player ;
};

struct stMsgPlayerShopBuyItemOrder
	:public stMsg
{
	stMsgPlayerShopBuyItemOrder(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_SHOP_BUY_ITEM_ORDER ; }
	uint16_t nShopItemID ;
	uint8_t nChannel ; // ePayChannel 
};

struct stMsgPlayerShopBuyItemOrderRet
	:public stMsg
{
	stMsgPlayerShopBuyItemOrderRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_SHOP_BUY_ITEM_ORDER ; }
	uint8_t nRet ; // 0 success , 1 can not find shop item ,2 client ip is null ,3 argument error ; 
	uint16_t nShopItemID ;
	uint8_t nChannel ; // ePayChannel 
	char cPrepayId[64] ;
	char cOutTradeNo[32];
};

// invite 
struct stMsgCheckInviter
	:public stMsg
{
	stMsgCheckInviter(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_PLAYER_CHECK_INVITER ; }
	uint32_t nInviterUID ;
};

struct stMsgCheckInviterRet
	:public stMsg
{
	stMsgCheckInviterRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_PLAYER_CHECK_INVITER ; }
	uint8_t nRet ; // 0 , success , 1 inviter not exsit , 2 you already have inviter ;
	uint32_t nInviterUID ;
};

// room msg 

struct stMsgToRoom
	:public stMsg
{
	stMsgToRoom(){ cSysIdentifer = ID_MSG_PORT_TAXAS; nSubRoomIdx = -1 ;}
	uint32_t nRoomID ;
	int8_t nSubRoomIdx ;
};

struct stMsgToNNRoom
	:public stMsgToRoom
{
	stMsgToNNRoom(){ cSysIdentifer = ID_MSG_PORT_NIU_NIU ;}
};

struct stMsgRequestRoomList
	:public stMsg
{
	stMsgRequestRoomList(){ cSysIdentifer = ID_MSG_PORT_NONE ; usMsgType = MSG_TP_REQUEST_ROOM_LIST; }
};

struct stMsgRequestRoomListRet
	:public stMsg
{
	stMsgRequestRoomListRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_REQUEST_ROOM_LIST ; }
	uint8_t nRoomType ;
	uint8_t nRoomCnt ;
	PLACE_HOLDER(uint32_t* vRoomIDs);
};

//struct stMsgRequestRoomItemDetail
//	:public stMsgToRoom
//{
//	stMsgRequestRoomItemDetail(){ cSysIdentifer = ID_MSG_PORT_NONE ; usMsgType = MSG_REQUEST_ROOM_ITEM_DETAIL ;}
//	uint32_t nUserUID ;
//};
//
//struct stMsgRequestRoomItemDetailRet
//	:public stMsg
//{
//	stMsgRequestRoomItemDetailRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_REQUEST_ROOM_ITEM_DETAIL ; }
//	uint8_t nRet ; // 0 success , 1 not find room ;
//	uint8_t nRoomType ;
//	uint32_t nRoomID ;
//	uint32_t nOwnerUID ; // 0 means public rooms , other value , private room ;
//	uint16_t nJsonLen ;
//	PLACE_HOLDER(char* pLen ); 
//	// public room : { configID : 23 ,name : "number 1 poker", openTime : 23345 , closeTime: 2345, state : 23  }
//	// private room:  { configID : 23 , closeTime: 2345, state : 23 ,createTime : 2345 , curCnt : 3 , offset : -34 };
//};

struct stMsgRequestRoomRewardInfo
	:public stMsgToRoom
{
	stMsgRequestRoomRewardInfo(){ cSysIdentifer = ID_MSG_PORT_NONE ; usMsgType = MSG_REQUEST_ROOM_REWARD_INFO ;}
};

struct stMsgRequestRoomRewardInfoRet
	:public stMsg
{
	stMsgRequestRoomRewardInfoRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_REQUEST_ROOM_REWARD_INFO ; }
	uint8_t nRoomType ;
	uint32_t nRoomID ;
	uint16_t nJsonLen ;
	PLACE_HOLDER(char* pLen ); // { 0 : "1 reward desc",1 : "1 reward desc" , 2 : "2 reward desc",3 : "3 reward desc" };
};

struct stMsgRequestRoomInfo
	:public stMsgToRoom
{
	stMsgRequestRoomInfo(){ cSysIdentifer = ID_MSG_PORT_TAXAS; usMsgType = MSG_REQUEST_ROOM_INFO ; }
};

// create room
//struct stMsgCreateRoom
//	:public stMsg
//{
//	stMsgCreateRoom(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_CREATE_ROOM ; }
//	uint8_t nRoomType ; // eRoomType ;
//	uint16_t nConfigID ;
//	uint16_t nMinites ;
//	char vRoomName[MAX_LEN_ROOM_NAME] ;
//};
//
//struct stMsgCreateRoomRet
//	:public stMsg
//{
//	stMsgCreateRoomRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_CREATE_ROOM ; }
//	uint8_t nRoomType ; // eRoomType ;
//	uint8_t nRet ; // 0 success , 1 config error , 2 no more chat room id , 3 can not connect to chat svr , 4 coin not enough , 5 reach your own room cnt up limit , 6 unknown room type ;
//	uint32_t nRoomID ; 
//	uint64_t nFinalCoin ; 
//};
//
//struct stMsgDeleteRoom
//	:public stMsg
//{
//	stMsgDeleteRoom(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_DELETE_ROOM ; }
//	uint8_t nRoomType ;
//	uint32_t nRoomID ;
//};
//
//struct stMsgDeleteRoomRet
//	:public stMsg
//{
//	stMsgDeleteRoomRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_DELETE_ROOM ; }
//	uint8_t nRet ; // 0 success , 1 you don't have target room , 2 unknown room type 
//	uint8_t nRoomType ;
//	uint32_t nRoomID ;
//};

struct stMsgCaculateRoomProfit
	:public stMsg
{
	stMsgCaculateRoomProfit(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_TP_CACULATE_ROOM_PROFILE ; }
	uint32_t nRoomID ;
	uint8_t nRoomType ;
};

struct stMsgCaculateRoomProfitRet
	:public stMsg
{
	stMsgCaculateRoomProfitRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_TP_CACULATE_ROOM_PROFILE ; }
	uint8_t nRet ; // 0 sucess , 1 you are not creator , 2 unknown room type , 3 can not find room ;
	uint8_t nRoomType ;
	uint32_t nRoomID ;
	bool bDiamond ;
	uint64_t nProfitMoney;
};

struct stMsgAddRoomRentTime
	:public stMsg
{
	stMsgAddRoomRentTime(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_ADD_RENT_TIME ; }
	uint8_t nRoomType ;
	uint32_t nRoomID ;
	uint16_t nAddDays ;
};

struct  stMsgAddRoomRentTimeRet
	: public stMsg
{
	stMsgAddRoomRentTimeRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_ADD_RENT_TIME ; }
	uint8_t nRet ; // 0 success , 1 you are not creator , 2 coin not enough , 3 unknown room Type, 4 can not find room ;
	uint8_t nRoomType ;
	uint32_t nRoomID ;
	uint16_t nAddDays ;
};

struct stMsgRoomEnterNewState
	:public stMsg
{
	stMsgRoomEnterNewState(){ cSysIdentifer = ID_MSG_PORT_CLIENT; usMsgType = MSG_ROOM_ENTER_NEW_STATE ; }
	uint16_t nNewState ;   // eRoomState 
	float m_fStateDuring ; 
};

// enter and leave 
struct stMsgPlayerEnterRoom
	:public stMsgToRoom
{
	stMsgPlayerEnterRoom(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_PLAYER_ENTER_ROOM ; nSubIdx = -1 ; }
	uint8_t nRoomGameType ;
	uint32_t nRoomID ;
	int8_t nSubIdx ; // -1 means sys decide ;
};

struct stMsgPlayerEnterRoomRet 
	:public stMsg
{
	stMsgPlayerEnterRoomRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_PLAYER_ENTER_ROOM ; }
	uint8_t nRet ; // 0 success , 1 already in this room , 2 not register player  can not enter ; 3 player coin is too few ; 4 ;  player coin is too many ; 5 can not find room id ,  6 room type error 
	uint32_t nRoomID;
};

struct stMsgPlayerLeaveRoom
	:public stMsgToRoom
{
	stMsgPlayerLeaveRoom(){ usMsgType = MSG_PLAYER_LEAVE_ROOM ;}
};

struct stMsgPlayerLeaveRoomRet
	:public stMsg
{
	stMsgPlayerLeaveRoomRet(){ usMsgType = MSG_PLAYER_LEAVE_ROOM ; cSysIdentifer = ID_MSG_PORT_CLIENT ; }
	uint8_t nRet ; // 0 success , 1 you are not in room ;
};

// enter and leave gold room
struct stMsgPlayerEnterGoldRoom
	:public stMsgToRoom
{
	stMsgPlayerEnterGoldRoom() { cSysIdentifer = ID_MSG_PORT_DATA; usMsgType = MSG_PLAYER_ENTER_GOLDROOM; nSubIdx = -1; }
	uint8_t nRoomGameType;
	uint32_t nRoomID;
	int8_t nSubIdx; // -1 means sys decide ;
};

struct stMsgPlayerEnterGoldRoomRet
	:public stMsg
{
	stMsgPlayerEnterGoldRoomRet() { cSysIdentifer = ID_MSG_PORT_CLIENT; usMsgType = MSG_PLAYER_ENTER_GOLDROOM; }
	uint8_t nRet; // 0 success , 1 already in this room , 2 not register player  can not enter ; 3 player coin is too few ; 4 ;  player coin is too many ; 5 can not find room id ,  6 room type error , 8 Queuing up
	uint32_t nRoomID;
};

// buy in 
//struct stMsgPlayerReBuyIn
//	:public stMsgToRoom
//{
//public:
//	stMsgPlayerReBuyIn(){ usMsgType = MSG_PLAYER_REBUY ;}
//	uint32_t nBuyInCoin ;
//};
//
//struct stMsgPlayerReBuyInRet
//	:public stMsg
//{
//public:
//	stMsgPlayerReBuyInRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_PLAYER_REBUY ;}
//	uint8_t nRet ; // 0 success , 1 coin not enough , 2 take in count error ;
//	uint32_t nBuyInCoin ;
//	uint32_t nFinalCoin ;
//};

struct stMsgPrivateRoomReBuyIn
	:public stMsg
{
public:
	stMsgPrivateRoomReBuyIn(){ usMsgType = MSG_PRIVATE_ROOM_PLAYER_REBUY ; cSysIdentifer = ID_MSG_PORT_CLIENT ;}
	uint8_t nIdx ;
	uint32_t nBuyInCoin ;
	uint32_t nFinalCoin ;
};

struct stMsgRequestPrivateRoomRecorder
	:public stMsgToRoom
{
public:
	stMsgRequestPrivateRoomRecorder(){ usMsgType = MSG_REQUEST_PRIVATE_ROOM_RECORDER ;}
};


struct stMsgRequestPrivateRoomRecorderRet
	:public stMsg
{
public:
	stMsgRequestPrivateRoomRecorderRet(){ usMsgType = MSG_REQUEST_PRIVATE_ROOM_RECORDER ; cSysIdentifer = ID_MSG_PORT_CLIENT ; }
	uint8_t nRet ; // 0 success , 1 can not find recorder ;
	uint32_t nRoomID ;
	uint32_t nCreaterUID ;
	uint16_t nConfigID ;
	uint32_t tTime ;
	uint8_t nRoomType ;
	uint16_t nJsLen ;
	PLACE_HOLDER(char* pJson);// [ { uid : 23 , buyIn : 234, offset : -30},{ uid : 23 , buyIn : 234, offset : -30}, .... ]
};

struct stMsgRequestPrivateRoomRecorderNew  // new edition 
	:public stMsgToRoom
{
public:
	stMsgRequestPrivateRoomRecorderNew(){ usMsgType = MSG_REQUEST_PRIVATE_ROOM_RECORDER_NEW; }
	uint32_t nSieralNum;
};


struct stMsgRequestPrivateRoomRecorderNewRet // new edition 
	:public stMsg
{
public:
	stMsgRequestPrivateRoomRecorderNewRet(){ usMsgType = MSG_REQUEST_PRIVATE_ROOM_RECORDER_NEW; cSysIdentifer = ID_MSG_PORT_CLIENT; }
	uint8_t nRet; // 0 success , 1 can not find recorder ;
	uint32_t nRoomID;
	uint32_t nCreaterUID;
	uint16_t nConfigID;
	uint32_t tTime;
	uint8_t nRoomType;
	uint32_t nSieralNum;
	uint16_t nJsLen;
	PLACE_HOLDER(char* pJson);// [ { uid : 23 , buyIn : 234, offset : -30},{ uid : 23 , buyIn : 234, offset : -30}, .... ]
};

struct stMsgPlayerRequestGameRecorder
	:public stMsg
{
	stMsgPlayerRequestGameRecorder(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_PLAYER_REQUEST_GAME_RECORDER ; }
};

struct stRecorderItem
{
	uint32_t nRoomID ;
	uint16_t nBaseBet ;
	uint32_t nCreateUID ;
	uint32_t nFinishTime ;
	uint32_t nDuiringSeconds ;
	int32_t nOffset ;
	uint32_t nBuyIn ;
	uint32_t nClubID ;
	char cRoomName[MAX_LEN_ROOM_NAME] ;
};

struct stMsgPlayerRequestGameRecorderRet
	:public stMsg
{
	stMsgPlayerRequestGameRecorderRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_PLAYER_REQUEST_GAME_RECORDER ; }
	uint8_t nCnt ; 
	PLACE_HOLDER(stRecorderItem*);
};

struct stMsgPlayerRequestGameRecorderNew  // new edition 
	:public stMsg
{
	stMsgPlayerRequestGameRecorderNew(){ cSysIdentifer = ID_MSG_PORT_DATA; usMsgType = MSG_PLAYER_REQUEST_GAME_RECORDER_NEW; }
};

struct stRecorderItemNew   // new edition 
{
	uint32_t nRoomID;
	uint16_t nBaseBet;
	uint32_t nCreateUID;
	uint32_t nFinishTime;
	uint32_t nDuiringSeconds;
	int32_t nOffset;
	uint32_t nBuyIn;
	uint32_t nClubID;
	uint32_t nSieralNum;
	char cRoomName[MAX_LEN_ROOM_NAME];
};

struct stMsgPlayerRequestGameRecorderNewRet  // new edition 
	:public stMsg
{
	stMsgPlayerRequestGameRecorderNewRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT; usMsgType = MSG_PLAYER_REQUEST_GAME_RECORDER_NEW; }
	uint8_t nCnt;
	PLACE_HOLDER(stRecorderItemNew*);
};

// sit down ;
struct stMsgPlayerSitDown
	:public stMsgToRoom
{
	stMsgPlayerSitDown(){ usMsgType = MSG_PLAYER_SITDOWN ;}
	uint32_t nTakeInCoin ; // 0 ,means take in all ; 
	uint8_t nIdx ;
};

struct stMsgPlayerSitDownRet
	:public stMsg
{
	stMsgPlayerSitDownRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_PLAYER_SITDOWN ; }
	uint8_t nRet ; // 0 success , 1 coin not engouht , 2 target have player , 3 not in room , 4 you already sit down, 5 diamond error  ;
};

struct stMsgRoomSitDown
	:public stMsg
{
	stMsgRoomSitDown(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_ROOM_SITDOWN ; }
	uint32_t nSitDownPlayerUserUID ;
	uint8_t nIdx ;
	uint32_t nTakeInCoin ;
};

// stand up 
struct stMsgPlayerStandUp
	:public stMsgToRoom
{
	stMsgPlayerStandUp(){ usMsgType = MSG_PLAYER_STANDUP ; }
};

struct stMsgPlayerStandUpRet
	:public stMsg
{
	stMsgPlayerStandUpRet() { cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_PLAYER_STANDUP ; }
	uint8_t nRet ; // 0 success , 1 you are not sit down , 2 other error ; 
};

struct stMsgRoomStandUp
	:public stMsg
{
	stMsgRoomStandUp(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_ROOM_STANDUP ; }
	uint8_t nIdx ;
};



struct stRoomRankEntry
{
	uint32_t nUserUID ;
	int64_t nGameOffset ;
	int64_t nOtherOffset ;  // total buy in when private room ;
};

struct stMsgRequestRoomRankRet
	:public stMsg
{
	stMsgRequestRoomRankRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_REQUEST_ROOM_RANK ; }
	uint8_t nCnt ;
	int16_t nSelfRankIdx ;  // -1 means , you are not in rank , other means you rank idx ;
	PLACE_HOLDER(stRoomRankEntry*);
};

struct stMsgRequestLastTermRoomRank
	:public stMsgToRoom
{
	stMsgRequestLastTermRoomRank(){ usMsgType = MSG_REQUEST_LAST_TERM_ROOM_RANK ;}
};

struct stMsgRequestLastTermRoomRankRet
	:public stMsg
{
	stMsgRequestLastTermRoomRankRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_REQUEST_LAST_TERM_ROOM_RANK ; }
	uint8_t nCnt ;
	PLACE_HOLDER(stRoomRankEntry*);
};


struct stMsgRequestNiuNiuMatchRoomList
	:public stMsg
{
public:
	stMsgRequestNiuNiuMatchRoomList(){ cSysIdentifer = ID_MSG_PORT_NIU_NIU ; usMsgType = MSG_REQUEST_MATCH_ROOM_LIST;}

};

struct stMsgRequestTaxasMatchRoomList
	:public stMsg
{
public:
	stMsgRequestTaxasMatchRoomList(){ cSysIdentifer = ID_MSG_PORT_TAXAS ; usMsgType = MSG_REQUEST_MATCH_ROOM_LIST;}

};

struct stMsgMatchRoomItem 
{
	uint32_t nRoomID ;
	char pRoomName[MAX_LEN_ROOM_NAME];
	uint32_t nBaseBet ; // or small bet ;
	uint32_t nChapionUID ;
	uint32_t nEndTime ;
};

struct stMsgRequestMatchRoomListRet
	:public stMsg
{
	stMsgRequestMatchRoomListRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_REQUEST_MATCH_ROOM_LIST;}
	uint8_t nRoomType ;
	uint8_t nItemCnt ;
	PLACE_HOLDER(stMsgMatchRoomItem*) ;
};

// poker circle 
struct stMsgPublishTopic
	:public stMsg
{
	stMsgPublishTopic(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_CIRCLE_PUBLISH_TOPIC ;}
	uint16_t nContentLen ;
	PLACE_HOLDER(char* jsonFormatContentString);
};

struct stMsgPublishTopicRet
	:public stMsg
{
	stMsgPublishTopicRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_CIRCLE_PUBLISH_TOPIC ; }
	uint8_t nRet ; // 0 success , 1 coin not enough, 2 reach limit , 3 you are not register ;
	uint64_t nTopicID ;
};

struct stMsgDeleteTopic
	:public stMsg
{
	stMsgDeleteTopic(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_CIRCLE_DELETE_TOPIC ; }
	uint64_t nDelTopicID ;
};

struct stMsgRequestTopicList
	:public stMsg
{
	stMsgRequestTopicList(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_CIRCLE_REQUEST_TOPIC_LIST ; }
	uint16_t nPageIdx ;
};

struct stMsgRequestTopicListRet
	:public stMsg
{
	stMsgRequestTopicListRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_CIRCLE_REQUEST_TOPIC_LIST ; }
	uint8_t nRet ; // 0 success , 1 overflow pageIdx ;
	uint16_t nPageIdx ;
	uint16_t nTotalPageCnt ;
	uint64_t vTopicIDs[CIRCLE_TOPIC_CNT_PER_PAGE];
};

struct stMsgRequestTopicDetail
	:public stMsg
{
	stMsgRequestTopicDetail(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_CIRCLE_REQUEST_TOPIC_DETAIL ; }
	uint64_t nTopicID ;
};

struct stMsgRequestTopicDetailRet
	:public stMsg
{
	stMsgRequestTopicDetailRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_CIRCLE_REQUEST_TOPIC_DETAIL ; }
	uint8_t nRet ; // 0 success , 1 can not find topic 
	uint64_t nTopicID ;
	uint32_t nAuthorUID ;
	uint32_t nPublishTime ; // utc time 
	uint16_t nContentLen ;
	PLACE_HOLDER(char* pContent);
};

// robot specail
struct stMsgTellPlayerType
	:public stMsg
{
	stMsgTellPlayerType(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_TELL_PLAYER_TYPE ; }
	uint8_t nPlayerType ; // ePlayerType ;
};

struct stMsgAddTempHalo
	:public stMsgToRoom
{
	stMsgAddTempHalo(){ cSysIdentifer = ID_MSG_PORT_NONE ; usMsgType = MSG_ADD_TEMP_HALO ; }
	uint8_t nTempHalo ; // ePlayerType ;
	uint32_t nTargetUID ;
};

struct stMsgRobotAddMoney
	:public stMsg
{
	stMsgRobotAddMoney()
	{
		cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_ADD_MONEY ;
	}
	int32_t nWantCoin ;
};

struct stMsgRobotAddMoneyRet
	:public stMsg
{
	stMsgRobotAddMoneyRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT; usMsgType = MSG_ADD_MONEY ; }
	uint8_t cRet ; // 0 success ;
	uint64_t nFinalCoin ;
};

struct stMsgRobotModifyRoomRank
	:public stMsgToRoom
{
	stMsgRobotModifyRoomRank(){ cSysIdentifer = ID_MSG_PORT_NONE ; usMsgType = MSG_MODIFY_ROOM_RANK ; }
	uint32_t nTargetUID ;
	int32_t nOffset ;
};

struct stMsgTellRobotIdle
	:public stMsg
{
	stMsgTellRobotIdle(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_TELL_ROBOT_IDLE ; }
	uint32_t nRobotUID ;
	uint8_t nRobotLevel ;
};

struct stMsgTellRobotEnterRoom
	:public stMsg
{
	stMsgTellRobotEnterRoom(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_TELL_ROBOT_ENTER_ROOM ; }
	uint8_t nRoomType ;
	uint32_t nRoomID ;
	uint8_t nSubRoomIdx ;
};

struct stMsgTellRobotLeaveRoom
	:public stMsg
{
	stMsgTellRobotLeaveRoom(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_TELL_ROBOT_LEAVE_ROOM ; }
};

struct stMsgReqRobotTotalGameOffset
	:public stMsg
{
	stMsgReqRobotTotalGameOffset(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_REQ_TOTAL_GAME_OFFSET ; }
};

struct stMsgReqRobotTotalGameOffsetRet
	:public stMsg
{
	stMsgReqRobotTotalGameOffsetRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_REQ_TOTAL_GAME_OFFSET ; }
	int32_t nTotalGameOffset ;
};

struct stMsgReqRobotCurGameOffset
	:public stMsgToRoom
{
	stMsgReqRobotCurGameOffset(){ cSysIdentifer = ID_MSG_PORT_NONE ; usMsgType = MSG_REQ_CUR_GAME_OFFSET ; }
};

struct stMsgReqRobotCurGameOffsetRet
	:public stMsg
{
	stMsgReqRobotCurGameOffsetRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_REQ_CUR_GAME_OFFSET ; }
	int32_t nCurGameOffset ;
};

// apns 
struct stMsgPushAPNSToken
	:public stMsg
{
	stMsgPushAPNSToken(){ cSysIdentifer = ID_MSG_PORT_APNS ; usMsgType = MSG_PUSH_APNS_TOKEN ; }
	uint8_t nReqTokenRet ; // 0 success ; 1 use disabled notification ;
	uint32_t nUserUID ;
	uint32_t nFlag ; 
	char vAPNsToken[32] ;  // must proccesed in client ; change to htonl();  // change to network big endain ;
};

struct stMsgPushAPNSTokenRet
	:public stMsg
{
	stMsgPushAPNSTokenRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_PUSH_APNS_TOKEN ; }
	unsigned char nReqTokenRet ; // 0 success ; 1 use disabled notification ;
};

// encrypt number 
struct stMsgRobotGenerateEncryptNumber
	:public stMsg
{
	stMsgRobotGenerateEncryptNumber(){ cSysIdentifer =  ID_MSG_PORT_DATA ; usMsgType = MSG_ROBOT_GENERATE_ENCRYPT_NUMBER ; }
	uint32_t nCoin ;
	uint16_t nRMB ;
	uint32_t nGenCount ;
	uint8_t nNumberType ;   // 1 new player , 2 newMal ; 
	uint8_t nCoinType ; // 0 diamond , 1 coin 
	uint16_t nChannelID ;  
};

struct  stMsgPlayerUseEncryptNumber
	:public stMsg
{
	stMsgPlayerUseEncryptNumber(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_PLAYER_USE_ENCRYPT_NUMBER ; }
	uint64_t nNumber ;
};

struct  stMsgPlayerUseEncryptNumberRet
	:public stMsg
{
	stMsgPlayerUseEncryptNumberRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_PLAYER_USE_ENCRYPT_NUMBER ; }
	uint8_t nRet ; // 0 success , 1 invalid number , 2 already used , 3 only can use one time ;
	uint8_t nCoinType ;  // 0 diamond , 1 coin ;
	uint32_t nAddCoin ;
	uint32_t nFinalcoin ;
	uint32_t nDiamond ;
};



































































///--------------------ablove is new , below is old------

struct stMsgCreateRole
	:public stMsg
{
	stMsgCreateRole(){ cSysIdentifer = ID_MSG_C2GAME; usMsgType = MSG_CREATE_ROLE ;}
	unsigned int nUserUID ;
	char cName[MAX_LEN_CHARACTER_NAME] ;
	unsigned char nSex ; // eSex ;
	unsigned short nDefaultPhotoID ; 
};

struct stMsgCreateRoleRet
	:public stMsg
{
	stMsgCreateRoleRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_CREATE_ROLE ; }
	unsigned char nRet ; // 0 success , 1 unknown error 
	unsigned int nUserUID ;
};







struct stMsgPlayerEnterGame
	:public stMsg
{
	stMsgPlayerEnterGame(){cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_PLAYER_ENTER_GAME ;}
	unsigned int nUserUID ;
};

struct stMsgShowContinueLoginDlg
	:public stMsg
{
	stMsgShowContinueLoginDlg(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_SHOW_CONTINUE_LOGIN_DLG ; }
	unsigned char nContinueIdx ;
};

struct stMsgGetContinueLoginReward
	:public stMsg
{
	stMsgGetContinueLoginReward(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_GET_CONTINUE_LOGIN_REWARD ; }
	unsigned char cRewardType ; // 0 common user , 1 vip ;
};

struct stMsgGetContinueLoginRewardRet
	:public stMsg
{
	stMsgGetContinueLoginRewardRet(){ cSysIdentifer = ID_MSG_GAME2C; usMsgType = MSG_GET_CONTINUE_LOGIN_REWARD ; }
	unsigned char nRet ; // 0 success , 1 already getted , 2 you are not vip  ,3 arg error , 4 unknown error;
	unsigned char cRewardType ;
	unsigned int nDayIdx ;
	uint64_t nFinalCoin ;
	unsigned int nDiamoned ;
};

struct stMsgPlayerUpdateVipLevel
	:public stMsg
{
	stMsgPlayerUpdateVipLevel(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_PLAYER_UPDATE_VIP_LEVEL ; }
	unsigned char nCurVIPLevel ;
};


struct stMsgPlayerRequestNewMoneyState
	:public stMsg
{
	stMsgPlayerRequestNewMoneyState(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_PLAYER_UPDATE_MONEY ; }
};

struct stMsgPlayerSlotMachine
	:public stMsg
{
	stMsgPlayerSlotMachine(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_PLAYER_SLOT_MACHINE ;}
	uint64_t nBetCoin ;
	unsigned int nBetDiamoned ;
	unsigned cLevel ; // bet money level ;
};

struct stMsgPlayerSlotMachineRet
	:public stMsg
{
	stMsgPlayerSlotMachineRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_PLAYER_SLOT_MACHINE ; }
	unsigned char nRet ; // 0 success , 1 money is not engough ;
	unsigned char vCard[5] ;
	uint64_t nFinalAllCoin ;
	unsigned int nFinalDiamoned ;
	uint64_t nTakeInCoin ;
	unsigned int nTakeInDiamoned ;
};

//struct stMsgPlayerContinueLogin
//	:public stMsg
//{
//	stMsgPlayerContinueLogin(){ cSysIdentifer = ID_MSG_GAME2C; usMsgType = MSG_PLAYER_CONTINUE_LOGIN ; }
//	unsigned short nCurDayIdx ;  // show this git to player ;
//	uint64_t nPlayerCurCoin ;
//	unsigned int nPlayerCurDiamoned ; 
//};

// private room 
struct stMsgPlayerCreatePrivateRoom
	:public stMsg 
{
	stMsgPlayerCreatePrivateRoom(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_PLAYER_CREATE_PRIVATE_ROOM ;}
	char cRoomName[MAX_LEN_ROOM_NAME] ;
	bool bDiamond ;
	uint64_t nBigBinld ; 
	uint64_t nOwnMoneyNeedToEnter ;
	int nPassword ;
};

struct stMsgPlayerCreatePrivateRoomRet
	:public stMsg 
{
	stMsgPlayerCreatePrivateRoomRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_PLAYER_CREATE_PRIVATE_ROOM ; }
	unsigned char nRet ; // 0 success  , 1 you are in other room can not create , 2 do not have the card to create a room , 3, nOwnMoneyNeedToEnter can not big than yours 4 , nBigBlind can not big than nOwnMoneyNeedToEnter ;
	unsigned short nRoomID ;
	int nPassword ;  // max 6 number , 0 means don't use password ;
};

struct stMsgRoomPlayerLeave
	:public stMsg
{
public:
	stMsgRoomPlayerLeave(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_ROOM_PLAYER_LEAVE ;}
	unsigned int nSessionID ; // the leaved player's seessionID ;
};

struct stMsgRoomEnter
	:public stMsg
{
	stMsgRoomEnter(){ cSysIdentifer = ID_MSG_C2GAME; usMsgType = MSG_ROOM_ENTER ; }
	unsigned char nRoomType ;  //eRoomType
	unsigned char nRoomLevel ; //eRoomLevel
	unsigned short nRoomID ;  // used in private room ;
	int nPassword ; // used in private room ; 
};

struct stMsgRoomEnterRet
	:public stMsg 
{
	stMsgRoomEnterRet(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_ROOM_ENTER ; } 
	unsigned char nRet ; // 0 success ; 1 room is full , 2 money is not engough, 3 aready in room ; 4  can not find proper room ; , 5 password error, 6 unknown error  ;
};

struct stMsgPlayerFollowToRoom
	:public stMsg
{
	stMsgPlayerFollowToRoom(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_PLAYER_FOLLOW_TO_ROOM ; }
	unsigned int nTargetPlayerUID ;
};

struct stMsgPlayerFollowToRoomRet
	:public stMsg
{
	stMsgPlayerFollowToRoomRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_PLAYER_FOLLOW_TO_ROOM ; }
	unsigned char nRet ; // 0 success , 1 target player not online , 2 target player not in room , 3 you don't meet that room enter condition , 4 you are not free state , you are already in some room , 5 password room can not to be follow;
};

struct stMsgRoomLeave
	:public stMsg
{
	stMsgRoomLeave(){cSysIdentifer = ID_MSG_C2GAME; usMsgType = MSG_ROOM_LEAVE ; }
};

struct stMsgRoomLeaveRet
	:public stMsg
{
	stMsgRoomLeaveRet(){ cSysIdentifer = ID_MSG_GAME2C; usMsgType = MSG_ROOM_LEAVE ; }
	unsigned char nRet ; // 0 success ; 1 error ;
};


// room common action 
struct stMsgRoomPlayerSpeak
	:public stMsg
{
	stMsgRoomPlayerSpeak(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_ROOM_SPEAK ; }
	unsigned char nContentType;  // 0 system text , 1 system face , 2 player inputer , 3 interactvie face ,4 send gift in the desk;
	unsigned short nSystemChatID ;  // system text or system face config ID ; configfile in client ;
	unsigned short nContentLen ;  // player input content len ;
	char* pContent;
	unsigned char nInteraciveWithPeerRoomIdx ; // interactvie face  item target ;
};

struct stMsgRoomPlayerSpeakRet
	: public stMsg
{
	stMsgRoomPlayerSpeakRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_ROOM_SPEAK ; }
	unsigned char nRet ; // 0 success ,1 item not enough , 2 target player is NULL ;
};

struct stMsgRoomOtherPlayerSpeak
	:public stMsg
{
	stMsgRoomOtherPlayerSpeak(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_ROOM_OTHER_SPEAK ; }
	unsigned char nSpeakerRoomIdx ;
	unsigned char nSpeakSex ; // eSex ;
	char pSpeakName[MAX_LEN_CHARACTER_NAME] ;
	unsigned char nContentType;  // 0 system text , 1 system face , 2 player inputer ;3 interactvie face ;
	unsigned short nSystemChatID ;  // system text or system face config ID ; configfile in client ;
	unsigned short nContentLen ;  // player input content len ;
	char* pContent;
	unsigned char nInteraciveWithPeerRoomIdx ; // interactvie face  item target ;
};

struct stMsgRoomRequestPeerDetail
	:public stMsg
{
	stMsgRoomRequestPeerDetail(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_ROOM_REQUEST_PEER_DETAIL ; }
	unsigned int nPeerSessionID ;
};

struct stMsgRoomRequestPeerDetailRet
	:public stMsg
{
	stMsgRoomRequestPeerDetailRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_ROOM_REQUEST_PEER_DETAIL ; }
	unsigned char nRet ; // 0 sucess , 1 peer not exsit ;
	unsigned int nPeerSessionID ; ;
	stPlayerDetailData stDetailInfo ;
};

struct stMsgKickPeer
	:public stMsg
{
	stMsgKickPeer(){ cSysIdentifer = ID_MSG_C2GAME; usMsgType = MSG_ROOM_KICK_PEER ;}
	unsigned char nIdxToBeKick ;
};

struct stMsgKickPeerRet
	:public stMsg
{
	stMsgKickPeerRet(){ cSysIdentifer = ID_MSG_GAME2C; usMsgType = MSG_ROOM_KICK_PEER ;}
	unsigned char nRet ;   // 0 success , 1 target not exsit ,2, kick card not enghout , 3 vip higher than you can not kick , 4 target can not be self;
};

struct stMsgKickOtherPeerKickPeer
	:public stMsg
{
	stMsgKickOtherPeerKickPeer(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_ROOM_OTHER_KICK_PEER ;}
	unsigned char nIdxWhoKick ;
	unsigned char nIdxWhoBeKicked ;
};

struct stMsgExeBeKicked
	:public stMsg
{
	stMsgExeBeKicked(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_ROOM_EXE_BE_KICKED ; }
	unsigned int nRoomIdxBeKicked ;
	char cNameKicer[MAX_LEN_CHARACTER_NAME] ;
};

//
//struct stMsgKickPeerFinalResult
//	:public stMsg
//{
//	stMsgKickPeerFinalResult(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_ROOM_PROCESSE_KIKED_RESULT ;}
//	unsigned char nCount ;
//	stKicPlayer* vBeKickedPeers ;
//};

// invite friend to join room 
struct stMsgPlayerInviteFriendToJoinRoom
	:public stMsg
{
	stMsgPlayerInviteFriendToJoinRoom(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_PlAYER_INVITED_FRIEND_TO_JOIN_ROOM ;}
	unsigned int nPlayerUID ;
	unsigned char nRoomIdx ;
};

struct stMsgPlayerInviteFriendToJoinRoomRet
	:public stMsg
{
	stMsgPlayerInviteFriendToJoinRoomRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_PlAYER_INVITED_FRIEND_TO_JOIN_ROOM ; }
	unsigned char nRet ; // 0 means success ; 1 pos idx not empty , 2 target player offline , 3 target player not meet room enter condtion , 4 target player not free , maybe aready in some room;
};

struct stMsgPlayerBeInvitedToJoinRoom
	:public stMsg
{
	stMsgPlayerBeInvitedToJoinRoom(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_PLAYER_BE_INVITED ; }
	char pNameWhoInviteMe[MAX_LEN_CHARACTER_NAME] ;
	unsigned int nUserUIDWhoInviteMe ;
	unsigned char nRoomType ;
	unsigned char nRoomLevel ;
	unsigned short nRoomID ;
	unsigned char nSitIdx ;
};

struct stMsgPlayerReplayBeInvitedToJoinRoom
	:public stMsg
{
	stMsgPlayerReplayBeInvitedToJoinRoom(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_PLAYER_REPLAY_BE_INVITED ; }
	unsigned int nReplyToUserUID ; // replay to the one who invited me ;
	unsigned char nReplyResult ; // 0 ok to join , 1 refuse ;
	unsigned char nRoomType ;
	unsigned char nRoomLevel ;
	unsigned short nRoomID ;
	unsigned char nSitIdx ;
};

//struct stMsgPlayerRecievedInviteReply
//	:public stMsg
//{
//	stMsgPlayerRecievedInviteReply(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_PLAYER_RECIEVED_INVITED_REPLAY ; }
//	char nReplyerName[MAX_LEN_CHARACTER_NAME]; // the player i just invited ,before ;
//	unsigned char nRet ; // 0 agree , 1 refused , 2 busy , 3 room condition not meet , 4 room is full 
//};

struct stMsgPlayerReplayBeInvitedToJoinRoomRet
	:public stMsg 
{
	stMsgPlayerReplayBeInvitedToJoinRoomRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_PLAYER_REPLAY_BE_INVITED ; }
	unsigned char nRet ; // 0 succcess ;  1 already in room ; 2 room is full , 3 not meet room condition ;
};

// mail ;
struct stMailGiftContent
{
	stPlayerBrifData stPresenter ; // who present me ;
	unsigned short nItemID ;
	unsigned short nShopItemID ;
	unsigned int nCount ;
	unsigned int nPrsentDiamond ;
	uint64_t nPresentCoin ;
};

struct stMailUnprocessedPurchaseVerifyContent
{
	unsigned int nTaregetForPlayerUID ;
	unsigned short nShopItemID ;
	unsigned int nCount ;
	bool bVerifyOK ;
};

struct stMailBeAddedFriend
{
	stPlayerBrifData peerWhoWantAddMe ;
};




struct stMsgPlayerProcessedMail
	:public stMsg
{
	stMsgPlayerProcessedMail(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_PLAYER_PROCESSED_MAIL ;}
	uint64_t nMailUIDProcessed ;
	unsigned char eProcessAct ; // eProcessMailAct
};

struct stMsgPlayerProcessedMailRet
	:public stMsg
{
	stMsgPlayerProcessedMailRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_PLAYER_PROCESSED_MAIL ;}
	unsigned char nRet ; // 0 success , 1 not find mail , 2 already processed !
	uint64_t nMailUIDProcessed ;
	unsigned char eProcessAct ; // eProcessMailAct
};

// firend
struct stFriendBrifData
{
	stPlayerBrifData tBrifData ;
	unsigned int nPresentCoinTimes ;
};



struct stMsgPlayerSearchPeer
	:public stMsg
{
	stMsgPlayerSearchPeer(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_PLAYER_SERACH_PEERS ; }
	char* pSearContent ;
	unsigned char nSearchContentLen ;
};

struct stMsgPlayerSearchPeerRet
	:public stMsg
{
	stMsgPlayerSearchPeerRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_PLAYER_SERACH_PEERS ; }
	unsigned char nRetCount ;
	stPlayerBrifData* pPeersInfo ;
};

////struct stMsgPlayerRequestFriendDetail
////	:public stMsg
////{
////	stMsgPlayerRequestFriendDetail(){ cSysIdentifer = ID_MSG_C2GAME; usMsgType = MSG_PLAYER_REQUEST_FRIEND_DETAIL ; }
////	unsigned int nFriendUserUID ;
////};

//struct stFriendDetail
//	:public stMsgCommonPeerDetail
//{
//	uint64_t nMaxSingleWin ;
//	unsigned char vMaxCard[5] ;
//};

////struct stMsgPlayerRequestFriendDetailRet
////	:public stMsg
////{
////	stMsgPlayerRequestFriendDetailRet(){ cSysIdentifer = ID_MSG_GAME2C; usMsgType = MSG_PLAYER_REQUEST_FRIEND_DETAIL ; }
////	unsigned char nRet ; // 0 success , 1 can not find ;
////	stPlayerDetailData stPeerInfo ;
////};
////
////struct stMsgPlayerRequestSearchedPeerDetail
////	:public stMsg
////{
////	stMsgPlayerRequestSearchedPeerDetail(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_PLAYER_REQUEST_SEARCH_PEER_DETAIL ; }
////	unsigned int nPeerUserUID ;
////};
////
////struct stMsgPlayerRequestSearchedPeerDetailRet
////	:public stMsg
////{
////	stMsgPlayerRequestSearchedPeerDetailRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_PLAYER_REQUEST_SEARCH_PEER_DETAIL ; }
////	unsigned char nRet ; // 0 success , 1 can not find ;
////	stPlayerDetailData stPeerInfo ;
////};

// item 
struct stPlayerItem
{
	unsigned short nItemID ;
	unsigned int nCount ;
};

struct stMsgPlayerRequestItemList
	:public stMsg
{
	stMsgPlayerRequestItemList(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_REQUEST_ITEM_LIST ; }
};

struct stMsgPlayerRequestItemListRet
	:public stMsg
{
	stMsgPlayerRequestItemListRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_REQUEST_ITEM_LIST ; }
	unsigned short nPlayerItemCount ;
	stPlayerItem* pPlayerItem ;
};

struct stMsgPlayerUseGigtRet
	:public stMsg
{
public:
	stMsgPlayerUseGigtRet(){ cSysIdentifer = ID_MSG_GAME2C; usMsgType = MSG_PLAYER_USE_GIFT ;}
	unsigned char nRet ; // 0 success , 1 error ;
	unsigned short nGiftID ;
	uint64_t nGetCoin ;
	unsigned int nGetDiamoned ;
	unsigned short nGetItemCount ;
	stPlayerItem* pItems ;
};

// dian dang
struct stMsgPlayerPawnAsset   
	:public stMsg
{
	stMsgPlayerPawnAsset(){ cSysIdentifer = ID_MSG_C2GAME; usMsgType = MSG_PLAYER_PAWN_ASSERT ; }
	unsigned short nAssetItemID ;
	unsigned short nCount ;
};

struct stMsgPlayerPawnAssetRet
	:public stMsg
{
	stMsgPlayerPawnAssetRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_PLAYER_PAWN_ASSERT ; }
	unsigned char nRet ; // 0 success ; 1 . you don't have enough assert , 2 unknow assert item;
	unsigned int short nAssertItemID ;
	unsigned short nCount ;
	unsigned int nFinalDiamond ;  
	uint64_t nFinalCoin ; 
};

// rank 
struct stMsgPlayerRequestRank
	:public stMsg
{
	stMsgPlayerRequestRank(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_REQUEST_RANK ; }
	unsigned char nRankType ;
	unsigned char nFromIdx ;
	unsigned char nCount ;
};

struct stRankPeerBrifInfo
{
	stPlayerBrifData tBrifData ;
	uint64_t nYesterDayWin ;
	uint64_t nSingleWinMost ;
	unsigned short nRankIdx ;
};

struct stMsgPlayerRequestRankRet
	:public stMsg
{
	stMsgPlayerRequestRankRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_REQUEST_RANK ; }
	unsigned char nRet ; // 0 means success ; 1 unknown rank type ;
	unsigned char nRankType ;
	unsigned char nCount ;
	stRankPeerBrifInfo* peers;
};

struct stMsgPlayerRequestRankPeerDetail
	:public stMsg
{
	stMsgPlayerRequestRankPeerDetail(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_REQUEST_RANK_PEER_DETAIL ; }
	unsigned int nRankPeerUID ;
	unsigned char nRankType ; // eRankType 
};
 
struct stMsgPlayerRequestRankPeerDetailRet
	:public stMsg
{
	stMsgPlayerRequestRankPeerDetailRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_REQUEST_RANK_PEER_DETAIL ; }
	unsigned char nRet ; // 0 means success ; 1 player not in the ranker ,please refresh the rank ;
	stPlayerDetailData stDetailInfo ;
};

// inform && brocast 
struct stMsgInformNewNotices
	:public stMsg
{
	stMsgInformNewNotices(){ cSysIdentifer = ID_MSG_GAME2C; usMsgType = MSG_INFORM_NEW_NOTICES ; }
	unsigned int cNewNoticeCount ;
};
struct stInform
{
	unsigned char nTitleLen ;
	char* pTitle ;
	unsigned short nContentLen ;
	char* pContent ;
};

struct stMsgRequestNewNotice
	:public stMsg
{
	stMsgRequestNewNotice(){ cSysIdentifer = ID_MSG_C2GAME; usMsgType = MSG_PLAYER_REQUEST_NOTICE ; }
};

struct stMsgRequestNewNoticeRet
	:public stMsg
{
	stMsgRequestNewNoticeRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_PLAYER_REQUEST_NOTICE ; }
	unsigned char nInformCount ;
	stInform* pInforms ;
};

// brocast message below ;

struct stMsgGlobalBracast
	:public stMsg
{
	stMsgGlobalBracast(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_GLOBAL_BROCAST ; }
	unsigned char nBrocastType ; // 0 popview system message, 1 system message ,2 player say brocast message , 3++ some event to broacst ;
};

// nBrocastType 0 data struct ;
struct stMsgBrocastPopviewSystem
	:public stMsgGlobalBracast
{
	stMsgBrocastPopviewSystem(){ nBrocastType = 0 ;}
	unsigned char pTitleLen ;
	char* pTitle ;
	unsigned short nContentLen ;
	char* pContent;
};

// 1 system message struct ;
struct stMsgBrocastSystem
	:public stMsgGlobalBracast
{
	stMsgBrocastSystem(){ nBrocastType = 1 ;}
	unsigned short nContentLen ;
	char* pContent ;
};

// 2 player say brocast message ;
struct stMsgBrocastPlayerSay
	:public stMsgGlobalBracast
{
	stMsgBrocastPlayerSay(){ nBrocastType = 2 ;}
	unsigned char nPlayerNameLen ;
	char* pName;
	unsigned short nContentLen ;
	char* pContent ;
};

struct stMsgPlayerSayBrocast
	:public stMsg
{
	stMsgPlayerSayBrocast(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_PLAYER_SAY_BROCAST ; }
	unsigned short nContentLen ;
	char* pContent ;
};

struct stMsgPlayerSayBrocastRet
	:public stMsg
{
	stMsgPlayerSayBrocastRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_PLAYER_SAY_BROCAST ; }
	unsigned char nRet ; // 0 success , 1 have no item ; 
};

// shop
struct stMsgPlayerRequestShopList
	:public stMsg 
{
public:
	stMsgPlayerRequestShopList(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_PLAYER_REQUEST_SHOP_LIST ; }
};

struct stMsgPlayerRequestShopListRet
	:public stMsg
{
public: 
	stMsgPlayerRequestShopListRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_PLAYER_REQUEST_SHOP_LIST ; }
	unsigned short nShopItemCount ;
	unsigned short* ShopItemIDs;
};



struct stMsgPlayerRecievedShopItemGift
	:public stMsg
{
	stMsgPlayerRecievedShopItemGift(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_PLAYER_RECIEVED_SHOP_ITEM_GIFT ; }
	unsigned int nPresenterUID ;
	char pPresenterName[MAX_LEN_CHARACTER_NAME] ;
	unsigned short nShopItemID ;
	unsigned int nShopItemCount ;
	uint64_t nFinalCoin ;
	unsigned int nFinalDiamond ;
};

// mission 
struct stMissionSate
{
	unsigned short nMissionID ;
	unsigned short nProcessCnt ;
	bool bFinish ;
	bool bGetedReward ;
};

struct stMsgPlayerRequestMissionList
	:public stMsg
{
	stMsgPlayerRequestMissionList(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_PLAYER_REQUEST_MISSION_LIST ; }
};

struct stMsgPlayerRequestMissionListRet
	:public stMsg
{
	stMsgPlayerRequestMissionListRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_PLAYER_REQUEST_MISSION_LIST ; }
	unsigned short nMissionCount ;
	stMissionSate* pList ;
};

struct stMsgPlayerNewMissionFinished
	:public stMsg
{
	stMsgPlayerNewMissionFinished(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_PLAYER_NEW_MISSION_FINISHED ; }
	unsigned nMissionID ;
};

struct stMsgPlayerRequestMissionReward
	:public stMsg
{
	stMsgPlayerRequestMissionReward(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_PLAYER_REQUEST_MISSION_REWORD ; }
	unsigned nMissionID ;
};

struct stMsgPlayerRequestMissionRewardRet
	:public stMsg
{
	stMsgPlayerRequestMissionRewardRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_PLAYER_REQUEST_MISSION_REWORD ; }
	unsigned char nRet ; // 0 success , 1 mission not finish , 2 already given reward , 3 don't exsit mission 
	unsigned short nMissionID ;
	uint64_t nFinalCoin ;
	unsigned int nDiamoned ;
};

// online box 
struct stMsgPlayerRequestOnlineBoxRewoard
	:public stMsg
{
	stMsgPlayerRequestOnlineBoxRewoard(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_PLAYER_REQUEST_ONLINE_BOX_REWARD ; }
	unsigned short nBoxID ;
};

struct stMsgPlayerRequestOnlineBoxRewoardRet
	:public stMsg
{
	stMsgPlayerRequestOnlineBoxRewoardRet(){ cSysIdentifer = ID_MSG_GAME2C; usMsgType = MSG_PLAYER_REQUEST_ONLINE_BOX_REWARD ; }
	unsigned nRet ; // 0 success , 1 time not reached , 2 nBoxID error ;
	unsigned short nFinishedBoxID ;
	uint64_t nFinalCoin ;
	unsigned int nFinalDiamoned ;
};

struct stMsgPlayerRequestOnlineBoxState
	:public stMsg
{
	stMsgPlayerRequestOnlineBoxState(){ cSysIdentifer = ID_MSG_C2GAME; usMsgType = MSG_PLAYER_REQUEST_ONLINE_BOX_STATE ; }
	unsigned short nBoxID ;
};

struct stMsgPlayerRequestOnlineBoxStateRet
	:public stMsg
{
	stMsgPlayerRequestOnlineBoxStateRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_PLAYER_REQUEST_ONLINE_BOX_STATE ; }
	unsigned char nRet ; // 0 success , 1 nBoxID not ok ;
	unsigned short nBoxID ;
	unsigned int nBoxRunedTime ; // by second ;
};

// robot message 
//struct stMsgRobotAddMoney
//	:public stMsg
//{
//	stMsgRobotAddMoney()
//	{
//		cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_ROBOT_ADD_MONEY ;
//	}
//	int nWantCoin ;
//};
//
//struct stMsgRobotAddMoneyRet
//	:public stMsg
//{
//	stMsgRobotAddMoneyRet(){ cSysIdentifer = ID_MSG_GAME2C; usMsgType = MSG_ROBOT_ADD_MONEY ; }
//	unsigned char cRet ; // 0 success ;
//	uint64_t nFinalCoin ;
//};

struct stMsgRobotOrderToEnterRoom
	:public stMsg
{
	stMsgRobotOrderToEnterRoom(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_ROBOT_ORDER_TO_ENTER_ROOM ; }
	unsigned char nRoomType ;
	unsigned char cLevel ;
	unsigned short nRoomID ;
};

struct stMsgRobotApplyToLevelRoom
	:public stMsg
{
	stMsgRobotApplyToLevelRoom(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_ROBOT_APPLY_TO_LEAVE ; }
};

struct stMsgRobotApplyToLeaveRoomRet
	:public stMsg
{
	stMsgRobotApplyToLeaveRoomRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_ROBOT_APPLY_TO_LEAVE ; }
	unsigned char nRet ; // 0 success , 1 error ;
};

struct stMsgRobotCheckBiggest
	:public stMsg 
{
	stMsgRobotCheckBiggest(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_ROBOT_CHECK_BIGGIEST ; }
};

struct stMsgRobotCheckBiggestRet
	:public stMsg
{
	stMsgRobotCheckBiggestRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_ROBOT_CHECK_BIGGIEST ; }
	unsigned char nRet ; // 1 you are biggest , 0 you are not ;
};

struct stMsgRobotInformIdle
	:public stMsg
{
	stMsgRobotInformIdle(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_ROBOT_INFORM_IDLE ; }
};


// end 
#pragma pack(pop)