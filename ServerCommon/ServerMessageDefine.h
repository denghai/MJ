#pragma once
#pragma pack(push)
#pragma pack(1)
// Define message , used between Servers ;mainly DBServer and GameServer 
#pragma warning(disable:4200)
#include "MessageDefine.h"
#include "ServerCommon.h"
#include "TaxasMessageDefine.h"
#include "ServerDefine.h"
#define DBServer_PORT 5001
// WARNNING:变长字符串，我们不包括终结符 \0 ;

struct stMsgGateServerInfo
	:public stMsg
{
	stMsgGateServerInfo(){ cSysIdentifer = ID_MSG_PORT_GATE; usMsgType = MSG_GATESERVER_INFO;  }
	bool	 bIsGateFull; // if gate is full can not set up , center svr say , can not set up more gate server , if want add more pls modify serverConfig.txt
	uint16_t  uIdx ;
	uint16_t  uMaxGateSvrCount ;
};

struct stMsgClientDisconnect
	:public stMsg
{
public:
	stMsgClientDisconnect(){ cSysIdentifer = ID_MSG_PORT_CENTER ;  usMsgType = MSG_DISCONNECT_CLIENT ; }
	uint32_t nSeesionID ;
	//bool bIsForClientReconnect ; 
};

struct stMsgNewClientConnected
	:public stMsg
{
	stMsgNewClientConnected(){ cSysIdentifer = ID_MSG_PORT_CENTER ; usMsgType = MSG_CONNECT_NEW_CLIENT; }
	uint32_t nNewSessionID ;
};

struct stMsgRequestDBCreatePlayerData
	:public stMsg
{
	stMsgRequestDBCreatePlayerData(){ cSysIdentifer = ID_MSG_PORT_DB; usMsgType = MSG_REQUEST_CREATE_PLAYER_DATA ; }
	uint32_t nUserUID ;
	uint8_t isRegister ;
};

struct stMsgOnPlayerBindAccount
	:public stMsg
{
	stMsgOnPlayerBindAccount(){ cSysIdentifer = ID_MSG_PORT_DATA; usMsgType = MSG_ON_PLAYER_BIND_ACCOUNT ; }
};

struct stMsgOnPlayerLogin
	:public stMsg
{
	stMsgOnPlayerLogin(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_PLAYER_LOGIN ; }
	uint32_t nUserUID ;
};

struct stMsgGetMaxRoomID
	:public stMsg
{
	stMsgGetMaxRoomID(){ cSysIdentifer = ID_MSG_PORT_DB ; usMsgType = MSG_GET_MAX_ROOM_ID; }
};

struct stMsgGetMaxRoomIDRet
	:public stMsg
{
	stMsgGetMaxRoomIDRet(){ cSysIdentifer = ID_MSG_PORT_NONE ; usMsgType = MSG_GET_MAX_ROOM_ID; }
	uint32_t nMaxRoomID ; 
};

struct stMsgAsyncRequest
	:public stMsg
{
	stMsgAsyncRequest(){ cSysIdentifer = ID_MSG_PORT_NONE ; usMsgType = MSG_ASYNC_REQUEST ;}
	uint16_t nReqType ;
	uint32_t nReqSerailID ;
	uint16_t nReqContentLen ;
	PLACE_HOLDER(char* jsReqContentLen);
};

struct stMsgAsyncRequestRet
	:public stMsg
{
	stMsgAsyncRequestRet(){ cSysIdentifer = ID_MSG_PORT_NONE ; usMsgType = MSG_ASYNC_REQUEST_RESULT ;}
	uint32_t nReqSerailID ;
	uint16_t nResultContentLen ;
	PLACE_HOLDER(char* jsResultContentLen);
};

struct stMsgSyncClientNetState
	:public stMsg
{
	stMsgSyncClientNetState(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_CLIENT_NET_STATE ; }
	uint8_t nState ; // 0 waiting for reconnected , 1 reconnected success ;
};

// sysn private room data 
struct stMsgSyncPrivateRoomResult
	:public stMsg
{
	stMsgSyncPrivateRoomResult(){ cSysIdentifer = ID_MSG_PORT_DATA ;  usMsgType = MSG_SYNC_PRIVATE_ROOM_RESULT ; }
	uint32_t nTargetPlayerUID ;
	uint32_t nRoomID ;
	uint32_t nFinalCoin ;
	uint32_t nBuyIn ;
	int32_t nOffset ;
	uint32_t nDuringTimeSeconds ;
	uint32_t nCreatorUID ;
	uint32_t nBaseBet ;
	uint32_t nClubID ;
	uint32_t nSiealNum;
	char cRoomName[MAX_LEN_ROOM_NAME];
};

struct stMsgSaveGameResult
	:public stMsg
{
public:
	stMsgSaveGameResult(){ usMsgType = MSG_SAVE_GAME_RESULT ; cSysIdentifer = ID_MSG_PORT_DB ; }
	uint32_t nRoomID ;
	uint32_t nCreaterUID ;
	uint16_t nConfigID ;
	uint32_t tTime ;
	uint32_t nDuringSeconds ;
	uint8_t nRoomType ;
	uint32_t nSieralNum;
	uint16_t nJsLen ;
	PLACE_HOLDER(char* pJson);// [ { uid : 23 , buyIn : 234, offset : -30},{ uid : 23 , buyIn : 234, offset : -30}, .... ]
};

struct stMsgReadGameResult
	:public stMsg
{
	stMsgReadGameResult(){ usMsgType = MSG_READ_GAME_RESULT ; cSysIdentifer = ID_MSG_PORT_DB ; }
	uint8_t nRoomType ;
};

struct stMsgReadGameResultRet
	:public stMsg
{
	stMsgReadGameResultRet(){ usMsgType = MSG_READ_GAME_RESULT ; cSysIdentifer = ID_MSG_PORT_NONE ; }
	bool isFinal ;

	uint32_t nRoomID ;
	uint32_t nCreaterUID ;
	uint16_t nConfigID ;
	uint32_t tTime ;
	uint8_t nRoomType ;
	uint32_t nDuringSeconds ;
	uint32_t nSieralNum;
	uint16_t nJsLen ;
	PLACE_HOLDER(char* pJson);// [ { uid : 23 , buyIn : 234, offset : -30},{ uid : 23 , buyIn : 234, offset : -30}, .... ]
};

// save player recorder 
struct stMsgSavePlayerGameRecorder
	:public stMsg
{
	stMsgSavePlayerGameRecorder(){ usMsgType = MSG_SAVE_PLAYER_GAME_RECORDER ; cSysIdentifer = ID_MSG_PORT_DB ; }
	uint32_t nRoomID ;
	uint8_t nRoomType ;
	uint32_t nCreateUID ;
	uint32_t nFinishTime ;
	uint32_t nDuiringSeconds ;
	int32_t nOffset ;
	uint32_t nUserUID ;
	uint32_t nBuyIn ;
	uint32_t nBaseBet ;
	uint32_t nClubID ;
	uint32_t nSieralNum;
	char cRoomName[MAX_LEN_ROOM_NAME];
};

struct stMsgReadPlayerGameRecorder
	:public stMsg
{
	stMsgReadPlayerGameRecorder(){ cSysIdentifer = ID_MSG_PORT_DB ; usMsgType = MSG_READ_PLAYER_GAME_RECORDER ; }
	uint32_t nUserUID ;
};

