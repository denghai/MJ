#pragma once
#include "IGameRoom.h"
#include "IGameRecorder.h"
#include "MJGameReplayManager.h"
class IMJPlayer;
struct stEnterRoomData;
class IMJPoker;
class IMJRoomState;
#define MAX_SEAT_CNT 4 
class IMJRoom;
class IMJRoomDelegate
{
public:
	virtual ~IMJRoomDelegate(){}
	virtual void onDidGameOver(IMJRoom* pRoom) {};
};

class CRobotDispatchStrategy;
class IMJRoom
	:public IGameRoom
{
public:
	typedef std::map<uint16_t, IMJRoomState*>	MAP_ID_ROOM_STATE;
public:
	IMJRoom() { setDelegate(nullptr); m_ptrGameRecorder = nullptr; }
	~IMJRoom();
	bool init(IGameRoomManager* pRoomMgr, stBaseRoomConfig* pConfig, uint32_t nSeialNum, uint32_t nRoomID, Json::Value& vJsValue) override;
	uint8_t checkPlayerCanEnter(stEnterRoomData* pEnterRoomPlayer)override;
	bool onPlayerEnter(stEnterRoomData* pEnterRoomPlayer)override;
	bool isRoomFull()override;
	void sendRoomInfo( uint32_t nSessionID )override;
	void setDelegate(IMJRoomDelegate* pDelegate){ m_pDelegate = pDelegate; }
	IMJRoomDelegate* getDelegate(){ return m_pDelegate; }
	bool isWaitPlayerActForever(){ return getDelegate() != nullptr; }
	bool isInternalShouldClosedAll()override{ return false; }
protected:
	virtual void sendPlayersCardInfo(uint32_t nSessionID );
	virtual void getSubRoomInfo(Json::Value& jsSubInfo){};
	virtual void packStartGameMsg(Json::Value& jsMsg );
public:
	uint32_t getRoomID()final;
	uint32_t getSeiralNum()final;
	stBaseRoomConfig* getRoomConfig()final{ return m_pRoomConfig; }
	void update(float fDelta) override;
	bool onMessage(stMsg* prealMsg, eMsgPort eSenderPort, uint32_t nPlayerSessionID)override;
	bool onMsg(Json::Value& prealMsg, uint16_t nMsgType, eMsgPort eSenderPort, uint32_t nSessionID) override;

	void sendRoomMsg(Json::Value& prealMsg, uint16_t nMsgType )final;
	void sendMsgToPlayer(Json::Value& prealMsg, uint16_t nMsgType, uint32_t nSessionID )final;
	void sendMsgToPlayer(stMsg* pmsg, uint16_t nLen, uint32_t nSessionID);
	bool sitdown(IMJPlayer* pPlayer , uint8_t nIdx );
	bool standup( uint32_t nUID );
	uint8_t getSeatCnt();
	IMJPlayer* getMJPlayerBySessionID(uint32_t nSessionid );
	IMJPlayer* getMJPlayerByUID( uint32_t nUID );
	IMJPlayer* getMJPlayerByIdx( uint8_t nIdx );

	virtual void startGame();
	virtual void willStartGame();
	virtual void onGameEnd();
	virtual void onGameDidEnd();
	virtual bool canStartGame();

	void goToState(IMJRoomState* pTargetState, Json::Value* jsValue = nullptr);
	void goToState(uint16_t nStateID, Json::Value* jsValue = nullptr);
	uint8_t getBankerIdx();
	void setBankIdx(uint8_t nIdx);
	void onPlayerSetReady( uint8_t nIdx );
	// mj function ;
	virtual void onWaitPlayerAct(uint8_t nIdx, bool& isCanPass);
	virtual uint8_t getAutoChuCardWhenWaitActTimeout(uint8_t nIdx);
	virtual uint8_t getAutoChuCardWhenWaitChuTimeout(uint8_t nIdx);
	virtual void onPlayerMo(uint8_t nIdx);
	virtual void onPlayerPeng(uint8_t nIdx, uint8_t nCard, uint8_t nInvokeIdx);
	virtual void onPlayerEat(uint8_t nIdx, uint8_t nCard, uint8_t nWithA, uint8_t nWithB, uint8_t nInvokeIdx);
	virtual void onPlayerMingGang(uint8_t nIdx, uint8_t nCard, uint8_t nInvokeIdx);
	virtual void onPlayerAnGang(uint8_t nIdx, uint8_t nCard);
	virtual void onPlayerBuGang(uint8_t nIdx, uint8_t nCard);
	virtual void onPlayerHu(std::vector<uint8_t>& vHuIdx, uint8_t nCard, uint8_t nInvokeIdx);
	virtual void onPlayerChu(uint8_t nIdx, uint8_t nCard);
	virtual bool isAnyPlayerPengOrHuThisCard( uint8_t nInvokeIdx , uint8_t nCard );
	virtual void onAskForPengOrHuThisCard(uint8_t nInvokeIdx, uint8_t nCard, std::vector<uint8_t>& vOutWaitHuIdx, std::vector<uint8_t>& vOutWaitPengGangIdx, bool& isNeedWaitEat);
	virtual bool isAnyPlayerRobotGang(uint8_t nInvokeIdx, uint8_t nCard);
	virtual void onAskForRobotGang(uint8_t nInvokeIdx, uint8_t nCard, std::vector<uint8_t>& vOutCandinates);
	virtual uint8_t getNextActPlayerIdx(uint8_t nCurActIdx);
	virtual bool isGameOver();
	virtual bool isCanGoOnMoPai();
	virtual IMJPlayer* doCreateMJPlayer() = 0;
	virtual IMJPoker* getMJPoker() = 0;
	virtual void onPlayerLouHu( uint8_t nIdx , uint8_t nInvokerIdx );
	virtual void onPlayerLouPeng(uint8_t nIdx, uint32_t nLouCard );
	virtual bool isHaveLouHu() { return true; };
	virtual bool isHaveLouPeng() { return false; }
	bool isOneCirleEnd()override{ return true; }
	IGameRoomManager* getRoomMgr(){ return m_pRoomMgr; }
	IMJRoomState* getCurRoomState(){ return m_pCurState; }
	uint32_t getCoinNeedToSitDown();
	CRobotDispatchStrategy* getRobotDispatchStrage(){ return m_pRobotDispatchStrage; }

	// tuo guan 
	void onCheckTrusteeForWaitPlayerAct( uint8_t nIdx , bool isMayHu);

	void onCheckTrusteeForHuOtherPlayerCard(std::vector<uint8_t> vPlayerIdx, uint8_t nTargetCard );
	void onPlayerTrusteedStateChange( uint8_t nPlayerIdx , bool isTrusteed );
	std::shared_ptr<IGameRoomRecorder> getRoomRecorder()override { return m_ptrGameRecorder;  }
	std::shared_ptr<MJReplayGame> getGameReplay() { return m_ptrGameReplay; }

	void changeIdx();
protected:
	bool addRoomState(IMJRoomState* pState);
	void setInitState(IMJRoomState* pState);
	virtual std::shared_ptr<IGameRoomRecorder> createRoomRecorder() = 0 ;
protected:
	IMJPlayer* m_vMJPlayers[MAX_SEAT_CNT];
	MAP_ID_ROOM_STATE m_vRoomStates;
	IMJRoomState* m_pCurState;
	IGameRoomManager* m_pRoomMgr;
	stBaseRoomConfig* m_pRoomConfig;
	uint32_t m_nRoomID;
	uint32_t m_nSeiralNum;
	uint8_t m_nBankerIdx;
	IMJRoomDelegate* m_pDelegate;

	std::shared_ptr<MJReplayGame> m_ptrGameReplay;

	CRobotDispatchStrategy* m_pRobotDispatchStrage;

	std::shared_ptr<IGameRoomRecorder> m_ptrGameRecorder;
};