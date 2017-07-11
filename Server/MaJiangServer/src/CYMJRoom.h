#pragma once
#include "IMJRoom.h"
#include "CommonDefine.h"
#include "MJCard.h"
#include "CYMJPlayerCard.h"
class CYMJRoom
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
	uint8_t getRoomType()override { return eRoom_MJ_CaoYang; }
	void sendPlayersCardInfo(uint32_t nSessionID)override;
	void onPlayerMingGang(uint8_t nIdx, uint8_t nCard, uint8_t nInvokeIdx)override;
	void onPlayerAnGang(uint8_t nIdx, uint8_t nCard);
	void onPlayerBuGang(uint8_t nIdx, uint8_t nCard);
	void onPlayerChu(uint8_t nIdx, uint8_t nCard)override;
	void onPlayerTing(uint8_t nIdx/*, std::vector<uint8_t> vecTingCards*/);
	void onPlayerPeng(uint8_t nIdx, uint8_t nCard, uint8_t nInvokeIdx);
	void onPlayerMoBao(uint8_t nIdx, uint8_t nBaoCard);
	void onPlayerEat(uint8_t nIdx, uint8_t nCard, uint8_t nWithA, uint8_t nWithB, uint8_t nInvokeIdx);
	uint8_t getBaoCard() { return m_nBaoCard; }
	void onWaitPlayerAct(uint8_t nIdx, bool& isCanPass);
	void checkTing(CYMJPlayerCard* pMJCard, std::map<uint8_t, std::set<uint8_t>>& tingCards);
	bool isAnyPlayerPengOrHuThisCard(uint8_t nInvokeIdx, uint8_t nCard);
	void onAskForPengOrHuThisCard(uint8_t nInvokeIdx, uint8_t nCard, std::vector<uint8_t>& vWaitHuIdx, std::vector<uint8_t>& vWaitPengGangIdx, bool& isNeedWaitEat);
	void setIsChiAndTing() { m_isChiAndTing = true; }
	std::shared_ptr<IGameRoomRecorder> createRoomRecorder()override;
	void setHuanBaoFlag(bool bFlag) { m_bHuanBaoCardFlag = bFlag; }
	bool getHuanBaoFlag() { return m_bHuanBaoCardFlag; }

	uint8_t getValidBaoCard();
	bool isBaoCardValid(uint8_t nBaoCard);

	void onPlayerMo(uint8_t nIdx);

	bool getModeLouBao() { return m_isLouBao; }
protected:
	void calculateGangCoin(Json::Value& jsMsg, std::vector<uint32_t>& vGangCoinGet, std::vector<uint32_t>& vGangCoinOut, bool isOnePay);
	void getSubRoomInfo(Json::Value& jsSubInfo)override;
	void onPlayerZiMo(bool isLoubao, uint8_t nIdx, uint8_t nCard, Json::Value& jsDetail, std::vector<uint32_t> vLoseCoin, std::vector<uint32_t> vWinCoin,
		std::vector<uint32_t> vGangCoinGet, std::vector<uint32_t> vGangCoinOut);
protected:
	bool m_isOnePay;
	bool m_isBiMenHu;
	bool m_isJiaHu;
	bool m_isBankerHu;
	bool m_isQingYiSe;
	bool m_isLouBao;
	CMJCard m_tPoker;
	
	bool m_isChiAndTing;
	uint8_t m_nBaoCard;
	bool m_bHuanBaoCardFlag;
};