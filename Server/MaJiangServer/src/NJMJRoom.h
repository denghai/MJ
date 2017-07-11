#pragma once 
#include "IMJRoom.h"
#include "CommonDefine.h"
#include "MJCard.h"
class NJMJRoom
	:public IMJRoom
{
public:
	struct stSettle
	{
		std::map<uint8_t,uint16_t> vWinIdxs;
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

	struct stChuedCards
	{
		uint8_t nCard;
		std::vector<uint8_t> vFollowedIdxs;
		void clear()
		{
			vFollowedIdxs.clear();
			nCard = 0;
		}

		void addChuedCard(uint8_t nChuCard, uint8_t nIdx)
		{
			if (nChuCard != nCard)
			{
				clear();
			}

			nCard = nChuCard;
			vFollowedIdxs.push_back(nIdx);
		}

		bool isInvokerFanQian(uint8_t& vTargetIdx)
		{
			if (vFollowedIdxs.size() == 4)
			{
				vTargetIdx = vFollowedIdxs.front();
			}

			return vFollowedIdxs.size() == 4;
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
	void onPlayerMo(uint8_t nIdx)override;
	void onPlayerBuHua(uint8_t nIdx, uint8_t nHuaCard );
	void onPlayerHuaGang(uint8_t nIdx, uint8_t nGangCard );
	void onPlayerPeng(uint8_t nIdx, uint8_t nCard, uint8_t nInvokeIdx) override;
	void onPlayerMingGang(uint8_t nIdx, uint8_t nCard, uint8_t nInvokeIdx)override;
	void onPlayerAnGang(uint8_t nIdx, uint8_t nCard)override;
	void onPlayerBuGang(uint8_t nIdx, uint8_t nCard)override;
	void onPlayerHu(std::vector<uint8_t>& vHuIdx, uint8_t nCard, uint8_t nInvokeIdx)override;
	void onPlayerChu(uint8_t nIdx, uint8_t nCard)override;
	bool isBiXiaHu();
	bool isCardByPenged( uint8_t nCard );
	bool canPlayerCardHuaGang( uint8_t nPlayerIdx , uint8_t nHuaCard );
	bool onPlayerApplyLeave(uint32_t nPlayerUID)override;
	uint8_t getRoomType()override { return eRoom_MJ_NanJing; }
	void sendPlayersCardInfo(uint32_t nSessionID)override;
	bool isInternalShouldClosedAll()override;
	bool isOneCirleEnd()override;
protected:
	void getSubRoomInfo(Json::Value& jsSubInfo)override;
	void addSettle(stSettle& tSettle );
	void onPlayerZiMo(uint8_t nIdx , uint8_t nCard , Json::Value& jsDetail );
	void settleInfoToJson(Json::Value& jsInfo);
	void packStartGameMsg(Json::Value& jsMsg)override;
	bool isHuaZa(){ return m_isEnableHuaZa; }
	bool isKuaiChong(){ return m_isKuaiChong; }
	void doAddOneRoundEntery();
protected:
	stChuedCards m_tChuedCards;
	std::vector<stSettle> m_vSettle;
	bool m_isBiXiaHu;
	bool m_isWillBiXiaHu;
	bool m_isBankerHu;
	bool m_isEnableBixiaHu;
	bool m_isEnableHuaZa;
	CMJCard m_tPoker;

	bool m_isKuaiChong;
	uint32_t m_nInitKuaiChongPool;
	uint32_t m_nKuaiChongPool;
};