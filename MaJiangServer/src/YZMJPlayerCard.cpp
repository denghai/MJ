#include "YZMJPlayerCard.h"
#include "log4z.h"
#include "MJCard.h"
#include <assert.h>

#define MJPAI_HUNMAX 4

bool YZMJPlayerCard::canEatCard(uint8_t nCard, uint8_t& nWithA, uint8_t& nWithB)
{
	return false;
}

bool YZMJPlayerCard::canPengWithCard(uint8_t nCard)
{
	if (nCard == m_nHunCard)
	{
		return false;
	}

	if (m_vLouPeng.end() != std::find(m_vLouPeng.begin(), m_vLouPeng.end(), nCard))
	{
		return false;
	} 

	return MJPlayerCard::canPengWithCard(nCard);
}

bool YZMJPlayerCard::canMingGangWithCard(uint8_t nCard)
{
	if (nCard == m_nHunCard)
	{
		return false;
	}
	return MJPlayerCard::canMingGangWithCard(nCard);
}

void YZMJPlayerCard::reset()
{
	MJPlayerCard::reset();
	m_vLouPeng.clear();
	m_isLouHu = false;
	m_vPengSongIdx.clear();
	m_bBanZi = false;
	m_nBanCard = uint8_t(-1);
	m_nHunCard = uint8_t(-1);
	m_bCanHuPao = true;
	m_haveYiTiaoLong = false;
	m_have7Pair = false;
}

bool YZMJPlayerCard::getCardInfo(Json::Value& jsPeerCards)
{
	// svr: { idx : 2 , anPai : [2,3,4,34] , chuPai: [2,34,4] , huaPai: [23,23,23] , anGangPai : [23,24],buGang : [23,45], pengCard : [23,45], isTing : 1, baoCard : 23 }
	IMJPlayerCard::VEC_CARD vAnPai, vChuPai, vAnGangedCard, vBuGang, vPenged;
	Json::Value jsAnPai, jsChuPai, jsAngangedPai, jsBuGang, jsPenged;

	getHoldCard(vAnPai);
	getChuedCard(vChuPai);
	getAnGangedCard(vAnGangedCard);
	getMingGangedCard(vBuGang);
	getPengedCard(vPenged);

	auto toJs = [](IMJPlayerCard::VEC_CARD& vCards, Json::Value& js)
	{
		for (auto& c : vCards)
		{
			js[js.size()] = c;
		}
	};

	toJs(vAnPai, jsAnPai);
	toJs(vChuPai, jsChuPai); 
	toJs(vAnGangedCard, jsAngangedPai); 
	toJs(vBuGang, jsBuGang); 
	toJs(vPenged, jsPenged);

	jsPeerCards["anPai"] = jsAnPai; 
	jsPeerCards["chuPai"] = jsChuPai; 
	jsPeerCards["anGangPai"] = jsAngangedPai; 
	jsPeerCards["buGang"] = jsBuGang;
	jsPeerCards["pengCard"] = jsPenged;

	return true;
}

