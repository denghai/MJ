#include "TestHuiModule.h"
#include "log4z.h"
#include "MJCard.h"
#include "assert.h"

#define MJPAI_HUNMAX 4

TestHuiModule::TestHuiModule(uint8_t nHuiCard, bool is7Pair, VEC_CARD vEatCard, VEC_CARD vPengGang)
{
	m_nHuiCard = nHuiCard;
	m_is7Pair = is7Pair;

	m_vEatCard.insert(m_vEatCard.end(), vEatCard.begin(), vEatCard.end());
	m_vPengGang.insert(m_vPengGang.end(), vPengGang.begin(), vPengGang.end());
}

void TestHuiModule::fillCards(VEC_CARD vSrcCard, vector<VEC_CARD>& vCard)
{
	for (auto nCard : vSrcCard)
	{
		if (nCard == m_nHuiCard)
		{
			vCard[eCT_Hun].push_back(nCard);
			continue;
		}
		auto eType = card_Type(nCard);
		if (eType < eCT_Wan || eType > eCT_Jian)
		{
			LOGFMTE("test hu failed, error card=%d", nCard);
			return;
		}
		//vCardCopy[eType].push_back(nCard);
		addCardToVecAsc(vCard[eType], nCard);
	}
}

//发的牌
bool TestHuiModule::testHuPai(VEC_CARD vCards)
{
	if (vecHu7Pair(m_nHuiCard, vCards))
	{
		return true;
	}

	std::vector<VEC_CARD> vCardCopy;

	vCardCopy.resize(eCT_Max);
	fillCards(vCards, vCardCopy);

	uint32_t curHunNum = vCardCopy[eCT_Hun].size();

	m_have19 = false;
	if (have19(vCardCopy))
	{
		m_have19 = true;
	}

	bool b3Men = false;
	if (!checkHuTiaojian(vCardCopy, curHunNum, b3Men))
	{
		return false;
	}
	if (!b3Men)
	{
		return false;
	}
	/*if (curHunNum > 3)
	{
	LOGFMTI("Four Hun!!\n");
	return true;
	}*/

	uint32_t nTotoalToPuNeedNum = 0;

	std::vector<uint32_t> vToPuNeedNum;
	vToPuNeedNum.resize(eCT_Max);

	for (int i = eCT_Wan; i < eCT_Hun; ++i)
	{
		vToPuNeedNum[i] = MJPAI_HUNMAX;
		getNeedHunNumToBePu(vCardCopy[i], 0, vToPuNeedNum[i]);

		LOGFMTI("Type[%d] ToPuNeedNum:%d", i, vToPuNeedNum[i]);

		nTotoalToPuNeedNum += vToPuNeedNum[i];
	}

	for (int i = eCT_Wan; i <= eCT_Hun; ++i)
	{
		uint32_t needHunNum = nTotoalToPuNeedNum - vToPuNeedNum[i];
		if (needHunNum <= curHunNum)
		{
			LOGFMTI("jiang in type[%d], needHunNum:%d curHunNum:%d", i, needHunNum, curHunNum);
			uint8_t nJiang = vecCanHu(curHunNum - needHunNum, vCardCopy[i]);
			if (nJiang != 0)
			{
				// 检查三门
				if (!b3Men && curHunNum - needHunNum < 2)
				{
					continue;
				}
				// 检查刻字是否满足
				if (m_vPengGang.size() == 0)
				{
					for (int j = eCT_Wan; j < eCT_Hun; ++j)
					{
						if (i == j)
						{
							if (haveKeZhiWithJiang(vCardCopy[j], vToPuNeedNum[j] - 1, nJiang))
							{
								return true;
							}
						}
						else if (haveKeZhi(vCardCopy[j], vToPuNeedNum[j]))
						{
							return true;
						}
					}
				}
				else
				{
					return true;
				}
			}
		}
	}

	return false;
}

bool TestHuiModule::haveKeZhiWithJiang(VEC_CARD vCardCopy, uint32_t nHumNum, uint8_t nJiang)
{
	if (card_Type(nJiang) == eCT_Jian)
	{
		return true;
	}

	auto nCount = std::count(vCardCopy.begin(), vCardCopy.end(), nJiang);
	if (nCount >= 2)
	{
		takeoutCards(vCardCopy, nJiang, 2);
	}
	else if (nCount == 1)
	{
		if (nHumNum <= 1)
		{
			return false;
		}
		takeoutCards(vCardCopy, nJiang, 1);
		nHumNum--;
	}
	else
	{
		if (nHumNum < 2)
		{
			return false;
		}
		nHumNum -= 2;
	}

	return haveKeZhi(vCardCopy, nHumNum);
}

