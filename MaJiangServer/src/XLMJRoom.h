#pragma once 
#include "IMJRoom.h"
#include "MJCard.h"
#include "XLMJSettle.h"
class XLMJRoom
	:public IMJRoom
{
public:
	typedef std::vector<ISettle*> VEC_SETTLE;
public:
	bool init(IGameRoomManager* pRoomMgr, stBaseRoomConfig* pConfig, uint32_t nSiealNum, uint32_t nRoomID, Json::Value& vJsValue) override;
	bool onPlayerApplyLeave(uint32_t nPlayerUID)override;
	uint8_t checkPlayerCanEnter(stEnterRoomData* pEnterRoomPlayer)override;
	/*void onWaitPlayerAct(uint8_t nIdx, bool& isCanPass)override;*/
	void onPlayerMingGang(uint8_t nIdx, uint8_t nCard, uint8_t nInvokeIdx)override;
	void onPlayerAnGang(uint8_t nIdx, uint8_t nCard)override;
	void onPlayerBuGang(uint8_t nIdx, uint8_t nCard)override;
	void onPlayerHu(std::vector<uint8_t>& vHuIdx, uint8_t nCard, uint8_t nInvokeIdx)override;
	bool isAnyPlayerPengOrHuThisCard(uint8_t nInvokeIdx, uint8_t nCard)override;
	bool isAnyPlayerRobotGang(uint8_t nInvokeIdx, uint8_t nCard)override;
	void onAskForPengOrHuThisCard(uint8_t nInvokeIdx, uint8_t nCard, std::vector<uint8_t>& vOutWaitHuIdx, std::vector<uint8_t>& vOutWaitPengGangIdx, bool& isNeedWaitEat)override;
	void onAskForRobotGang(uint8_t nInvokeIdx, uint8_t nCard, std::vector<uint8_t>& vOutCandinates)override;
	IMJPlayer* doCreateMJPlayer()override;
	IMJPoker* getMJPoker()override;
	uint8_t getNextActPlayerIdx(uint8_t nCurActIdx);
	uint8_t getRoomType()override{ return eMJ_BloodRiver; }
	void willStartGame()override;
	void startGame()override;
	void onGameEnd()override;
	void onGameDidEnd()override;
	bool isGameOver()override;
	bool getWaitSupplyCoinPlayerIdxs(std::vector<uint8_t>& vOutWaitSupplyIdx);
	void infoPlayerSupplyCoin(std::vector<uint8_t>& vOutWaitSupplyIdx );
	uint8_t getAutoChuCardWhenWaitActTimeout(uint8_t nIdx)override;
	uint8_t getAutoChuCardWhenWaitChuTimeout(uint8_t nIdx)override;
protected:
	void sendPlayersCardInfo(uint32_t nSessionID)override;
	void onPlayerZiMo(uint8_t nIdx, uint8_t nCard);
	void doChaHuaZhu(std::vector<uint8_t>& vHuaZhu);
	void doChaDaJiao(std::vector<uint8_t>& vHuaZhu );
	void addSettle(ISettle* pSettle );
	void removeSettle(ISettle* pSettle );
	void clearAllSettle();
	uint32_t getBaseBet();
	void giveBackGangWin( uint8_t nIdx );
	void sendPlayerDetailBillInfo(uint8_t nIdx );
	virtual bool canKouPlayerCoin(uint8_t nPlayerIdx);
protected:
	VEC_SETTLE m_vSettleInfos;
	CMJCard m_tPoker;
};