bool YZMJPlayerCard::onDoHu(bool isZiMo, uint8_t nCard, std::vector<uint16_t>& vHuTypes, int32_t& nFanCnt)
{
	vHuTypes.clear();
	// if not zi mo , must add to fo check hu ;
	if (!isZiMo)
	{
		auto type = card_Type(nCard);
		if (type >= eCT_Max)
		{
			LOGFMTE("invalid card type for card = %u", nCard);
			return false;
		}
		addCardToVecAsc(m_vCards[type], nCard);
	}

	auto funRemoveAddToCard = [this](uint8_t nCard)
	{
		auto type = card_Type(nCard);
		auto iter = std::find(m_vCards[type].begin(), m_vCards[type].end(), nCard);
		if (iter == m_vCards[type].end())
		{
			LOGFMTE("hu this card should already addto hold card , but can not remove  whay card = %u", nCard);
			return;
		}
		m_vCards[type].erase(iter);
	};

	//if (isHoldCardCanHu() == false)
	//{
	//	LOGFMTE("do hu act , but can not hu ? why ? bug card = %u ", nCard);
	//	debugCardInfo();
	//	if (!isZiMo)
	//	{
	//		funRemoveAddToCard(nCard);
	//	}
	//	return false;
	//}

	if (checkDuiDuiHu())
	{
		if (!isAllFeng())
		{
			vHuTypes.push_back(eFanxing_DuiDuiHu);
			nFanCnt += 4;
		} 
		else
		{
			vHuTypes.push_back(eFanxing_FengQing);
			nFanCnt += 16;
		}
		
	}
	if (checkQingYiSe(false))
	{
		vHuTypes.push_back(eFanxing_QingYiSe);
		nFanCnt += 8;
	}
	else
	{
		if (checkQingYiSe(true))
		{
			vHuTypes.push_back(eFanxing_QingYiSe);
			nFanCnt += 4;
		}
	}
	if (checkHunYiSe(false))
	{
		vHuTypes.push_back(eFanxing_HunYiSe);
		nFanCnt += 4;
	}
	else
	{
		if (checkHunYiSe(true))
		{
			vHuTypes.push_back(eFanxing_HunYiSe);
			nFanCnt += 3;
		}
	}
	if (m_haveYiTiaoLong && checkYiTiaoLong())
	{
		vHuTypes.push_back(eFanxing_YiTiaoLong);
		nFanCnt += 4;
	}

	auto nCount = check7Pair(nCard);
	if (nCount == 3)
	{
		vHuTypes.push_back(eFanxing_3LongJia);
		nFanCnt += 32;
	}
	else if (nCount == 2)
	{
		vHuTypes.push_back(eFanxing_2LongJia);
		nFanCnt += 16;
	}
	else if (nCount == 1)
	{
		vHuTypes.push_back(eFanxing_LongJia);
		nFanCnt += 8;
	}
	else if (nCount == 0)
	{
		if (!isAllFeng())
		{
			vHuTypes.push_back(eFanxing_QiDui);
			nFanCnt += 4;
		}
		else
		{
			vHuTypes.push_back(eFanxing_FengQing);
			nFanCnt += 16;
		}
	}

	if (vHuTypes.empty())
	{
		vHuTypes.push_back(eFanxing_PingHu);
		nFanCnt += 2;
	}

	if (m_bBanZi)
	{
		//手里四个配子(未打出的，碰不算)*4
		auto eHunType = card_Type(m_nHunCard);
		if (eHunType >= eCT_Wan && eHunType <= eCT_Jian)
		{
			auto nCount = std::count(m_vCards[eHunType].begin(), m_vCards[eHunType].end(), m_nHunCard);
			if (nCount == 4)
			{
				nFanCnt += 8;
			}
		}
		else
		{
			LOGFMTE("parse card type error ,HunCard have this card = %u", m_nHunCard);
		}
	}

	if (!isZiMo)
	{
		funRemoveAddToCard(nCard);
	}

	return true;
}

bool YZMJPlayerCard::canHuWitCard(uint8_t nCard)
{
	if (m_bCanHuPao == false)
	{
		return false;
	}

	auto eType = card_Type(nCard);
	if (eType >= eCT_Max)
	{
		LOGFMTE("parse card type error ,canHuWitCard have this card = %u", nCard);
		return false;
	}

	addCardToVecAsc(m_vCards[eType], nCard);
	bool bSelfHu = checkHu(nCard);
	auto iter = std::find(m_vCards[eType].begin(), m_vCards[eType].end(), nCard);
	m_vCards[eType].erase(iter);
	//debugCardInfo();
	return bSelfHu;
}

bool YZMJPlayerCard::isHoldCardCanHu()
{
	return checkHu(getNewestFetchedCard());
}

//bool YZMJPlayerCard::getHoldCardThatCanBuGang(VEC_CARD& vGangCards)
//{
//	for (auto& ref : m_vPenged)
//	{
//		auto it = std::find(m_vCanNotGang.begin(), m_vCanNotGang.end(), ref);
//		if (isHaveCard(ref) && it == m_vCanNotGang.end())
//		{
//			vGangCards.push_back(ref);
//		}
//	}
//
//	return !vGangCards.empty();
//}