bool TestHuiModule::haveKeZhi(VEC_CARD vCard, uint32_t nHumNum)
{
	for (int i = 0; i < vCard.size(); i++)
	{
		VEC_CARD vCardCopy;
		vCardCopy.insert(vCardCopy.begin(), vCard.begin(), vCard.end());

		auto type = card_Type(vCardCopy[i]);
		if (type != eCT_Jian)
		{
			auto nCount = std::count(vCardCopy.begin(), vCardCopy.end(), vCardCopy[i]);
			if (nCount >= 3)
			{
				takeoutCards(vCardCopy, vCardCopy[i], 3);
				uint32_t needMinHunNum = MJPAI_HUNMAX;
				getNeedHunNumToBePu(vCardCopy, 0, needMinHunNum);
				if (nHumNum >= needMinHunNum)
				{
					return true;
				}
			}
			else if (nCount == 2)
			{
				if (nHumNum < 1)
				{
					continue;
				}
				takeoutCards(vCardCopy, vCardCopy[i], 2);
				uint32_t needMinHunNum = MJPAI_HUNMAX;
				getNeedHunNumToBePu(vCardCopy, 0, needMinHunNum);
				if (nHumNum - 1 >= needMinHunNum)
				{
					return true;
				}
			}
			else
			{
				if (nHumNum < 2)
				{
					continue;
				}
				takeoutCards(vCardCopy, vCardCopy[i], 1);
				uint32_t needMinHunNum = MJPAI_HUNMAX;
				getNeedHunNumToBePu(vCardCopy, 0, needMinHunNum);
				if (nHumNum - 2 >= needMinHunNum)
				{
					return true;
				}
			}
		} 
		else
		{
			auto nCount = std::count(vCardCopy.begin(), vCardCopy.end(), vCardCopy[i]);
			if (nCount >= 2)
			{
				takeoutCards(vCardCopy, vCardCopy[i], nCount);
				uint32_t needMinHunNum = MJPAI_HUNMAX;
				getNeedHunNumToBePu(vCardCopy, 0, needMinHunNum);
				if (nHumNum >= needMinHunNum)
				{
					return true;
				}
			}
			else
			{
				if (nHumNum < 1)
				{
					continue;
				}
				takeoutCards(vCardCopy, vCardCopy[i], 1);
				uint32_t needMinHunNum = MJPAI_HUNMAX;
				getNeedHunNumToBePu(vCardCopy, 0, needMinHunNum);
				if (nHumNum - 1 >= needMinHunNum)
				{
					return true;
				}
			}
		}
	}
	return false;
}

void TestHuiModule::takeoutCards(VEC_CARD& vCard, uint8_t nCard, uint32_t nCount)
{
	auto it = vCard.begin();
	while (it != vCard.end() && nCount > 0)
	{
		if (*it == nCard)
		{
			it = vCard.erase(it);
			nCount--;
		}
		else
		{
			++it;
		}
	}
}

bool TestHuiModule::vecHu7Pair(uint8_t nHunCard, VEC_CARD vHuPai)
{
	if (!m_is7Pair)
	{
		return false;
	}

	if (vHuPai.size() != 14)
	{
		return false;
	}

	auto nHunCount = std::count(vHuPai.begin(), vHuPai.end(), nHunCard);

	VEC_CARD vCardCopy[eCT_Max];
	for (auto nCard : vHuPai)
	{
		auto eType = card_Type(nCard);
		if (eType < eCT_Wan || eType > eCT_Jian)
		{
			LOGFMTE("test hu failed, error card=%d", nCard);
			continue;
		}
		if (nCard == nHunCard)
		{
			continue;
		}
		vCardCopy[eType].push_back(nCard);
	}

	VEC_CARD vQueCards;
	auto nQueCnt = get7PairQueCnt(vCardCopy);//get7PairQueCnt19(vCardCopy, vQueCards);
	if (nQueCnt == 0)
	{
		return true;
	}
	if (nQueCnt <= nHunCount)
	{
		//if (vQueCards.size() > 0)
		//{
		//	if (vQueCards.size() == 1 && m_nHuiCard == vQueCards.at(0))
		//	{
		//		return true;
		//	}
		//}
		//else
		//{
			return true;
		//}
	}
	return false;
}