struct stMsgReadPlayerGameRecorderRet 
	:public stMsg
{
	stMsgReadPlayerGameRecorderRet() { cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_READ_PLAYER_GAME_RECORDER ; }
	bool isFinal ;
	uint32_t nRoomID ;
	uint32_t nCreateUID ;
	uint32_t nFinishTime ;
	uint32_t nDuiringSeconds ;
	int32_t nOffset ;
	uint32_t nBuyIn ;
	uint32_t nBaseBet ;
	uint32_t nClubID ;
	uint32_t nSeiralNum;
	char cRoomName[MAX_LEN_ROOM_NAME];
};


// game and db 
struct stMsgDataServerGetBaseData
	:public stMsg
{
	stMsgDataServerGetBaseData(){cSysIdentifer = ID_MSG_PORT_DB ; usMsgType = MSG_READ_PLAYER_BASE_DATA ; }
	unsigned int nUserUID ;
};

struct stMsgDataServerGetBaseDataRet
	:public stMsg
{
	stMsgDataServerGetBaseDataRet(){cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_READ_PLAYER_BASE_DATA ; }
	unsigned char nRet ; // 0 success , 1 not exsit  ;
	stServerBaseData stBaseData ;
};


struct stSaveRoomPlayerEntry
{
	uint32_t nUserUID ;
	int32_t nGameOffset ;
	int32_t nOtherOffset ;
};

struct stMsgSaveRoomPlayer
	:public stMsg
{
	stMsgSaveRoomPlayer(){ cSysIdentifer = ID_MSG_PORT_DB ; usMsgType = MSG_SAVE_ROOM_PLAYER ; }
	uint32_t nRoomSerialNum;
	uint32_t nTermNumber ;
	stSaveRoomPlayerEntry savePlayer ;
};

struct  stMsgReadRoomPlayer 
	:public stMsg
{
	stMsgReadRoomPlayer(){ cSysIdentifer = ID_MSG_PORT_DB ; usMsgType = MSG_READ_ROOM_PLAYER ;}
	uint32_t nRoomSerialNum;
	uint32_t nRoomID;
	uint32_t nTermNumber ;
};

struct stMsgReadRoomPlayerRet 
	:public stMsgToRoom
{
	stMsgReadRoomPlayerRet(){ cSysIdentifer = ID_MSG_PORT_NONE ; usMsgType = MSG_READ_ROOM_PLAYER ; }
	uint8_t nCnt ;
	uint32_t nTermNumber ;
	bool bIsLast ;
	PLACE_HOLDER(stSaveRoomPlayerEntry*);
};

// private room player data ;
struct stMsgSavePrivateRoomPlayer
	:public stMsg
{
	stMsgSavePrivateRoomPlayer(){ cSysIdentifer = ID_MSG_PORT_DB ; usMsgType = MSG_SAVE_PRIVATE_ROOM_PLAYER ;}
	uint32_t nRoomSerialNum;
	uint32_t nUserUID ;
	uint16_t nJsonLen ;
	PLACE_HOLDER(char* pJs) ; // stPrivateRoomPlayerItem  joson object ;
};

struct stMsgReadPrivateRoomPlayer
	:public stMsg
{
	stMsgReadPrivateRoomPlayer(){ cSysIdentifer = ID_MSG_PORT_DB ; usMsgType = MSG_READ_PRIVATE_ROOM_PLAYER ; }
	uint32_t nRoomID;
	uint32_t nRoomSerialNum ;
};

struct stMsgReadPrivateRoomPlayerRet
	:public stMsgToRoom
{
	stMsgReadPrivateRoomPlayerRet(){ cSysIdentifer = ID_MSG_PORT_NONE ; usMsgType = MSG_READ_PRIVATE_ROOM_PLAYER ; }
	uint16_t nJsonLen ;
	PLACE_HOLDER(char* pJs) ; // stPrivateRoomPlayerItem  joson object ;
};

struct stMsgRemoveRoomPlayer
	:public stMsg
{
	stMsgRemoveRoomPlayer(){ cSysIdentifer = ID_MSG_PORT_DB ; usMsgType = MSG_REMOVE_ROOM_PLAYER ;}
	uint8_t nRoomType ;
	uint32_t nRoomID ;
};

// save exchanges 
struct stMsgSaveExchanges
	:public stMsg
{
	 stMsgSaveExchanges(){ cSysIdentifer = ID_MSG_PORT_DB; usMsgType = MSG_SAVE_EXCHANGE ; }
	 uint16_t nExchangeID ;
	 uint32_t nCount ;
};

struct stMsgReadExchanges
	:public stMsg
{
	stMsgReadExchanges(){ cSysIdentifer = ID_MSG_PORT_DB ; usMsgType = MSG_READ_EXCHANGE ;}
};

struct stMsgReadExchangesRet 
	:public stMsg
{
	stMsgReadExchangesRet(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_READ_EXCHANGE ; }
	uint16_t nCnt ;
	PLACE_HOLDER(stExchangeItem* pItems);
};
// save player data ;
struct stMsgSavePlayerInfo   
	:public stMsg
{
	stMsgSavePlayerInfo(){ cSysIdentifer = ID_MSG_PORT_DB; usMsgType = MSG_PLAYER_SAVE_PLAYER_INFO ; }
	uint32_t nUserUID ;
	char vName[MAX_LEN_CHARACTER_NAME] ;
	char vSigure[MAX_LEN_SIGURE] ;
	uint8_t vUploadedPic[MAX_UPLOAD_PIC] ;
	uint16_t nPhotoID ;
	uint8_t nIsRegister ;
	uint32_t nInviterUID ; 
	uint8_t nSex ;
};

struct stMsgSavePlayerMoney
	:public stMsg
{
	stMsgSavePlayerMoney(){cSysIdentifer = ID_MSG_PORT_DB; usMsgType = MSG_SAVE_PLAYER_MONEY ; }
	uint32_t nUserUID ;
	uint64_t nCoin ;
	uint32_t nDiamoned ;
	uint32_t nCupCnt ;
};

struct stMsgReadPlayerTaxasData
	:public stMsg
{
	stMsgReadPlayerTaxasData(){ cSysIdentifer = ID_MSG_PORT_DB ; usMsgType = MSG_READ_PLAYER_TAXAS_DATA ; }
	uint32_t nUserUID ;
};

struct stMsgReadPlayerTaxasDataRet
	:public stMsg
{
	stMsgReadPlayerTaxasDataRet(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_READ_PLAYER_TAXAS_DATA ; }
	uint8_t nRet ;
	uint32_t nUserUID ;
	stPlayerGameData tData ;
};


struct stMsgSavePlayerGameData
	:public stMsg
{
	stMsgSavePlayerGameData(){cSysIdentifer = ID_MSG_PORT_DB; usMsgType = MSG_SAVE_PLAYER_GAME_DATA ; }
	uint32_t nUserUID ;
	uint16_t nJsonLen ;
	uint8_t nGameType ;
	PLACE_HOLDER(char* pJsonStr); // { gameType : eRoom_NiuNiu, data : { nWinTimes : 23 , nPlayTimes : 23, nSingleWinMost : 23400, nChampionTimes : 23 ,nRun_upTimes : 2, nThird_placeTimes : 23, vMaxCards : [23,455,345,634] } }
};

struct stMsgReadPlayerGameData
	:public stMsg
{
	stMsgReadPlayerGameData(){ cSysIdentifer = ID_MSG_PORT_DB ; usMsgType = MSG_READ_PLAYER_GAME_DATA ; }
	uint32_t nUserUID ;
};