bool YZMJPlayerCard::checkDuiDuiHu()
{
	if (m_vEated.size() > 0)
	{
		return false;
	}

	VEC_CARD vAllCard;
	getHoldCard(vAllCard);
	std::sort(vAllCard.begin(), vAllCard.end());
	bool bFindJiang = false;
	for (uint8_t nIdx = 0; (nIdx + 1) < vAllCard.size(); )
	{
		auto nThirdIdx = nIdx + 2;
		if (nThirdIdx < vAllCard.size() && vAllCard[nIdx] == vAllCard[nThirdIdx])
		{
			nIdx += 3;
			continue;
		}

		if (false == bFindJiang && vAllCard[nIdx] == vAllCard[nIdx + 1])
		{
			nIdx += 2;
			bFindJiang = true;
			continue;
		}
		return false;
	}
	return bFindJiang;
}

//混一色（1番）
//全部是一种花色和字牌的平胡。
bool YZMJPlayerCard::checkHunYiSe(bool haveHun)
{
	VEC_CARD vAllCard;
	getHoldCard(vAllCard);

	VEC_CARD vTemp;
	getAnGangedCard(vTemp);
	vAllCard.insert(vAllCard.end(), vTemp.begin(), vTemp.end());

	vTemp.clear();
	getMingGangedCard(vTemp);
	vAllCard.insert(vAllCard.end(), vTemp.begin(), vTemp.end());

	vTemp.clear();
	getPengedCard(vTemp);
	vAllCard.insert(vAllCard.end(), vTemp.begin(), vTemp.end());

	if (haveHun && m_bBanZi && m_nHunCard != uint8_t(-1))
	{
		while (true)
		{
			VEC_CARD::iterator it = std::find(vAllCard.begin(), vAllCard.end(), m_nHunCard);
			if (it != vAllCard.end())
			{
				vAllCard.erase(it);
			}
			else
			{
				break;
			}
		}
	}

	auto nType = eCT_None;
	for (size_t i = 0; i < vAllCard.size(); ++i)
	{
		auto nType_ = card_Type(vAllCard.at(i));
		if (nType_ == eCT_Tiao || nType_ == eCT_Wan || nType_ == eCT_Tong)
		{
			nType = nType_;
			break;
		}
	}

	bool bFoundFeng = false;
	if (nType != eCT_None)
	{
		for (auto& ref : vAllCard)
		{
			auto tt = card_Type(ref);
			if (tt == eCT_Feng || tt == eCT_Jian)
			{
				bFoundFeng = true;
				continue;
			}
			if (nType != tt)
			{
				return false;
			}
		}
	}
	else
	{
		return false;
	}
	if (!bFoundFeng)
	{
		return false;
	}
	return true;
}

//清一色（2番）
//全部是一种花色的平胡。
bool YZMJPlayerCard::checkQingYiSe(bool haveHun)
{
	VEC_CARD vAllCard;
	getHoldCard(vAllCard);

	VEC_CARD vTemp;
	getAnGangedCard(vTemp);
	vAllCard.insert(vAllCard.end(), vTemp.begin(), vTemp.end());

	vTemp.clear();
	getMingGangedCard(vTemp);
	vAllCard.insert(vAllCard.end(), vTemp.begin(), vTemp.end());

	vTemp.clear();
	getPengedCard(vTemp);
	vAllCard.insert(vAllCard.end(), vTemp.begin(), vTemp.end());

	if (haveHun && m_bBanZi && m_nHunCard != uint8_t(-1))
	{
		while (true)
		{
			VEC_CARD::iterator it = std::find(vAllCard.begin(), vAllCard.end(), m_nHunCard);
			if (it != vAllCard.end())
			{
				vAllCard.erase(it);
			}
			else
			{
				break;
			}
		}
	}

	auto nType = card_Type(vAllCard.at(0));
	if (nType != eCT_None)
	{
		for (auto& ref : vAllCard)
		{
			auto tt = card_Type(ref);
			if (nType != tt)
			{
				return false;
			}
		}
	}
	else
	{
		return false;
	}

	return true;
}