//成为整扑需要的癞子个数
void TestHuiModule::getNeedHunNumToBePu(VEC_CARD typeVec, uint32_t needNum, uint32_t& needMinHunNum)
{
	if (needMinHunNum == 0) return;
	if (needNum >= needMinHunNum) return;

	int vSize = typeVec.size();
	if (vSize == 0)
	{
		needMinHunNum = min(needNum, needMinHunNum);
		return;
	}
	else if (vSize == 1)
	{
		needMinHunNum = min(needNum + 2, needMinHunNum);
		return;
	}
	else if (vSize == 2)
	{
		auto nCard1 = typeVec.at(0);
		auto nCard2 = typeVec.at(1);

		//如果后一个是东西南北中发白  不可能是出现顺牌
		auto eType = card_Type(nCard2);
		if (eType == eCT_Feng || eType == eCT_Jian)
		{
			if (nCard1 == nCard2)
			{
				needMinHunNum = min(needNum + 1, needMinHunNum);
			}
			else
			{
				needMinHunNum = min(needNum + MJPAI_HUNMAX, needMinHunNum);
			}
		}
		else if (nCard2 - nCard1 < 3 && eType == card_Type(nCard1))
		{
			needMinHunNum = min(needNum + 1, needMinHunNum);
		}
		return;
	}
	//大于等于3张牌
	auto nCard1 = typeVec.at(0);
	auto nCard2 = typeVec.at(1);
	auto nCard3 = typeVec.at(2);

	VEC_CARD vVecCopy;
	VEC_CARD vRemove;

	//第一个自己一扑
	if (needNum + 2 < needMinHunNum)
	{
		vRemove.push_back(nCard1);
		VecEliminateCopy(typeVec, vVecCopy, vRemove);

		getNeedHunNumToBePu(vVecCopy, needNum + 2, needMinHunNum);
	}

	//第一个跟其它的一个一扑
	if (needNum + 1 < needMinHunNum)
	{
		//nCard1是风
		auto eType = card_Type(nCard1);
		if (eType == eCT_Feng || eType == eCT_Jian)
		{
			if (nCard1 == nCard2)
			{
				vRemove.clear();
				vRemove.push_back(nCard1);
				vRemove.push_back(nCard2);
				VecEliminateCopy(typeVec, vVecCopy, vRemove);

				getNeedHunNumToBePu(vVecCopy, needNum + 1, needMinHunNum);
			}
		}
		else
		{
			for (unsigned int i = 1; i < typeVec.size(); i++)
			{
				if (needNum + 1 >= needMinHunNum)
				{
					break;
				}
				nCard2 = typeVec[i];
				// 455567这里可结合的可能为 45 46 否则是45 45 45 46
				// 如果当前的value不等于下一个value则和下一个结合避免重复
				if (i + 1 != typeVec.size())
				{
					nCard3 = typeVec[i + 1];
					if (nCard3 == nCard2)
					{
						continue;
					}
				}

				if (nCard2 - nCard1 < 3)
				{
					vRemove.clear();
					vRemove.push_back(nCard1);
					vRemove.push_back(nCard2);
					VecEliminateCopy(typeVec, vVecCopy, vRemove);

					getNeedHunNumToBePu(vVecCopy, needNum + 1, needMinHunNum);
				}
				else
				{
					break;
				}
			}
		}
	}

	//第一个和其它两个一扑
	//后面间隔两张张不跟前面一张相同222234 
	//可能性为222 234
	for (unsigned int i = 1; i < typeVec.size(); i++)
	{
		if (needNum >= needMinHunNum) break;
		nCard2 = typeVec[i];
		if (i + 2 < typeVec.size())
		{
			if (typeVec[i + 2] == nCard2) continue;
		}
		for (unsigned int j = i + 1; j < typeVec.size(); j++)
		{
			if (needNum >= needMinHunNum) break;
			nCard3 = typeVec[j];
			if (nCard1 == nCard3)
			{
				LOGFMTE("error!!\n");
			}

			if (j + 1 < typeVec.size())
			{
				if (nCard3 == typeVec[j + 1])
				{
					continue;
				}
			}
			if (test3Combine(nCard1, nCard2, nCard3))
			{
				vRemove.clear();
				vRemove.push_back(nCard1);
				vRemove.push_back(nCard2);
				vRemove.push_back(nCard3);
				VecEliminateCopy(typeVec, vVecCopy, vRemove);

				getNeedHunNumToBePu(vVecCopy, needNum, needMinHunNum);
			}
			//4556
		}
	}
}

