#pragma once
#include "MJPlayerCard.h"
#include "json/json.h"
class HHMJPlayerCard
	:public MJPlayerCard
{
public:
	struct tSongGangIdx {
		uint8_t eAct;
		uint8_t nGangCard;
		uint8_t nIdx;
	};
public:
	bool canEatCard(uint8_t nCard, uint8_t& nWithA, uint8_t& nWithB);
	bool canPengWithCard(uint8_t nCard);
	void reset() override;
	bool getCardInfo(Json::Value& jsPeerCards);
	bool onDoHu(bool isZiMo, uint8_t nCard, bool bLanPai, bool haveYiTiaoLong, bool haveLuoDiLong, bool haveShouDaiLong, bool bShouDaiLong, std::vector<uint16_t>& vHuTypes, int32_t& nFanCnt);
	bool onDoTangZhiHu(std::vector<uint16_t>& vHuTypes, int32_t& nFanCnt, bool is7Pair);
	bool canHuWitCard(uint8_t nCard)override;
	bool isHoldCardCanHu();
	void pickoutHoldCard(uint8_t nCard);
	void setLouPeng(uint8_t nCard) { m_vLouPeng.push_back(nCard); }
	void clearLouPeng() { m_vLouPeng.clear(); }
	void setLouHu(bool isLouHu) { m_isLouHu = isLouHu; }
	bool checkHu(uint8_t nCard);
	void addSongGangIdx(uint8_t nCard, uint8_t nIdx, uint8_t eAct);
	std::vector<tSongGangIdx> getMingGangSongIdx() { return m_vMingGangSongIdx; }
	void addSongPengIdx(uint8_t nCard, uint8_t nIdx);
	std::vector<tSongGangIdx> getPengSongIdx() { return m_vPengSongIdx; }
	std::vector<tSongGangIdx> getGangTimes() { return m_vGangTimes; }
	void clearGangTimes() { m_vGangTimes.clear(); }
	void addGangTimes(uint8_t nCard, uint8_t nIdx);

	bool getChiPengGangFlag() { return m_isChiPengGang; }
	void setChiPengGangFlag() { m_isChiPengGang = true; }
	void addTangZhiCard(uint8_t nCard) { m_vChuedCardTangZhi.push_back(nCard); }
	void getTangZhiCard(VEC_CARD& vCards);
	bool isTangZhiHu();
	bool isTangZhiHu13();
	bool isTangZhiHu14();

	int check7Pair(bool isZiMo, uint8_t nHuCard);

	bool checkLuoDiLong();
	bool checkShouDaiLong();

	bool isAllZiPai(VEC_CARD vCards);

protected:
	bool checkDuiDuiHu();
	bool checkZhengGuiLan();
	//bool checkRuanLan();
	bool check7Stair();
	bool checkYiBanGao();
	/*bool checkLuoDiLong();
	bool checkShouDaiLong();*/
	bool checkHunYiSe();
	bool checkQingYiSe();

	uint8_t checkTZ_6Pair(bool is13);
	bool checkTZ_10Old();
	bool checkTZ_BadCard(bool is13);
	bool checkTZ_13();
	bool checkTZ_7Stair(bool is13);
	bool checkTZ_YiBanGao();
	bool checkTZ_7StairYiBanGao(bool is13);

	uint8_t get147258369(std::vector<uint8_t> vCard, std::vector<uint8_t>& vZiCards);
	uint8_t getRuanLan(std::vector<uint8_t> vCard, std::vector<uint8_t>& vZiCards);
	uint8_t getYiBanGao(std::vector<uint8_t> vCard, std::vector<uint8_t>& vZiCards);
	uint8_t haveYiTiaoLong(std::vector<uint8_t> vCard);

protected:
	std::vector<uint8_t> m_vLouPeng;
	bool m_isLouHu;
	
	std::vector<tSongGangIdx> m_vMingGangSongIdx;
	std::vector<tSongGangIdx> m_vPengSongIdx;
	std::vector<tSongGangIdx> m_vGangTimes; // Á¬¸Ü´ÎÊý

	bool m_isChiPengGang;
	VEC_CARD m_vChuedCardTangZhi;
};