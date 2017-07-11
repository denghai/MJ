#pragma once

#include "MJPlayerCard.h"
#include "json/json.h"

using namespace std;

class TestHuiModule : public MJPlayerCard
{
	typedef vector<uint8_t> VEC_CARD;
public:
	TestHuiModule(uint8_t nHuiCard, bool is7Pair, VEC_CARD vEatCard, VEC_CARD vPengGang);

	bool testHuPai(VEC_CARD vCards);
	bool vecHu7Pair(uint8_t nHunCard, VEC_CARD vHuPai);

private:
	TestHuiModule();

	void getNeedHunNumToBePu(VEC_CARD typeVec, uint32_t needNum, uint32_t& needMinHunNum);
	uint8_t vecCanHu(uint32_t hunNum, VEC_CARD vHuPai);

	bool test3Combine(uint8_t nCard1, uint8_t nCard2, uint8_t nCard3);

	void eraseVector(uint8_t p, VEC_CARD& typeVec);
	void insertVector(uint8_t p, VEC_CARD& typeVec);
	void VecEliminateCopy(VEC_CARD vSrcPai, VEC_CARD& vPaiCopy, VEC_CARD vPaiRemove);

	uint8_t get7PairQueCnt19(VEC_CARD vCards[eCT_Max], VEC_CARD& vQueCards);

	bool have19(vector<VEC_CARD> vMyPaiCopy);
	uint32_t getLessQue3Men(vector<VEC_CARD> vMyPaiCopy);
	uint32_t getLessQueKeZhi(vector<VEC_CARD> vMyPaiCopy);
	uint32_t getLessQue19(vector<VEC_CARD> vMyPaiCopy, VEC_CARD& vRemoves);
	bool checkHuTiaojian(vector<VEC_CARD> vMyPaiCopy, uint32_t& curHunNum, bool& b3Men);

	bool haveKeZhiWithJiang(VEC_CARD vCard, uint32_t nHumNum, uint8_t nJiang);
	bool haveKeZhi(VEC_CARD vCard, uint32_t nHumNum);

	void takeoutCards(VEC_CARD& vCard, uint8_t nCard, uint32_t nCount);

	void fillCards(VEC_CARD vSrcCard, vector<VEC_CARD>& vCard);

private:
	uint8_t m_nHuiCard;
	bool m_is7Pair;
	bool m_have19;

	VEC_CARD m_vEatCard;
	VEC_CARD m_vPengGang;
};