//判断某一类型能否成为整扑一将
uint8_t TestHuiModule::vecCanHu(uint32_t hunNum, VEC_CARD vHuPai)
{
	LOGFMTI("check can hu...");
	int huSize = vHuPai.size();
	//DebugPlayer(HUVEC);
	if (huSize <= 0)
	{
		if (hunNum >= 2)
		{
			return m_nHuiCard;
		}
		return 0;
	}

	VEC_CARD huPaiCopy;
	VEC_CARD vRemovePai;
	for (auto i = 0; i < huSize; ++i)
	{
		if (i == huSize - 1) // 最后一张
		{
			LOGFMTD("last iterator!!");
			if (hunNum > 0)
			{
				vRemovePai.clear();
				vRemovePai.push_back(vHuPai[i]);
				VecEliminateCopy(vHuPai, huPaiCopy, vRemovePai);

				uint32_t needMinHunNum = MJPAI_HUNMAX;
				getNeedHunNumToBePu(huPaiCopy, 0, needMinHunNum);
				if (needMinHunNum <= hunNum - 1)
				{
					LOGFMTD("T:%d V:%d\n", card_Type(vHuPai[i]), card_Value(vHuPai[i]));
					return vHuPai[i];
					//for (auto p : huPaiCopy)
					//{
					//	VEC_CARD vPai;
					//	VEC_CARD vRemove;
					//	//vRemove.push_back(p);
					//	VecEliminateCopy(huPaiCopy, vPai, vRemove);
					//	auto hNum = hunNum - needMinHunNum;
					//	for (int i = 0; i < hNum; ++i)
					//	{
					//		vPai.push_back(m_nHuiCard);
					//	}
					//	TestHuiModule huiModule(m_nHuiCard, m_is7Pair);
					//	huiModule.testHuPai(vPai);
					//}
				}
			}
		}
		else
		{
			if (i + 2 == huSize || vHuPai[i] != vHuPai[i + 2])
			{
				LOGFMTD("other iterator!!\n");
				if (vHuPai[i] == vHuPai[i + 1])
				{
					LOGFMTD("%d %d\n", card_Type(vHuPai[i]), card_Value(vHuPai[i]));

					vRemovePai.clear();
					vRemovePai.push_back(vHuPai[i]);
					vRemovePai.push_back(vHuPai[i + 1]);
					VecEliminateCopy(vHuPai, huPaiCopy, vRemovePai);

					uint32_t needMinHunNum = MJPAI_HUNMAX;
					getNeedHunNumToBePu(huPaiCopy, 0, needMinHunNum);
					if (needMinHunNum <= hunNum)
					{
						LOGFMTD("T:%d V:%d\n", card_Type(vHuPai[i]), card_Value(vHuPai[i]));
						return vHuPai[i];
						//for (auto p : huPaiCopy)
						//{
						//	VEC_CARD vPai;
						//	VEC_CARD vRemove;
						//	//vRemove.push_back(p);
						//	VecEliminateCopy(huPaiCopy, vPai, vRemove);
						//	auto hNum = hunNum - needMinHunNum;
						//	for (int i = 0; i < hNum; ++i)
						//	{
						//		vPai.push_back(m_nHuiCard);
						//	}
						//	TestHuiModule huiModule(m_nHuiCard, m_is7Pair);
						//	huiModule.testHuPai(vPai);
						//}
					}
				}
			}

			if (hunNum > 0 && vHuPai[i] != vHuPai[i + 1])
			{
				LOGFMTD("%d %d\n", card_Type(vHuPai[i]), card_Value(vHuPai[i]));

				vRemovePai.clear();
				vRemovePai.push_back(vHuPai[i]);
				VecEliminateCopy(vHuPai, huPaiCopy, vRemovePai);

				uint32_t needMinHunNum = MJPAI_HUNMAX;
				getNeedHunNumToBePu(huPaiCopy, 0, needMinHunNum);
				if (needMinHunNum <= hunNum - 1)
				{
					LOGFMTD("T:%d V:%d\n", card_Type(vHuPai[i]), card_Value(vHuPai[i]));
					return vHuPai[i];
					//for (auto p : huPaiCopy)
					//{
					//	VEC_CARD vPai;
					//	VEC_CARD vRemove;
					//	//vRemove.push_back(p);
					//	VecEliminateCopy(huPaiCopy, vPai, vRemove);
					//	auto hNum = hunNum - needMinHunNum;
					//	for (int i = 0; i < hNum; ++i)
					//	{
					//		vPai.push_back(m_nHuiCard);
					//	}
					//	TestHuiModule huiModule(m_nHuiCard, m_is7Pair);
					//	huiModule.testHuPai(vPai);
					//}
				}
			}
		}
	}
	return 0;
}

