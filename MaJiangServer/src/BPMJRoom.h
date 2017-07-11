#pragma once
#include "IMJRoom.h"
#include "CommonDefine.h"
#include "MJCard.h"
#include "BPMJPlayerCard.h"
class BPMJRoom
	:public IMJRoom
{
	struct stSettle
	{
		std::map<uint8_t, uint16_t> vWinIdxs;
		std::map<uint8_t, uint16_t> vLoseIdx;
		eMJActType eSettleReason;
		void addWin(uint8_t nIdx, uint16_t nWinCoin)
		{
			vWinIdxs[nIdx] = nWinCoin;
		}

		void addLose(uint8_t nIdx, uint16_t nLoseCoin)
		{
			vLoseIdx[nIdx] = nLoseCoin;
		}
	};
public:
	bool init(IGameRoomManager* pRoomMgr, stBaseRoomConfig* pConfig, uint32_t nSeialNum, uint32_t nRoomID, Json::Value& vJsValue) override;
	void willStartGame()override;
	void startGame()override;
	void onGameDidEnd()override;
	void onGameEnd()override;
	IMJPlayer* doCreateMJPlayer()override;
	IMJPoker* getMJPoker()override;
	bool isGameOver()override;
	void onPlayerHu(std::vector<uint8_t>& vHuIdx, uint8_t nCard, uint8_t nInvokeIdx)override;
	bool onPlayerApplyLeave(uint32_t nPlayerUID)override;
	uint8_t getRoomType()override { return eRoom_MJ_BeiPiao; }
	void sendPlayersCardInfo(uint32_t nSessionID)override;
	void onPlayerMingGang(uint8_t nIdx, uint8_t nCard, uint8_t nInvokeIdx)override;
	void onPlayerBuGang(uint8_t nIdx, uint8_t nCard);
	void onPlayerAnGang(uint8_t nIdx, uint8_t nCard);
	void onPlayerChu(uint8_t nIdx, uint8_t nCard)override;
	void onPlayerTing(uint8_t nIdx/*, std::vector<uint8_t> vecTingCards*/);
	void onPlayerPeng(uint8_t nIdx, uint8_t nCard, uint8_t nInvokeIdx);
	void onPlayerEat(uint8_t nIdx, uint8_t nCard, uint8_t nWithA, uint8_t nWithB, uint8_t nInvokeIdx);
	void onWaitPlayerAct(uint8_t nIdx, bool& isCanPass);
	bool isAnyPlayerPengOrHuThisCard(uint8_t nInvokeIdx, uint8_t nCard);
	void onAskForPengOrHuThisCard(uint8_t nInvokeIdx, uint8_t nCard, std::vector<uint8_t>& vWaitHuIdx, std::vector<uint8_t>& vWaitPengGangIdx, bool& isNeedWaitEat);
	std::shared_ptr<IGameRoomRecorder> createRoomRecorder()override;

	uint8_t getHuiCard() { return m_nHuiCard; }

protected:
	void calculateGangCoin(Json::Value& jsMsg, std::vector<uint32_t>& vGangCoinGet, std::vector<uint32_t>& vGangCoinOut, bool isOnePay);
	void getSubRoomInfo(Json::Value& jsSubInfo)override;
	void onPlayerZiMo(uint8_t nIdx, uint8_t nCard, Json::Value& jsDetail, std::vector<uint32_t> vLoseCoin, std::vector<uint32_t> vWinCoin,
		std::vector<uint32_t> vGangCoinGet, std::vector<uint32_t> vGangCoinOut);

	void checkJiGang(uint8_t nIdx, uint8_t nCard);
protected:
	std::vector<stSettle> m_vSettle;

	bool m_isOnePay;
	bool m_isBiMenHu;
	bool m_isJiaHu;
	bool m_isHuiPai;
	//bool m_isQiongHu;
	bool m_is7Pair;
	bool m_isJiXiaDan;
	bool m_isGangLiuLei;
	bool m_isBankerHu;
	CMJCard m_tPoker;

	uint8_t m_nHuiCard;
};