struct stMsgReadPlayerGameDataRet
	:public stMsg
{
	stMsgReadPlayerGameDataRet(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_READ_PLAYER_GAME_DATA ; }
	uint8_t nRet ;
	uint32_t nUserUID ;
	uint16_t nJsonLen ;
	PLACE_HOLDER(char* pJsonStr); // { [ { gameType : eRoom_NiuNiu, data : { nWinTimes : 23 , nPlayTimes : 23, nSingleWinMost : 23400, nChampionTimes : 23 ,nRun_upTimes : 2, nThird_placeTimes : 23, vMaxCards : [23,455,345,634] } } , ... ]  } 
};

//struct stMsgSaveCreateTaxasRoomInfo
//	:public stMsg
//{
//	stMsgSaveCreateTaxasRoomInfo(){ cSysIdentifer = ID_MSG_PORT_DB; usMsgType = MSG_SAVE_CREATE_TAXAS_ROOM_INFO; }
//	uint32_t nRoomID ;
//	uint16_t nConfigID ;
//	uint32_t nRoomOwnerUID ;
//	uint32_t nCreateTime ;
//	uint64_t nChatRoomID ;
//};
//
//struct stMsgSaveUpdateTaxasRoomInfo
//	:public stMsg
//{
//	stMsgSaveUpdateTaxasRoomInfo(){ cSysIdentifer = ID_MSG_PORT_DB ; usMsgType = MSG_SAVE_UPDATE_TAXAS_ROOM_INFO ;}
//	uint32_t nRoomID ;
//	uint32_t nDeadTime ;
//	uint16_t nAvataID ;
//	uint32_t nInformSerial;
//	uint64_t nRoomProfit;
//	uint64_t nTotalProfit;
//	char vRoomName[MAX_LEN_ROOM_NAME];
//	char vRoomDesc[MAX_LEN_ROOM_DESC];
//	uint16_t nInformLen ;
//	PLACE_HOLDER(char* pRoomInfom);
//};

struct stMsgSaveDeleteRoom
	:public stMsg
{
public:
	stMsgSaveDeleteRoom(){ cSysIdentifer = ID_MSG_PORT_DB ; usMsgType = MSG_DELETE_ROOM ; }
	uint8_t nRoomType ; // eRoomType ;
	uint32_t nRoomID ;
};

//struct stMsgSaveCreateRoomInfo
//	:public stMsg
//{
//	stMsgSaveCreateRoomInfo(){ cSysIdentifer = ID_MSG_PORT_DB; usMsgType = MSG_SAVE_CREATE_ROOM_INFO; }
//	uint8_t nRoomType ; // eRoomType ;
//	uint32_t nRoomID ;
//	uint16_t nConfigID ;
//	uint32_t nRoomOwnerUID ;
//	uint32_t nCreateTime ;
//	uint64_t nChatRoomID ;
//};

struct stMsgSaveUpdateRoomInfo
	:public stMsg
{
	stMsgSaveUpdateRoomInfo(){ cSysIdentifer = ID_MSG_PORT_DB ; usMsgType = MSG_SAVE_UPDATE_ROOM_INFO ;}
	uint8_t nRoomType ; // eRoomType ;
	uint32_t nRoomID ;
	uint32_t nRoomOwnerUID ;
	uint32_t nConfigID ;
	uint16_t nJsonLen ;
	PLACE_HOLDER(char* pJsonArg);
};

struct stMsgReadRoomInfo
	:public stMsg
{
	stMsgReadRoomInfo(){ cSysIdentifer = ID_MSG_PORT_DB ; usMsgType = MSG_READ_ROOM_INFO ; }
	uint8_t nRoomType ; // eRoomType ;
};

struct stMsgReadRoomInfoRet
	:public stMsg
{
	stMsgReadRoomInfoRet(){ cSysIdentifer = ID_MSG_PORT_NONE; usMsgType = MSG_READ_ROOM_INFO ; }
	uint8_t nRoomType ; // eRoomType ;
	uint32_t nRoomID ;
	uint32_t nRoomOwnerUID ;
	uint32_t nConfigID ;
	uint16_t nJsonLen ;
	PLACE_HOLDER(char* pJsonArg);
};

struct stMsgSaveTaxasRoomPlayer
	:public stMsg
{
	stMsgSaveTaxasRoomPlayer(){ cSysIdentifer = ID_MSG_PORT_DB ; usMsgType = MSG_SAVE_TAXAS_ROOM_PLAYER ; }
	bool isUpdate ; // update or add 
	uint32_t nRoomID ;
	uint32_t nPlayerUID ;
	uint32_t m_nReadedInformSerial; 
	uint64_t nTotalBuyInThisRoom ; 
	uint64_t nFinalLeftInThisRoom ;  
	uint32_t nWinTimesInThisRoom ;
	uint32_t nPlayeTimesInThisRoom ;
};

struct stMsgSaveRemoveTaxasRoomPlayers
	:public stMsg
{
	stMsgSaveRemoveTaxasRoomPlayers(){cSysIdentifer = ID_MSG_PORT_DB ; usMsgType = MSG_SAVE_REMOVE_TAXAS_ROOM_PLAYERS ;}
	uint32_t nRoomID ;
};

struct stMsgReadTaxasRoomPlayers
	:public stMsg
{
	stMsgReadTaxasRoomPlayers(){ cSysIdentifer = ID_MSG_PORT_DB ; usMsgType = MSG_READ_TAXAS_ROOM_PLAYERS ; }
	uint32_t nRoomID ;
};

struct stMsgReadTaxasRoomPlayersRet
	:public stMsgToRoom
{
	stMsgReadTaxasRoomPlayersRet(){ cSysIdentifer = ID_MSG_PORT_TAXAS; usMsgType = MSG_READ_TAXAS_ROOM_PLAYERS ; }
	uint32_t nPlayerUID ;
	uint32_t m_nReadedInformSerial; 
	uint64_t nTotalBuyInThisRoom ; 
	uint64_t nFinalLeftInThisRoom ;  
	uint32_t nWinTimesInThisRoom ;
	uint32_t nPlayeTimesInThisRoom ;
};


struct stMsgSavePlayerCommonLoginData
	:public stMsg
{
	stMsgSavePlayerCommonLoginData(){ cSysIdentifer = ID_MSG_PORT_DB ; usMsgType = MSG_SAVE_COMMON_LOGIC_DATA;  } 
	uint32_t nUserUID ;
	uint64_t nMostCoinEver;
	uint8_t nVipLevel ;
	int64_t nYesterdayCoinOffset ;
	int64_t nTodayCoinOffset ;
	int32_t nTotalGameCoinOffset ;
	uint32_t tOfflineTime ;  // last offline time ;
	uint32_t nContinueDays ;
	uint32_t tLastLoginTime;
	uint32_t tLastTakeCharityCoinTime ;
	uint8_t nTakeCharityTimes ;
	uint32_t nTotalInvitePrizeCoin ;
	uint8_t nNewPlayerHaloWeight ;
	double dfLongitude;
	double dfLatidue;
	uint8_t nCardType ;
	uint32_t nCardEndTime ;
	uint32_t nLastTakeCardGiftTime ;
	uint32_t vJoinedClubID[MAX_JOINED_CLUB_CNT] ;
};