bool YZMJPlayerCard::checkHu(uint8_t nCard)
{
	auto funRemoveAddToCard = [this](uint8_t nCard)
	{
		auto type = card_Type(nCard);
		auto iter = std::find(m_vCards[type].begin(), m_vCards[type].end(), nCard);
		if (iter == m_vCards[type].end())
		{
			LOGFMTE("hu this card should already addto hold card , but can not remove  whay card = %u", nCard);
			return;
		}
		m_vCards[type].erase(iter);
	};

	if (m_isLouHu)
	{
		funRemoveAddToCard(nCard);

		uint32_t nFanCnt = 2; // 平胡最小
		for (auto nPassHuCard : m_vLouHuCard)
		{
			std::vector<uint16_t> vType;
			int32_t nAllFanCnt = 0;
			onDoHu(false, nPassHuCard, vType, nAllFanCnt);
			if (nAllFanCnt > nFanCnt)
			{
				nFanCnt = nAllFanCnt;
			}
		}
		
		std::vector<uint16_t> vType;
		int32_t nAllFanCnt = 0;
		onDoHu(false, nCard, vType, nAllFanCnt);

		auto type = card_Type(nCard);
		if (type >= eCT_Max)
		{
			LOGFMTE("invalid card type for card = %u", nCard);
			return false;
		}
		addCardToVecAsc(m_vCards[type], nCard);

		if (nAllFanCnt <= nFanCnt)
		{
			return false;
		}
	}

	VEC_CARD vHoldCard;
	getHoldCard(vHoldCard);
	
	if (m_bBanZi)
	{
		bool bSelfHu = testHuPai(nCard, vHoldCard, m_nHunCard);
		if (bSelfHu)
		{
			return true;
		}
	} 
	else
	{
		bool bSelfHu = MJPlayerCard::isHoldCardCanHu();
		if (bSelfHu)
		{
			return true;
		}
	}
	
	//debugCardInfo();
	return false;
}

bool YZMJPlayerCard::test3Combine(uint8_t nCard1, uint8_t nCard2, uint8_t nCard3)
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

void YZMJPlayerCard::eraseVector(uint8_t p, VEC_CARD& typeVec)
{
	VEC_CARD::iterator it = find(typeVec.begin(), typeVec.end(), p);
	if (it != typeVec.end())
	{
		typeVec.erase(it);
	}
}

void YZMJPlayerCard::insertVector(uint8_t p, VEC_CARD& typeVec)
{
	typeVec.push_back(p);
	sort(typeVec.begin(), typeVec.end());
}

void YZMJPlayerCard::VecEliminateCopy(VEC_CARD vSrcPai, VEC_CARD& vPaiCopy, VEC_CARD vPaiRemove)
{
	vPaiCopy.clear();
	vPaiCopy.insert(vPaiCopy.begin(), vSrcPai.begin(), vSrcPai.end());
	for (auto p : vPaiRemove)
	{
		eraseVector(p, vPaiCopy);
	}
}

//成为整扑需要的癞子个数
void YZMJPlayerCard::getNeedHunNumToBePu(VEC_CARD typeVec, uint32_t needNum, uint32_t& needMinHunNum)
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
bool YZMJPlayerCard::vecCanHu(uint32_t hunNum, VEC_CARD vHuPai)
{
	LOGFMTI("check can hu...");
	int huSize = vHuPai.size();
	//DebugPlayer(HUVEC);
	if (huSize <= 0)
	{
		if (hunNum >= 2)
			return true;
		return false;
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
					return true;
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
						return true;
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
					return true;
				}
			}
		}
	}
	return false;
}

bool YZMJPlayerCard::vecHu7Pair(uint8_t nHunCard, VEC_CARD vHuPai)
{
	if (!m_have7Pair)
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
	auto nQueCnt = get7PairQueCnt(vCardCopy);
	if (nQueCnt == 0)
	{
		return true;
	}
	if (nQueCnt <= nHunCount)
	{
		return true;
	}
	return false;
}

//发的牌
bool YZMJPlayerCard::testHuPai(uint8_t nHuCard, VEC_CARD vCards, uint8_t nHunCard)
{
	if (vecHu7Pair(nHunCard, vCards))
	{
		return true;
	}

	std::vector<VEC_CARD> vCardCopy;
	
	vCardCopy.resize(eCT_Max);

	for (auto nCard : vCards)
	{
		if (nCard == nHunCard)
		{
			vCardCopy[eCT_Hun].push_back(nCard);
			continue;
		}
		auto eType = card_Type(nCard);
		if (eType < eCT_Wan || eType > eCT_Jian)
		{
			LOGFMTE("test hu failed, error card=%d", nCard);
			return false;
		}
		vCardCopy[eType].push_back(nCard);
	}

	uint32_t curHunNum = vCardCopy[eCT_Hun].size();
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
		printf("Type[%d] ToPuNeedNum:%d\n", i, vToPuNeedNum[i]);

		nTotoalToPuNeedNum += vToPuNeedNum[i];
	}

	for (int i = eCT_Wan; i <= eCT_Hun; ++i)
	{
		uint32_t needHunNum = nTotoalToPuNeedNum - vToPuNeedNum[i];
		if (needHunNum <= curHunNum)
		{
			LOGFMTI("jiang in type[%d], needHunNum:%d curHunNum:%d", i, needHunNum, curHunNum);
			printf("jiang in type[%d], needHunNum:%d curHunNum:%d\n", i, needHunNum, curHunNum);
			bool isHu = vecCanHu(curHunNum - needHunNum, vCardCopy[i]);
			if (isHu)
			{
				return true;
			}
		}
	}

	return false;
}

