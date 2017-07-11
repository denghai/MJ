#pragma once 
#include "IMJRoom.h"
#include "MJCard.h"
class HZMJRoom
	:public IMJRoom
{
public:
	bool init(IGameRoomManager* pRoomMgr, stBaseRoomConfig* pConfig, uint32_t nRoomID, Json::Value& vJsValue) override;
	bool onPlayerApplyLeave(uint32_t nPlayerUID)override;
	void startGame()override;
	void onGameEnd()override;
	void willStartGame()override;
	void onGameDidEnd()override;
	void onWaitPlayerAct(uint8_t nIdx, bool& isCanPass)override;
	bool isAnyPlayerPengOrHuThisCard(uint8_t nInvokeIdx, uint8_t nCard)override;;
	bool isAnyPlayerRobotGang(uint8_t nInvokeIdx, uint8_t nCard)override;;
	void onAskForPengOrHuThisCard(uint8_t nInvokeIdx, uint8_t nCard, std::vector<uint8_t>& vOutWaitHuIdx, std::vector<uint8_t>& vOutWaitPengGangIdx, bool& isNeedWaitEat)override;
	bool isGameOver()override;
	bool isCanGoOnMoPai()override;
	IMJPlayer* doCreateMJPlayer()override;
	IMJPoker* getMJPoker(){ return &m_tPoker; }
	void onPlayerChu(uint8_t nIdx, uint8_t nCard)override;
	void onPlayerHu(std::vector<uint8_t>& vHuIdx, uint8_t nCard, uint8_t nInvokeIdx)override;
	bool isHavePiao();
	bool isPlayerPiao(uint8_t nIdx);
	uint8_t getRoomType()override{ return eMJ_HZ; }
protected:
	uint8_t m_nContinueBankes;
	bool m_vCaiPiaoFlag[MAX_SEAT_CNT];
	CMJCard m_tPoker;
	uint8_t m_nLeasetLeftCard;
};