struct stMsgSelectPlayerData
	:public stMsg
{
	stMsgSelectPlayerData(){ cSysIdentifer = ID_MSG_PORT_DB ; usMsgType = MSG_SELECT_DB_PLAYER_DATA ;}
	bool isDetail ;
	uint32_t nTargetPlayerUID ;
};

struct stMsgSelectPlayerDataRet
	:public stMsg
{
	stMsgSelectPlayerDataRet(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_SELECT_DB_PLAYER_DATA ;}
	uint8_t nRet ; // 0 success , 1 can not find player ;
	bool isDetail ;
	uint32_t nDataPlayerUID ;
	PLACE_HOLDER(stPlayerBrifData* tData ) ;
};

// data and taxas server 
struct stMsgRequestTaxasPlayerData
	:public stMsg
{
	uint32_t nRoomID ;
	stMsgRequestTaxasPlayerData(){ cSysIdentifer = ID_MSG_PORT_DATA; usMsgType = MSG_TP_REQUEST_PLAYER_DATA ; }
};

struct stMsgRequestTaxasPlayerDataRet
	:public stMsg
{
	stMsgRequestTaxasPlayerDataRet(){ cSysIdentifer = ID_MSG_PORT_TAXAS ; usMsgType = MSG_TP_REQUEST_PLAYER_DATA ; }
	uint8_t nRet ; // 0 succes , 1 not find player data , 2 already in another taxas room ;
	stTaxasInRoomPeerData tData ;
};

struct stMsgReadMyOwnTaxasRooms
	:public stMsg
{
	stMsgReadMyOwnTaxasRooms(){ cSysIdentifer = ID_MSG_PORT_TAXAS ; usMsgType = MSG_TP_READ_MY_OWN_ROOMS ; }
	uint32_t nUserUID ;
};

struct stMsgReadMyOwnRooms
	:public stMsg
{
	stMsgReadMyOwnRooms(){ cSysIdentifer = ID_MSG_PORT_NIU_NIU ; usMsgType = MSG_READ_MY_OWN_ROOMS ; }
	uint32_t nUserUID ;
};

struct stMyOwnRoom
{
	uint32_t nRoomID;
};

struct stMsgReadMyOwnTaxasRoomsRet
	:public stMsg
{
	stMsgReadMyOwnTaxasRoomsRet(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_TP_READ_MY_OWN_ROOMS ; }
	uint8_t nRoomType ;
	uint16_t nCnt ;
	PLACE_HOLDER(stMyOwnRoom* vMyOwnRooms);
};

struct stMsgReadMyOwnRoomsRet
	:public stMsg
{
	stMsgReadMyOwnRoomsRet(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_READ_MY_OWN_ROOMS ; }
	uint8_t nRoomType ;
	uint16_t nCnt ;
	PLACE_HOLDER(stMyOwnRoom* vMyOwnRooms);
};

//struct stMsgPlayerRequestCoin
//	:public stMsg
//{
//	stMsgPlayerRequestCoin(){ cSysIdentifer = ID_MSG_PORT_DATA; usMsgType = MSG_REQUEST_MONEY ; nAtLeast = 0; }
//	uint8_t nReqType ;  // eReqMoneyType
//	uint32_t nUserUID ;  // if do not known , mark it 1 ;
//	uint32_t nSessionID ;
//	uint64_t nWantMoney;
//	uint64_t nAtLeast;
//	bool bIsDiamond ;
//	int32_t nBackArg[eReqMoneyArgCnt] ;
//};
//
//struct stMsgPlayerRequestCoinRet
//	:public stMsg
//{
//	stMsgPlayerRequestCoinRet(){ cSysIdentifer = ID_MSG_PORT_TAXAS; usMsgType = MSG_REQUEST_MONEY ; }
//	uint8_t nRet ; // 0 success , 1 not enough , 2 canot find player , 3 not in a taxas room 
//	uint8_t nReqType ; // eReqMoneyType
//	uint32_t nUserUID ;
//	uint64_t nAddedMoney;  // 0 means money not enough ;
//	bool bIsDiamond ;
//	int32_t nBackArg[eReqMoneyArgCnt] ;
//};
//
//struct stMsgTaxasPlayerRequestCoinComfirm
//	:public stMsg
//{
//	stMsgTaxasPlayerRequestCoinComfirm(){ cSysIdentifer = ID_MSG_PORT_DATA, usMsgType = MSG_REQUEST_MONEY_COMFIRM ; }
//	uint8_t nRet ; // 0 success , 1 failed;
//	bool bDiamond ;
//	uint64_t nWantedMoney ;
//	uint32_t nUserUID ;
//};

struct  stMsgSyncTaxasPlayerData
	:public stMsg
{
	stMsgSyncTaxasPlayerData(){ cSysIdentifer = ID_MSG_PORT_DATA,usMsgType = MSG_TP_SYNC_PLAYER_DATA ;  }
	uint32_t nUserUID ;
	uint32_t nWinTimes ;
	uint32_t nPlayTimes ;
	uint64_t nSingleWinMost ;
	uint8_t  vBestCard[MAX_TAXAS_HOLD_CARD];
};

struct stMsgInformTaxasPlayerLeave
	:public stMsg
{
	//stMsgInformTaxasPlayerLeave(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_TP_INFORM_LEAVE; }
	uint32_t nUserUID ;
	//uint64_t nTakeInMoney ;
	//bool bIsDiamond ;
};

struct stMsgOrderTaxasPlayerLeave
	:public stMsgToRoom
{
	//stMsgOrderTaxasPlayerLeave(){ cSysIdentifer = ID_MSG_PORT_TAXAS; usMsgType = MSG_TP_ORDER_LEAVE ; }
	uint32_t nSessionID ;
	uint32_t nUserUID ;
};

struct stMsgOrderTaxasPlayerLeaveRet
	:public stMsg
{
	//stMsgOrderTaxasPlayerLeaveRet(){ cSysIdentifer = ID_MSG_PORT_DATA; usMsgType = MSG_TP_ORDER_LEAVE ; }
	uint32_t nUserUID ;
	uint8_t nRet ; // 0 success , 1 can not find room ;
};

struct stMsgSvrEnterRoom
	:public stMsg
{
	stMsgSvrEnterRoom(){ cSysIdentifer = ID_MSG_PORT_NIU_NIU ; usMsgType = MSG_SVR_ENTER_ROOM ; }
	uint32_t nRoomID ;
	int8_t nSubIdx ;  // -1 means sys decide ;
	stEnterRoomData tPlayerData ;
};

struct stMsgSvrEnterRoomRet
	:public stMsg
{
	stMsgSvrEnterRoomRet(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_SVR_ENTER_ROOM ; }
	uint8_t nRet ; // 0 success , 1 already in this room , 2 not register player  can not enter ; 3 diamond is not enough ; 4 ;  player coin is too many ; 5 can not find room id ,  6 room type error, 7 Queuing up
	uint8_t nGameType ;
	uint32_t nRoomID ;
	int8_t nSubIdx ; 
};

struct stMsgSvrEnterGoldRoom
	:public stMsg
{
	stMsgSvrEnterGoldRoom() { cSysIdentifer = ID_MSG_PORT_NIU_NIU; usMsgType = MSG_SVR_ENTER_GOLDROOM; }
	uint32_t nRoomID;
	int8_t nSubIdx;  // -1 means sys decide ;
	uint8_t nGameType;
	stEnterRoomData tPlayerData;
};

