#pragma once
#include "IMJRoom.h"
#include "CommonDefine.h"
#include "MJCard.h"
class SZMJRoom
	:public IMJRoom
{
public:
	bool init(IGameRoomManager* pRoomMgr, stBaseRoomConfig* pConfig, uint32_t nSeialNum, uint32_t nRoomID, Json::Value& vJsValue) override;
	void willStartGame()override;
	void startGame()override;
	void onGameDidEnd()override;
	void onGameEnd()override;
	void onPlayerMo(uint8_t nIdx)override;
	IMJPlayer* doCreateMJPlayer()override;
	IMJPoker* getMJPoker()override;
	bool isGameOver()override;
	void onPlayerBuHua(uint8_t nIdx, uint8_t nHuaCard);
	void onPlayerHu(std::vector<uint8_t>& vHuIdx, uint8_t nCard, uint8_t nInvokeIdx)override;
	bool onPlayerApplyLeave(uint32_t nPlayerUID)override;
	//uint8_t getRoomType()override { return eRoom_MJ_SuZhou; }
	void sendPlayersCardInfo(uint32_t nSessionID)override;
	bool isOneCirleEnd()override;
	void onPlayerMingGang(uint8_t nIdx, uint8_t nCard, uint8_t nInvokeIdx)override;
	void onPlayerChu(uint8_t nIdx, uint8_t nCard)override;
	uint8_t getZiMoHuaRequire();
	uint8_t getDianPaoHuHuaRequire();
protected:
	bool isFanBei() { return m_isFanBei; }
	void getSubRoomInfo(Json::Value& jsSubInfo)override;
	void onPlayerZiMo(uint8_t nIdx, uint8_t nCard, Json::Value& jsDetail);
	void packStartGameMsg(Json::Value& jsMsg)override;
	std::shared_ptr<IGameRoomRecorder> createRoomRecorder()override;
protected:
	bool m_isFanBei;
	bool m_isWillFanBei;
	bool m_isBankerHu;
	CMJCard m_tPoker;
	uint8_t m_nRuleMode; //1 代表两摸三冲 2 代表三摸四冲
};