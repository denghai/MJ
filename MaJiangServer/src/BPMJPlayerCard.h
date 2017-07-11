#pragma once
#include "MJPlayerCard.h"
#include "json/json.h"
class BPMJPlayerCard
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
	bool onDoHu(bool isZiMo, bool isHaiDiLoaYue, uint8_t nCard, std::vector<uint16_t>& vHuTypes, int32_t& nFanCnt);
	uint8_t getSongGangIdx();
	void setSongGangIdx(uint8_t nIdx);
	bool canHuWitCard(uint8_t nCard)override;
	bool isHoldCardCanHu();
	bool pickoutHoldCard(uint8_t nCard);

	void setLouPeng(uint8_t nCard) { m_vLouPeng.push_back(nCard); }
	void clearLouPeng() { m_vLouPeng.clear(); }
	void setLouHu(bool isLouHu) { m_isLouHu = isLouHu; }
	bool checkHu(bool isHold, uint8_t nCard);

	void addSongGangIdx(uint8_t nCard, uint8_t nIdx, uint8_t eAct);
	std::vector<tSongGangIdx> getMingGangSongIdx() { return m_vMingGangSongIdx; }
	void addSongPengIdx(uint8_t nCard, uint8_t nIdx);
	std::vector<tSongGangIdx> getPengSongIdx() { return m_vPengSongIdx; }

	uint8_t checkMingPiao();

	void setIsBiMenHu(bool isBiMenHu) { m_isBiMenHu = isBiMenHu; }
	void setIsHuiPai(bool isHuiPai) { m_isHuiPai = isHuiPai; }
	void setIs7Pair(bool is7Pair) { m_is7Pair = is7Pair; }
	void setHuiCard(uint8_t nHuiCard) { m_nHuiCard = nHuiCard; }

	uint8_t checkJiaHu(uint8_t nCard, bool isZiMo);

	uint8_t isMingPiao(uint8_t nHuCard);

	bool getCanHuCards_1(std::set<uint8_t>& vCanHuCards);
	bool canHuWitCard_1(uint8_t nCard);

	bool isTingPai();
	bool getCanHuCards(std::set<uint8_t>& vCanHuCards);

	uint8_t getJiang() { return m_nJIang; }

	virtual bool canHoldCard7PairHu();

protected:
	bool checkDuiDuiHu();

	bool checkKeZhi(bool isHold, uint8_t nCard);
	bool takeKeZhi(VEC_CARD vCards);

	bool check7Pair();
	
protected:
	uint8_t m_nSongZhiGangIdx;

	std::vector<uint8_t> m_vLouPeng;
	bool m_isLouHu;
	std::vector<tSongGangIdx> m_vMingGangSongIdx;
	std::vector<tSongGangIdx> m_vPengSongIdx;

	bool m_is7Pair;
	bool m_isBiMenHu;
	bool m_isHuiPai;
	uint8_t m_nHuiCard;
};