struct stMsgSvrEnterGoldRoomRet
	:public stMsg
{
	stMsgSvrEnterGoldRoomRet() { cSysIdentifer = ID_MSG_PORT_DATA; usMsgType = MSG_SVR_ENTER_GOLDROOM; }
	uint8_t nRet; // 0 success , 1 already in this room , 2 not register player  can not enter ; 3 diamond is not enough ; 4 ;  player coin is too many ; 5 can not find room id ,  6 room type error, 8 Queuing up
	uint8_t nGameType;
	uint32_t nRoomID;
	int8_t nSubIdx;
};

//struct stMsgSvrApplyLeaveRoom
//	:public stMsg
//{
//	stMsgSvrApplyLeaveRoom(){ cSysIdentifer = ID_MSG_PORT_NIU_NIU ; usMsgType = MSG_SVR_DO_LEAVE_ROOM ; }
//	uint32_t nUserUID ;
//	uint32_t nSessionID ;
//	uint32_t nRoomID ;
//	uint32_t nRoomType ;
//};

struct stMsgSvrDoLeaveRoom
	:public stMsg
{
	stMsgSvrDoLeaveRoom(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_SVR_DO_LEAVE_ROOM ; }
	uint8_t nGameType ;
	uint8_t nRoomID ;
	uint32_t nUserUID ;
	uint64_t nCoin ;
	uint32_t nPlayerTimes ;
	uint32_t nWinTimes ;
	uint32_t nSingleWinMost ;
	int32_t nGameOffset ;
};

struct stMsgSvrDelayedLeaveRoom
	:public stMsgSvrDoLeaveRoom
{
	stMsgSvrDelayedLeaveRoom(){ usMsgType = MSG_SVR_DELAYED_LEAVE_ROOM ;}
	uint32_t nUserUID ;
};

// other with Verify Server ;
struct stMsgToVerifyServer
	:public stMsg
{
	stMsgToVerifyServer(){ cSysIdentifer = ID_MSG_PORT_VERIFY ; usMsgType = MSG_VERIFY_TANSACTION ; }
	unsigned int nBuyerPlayerUserUID ; 
	unsigned int nBuyForPlayerUserUID ;
	unsigned short nShopItemID ; // for mutilp need to verify ;
	uint32_t nMiUserUID ;
	uint8_t nChannel ; 
	unsigned short nTranscationIDLen ;
	char* ptransactionID ;   // base64 ed ;
	unsigned int nPrice;
};

struct stMsgFromVerifyServer
	:public stMsg
{
	stMsgFromVerifyServer(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_VERIFY_TANSACTION ; }
	unsigned int nBuyerPlayerUserUID ; 
	unsigned int nBuyForPlayerUserUID ;
	unsigned short nShopItemID ; // for mutilp need to verify ;
	unsigned char nRet ; // 0 apple check error iap free crack ,2 duplicate tansactionid  , 4 Success ;
};


struct stMsgVerifyItemOrder
	:public stMsg
{
	stMsgVerifyItemOrder(){ cSysIdentifer = ID_MSG_PORT_VERIFY ; usMsgType = MSG_VERIFY_ITEM_ORDER ; }
	char cShopDesc[50] ;
	char cOutTradeNo[32] ; // [shopItemID]E[playerUID]E[utc time] 
	uint32_t nPrize ; // fen wei dan wei ;
	char cTerminalIp[17] ;
	uint8_t nChannel ;
};

struct stMsgVerifyItemOrderRet
	:public stMsg
{
	stMsgVerifyItemOrderRet(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_VERIFY_ITEM_ORDER ; }
	uint8_t nRet ; // 0 success , 1 argument error ;
	char cPrepayId[64] ;
	char cOutTradeNo[32] ; // [shopItemID]E[playerUID]E[utc time] 
	uint8_t nChannel ;
};

// request ipAddress ;
struct stMsgLoginSvrInformGateSaveLog
	:stMsg
{
	stMsgLoginSvrInformGateSaveLog(){ cSysIdentifer = ID_MSG_PORT_GATE ; usMsgType = MSG_LOGIN_INFORM_GATE_SAVE_LOG ; }
	uint32_t nUserUID ;
	uint16_t nlogType ;
};

struct stMsgSaveLog
	:public stMsg
{
	stMsgSaveLog(){ cSysIdentifer = ID_MSG_PORT_LOG,usMsgType = MSG_SAVE_LOG; nJsonExtnerLen = 0 ; }
	uint16_t nLogType ;
	uint32_t nTargetID ;
	int64_t vArg[LOG_ARG_CNT];
	uint16_t nJsonExtnerLen ;
	PLACE_HOLDER(char* pJsonstring);
};

// cross server request 
struct stMsgCrossServerRequest
	:public stMsg
{
	stMsgCrossServerRequest(){ cSysIdentifer = ID_MSG_PORT_NONE; usMsgType = MSG_CROSS_SERVER_REQUEST ; nJsonsLen = 0 ;}
	uint32_t nTargetID ; // when represent for player , it is UID ;
	uint32_t nReqOrigID ; // when represent for player , it is UID ;
	uint16_t nRequestType ;
	uint16_t nRequestSubType ;
	int64_t vArg[CROSS_SVR_REQ_ARG];
	uint16_t nJsonsLen ;
	PLACE_HOLDER(char* pJsonString);
};

struct stMsgCrossServerRequestRet
	:public stMsg
{
	stMsgCrossServerRequestRet(){ cSysIdentifer = ID_MSG_PORT_DATA; usMsgType = MSG_CROSS_SERVER_REQUEST_RET ; nJsonsLen = 0 ;}
	uint8_t nRet ;
	uint32_t nTargetID ; // when represent for player , it is UID ;
	uint32_t nReqOrigID ; // when represent for player , it is UID ;
	uint16_t nRequestType ;
	uint16_t nRequestSubType ;
	int64_t vArg[CROSS_SVR_REQ_ARG];
	uint16_t nJsonsLen ;
	PLACE_HOLDER(char* pJsonString);
};

// friend 
struct stMsgSaveFirendList
	:public stMsg
{
	stMsgSaveFirendList(){ cSysIdentifer = ID_MSG_PORT_DB ;usMsgType = MSG_SAVE_FRIEND_LIST ; }
	uint32_t nUserUID ;
	uint16_t nFriendCountLen ;
	PLACE_HOLDER(char* vFriendUIDS);
};

struct stMsgReadFriendList
	:public stMsg
{
	stMsgReadFriendList(){ cSysIdentifer = ID_MSG_PORT_DB ; usMsgType = MSG_READ_FRIEND_LIST ; }
	uint32_t nUserUID ;
};

struct stMsgReadFriendListRet
	:public stMsg
{
	stMsgReadFriendListRet(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_READ_FRIEND_LIST ; }
	uint16_t nFriendCountLen ;
	PLACE_HOLDER(char* vFriendUIDS);
};

// mail module 
struct stMsgSaveMail
	:public stMsg
{
	stMsgSaveMail(){ usMsgType = MSG_PLAYER_SAVE_MAIL ;  cSysIdentifer = ID_MSG_PORT_DB ;}
	uint32_t nUserUID ;
	stMail pMailToSave ;
};

struct stMsgReadMailList
	:public stMsg
{
	stMsgReadMailList(){ usMsgType = MSG_PLAYER_READ_MAIL_LIST ; cSysIdentifer = ID_MSG_PORT_DB ; }
	uint32_t nUserUID ;
};

