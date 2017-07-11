#pragma once
#pragma pack(push)
#pragma pack(1)
#include "CommonDefine.h"
// base data about 
struct stPlayerBrifData
{
	char cName[MAX_LEN_CHARACTER_NAME];
	uint32_t nUserUID ;
	uint8_t nSex ; // eSex ;
	uint8_t nVipLevel ;
	uint16_t nPhotoID ;
	uint32_t nCoin ;
	uint32_t nDiamoned ;
	bool bIsOnLine ;
	uint32_t nCurrentRoomID ;
};

struct stPlayerGameData
{
	uint32_t nWinTimes ;
	uint32_t nPlayTimes ;
	uint64_t nSingleWinMost ;
	uint8_t vMaxCards[MAX_TAXAS_HOLD_CARD] ;
	uint32_t nChampionTimes ;
	uint32_t nRun_upTimes ;
	uint32_t nThird_placeTimes ;
};

struct stPlayerDetailData
	:public stPlayerBrifData
{
	char cSignature[MAX_LEN_SIGURE] ;
	uint64_t nMostCoinEver;
	double dfLongitude;
	double dfLatidue;
	uint32_t tOfflineTime ;  // last offline time ;
	uint32_t nCupCnt ;  // jiang bei ge shu ;
	uint8_t vUploadedPic[MAX_UPLOAD_PIC] ;
	uint32_t vJoinedClubID[MAX_JOINED_CLUB_CNT] ;
};


struct stPlayerDetailDataClient
	:public stPlayerDetailData
{
	
};

struct stCommonBaseData
	:public stPlayerDetailData
{
	int64_t nYesterdayCoinOffset ;
	int64_t nTodayGameCoinOffset ;
	uint32_t nInviteUID ;
	uint8_t nCardType ;  // eVipCardType 
	uint32_t nCardEndTime ;
	uint32_t nTotalInvitePrizeCoin ; 
};

struct stServerBaseData
	:public stCommonBaseData
{
	uint32_t nContinueDays ;
	uint32_t tLastLoginTime;
	int32_t  nTotalGameCoinOffset ;
	uint32_t tLastTakeCharityCoinTime ;
	uint8_t nTakeCharityTimes ;
	bool isRegister ;
	uint8_t nNewPlayerHaloWeight ;
	uint32_t tLastTakeCardGiftTime ;
};

struct stTaxasInRoomPeerData
{
	uint32_t nSessionID ;
	uint32_t nUserUID ;
	char cName[MAX_LEN_CHARACTER_NAME];
	uint8_t nSex ; // eSex ;
	uint8_t nVipLevel ;
	uint16_t nPhotoID ;
};

struct stTaxasPeerBaseData
	:public stTaxasInRoomPeerData
{
	uint64_t nTakeInMoney ; // maybe coin or diamond
	uint8_t eCurAct ;  // eRoomPeerAction
	uint64_t nBetCoinThisRound ;
	uint8_t vHoldCard[TAXAS_PEER_CARD];
	uint32_t nStateFlag ;
	uint8_t nSeatIdx ;
};




//-------------------up are new ----

struct stRoomBaseDataOnly
{
	unsigned int nRoomID ;
	unsigned char cGameType ; // eRoomType ;
	unsigned short nRoomLevel ;
	unsigned char cCurRoomState ; // eeRoomState ;
	unsigned char fOperateTime ;  // by second , wait player act ;
	float fTimeTick ;     // by second 
	unsigned char cMaxPlayingPeers ; // not include  standup peers , in some game ;
	unsigned int cMiniCoinNeedToEnter; 
};

// golden 
//struct stGoldenPeerData
//	:public stPeerBaseData
//{
//	uint64_t nBetCoin ;
//	unsigned char nShowedCardCnt ;
//	unsigned char vShowedCardIdx[GOLDEN_PEER_CARD] ;
//	unsigned char vHoldCard[GOLDEN_PEER_CARD];
//	unsigned short nChangeCardUsedDiamond ; 
//	unsigned char nChangeCardTimes ; 
//	unsigned short nPKTimes;     // fan bei ka shi yong
//};

struct stRoomGoldenDataOnly
	:public stRoomBaseDataOnly
{
	uint64_t nAllBetCoin ;
	unsigned char cBankerIdx ;
	char cCurActIdx ;
	unsigned char nRound ;   // begin with 0 ;
	unsigned int nMiniBet ; 
	uint64_t nCurMaxBet ; 
	unsigned short nTitleNeedToEnter ;
	unsigned char nChangeCardRound; 
	bool bCanDoublePK ;
};

// taxpoker peer 
//struct stTaxasPeerData
//	:public stPeerBaseData
//{
//	unsigned char eCurAct ;  // eRoomPeerAction
//	uint64_t nAllBetCoin ;
//	uint64_t nWinCoinFromPools ; // include the nAllBetCoin
//	uint64_t nBetCoinThisRound ;
//	unsigned char vHoldCard[TAXAS_PEER_CARD];
//};

struct stTaxasRoomDataSimple
	:public stRoomBaseDataOnly
{
	char cBankerIdx ;
	unsigned char cLittleBlindIdx ;
	unsigned char cBigBlindIdx ;
	char cCurWaitPlayerActionIdx ;

	uint64_t  nCurMainBetPool ;
	uint64_t  nMostBetCoinThisRound;

	unsigned char vPublicCardNums[TAXAS_PUBLIC_CARD] ; 
	unsigned char ePrePeerAction ; 
	unsigned char nBetRound ; //valid value , 0,1 , 2 , 3 ,4 

	unsigned int nBigBlindBet ;
	uint64_t nMaxTakeIn ;
	float fCurStateTime;  // cur state will last seconds ;
};

#pragma pack(pop)