void YZMJPlayerCard::setBanZiCard(uint8_t nBanZiCard)
{
	m_bBanZi = true;
	m_nBanCard = nBanZiCard;

	if (m_nBanCard != uint8_t(-1))
	{
		auto cardType = card_Type(m_nBanCard);
		auto cardValue = card_Value(m_nBanCard);
		cardValue++;

		if (cardType == eCT_Wan || cardType == eCT_Tong || cardType == eCT_Tiao)
		{
			if (cardValue > 9)
			{
				cardValue = 1;
			}
		}
		else if (cardType == eCT_Feng)
		{
			if (cardValue > 4)
			{
				cardValue = 1;
			}
		}
		else if (cardType == eCT_Jian)
		{
			if (cardValue > 3)
			{
				cardValue = 1;
			}
		}
		m_nHunCard = CMJCard::makeCardNumber(cardType, cardValue);
	}
	else
	{
		m_nHunCard = CMJCard::makeCardNumber(eCT_Jian, 3);
	}

	checkCanHuPao();
}

bool YZMJPlayerCard::isAllFeng()
{
	VEC_CARD vAllCard;
	getHoldCard(vAllCard);

	VEC_CARD vTemp;
	getAnGangedCard(vTemp);
	vAllCard.insert(vAllCard.end(), vTemp.begin(), vTemp.end());

	vTemp.clear();
	getMingGangedCard(vTemp);
	vAllCard.insert(vAllCard.end(), vTemp.begin(), vTemp.end());

	vTemp.clear();
	getPengedCard(vTemp);
	vAllCard.insert(vAllCard.end(), vTemp.begin(), vTemp.end());

	for (auto& ref : vAllCard)
	{
		auto tt = card_Type(ref);
		if (tt != eCT_Feng && tt != eCT_Jian)
		{
			return false;
		}
	}
	return true;
}

void YZMJPlayerCard::addCanNotGang(uint8_t nCard)
{
	auto it = std::find(m_vCanNotGang.begin(), m_vCanNotGang.end(), nCard);
	if (it == m_vCanNotGang.end())
	{
		m_vCanNotGang.push_back(nCard);
	}
}

void YZMJPlayerCard::setLouHu(bool isLouHu, uint8_t nHuCard)
{
	if (isLouHu == false)
	{
		m_isLouHu = false;
		m_vLouHuCard.clear();
		return;
	}

	m_isLouHu = true;
	auto it = std::find(m_vLouHuCard.begin(), m_vLouHuCard.end(), nHuCard);
	if (it == m_vLouHuCard.end())
	{
		m_vLouHuCard.push_back(nHuCard);
	}
}

bool YZMJPlayerCard::haveThreeBanZi()
{
	if (m_nBanCard == uint8_t(-1))
	{
		return false;
	}

	//胡牌时手里三个搬子算暗杠
	auto eBanType = card_Type(m_nBanCard);
	if (eBanType >= eCT_Wan && eBanType <= eCT_Jian)
	{
		auto nCount = std::count(m_vCards[eBanType].begin(), m_vCards[eBanType].end(), m_nBanCard);
		if (nCount >= 3)
		{
			return true;
		}
	}
	else
	{
		LOGFMTE("parse card type error ,BanZiCard have this card = %u", m_nBanCard);
		return false;
	}
	return false;
}