struct stMsgReadMailListRet
	:public stMsg
{
	stMsgReadMailListRet(){ usMsgType = MSG_PLAYER_READ_MAIL_LIST ; cSysIdentifer = ID_MSG_PORT_DATA ; }
	bool bFinal ;
	uint32_t nUserUID ;
	stMail pMails ;
};

struct stMsgResetMailsState
	:public stMsg
{
	stMsgResetMailsState(){ cSysIdentifer = ID_MSG_PORT_DB ; usMsgType = MSG_PLAYER_SET_MAIL_STATE ; }
	uint32_t nUserUID ;
	uint8_t eType ; 
	uint32_t nTimeLatest ;
};

struct  stMsgSavePlayerAdvice
	:public stMsg
{
	stMsgSavePlayerAdvice(){ cSysIdentifer = ID_MSG_PORT_LOG ; usMsgType = MSG_SAVE_PLAYER_ADVICE ; }
	uint32_t nUserUID ;
	uint16_t nLen ;
	PLACE_HOLDER(char* pContent);
};

// poker circle 
struct stCircleTopicItem
{
	uint64_t nTopicID ;
	uint32_t nAuthorUID ;
	uint32_t nPublishTime ;
	uint16_t nContentLen ;
	PLACE_HOLDER(char* pContent);
};

struct stMsgSaveAddCircleTopic
	:public stMsg
{
	stMsgSaveAddCircleTopic(){ cSysIdentifer = ID_MSG_PORT_DB ; usMsgType = MSG_CIRCLE_SAVE_ADD_TOPIC ; }
	stCircleTopicItem item ;
};

struct stMsgSaveDeleteCircleTopic
	:public stMsg
{
	stMsgSaveDeleteCircleTopic(){ cSysIdentifer = ID_MSG_PORT_DB ; usMsgType = MSG_CIRCLE_SAVE_DELETE_TOPIC ; }
	uint64_t nTopicID ;
};

struct stMsgReadCircleTopics
	:public stMsg
{
	stMsgReadCircleTopics(){ cSysIdentifer = ID_MSG_PORT_DB ; usMsgType = MSG_CIRCLE_READ_TOPICS ; }
};

struct stMsgReadCircleTopicsRet
	:public stMsg
{
	stMsgReadCircleTopicsRet(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_CIRCLE_READ_TOPICS ; }
	uint8_t nCnt ;
	PLACE_HOLDER(stCircleTopicItem* pItems);
};


// get client ip ;
struct stMsgRequestClientIp
	:public stMsg
{
	stMsgRequestClientIp(){ cSysIdentifer = ID_MSG_PORT_GATE ; usMsgType = MSG_REQUEST_CLIENT_IP ; }
};

struct stMsgRequestClientIpRet
	:public stMsg
{
	stMsgRequestClientIpRet(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_REQUEST_CLIENT_IP ; }
	uint8_t nRet ; // can not find client ;
	char vIP[17];
};

// invite 
struct stMsgDBCheckInvite
	:public stMsg
{
	stMsgDBCheckInvite(){ cSysIdentifer = ID_MSG_PORT_DB ; usMsgType = MSG_DB_CHECK_INVITER ; }
	uint32_t nInviteUserUID ;
};

struct stMsgDBCheckInviteRet
	:public stMsg
{
	stMsgDBCheckInviteRet(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_DB_CHECK_INVITER ; }
	uint8_t nRet ; // 0 success , 1 inviter not exsit ;
	uint32_t nInviteUseUID ;
};

// push notification 
struct stMsgReadNoticePlayer
	:public stMsg
{
	stMsgReadNoticePlayer(){ cSysIdentifer = ID_MSG_PORT_DB ; usMsgType  = MSG_READ_NOTICE_PLAYER ;}
	uint32_t nUserUID ;
};

struct stNoticePlayerEntry
{
	uint32_t nUserUID ;
	uint32_t nNoticeFlag ; 
	char pToken[32] ;
};

struct stMsgReadNoticePlayerRet
	:public stMsg
{
	stMsgReadNoticePlayerRet(){ cSysIdentifer = ID_MSG_PORT_APNS ; usMsgType = MSG_READ_NOTICE_PLAYER ; }
	uint8_t nRet ; // 0 success , 1 can not find ;
	stNoticePlayerEntry tPlayerEntery ;
};

struct stMsgSaveNoticePlayer
	: public stMsg
{
	stMsgSaveNoticePlayer(){ cSysIdentifer = ID_MSG_PORT_DB ; usMsgType = MSG_SAVE_NOTICE_PLAYER ; }
	bool nOpt ; // 0 add , 1 reamove flag ;
	stNoticePlayerEntry tPlayer ;
};

struct stMsgPushNotice
	:public stMsg
{
	stMsgPushNotice(){ cSysIdentifer = ID_MSG_PORT_APNS ; usMsgType  = MSG_PUSH_NOTICE ; }
	uint16_t nJonsLen ;
	PLACE_HOLDER(char* jsonString); // { targets:[345,3455,355],flag : 23 ,content: "this is a message" }
};


// robot 
struct stMsgRequestRobotToEnterRoom
	:public stMsg
{
	stMsgRequestRobotToEnterRoom(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_REQ_ROBOT_ENTER_ROOM ; }
	uint8_t nRoomType ;
	uint32_t nRoomID ; 
	uint32_t nSubRoomIdx ;
	uint8_t nReqRobotLevel ;
};

struct stMsgSaveEncryptNumber
	:public stMsg
{
	stMsgSaveEncryptNumber(){ cSysIdentifer = ID_MSG_PORT_DB ; usMsgType = MSG_SAVE_ENCRYPT_NUMBER ; }
	uint64_t nEncryptNumber ;
	uint32_t nCoin ;
	uint16_t nRMB ;
	uint8_t nNumberType ;   // 1 new player , 2 newMal ; 
	uint8_t nCoinType ; // 1 coin，0 diamond
	uint16_t nChannelID ;
};

struct stMsgSaveEncryptNumberRet
	:public stMsg
{
	stMsgSaveEncryptNumberRet(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_SAVE_ENCRYPT_NUMBER ; }
	uint8_t nRet ; // 0 success , 1 duplicate number  
};

struct stMsgVerifyEncryptNumber
	:public stMsg
{
	stMsgVerifyEncryptNumber(){ cSysIdentifer = ID_MSG_PORT_DB ; usMsgType = MSG_VERIFY_ENCRYPT_NUMBER ; }
	uint64_t nNumber ;
	uint32_t nUserUID ;
};

struct stMsgVerifyEncryptNumberRet
	:public stMsg
{
	stMsgVerifyEncryptNumberRet(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_VERIFY_ENCRYPT_NUMBER ; }
	uint8_t nRet ; // 0 success , 1 invalid number , 2 already used , 3 only can use one time ;
	uint32_t nAddCoin ;
	uint32_t nUserUID ;
	uint8_t nCoinType ;   // 1 coin，0 diamond
};

struct stMsgInformPlayerOnlineState
	:public stMsgToRoom
{
	stMsgInformPlayerOnlineState()
	{
		cSysIdentifer = ID_MSG_PORT_MJ;
		usMsgType = MSG_INFORM_PLAYER_ONLINE_STATE;
	}
	uint32_t nUID;
	bool isOnline;
};
//----above is new , below is old---------

































































