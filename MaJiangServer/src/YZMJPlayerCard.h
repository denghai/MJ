#pragma once
#include "MJPlayerCard.h"
#include "json/json.h"
class YZMJPlayerCard
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
	bool canMingGangWithCard(uint8_t nCard);
	void reset() override;
	bool getCardInfo(Json::Value& jsPeerCards);
	bool onDoHu(bool isZiMo, uint8_t nCard, std::vector<uint16_t>& vHuTypes, int32_t& nFanCnt);
	bool canHuWitCard(uint8_t nCard)override;
	bool isHoldCardCanHu();
	//bool getHoldCardThatCanBuGang(VEC_CARD& vGangCards)override;
	void setLouPeng(uint8_t nCard) { m_vLouPeng.push_back(nCard); }
	void clearLouPeng() { m_vLouPeng.clear(); }
	void setLouHu(bool isLouHu, uint8_t nHuCard);
	bool checkHu(uint8_t nCard);

	void addSongPengIdx(uint8_t nCard, uint8_t nIdx);
	std::vector<tSongGangIdx> getPengSongIdx() { return m_vPengSongIdx; }

	bool testHuPai(uint8_t nHuCard, VEC_CARD vCards, uint8_t nHunCard);

	void setBanZiCard(uint8_t nBanZiCard);

	void addCanNotGang(uint8_t nCard);
	bool haveThreeBanZi();

	uint8_t check7Pair(uint8_t nHuCard);
	
	void checkCanHuPao();

	void setHaveYiTiaoLong() { m_haveYiTiaoLong = true; }
	void setHave7Pair() { m_have7Pair = true; }

	virtual bool canHoldCard7PairHu();

	std::vector<uint8_t> getCanNotGang() { return m_vCanNotGang; }

protected:
	void pickoutHoldCard(uint8_t nCard);

	uint8_t haveYiTiaoLong(std::vector<uint8_t> vCard);
	bool checkYiTiaoLong();

	bool checkDuiDuiHu();
	bool checkHunYiSe(bool haveHun);
	bool checkQingYiSe(bool haveHun);
	bool isAllFeng();

	bool vecHu7Pair(uint8_t nHunCard, VEC_CARD vHuPai);
	void getNeedHunNumToBePu(VEC_CARD typeVec, uint32_t needNum, uint32_t& needMinHunNum);
	bool vecCanHu(uint32_t hunNum, VEC_CARD vHuPai);

	bool test3Combine(uint8_t nCard1, uint8_t nCard2, uint8_t nCard3);

	void eraseVector(uint8_t p, VEC_CARD& typeVec);
	void insertVector(uint8_t p, VEC_CARD& typeVec);
	void VecEliminateCopy(VEC_CARD vSrcPai, VEC_CARD& vPaiCopy, VEC_CARD vPaiRemove);

protected:
	VEC_CARD m_vLouPeng;
	bool m_isLouHu;
	VEC_CARD m_vLouHuCard;

	std::vector<tSongGangIdx> m_vPengSongIdx;

	std::vector<uint8_t> m_vCanNotGang;

	bool m_bBanZi;
	uint8_t m_nBanCard; // 搬子
	uint8_t m_nHunCard; // 配子（百搭）
	bool m_bCanHuPao; // 摸到过配子只能自摸
	bool m_haveYiTiaoLong;
	bool m_have7Pair;
};