bool TestHuiModule::test3Combine(uint8_t nCard1, uint8_t nCard2, uint8_t nCard3)
{
	// 牌型不同不能组合
	if (card_Type(nCard1) != card_Type(nCard2) || card_Type(nCard1) != card_Type(nCard2))
	{
		return false;
	}
	// 重牌
	if (nCard1 == nCard2 && nCard1 == nCard3)
	{
		return true;
	}
	// 以下是连牌
	if (card_Type(nCard1) == eCT_Feng || card_Type(nCard1) == eCT_Jian)
	{
		return false;
	}
	if (nCard1 + 1 == nCard2 && nCard1 + 2 == nCard3)
	{
		return true;
	}
	return false;
}

void TestHuiModule::eraseVector(uint8_t p, VEC_CARD& typeVec)
{
	VEC_CARD::iterator it = find(typeVec.begin(), typeVec.end(), p);
	if (it != typeVec.end())
	{
		typeVec.erase(it);
	}
}

void TestHuiModule::insertVector(uint8_t p, VEC_CARD& typeVec)
{
	typeVec.push_back(p);
	sort(typeVec.begin(), typeVec.end());
}

void TestHuiModule::VecEliminateCopy(VEC_CARD vSrcPai, VEC_CARD& vPaiCopy, VEC_CARD vPaiRemove)
{
	vPaiCopy.clear();
	vPaiCopy.insert(vPaiCopy.begin(), vSrcPai.begin(), vSrcPai.end());
	for (auto p : vPaiRemove)
	{
		eraseVector(p, vPaiCopy);
	}
}

uint8_t TestHuiModule::get7PairQueCnt19(VEC_CARD vCards[eCT_Max], VEC_CARD& vQueCards)
{
	uint8_t nUnpairCnt = 0;
	for (uint8_t nType = eCT_None; nType < eCT_Max; ++nType)
	{
		auto& vCard = vCards[nType];
		if (vCard.empty())
		{
			continue;
		}

		for (uint8_t nIdx = 0; nIdx < vCard.size();)
		{
			auto nNext = 0;
			if (uint8_t(1 + nIdx) < vCard.size())
			{
				nNext = vCard[1 + nIdx];
			}

			if (vCard[nIdx] == nNext)
			{
				nIdx += 2;
			}
			else
			{
				auto nValue = card_Value(vCard[nIdx]);
				if (nValue == 1 || nValue == 9)
				{
					vQueCards.push_back(vCard[nIdx]);
				}
				++nUnpairCnt;
				++nIdx;
			}
		}
	}
	return nUnpairCnt;
}

bool TestHuiModule::checkHuTiaojian(vector<VEC_CARD> vMyPaiCopy, uint32_t& curHunNum, bool& b3Men)
{
	if (!have19(vMyPaiCopy))
	{
		return false;

		vector<uint8_t> vRemoves;
		uint32_t nQue19 = getLessQue19(vMyPaiCopy, vRemoves);
		if (nQue19 >= curHunNum)
		{
			return false;
		}
		if (nQue19 > 0)
		{
			curHunNum -= nQue19;
			for (auto p : vRemoves)
			{
				eraseVector(p, vMyPaiCopy[card_Type(p)]);
			}
		}
	}

	uint32_t nQue3Men = getLessQue3Men(vMyPaiCopy);
	if (nQue3Men <= curHunNum)
	{
		if (nQue3Men == 0)
		{
			b3Men = true;
		}
		curHunNum -= nQue3Men;
	}
	else
		return false;

	uint32_t nQueKeZhiNum = getLessQueKeZhi(vMyPaiCopy);
	if (nQueKeZhiNum > curHunNum)
		return false;

	return true;
}

