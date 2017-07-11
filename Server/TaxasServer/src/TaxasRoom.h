#pragma once
#include "ServerMessageDefine.h"
#include "log4z.h"
#include "RoomConfig.h"
#include "CommonData.h"
#include "CardPoker.h"
#include <json/json.h>
#include <cassert>
#include "ISitableRoom.h"
class CTaxasPlayer ;
class CInsuranceCheck ;
typedef std::vector<uint8_t> VEC_INT8 ;
struct stTaxasInRoomPeerDataExten
	:public stTaxasInRoomPeerData
{
	// when play stand up, nCoinInRoom is player total coin, 
	//when player sit down (nCoinInRoom + nTakeInCoin) is player total coin
	// when player sitdown again (but not leave room ) TakeIn coin first from  nCoinInRoom, if not enough
	// then rquest from data svr ;
	// when player standup ,should add 'nTakeInCoin' to  'nCoinInRoom'
	//uint64_t nCoinInRoom ; 
	//uint32_t nStateFlag ;
	uint32_t m_nReadedInformSerial; 
	uint64_t nTotalBuyInThisRoom ; // not real coin , just for record
	uint64_t nFinalLeftInThisRoom ;   // not real coin , just for record
	uint32_t nWinTimesInThisRoom ;
	uint32_t nPlayeTimesInThisRoom ;
	bool bDataDirty ;
	//bool IsHaveState( eRoomPeerState estate ) { return ( nStateFlag & estate ) == estate ; }
};

struct stTaxasPeerData
	:public stTaxasPeerBaseData
{
	uint64_t nAllBetCoin ;  // used for tell win or lose
	uint64_t nTotalBuyInThisRoom ; // used for record
	uint64_t nWinCoinThisGame ;    // used for tell win or lose
	uint32_t nWinTimes ;
	uint32_t nPlayTimes ;
	uint64_t nSingleWinMost ;
	stTaxasInRoomPeerDataExten* pHistoryData;
	uint8_t vBestCards[MAX_TAXAS_HOLD_CARD];

	bool IsHaveState( eRoomPeerState estate ) { return ( nStateFlag & estate ) == estate ; } ;
	bool IsInvalid(){ return (nSessionID == 0) && (nUserUID == 0);}
	bool BetCoin( uint64_t nBetCoin )
	{ 
		assert(pHistoryData&&"must not null");
		if ( nTakeInMoney >= nBetCoin )
		{
			pHistoryData->nFinalLeftInThisRoom -= nBetCoin ;
			nTakeInMoney -= nBetCoin ;
			nAllBetCoin += nBetCoin ;
			nBetCoinThisRound += nBetCoin ;
			LOGFMTI("uid= %d , betCoin = %lld, nBetThisRound = %lld",nUserUID,nBetCoin,nBetCoinThisRound);
			return true ;
		} 
		return false ;
	}
};

struct stPlayedPeerRecord
{
	uint32_t nMoneyOffset ;
	uint32_t nMoneyLastLeft ;
};

struct stVicePool
{
	uint8_t nIdx ;
	bool bUsed ;
	uint32_t nCoin ;
	VEC_INT8 vInPoolPlayerIdx ;
	VEC_INT8 vWinnerIdxs ;

	void Reset(){ bUsed = false ; nCoin = 0 ; vInPoolPlayerIdx.clear() ; vWinnerIdxs.clear(); }
	void RemovePlayer(uint8_t nIdx)
	{
		VEC_INT8::iterator iter = vInPoolPlayerIdx.begin();
		for ( ; iter != vInPoolPlayerIdx.end(); ++iter )
		{
			if ( (*iter) == nIdx  )
			{
				vInPoolPlayerIdx.erase(iter) ;
				return ;
			}
		}
	}

	bool isPlayerInThisPool(uint8_t nIdx)
	{
		VEC_INT8::iterator iter = vInPoolPlayerIdx.begin();
		for ( ; iter != vInPoolPlayerIdx.end(); ++iter )
		{
			if ( (*iter) == nIdx  )
			{
				return true;
			}
		}
		return false ;
	}
};

