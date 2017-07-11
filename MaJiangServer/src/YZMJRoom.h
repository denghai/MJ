#pragma once
#include "IMJRoom.h"
#include "CommonDefine.h"
#include "MJCard.h"
#include "YZMJPlayerCard.h"
class YZMJRoom
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
	void addSettle(stSettle& tSettle);
	void onPlayerHu(std::vector<uint8_t>& vHuIdx, uint8_t nCard, uint8_t nInvokeIdx)override;
	void onPlayerMingGang(uint8_t nIdx, uint8_t nCard, uint8_t nInvokeIdx);
	void onPlayerAnGang(uint8_t nIdx, uint8_t nCard);
	void onPlayerBuGang(uint8_t nIdx, uint8_t nCard);
	void onPlayerPeng(uint8_t nIdx, uint8_t nCard, uint8_t nInvokeIdx)override;
	void onPlayerMo(uint8_t nIdx)override;
	bool onPlayerApplyLeave(uint32_t nPlayerUID)override;
	uint8_t getRoomType()override { return eRoom_MJ_YangZhou; }
	void sendPlayersCardInfo(uint32_t nSessionID)override;
	std::shared_ptr<IGameRoomRecorder> createRoomRecorder()override;
	bool isInternalShouldClosedAll()override;
	bool isAnyPlayerPengOrHuThisCard(uint8_t nInvokeIdx, uint8_t nCard);
	void onAskForPengOrHuThisCard(uint8_t nInvokeIdx, uint8_t nCard, std::vector<uint8_t>& vWaitHuIdx, 
		std::vector<uint8_t>& vWaitPengGangIdx, bool& isNeedWaitEat);
protected:
	void getSubRoomInfo(Json::Value& jsSubInfo)override;
	void onPlayerZiMo(uint8_t nIdx, uint8_t nCard, Json::Value& jsDetail, std::vector<uint32_t> vWinCoin, std::vector<uint32_t> vLoseCoin,
		std::vector<uint32_t> vGangCoinGet, std::vector<uint32_t> vGangCoinOut);
	void settleInfoToJson(Json::Value& jsRealTime, std::vector<uint32_t> vGangCoinGet, std::vector<uint32_t> vGangCoinOut);

protected:
	CMJCard m_tPoker;
	bool m_isBankerHu;
	std::vector<stSettle> m_vSettle;

	bool m_isJingYuanZi; // 1:进园子；0:紧淌
	uint32_t m_nJingYuanZiFen; // 10、20、30
	bool m_isPeiZi; // 1:配子玩法；0:不是配子玩法
	bool m_isBaiBanPeiZi; // 1:白板配子玩法；0:不是白板配子玩法
	bool m_haveYiTiaoLong; // 1:有一条龙番；0:没有一条龙番
	bool m_have7Pair; // 1:7对可胡；0:7对不可胡

	uint8_t m_nBanZiCard; 

	uint32_t m_nInitCoin;
};
