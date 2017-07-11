#pragma once
#include "ISitableRoom.h"
#include <json/json.h>
#include "ConfigDefine.h"
#include "AutoBuffer.h"
class CNiuNiuRoom
	:public ISitableRoom
{
public:
	CNiuNiuRoom();
	bool onFirstBeCreated(IRoomManager* pRoomMgr, uint32_t nRoomID , const Json::Value& vJsValue)override;
	void serializationFromDB(IRoomManager* pRoomMgr,stBaseRoomConfig* pConfig,uint32_t nRoomID , Json::Value& vJsValue )override;
	void willSerializtionToDB(Json::Value& vOutJsValue)override;
	void roomItemDetailVisitor(Json::Value& vOutJsValue)override;
	void prepareState()override ;
	void roomInfoVisitor(Json::Value& vOutJsValue)override ;
	void sendRoomPlayersInfo(uint32_t nSessionID)override ;
	void setBankerIdx(uint8_t nIdx ) ;
	void clearBanker(){ m_nBankerIdx = -1 ;}
	uint8_t getBankerIdx(){ return m_nBankerIdx ;}
	void setBetBottomTimes(uint8_t nTimes ){ m_nBetBottomTimes = nTimes ;}
	uint8_t getBetBottomTimes(){ return m_nBetBottomTimes ;}
	uint8_t getMaxRate();
	uint8_t getDistributeCardCnt();
	uint32_t getBaseBet(); // ji chu di zhu ;
	uint32_t& getBankCoinLimitForBet();
	void setBankCoinLimitForBet( uint64_t nCoin );
	uint8_t getReateByNiNiuType(uint8_t nType , uint8_t nPoint );
	uint32_t getLeastCoinNeedForBeBanker( uint8_t nBankerTimes );
	void onGameWillBegin()override ;
	void onGameDidEnd()override ;
	void onPlayerWillStandUp( ISitableRoomPlayer* pPlayer )override ;
	uint32_t getLeastCoinNeedForCurrentGameRound(ISitableRoomPlayer* pp)override ;
	uint8_t getRoomType()override{ return eRoom_NiuNiu ;}
	void prepareCards()override;
	uint32_t coinNeededToSitDown()override;
	void caculateGameResult();
	bool getPlayersHaveGrabBankerPrivilege(std::vector<uint8_t>& vAllPrivilegeIdxs); 
	bool getPlayersWillBetPlayer(std::vector<uint8_t>& vWillBetIdx);
	bool isHaveBanker();
	bool onMessage( Json::Value& prealMsg ,uint16_t nMsgType, eMsgPort eSenderPort , uint32_t nSessionID  )override ;
	bool onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID )override ;
	bool canStartGame()override ;
#if defined(GAME_365) || defined(GAME_panda)
	uint32_t getMaxTakeIn(){ return 1000; };
#endif 
	void sendResultToPlayerWhenDuringResultState(uint32_t nSessionID)override;
protected:
	ISitableRoomPlayer* doCreateSitableRoomPlayer() override;
protected:
	uint8_t m_nBankerIdx ;
	uint8_t m_nBetBottomTimes ;
	uint32_t m_nBankerCoinLimitForBet ; // 
	uint32_t m_nBaseBet ;
	uint8_t m_nResignBankerCtrl ;  // // 0 no niu leave banker , 1 lose to all  leave banker , 2 manual leave banker , 3 everybody ones , lunliu zuo zhuang;
	uint8_t m_nRateLevel;  
	bool m_isWillManualLeaveBanker ;

	Json::Value m_arrPlayers ;

	CAutoBuffer m_tGameResult;
};