uint32_t TestHuiModule::getLessQueKeZhi(vector<VEC_CARD> vMyPaiCopy)
{
	if (m_vPengGang.size() > 0)
	{
		return 0;
	}

	uint32_t nQueCnt = 3;
	for (int i = eCT_Wan; i <= eCT_Jian; ++i)
	{
		if (vMyPaiCopy[i].size() == 0)
		{
			continue;
		}
		if (vMyPaiCopy[i].size() == 1)
		{
			if (nQueCnt > 2)
			{
				nQueCnt = 2;
			}
			continue;
		}
		std::sort(vMyPaiCopy[i].begin(), vMyPaiCopy[i].end());
		for (int j = 0; j < vMyPaiCopy[i].size() - 1; ++j)
		{
			if (j + 2 < vMyPaiCopy[i].size() && vMyPaiCopy[i][j] == vMyPaiCopy[i][j + 2])
			{
				if (nQueCnt > 0)
				{
					nQueCnt = 0;
					return nQueCnt;
				}
			}
			else if (vMyPaiCopy[i][j] == vMyPaiCopy[i][j + 1])
			{
				if (card_Type(vMyPaiCopy[i][j]) == eCT_Jian)
				{
					nQueCnt = 0;
					return nQueCnt;
				} 
				else
				{
					if (nQueCnt > 1)
					{
						nQueCnt = 1;
					}
					//j += 2;
					j++;
				}
			}
		}
	}
	return nQueCnt;
}

uint32_t TestHuiModule::getLessQue19(vector<VEC_CARD> vMyPaiCopy, VEC_CARD& vRemoves)
{
	vRemoves.clear();

	uint32_t nQueCnt = 0;
	for (int i = eCT_Wan; i <= eCT_Tiao; ++i)
	{
		if (vMyPaiCopy[i].size() == 1)
		{
			auto p = vMyPaiCopy[i].at(0);
			if (card_Value(p) == 1 || card_Value(p) == 9)
			{
				nQueCnt += 2;
				vRemoves.push_back(p);
			}
		}
	}
	return nQueCnt;
}

uint32_t TestHuiModule::getLessQue3Men(vector<VEC_CARD> vMyPaiCopy)
{
	uint32_t nQueCnt = 0;
	for (int i = eCT_Wan; i <= eCT_Tiao; ++i)
	{
		if (vMyPaiCopy[i].size() == 0)
		{
			bool bFound = false;
			for (int k = 0; k < m_vEatCard.size(); ++k)
			{
				if (card_Type(m_vEatCard[k]) == i)
				{
					bFound = true;
					break;
				}
			}
			if (bFound)
			{
				continue;
			}
			for (int k = 0; k < m_vPengGang.size(); ++k)
			{
				if (card_Type(m_vPengGang[k]) == i)
				{
					bFound = true;
					break;
				}
			}
			if (bFound)
			{
				continue;
			}
			nQueCnt += 2;
		}
	}
	return nQueCnt;
}

bool TestHuiModule::have19(vector<VEC_CARD> vMyPaiCopy)
{
	auto pfunCheck19 = [](eMJCardType type, VEC_CARD vMyPaiCopy)->bool {
		if (type >= eCT_Wan && type <= eCT_Tiao)
		{
			for (auto p : vMyPaiCopy)
			{
				if (card_Value(p) == 1 || card_Value(p) == 9)
				{
					return true;
				}
			}
		}
		else if (type == eCT_Feng || type == eCT_Jian)
		{
			if (vMyPaiCopy.size() > 0)
			{
				return true;
			}
		}
		return false;
	};

	std::vector<VEC_CARD> vEatedCards;
	vEatedCards.resize(eCT_Max);
	fillCards(m_vEatCard, vEatedCards);
	
	std::vector<VEC_CARD> vPengGangCards;
	vPengGangCards.resize(eCT_Max);
	fillCards(m_vPengGang, vPengGangCards);

	for (int i = eCT_Wan; i <= eCT_Jian; ++i)
	{
		if (pfunCheck19((eMJCardType)i, vEatedCards[i]))
		{
			return true;
		}
		if (pfunCheck19((eMJCardType)i, vPengGangCards[i]))
		{
			return true;
		}
		if (pfunCheck19((eMJCardType)i, vMyPaiCopy[i]))
		{
			return true;
		}
	}
	return false;
}
