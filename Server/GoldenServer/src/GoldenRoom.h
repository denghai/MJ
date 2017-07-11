#pragma once
#include "ISitableRoom.h"
#include <json/json.h>
class CGoldenRoom
	:public ISitableRoom
{
public:
	CGoldenRoom();
	bool onFirstBeCreated(IRoomManager* pRoomMgr,uint32_t nRoomID , const Json::Value& vJsValue)override;
	void serializationFromDB(IRoomManager* pRoomMgr,stBaseRoomConfig* pConfig,uint32_t nRoomID , Json::Value& vJsValue )override;
	void prepareState()override ;
	bool onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID ) override;
	bool onMessage( Json::Value& prealMsg ,uint16_t nMsgType, eMsgPort eSenderPort , uint32_t nSessionID )override{ return ISitableRoom::onMessage(prealMsg,nMsgType,eSenderPort,nSessionID) ;};
	void roomInfoVisitor(Json::Value& vOutJsValue)override ;
	void sendRoomPlayersInfo(uint32_t nSessionID)override ;
	void setBankerIdx(uint8_t nIdx ){ m_nBankerIdx = nIdx ;}
	uint8_t getBankerIdx(){ return m_nBankerIdx ;}
	uint32_t getBaseBet(); // ji chu di zhu ;
	uint32_t getCurBet(){ return m_nCurBet ;}
	void onGameWillBegin()override ;
	void onGameDidEnd()override ;
	void onPlayerWillStandUp( ISitableRoomPlayer* pPlayer )override ;
	uint32_t getLeastCoinNeedForCurrentGameRound(ISitableRoomPlayer* pp)override ;
	uint8_t getRoomType()override{ return eRoom_Golden ;}
	void prepareCards()override;
	uint32_t coinNeededToSitDown()override;
	void caculateGameResult();
	uint8_t onPlayerAction(uint32_t nAct, uint32_t& nValue ,ISitableRoomPlayer* pPlayer );
	uint8_t getCurActIdx(){ return m_nCurActIdx ;}
	uint8_t informPlayerAct( bool bStepNext = false );
	bool isReachedMaxRound();
	bool onPlayerPK(ISitableRoomPlayer* pActPlayer , ISitableRoomPlayer* pTargetPlayer );
	void sendResultToPlayerWhenDuringResultState(uint32_t nSessionID)override;
protected:
	ISitableRoomPlayer* doCreateSitableRoomPlayer() override;
protected:
	uint8_t m_nBankerIdx ;
	uint32_t m_nCurBet ;
	uint32_t m_nBaseBet ;
	uint32_t m_nMailPool ;
	uint8_t  m_nCurActIdx ;
	uint16_t m_nBetRound ;
	uint8_t  m_nMaxBetRound;

	Json::Value m_arrPlayers ;
	Json::Value m_jsGameResult;
};