class CTaxasRoom
	:public ISitableRoom
{
public:
	typedef std::vector<stTaxasInRoomPeerDataExten*> VEC_IN_ROOM_PEERS ;
public:
	CTaxasRoom();
	virtual ~CTaxasRoom();
	uint8_t getRoomType()override ;
	bool onFirstBeCreated(IRoomManager* pRoomMgr,uint32_t nRoomID, const Json::Value& vJsValue )override;
	void serializationFromDB(IRoomManager* pRoomMgr,stBaseRoomConfig* pConfig,uint32_t nRoomID , Json::Value& vJsValue )override;
	void willSerializtionToDB(Json::Value& vOutJsValue)override;
	void roomItemDetailVisitor(Json::Value& vOutJsValue)override;
	void prepareState();

	bool onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID )override;
	bool onMessage( Json::Value& prealMsg ,uint16_t nMsgType, eMsgPort eSenderPort , uint32_t nSessionID )override{ return ISitableRoom::onMessage(prealMsg,nMsgType,eSenderPort,nSessionID) ;};
	
	ISitableRoomPlayer* doCreateSitableRoomPlayer()override ;
	uint32_t coinNeededToSitDown()override ;
	void prepareCards()override ;

	void onPlayerWillStandUp(ISitableRoomPlayer* pPlayer )override ;
	uint32_t getLeastCoinNeedForCurrentGameRound(ISitableRoomPlayer* pp)override ;
	void roomInfoVisitor(Json::Value& vOutJsValue)override ;
	void sendRoomPlayersInfo(uint32_t nSessionID)override ;

	// taxas define 
	uint8_t OnPlayerAction( uint8_t nSeatIdx ,eRoomPeerAction act , uint32_t& nValue );  // return error code , 0 success ;
	uint8_t GetCurWaitActPlayerIdx(){ return m_nCurWaitPlayerActionIdx ; }

	// logic function 
	void onGameDidEnd()override ;
	void onGameWillBegin()override ;

	void startGame();
	void PreparePlayersForThisRoundBet();
	uint8_t InformPlayerAct();
	void OnPlayerActTimeOut();
	bool IsThisRoundBetOK();
	uint8_t CaculateOneRoundPool();  // return produced vice pool cunt this round ;
	uint8_t DistributePublicCard(); // return dis card cnt ;
	uint8_t getPublicCardRound(){ return m_nPublicCardRound ;}
	uint8_t CaculateGameResult(); //return pool cnt ;
	//uint64_t GetAllBetCoinThisRound();
	bool IsPublicDistributeFinish();

	// insurance module ;
	bool isNeedByInsurance();
	bool isAnyOneBuyInsurace();
	void doCaculateInsurance();
	CInsuranceCheck* getInsuranceCheck();
	void setInsuredPlayerIdx(uint8_t nIdx );

	// debug info ;

	uint32_t getLittleBlind(){ return m_nLittleBlind ;}
	uint32_t getMaxTakeIn()override{ return m_nMaxTakeIn ; }
	uint32_t getMinTakeIn(){ return m_nMinTakeIn ; }
	uint64_t getMostBetCoinThisRound(){ return m_nMostBetCoinThisRound ;}
	void didCaculateGameResult();
	uint8_t getDistributedPublicCardRound(){ return m_nPublicCardRound ;}
protected:
	void writeGameResultLog();
	void writePlayerResultLogToJson(CTaxasPlayer* pWritePlayer);
	stVicePool& GetFirstCanUseVicePool();
	void CaculateVicePool(stVicePool& pPool );
	//void syncPlayerDataToDataSvr( stTaxasPeerData& pPlayerData );
protected:
	// static data 
	uint32_t m_nLittleBlind;
	uint32_t m_nMinTakeIn;
	int32_t m_nMaxTakeIn ;
	// running members ;
	uint8_t m_nBankerIdx ;
	uint8_t m_nLittleBlindIdx ;
	uint8_t m_nBigBlindIdx ;
	int8_t m_nCurWaitPlayerActionIdx ;
	uint32_t  m_nCurMainBetPool ;
	uint32_t  m_nMostBetCoinThisRound;
	uint8_t m_vPublicCardNums[TAXAS_PUBLIC_CARD] ; 
	uint8_t m_nPublicCardRound ; //valid value , 0,1 , 2 , 3 ,4 
	stVicePool m_vAllVicePools[MAX_PEERS_IN_TAXAS_ROOM] ;

	Json::Value m_arrPlayers ;

	// insurance 
	bool m_isInsured ;
	CInsuranceCheck* m_pInsurance ;
	uint8_t m_nBuyInsuraceIdx ;
};