//--------------------------------------------
// message between DB and Gamesever ;
struct stMsgGM2DB
	:public stMsg
{
	stMsgGM2DB(){ cSysIdentifer = ID_MSG_GM2DB, usMsgType = MSG_NONE ; }
public:
	unsigned int nSessionID;
};

struct stMsgDB2GM
	:public stMsg
{
	stMsgDB2GM(){ cSysIdentifer = ID_MSG_DB2GM, usMsgType = MSG_NONE ; }
	unsigned int nSessionID;
};
//
//struct stMsgPushBaseDataToGameServer
//	:stMsgDB2GM
//{
//public:
//	stMsgPushBaseDataToGameServer(){usMsgType = MSG_PUSH_BASE_DATA ;}
//	stBaseData stData ;
//};

//-----------------------------------------------
// message between GateServer and Gamesever ;
//struct stMsgPeerDisconnect
//	:public stMsg
//{
//	stMsgPeerDisconnect()
//	{
//		cSysIdentifer = ID_MSG_GA2GM ;
//		usMsgType = MSG_DISCONNECT;
//	}
//	unsigned int nSessionID ;
//	
//};


struct stMsgGameServerCreateRole
	:public stMsgCreateRole
{
	stMsgGameServerCreateRole(){ cSysIdentifer = ID_MSG_GM2DB; usMsgType = MSG_CREATE_ROLE ; }
	unsigned int nSessionID ;
};

struct stMsgGameServerCreateRoleRet
	:public stMsgCreateRoleRet
{
	stMsgGameServerCreateRoleRet(){ cSysIdentifer = ID_MSG_DB2GM ; usMsgType = MSG_CREATE_ROLE ; }
	unsigned int nSessionID ;
};



struct stMsgGameServerGetMaxMailUID
	: public stMsgGM2DB 
{
	stMsgGameServerGetMaxMailUID(){ usMsgType = MSG_GAME_SERVER_GET_MAX_MAIL_UID ; }
};

struct stMsgGameServerGetMaxMailUIDRet
	:public stMsgDB2GM
{
	stMsgGameServerGetMaxMailUIDRet(){ usMsgType = MSG_GAME_SERVER_GET_MAX_MAIL_UID ; }
	uint64_t nMaxMailUID ;
};

// friend 
struct stServerSaveFrienItem
{
	unsigned int nFriendUserUID ;
	unsigned short nPresentTimes ;
};

struct stMsgGameServerRequestFirendList
	:public stMsgGM2DB
{
	stMsgGameServerRequestFirendList(){ cSysIdentifer = ID_MSG_GM2DB; usMsgType = MSG_REQUEST_FRIEND_LIST ; }
	unsigned int nUserUID ;
};

struct stMsgGameServerRequestFirendListRet
	:public stMsgDB2GM
{
	stMsgGameServerRequestFirendListRet(){ cSysIdentifer = ID_MSG_DB2GM; usMsgType = MSG_REQUEST_FRIEND_LIST ; }
	unsigned short nFriendCount ;
	stServerSaveFrienItem* pFirendsInfo;
};

////struct stMsgGameServerRequestFriendBrifDataList
////	:public stMsgGM2DB
////{
////	stMsgGameServerRequestFriendBrifDataList(){ cSysIdentifer = ID_MSG_GM2DB; usMsgType = MSG_REQUEST_FRIEND_BRIFDATA_LIST ; }
////	unsigned int nFriendCount ;
////	unsigned int* pFriendUserUIDs ;
////};

//struct stMsgGameServerRequestFriendBrifDataListRet
//	:public stMsgDB2GM
//{
//	stMsgGameServerRequestFriendBrifDataListRet(){ cSysIdentifer = ID_MSG_DB2GM ; usMsgType = MSG_REQUEST_FRIEND_BRIFDATA_LIST ; }
//	unsigned int nCount ;
//	stPlayerBrifData* pFriendBrifData ;
//};

////struct stMsgGameServerGetFriendDetail
////	:public stMsgGM2DB
////{
////	stMsgGameServerGetFriendDetail(){ cSysIdentifer = ID_MSG_GM2DB; usMsgType = MSG_PLAYER_REQUEST_FRIEND_DETAIL ; }
////	unsigned int nFriendUID ;
////};
////
////struct stMsgGameServerGetFriendDetailRet
////	:public stMsgDB2GM
////{
////	stMsgGameServerGetFriendDetailRet(){ usMsgType = MSG_PLAYER_REQUEST_FRIEND_DETAIL ; }
////	unsigned char nRet ; // 0 success ; 1 failed ;
////	stPlayerDetailData stPeerInfo ;
////};

struct stMsgGameServerGetSearchFriendResult
	:public stMsgGM2DB
{
	stMsgGameServerGetSearchFriendResult(){ cSysIdentifer = ID_MSG_GM2DB ;  usMsgType = MSG_PLAYER_SERACH_PEERS ; }
	char* pSearchContent ;
	unsigned char nLen ;
};

struct stMsgGameServerGetSearchFriendResultRet
	:public stMsgDB2GM
{
	stMsgGameServerGetSearchFriendResultRet(){ cSysIdentifer = ID_MSG_DB2GM ; usMsgType = MSG_PLAYER_SERACH_PEERS ; }
	unsigned char nResultCount ;
	stPlayerBrifData* pPeersInfo ;
};

//struct stMsgGameServerGetSearchedPeerDetail
//	:public stMsgGM2DB
//{
//	stMsgGameServerGetSearchedPeerDetail(){ usMsgType = MSG_PLAYER_REQUEST_SEARCH_PEER_DETAIL ; }
//	unsigned int nPeerUserUID ;
//};
//
//struct stMsgGameServerGetSearchedPeerDetailRet
//	:public stMsgDB2GM 
//{
//	stMsgGameServerGetSearchedPeerDetailRet(){ usMsgType = MSG_PLAYER_REQUEST_SEARCH_PEER_DETAIL ; }
//	unsigned char nRet ; // 0 success , 1 can not find ;
//	stPlayerDetailData stPeerInfo ;
//};
// item 
struct stMsgGameServerRequestItemList
	:public stMsgGM2DB
{
	stMsgGameServerRequestItemList(){usMsgType = MSG_REQUEST_ITEM_LIST ; }
	unsigned int nUserUID ;
};

struct stMsgGameServerRequestItemListRet 
	:public stMsgDB2GM
{
	stMsgGameServerRequestItemListRet(){ usMsgType = MSG_REQUEST_ITEM_LIST ; }
	unsigned short nOwnItemKindCount ;
	stPlayerItem* pItemsBuffer ;
};

struct stMsgGameServerSaveItemList
	:public stMsgGM2DB
{
	stMsgGameServerSaveItemList(){ usMsgType = MSG_SAVE_ITEM_LIST ; }
	unsigned int nUserUID ;
	unsigned short nOwnItemKindCount ;
	stPlayerItem* pItemsBuffer ;
};

// rank 
struct stMsgGameServerRequestRank
	:public stMsg
{
	stMsgGameServerRequestRank(){ usMsgType = MSG_REQUEST_RANK ; cSysIdentifer = ID_MSG_GM2DB ; }
	unsigned char eType ; // eRankType ;
};