void YZMJPlayerCard::pickoutHoldCard(uint8_t nCard)
{
	auto eType = card_Type(nCard);

	std::vector<uint8_t>::iterator it = std::find(begin(m_vCards[eType]), end(m_vCards[eType]), nCard);
	if (it != m_vCards[eType].end())
	{
		m_vCards[eType].erase(it);
	}
	else
	{
		assert(0);
	}
}

uint8_t YZMJPlayerCard::haveYiTiaoLong(std::vector<uint8_t> vCard)
{
	std::vector<std::vector<uint8_t>> vTypeCards;
	vTypeCards.resize(eCT_Tiao + 1);

	for (auto nCard : vCard)
	{
		auto eType = card_Type(nCard);
		if (eType == eCT_Wan)
		{
			vTypeCards[eCT_Wan].push_back(nCard);
		}
		else if (eType == eCT_Tong)
		{
			vTypeCards[eCT_Tong].push_back(nCard);
		}
		else if (eType == eCT_Tiao)
		{
			vTypeCards[eCT_Tiao].push_back(nCard);
		}
	}

	auto eType = eCT_Max;
	for (uint8_t i = eCT_Wan; i <= eCT_Tiao; ++i)
	{
		if (vTypeCards[i].size() >= 9)
		{
			auto type_ = (eMJCardType)i;

			for (int j = 1; j <= 9; ++j)
			{
				auto nCard = CMJCard::makeCardNumber(type_, j);
				std::vector<uint8_t>::iterator it = std::find(vTypeCards[i].begin(), vTypeCards[i].end(), nCard);
				if (it == vTypeCards[i].end())
				{
					return eCT_Max;
				}
			}
			return type_;
		}
	}
	return eCT_Max;
}

bool YZMJPlayerCard::checkYiTiaoLong()
{
	std::vector<uint8_t> vCards, vHoldCards;
	getHoldCard(vHoldCards);

	vCards.insert(vCards.end(), vHoldCards.begin(), vHoldCards.end());

	auto eType = (eMJCardType)haveYiTiaoLong(vCards);
	if (eType != eCT_Wan && eType != eCT_Tong && eType != eCT_Tiao)
	{
		return false;
	}

	bool canHu = false;

	for (int j = 1; j <= 9; ++j)
	{
		pickoutHoldCard(CMJCard::makeCardNumber(eType, j));
	}

	if (isHoldCardCanHu())
	{
		canHu = true;
	}

	for (int j = 1; j <= 9; ++j)
	{
		addDistributeCard(CMJCard::makeCardNumber(eType, j));
	}

	return canHu;
}

uint8_t YZMJPlayerCard::check7Pair(uint8_t nHuCard)
{
	VEC_CARD vCards;
	getHoldCard(vCards);
	if (vecHu7Pair(m_nHunCard, vCards) == false)
	{
		return uint8_t(-1);
	}

	int nCount = 0;
	for (uint8_t i = eCT_Wan; i <= eCT_Jian; ++i)
	{
		for (int j = 0; j < m_vCards[i].size(); ++j)
		{
			if (j + 3 < m_vCards[i].size())
			{
				if (m_vCards[i].at(j) == m_vCards[i].at(j + 3))
				{
					nCount++;
					j += 3;
				}
			}
			else
			{
				break;
			}
		}
	}
	return nCount;
}

void YZMJPlayerCard::addSongPengIdx(uint8_t nCard, uint8_t nIdx)
{
	tSongGangIdx tSong;
	tSong.eAct = eMJAct_Peng;
	tSong.nGangCard = nCard;
	tSong.nIdx = nIdx;
	m_vPengSongIdx.push_back(tSong);
}

void YZMJPlayerCard::checkCanHuPao()
{
	if (!m_bCanHuPao)
	{
		return;
	}
	auto hunCardType = card_Type(m_nHunCard);
	if (hunCardType < eCT_Wan || hunCardType > eCT_Jian)
	{
		LOGFMTE("checkCanHuPao error hun card=%d", m_nHunCard);
		return;
	}
	auto it = std::find(m_vCards[hunCardType].begin(), m_vCards[hunCardType].end(), m_nHunCard);
	if (it != m_vCards[hunCardType].end())
	{
		m_bCanHuPao = false;
	}
}

bool YZMJPlayerCard::canHoldCard7PairHu()
{
	if (m_have7Pair)
	{
		return MJPlayerCard::canHoldCard7PairHu();
	} 
	else
	{
		return false;
	}
}