//struct stRankPeerInfo
//	:public stRankPeerBrifInfo
//{
//	uint64_t nAllCoin ;
//	uint64_t nSingleWinMost ;
//	uint64_t nCurDayWin ;
//	unsigned int nDiamoned ;
//	char strSigure[MAX_LEN_SIGURE] ;
//	int nWinTimes ;
//	int nLostTimes ;
//	unsigned char eSex ;
//	unsigned int vAssetCount[eItem_Asset] ; 
//};

struct stServerGetRankPeerInfo
{
	stPlayerDetailData tDetailData ;
	uint64_t nYesterDayWin ;
};

struct stMsgGameServerRequestRankRet
	:public stMsg
{
	stMsgGameServerRequestRankRet(){ usMsgType = MSG_REQUEST_RANK ;cSysIdentifer = ID_MSG_DB2GM ; }
	unsigned char eType ; // eRankType ;
	unsigned char nPeerCount ;
	stServerGetRankPeerInfo* peers ;
};

// shop 
struct stMsgGameServerSaveShopBuyRecord
	:public stMsgGM2DB
{
public:
	stMsgGameServerSaveShopBuyRecord(){ usMsgType = MSG_SAVE_SHOP_BUY_RECORD ;}
	bool bAdd ; // or just update ;
	unsigned int nUserUID ;
	unsigned short nBufferLen ;
	char* pBuffer ;
};

struct stMsgGameServerGetShopBuyRecord
	:public stMsgGM2DB
{
	stMsgGameServerGetShopBuyRecord(){ usMsgType = MSG_GET_SHOP_BUY_RECORD ; }
	unsigned int nUserUID ;
};

struct stMsgGameServerGetShopBuyRecordRet
	:public stMsgDB2GM
{
	stMsgGameServerGetShopBuyRecordRet(){ usMsgType = MSG_GET_SHOP_BUY_RECORD ; }
	unsigned short nBufferLen ;
	char* pBuffer ;
};

// mission 
struct stMsgGameServerSaveMissionData
	:public stMsgGM2DB
{
	stMsgGameServerSaveMissionData(){ usMsgType = MSG_GAME_SERVER_SAVE_MISSION_DATA ; }
	unsigned int nUserUID ;
	unsigned int nSavetime ;
	unsigned short nMissonCount ;
	stMissionSate* pMissonData ;
};

struct stMsgGameServerGetMissionData
	:public stMsgGM2DB
{
	stMsgGameServerGetMissionData(){ usMsgType = MSG_GAME_SERVER_GET_MISSION_DATA ; }
	unsigned int nUserUID ;
};

struct stMsgGameServerGetMissionDataRet
	:public stMsgDB2GM
{
	stMsgGameServerGetMissionDataRet(){ usMsgType = MSG_GAME_SERVER_GET_MISSION_DATA ; }
	unsigned int nLastSaveTime ;
	unsigned short nMissonCount ;
	stMissionSate* pMissonData ;
};

// other game msg;


struct stMsgTransferData
	:public stMsg
{
	stMsgTransferData()
	{
		cSysIdentifer = ID_MSG_TRANSFER ;
		usMsgType = MSG_TRANSER_DATA;
		bBroadCast = false ;
		nSessionID = 0 ;
	}
	uint16_t nSenderPort ; // who send this msg ;  eMsgPort
	uint32_t nSessionID ;
	bool bBroadCast ;
	char pData[0] ;
};

// push server message 
struct stMsgToAPNSServer
	:public stMsg
{
	stMsgToAPNSServer(){ cSysIdentifer = ID_MSG_TO_APNS_SERVER, usMsgType = MSG_APNS_INFO; }
	unsigned short nBadge ; // >= 1 ;
	char pDeveiceToken[32] ;  // must proccesed in client ; change to htonl();  // change to network big endain ;
	char* cSound;   // sound file name ; the file must exsit in client ;
	unsigned char nSoundLen ; // < 50 ;
	char* pAlert ;   // a json right value , Warnning: must include [" " ] if not { } value ;
	unsigned char nAlertLen ; // < 219 ;
};

// DB log ;
enum eDBLog
{
	eDBLog_None,
	eDBLog_Login,
	eDBLog_Impawn,
	eDBLog_MissionReward,
	eDBLog_Shop,
	eDBLog_OtherMoneyOffset,
	eDBLog_StayInRoom,
	eDBLog_PresentAsset,
	eDBLog_RobotCoin,
	eDBLog_Max,
};

struct stDBLog
{
	eDBLog eLogType ;
	unsigned int nUserUID ;
	char cPlayerName[MAX_LEN_CHARACTER_NAME] ;
	uint64_t nCurCoin ;
	unsigned int nCurDiamond ;
	stDBLog(){ eLogType = eDBLog_Max;}
};

struct stPlayerLoginDBLog
	:public stDBLog
{
	unsigned int nLoginTime ;
	stPlayerLoginDBLog(){eLogType = eDBLog_Login ;}
};

struct stPlayerImpawnDBLog
	:public stDBLog
{
	unsigned short nImpawnItemID ;
	unsigned int nItemCount ;
	uint64_t nImpawnCoin ;
	unsigned int nImpawnDiamoned ;
	stPlayerImpawnDBLog(){eLogType = eDBLog_Impawn ;}
};

struct stPlayerGetMissionRewardDBlog
	:public stDBLog
{
	unsigned short nMissionID ;
	unsigned int nGetCoin ;
	stPlayerGetMissionRewardDBlog(){eLogType = eDBLog_MissionReward ;}
};

struct stPlayShopDBLog
	:public stDBLog
{
	unsigned char nMoneyType ; //0 钻石， 1 金币，3  RMB
	unsigned int nSpendMoney;
	unsigned char cChannel ; //渠道标示，0. appstore  1. pp 助手，2.  91商店 3. 360商店 4.winphone store , 100 淘宝充值
	unsigned short nShopID ;
	unsigned int nShopCnt ;
	stPlayShopDBLog(){eLogType = eDBLog_Shop ;}
};

struct stPlayerOtherMoneyActDBLog
	:public stDBLog
{
	unsigned char cActType ; // 0 好友免费赠送，1 师傅关系获得 ， 2  徒弟关系获得
	int nCoinOffset ;
	int nDiamondOffset ;
	stPlayerOtherMoneyActDBLog(){eLogType = eDBLog_OtherMoneyOffset ;}
};

struct stPlayerInRoomDBLog
	:public stDBLog
{
	unsigned int nRoomID ;
	uint64_t nCoinBeforEnterRoom ;
	unsigned int nEnterRoomTime ;
	stPlayerInRoomDBLog(){eLogType = eDBLog_StayInRoom ;}
};

struct stPlayerPresentAssertDBLog
	:public stDBLog
{
	unsigned int nTargetPlayerUID ;
	unsigned short nAssertID ;
	unsigned short nAssetCount ;
	unsigned short nPresentReason ; // 0 普通赠送， 1 拜师赠送 
	stPlayerPresentAssertDBLog(){eLogType = eDBLog_PresentAsset ;}
};

struct stRobotCoinDBLog
	:public stDBLog
{
	int64_t vRoomLevelRobotOffset[eRoomLevel_Max] ;
	int64_t vAllRobotOffset ;
	stRobotCoinDBLog(){eLogType = eDBLog_RobotCoin ;}
};

struct stMsgToLogDBServer
	:public stMsg
{
	stMsgToLogDBServer(){ cSysIdentifer = ID_MSG_TO_GM2LOG; usMsgType = MSG_SAVE_DB_LOG; }
	stDBLog* pLogContent ;
};

#pragma pack(pop)//
