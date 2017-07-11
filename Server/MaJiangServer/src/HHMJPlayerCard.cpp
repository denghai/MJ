#include "HHMJPlayerCard.h"
#include "log4z.h"
#include "MJCard.h"
#include <assert.h>

bool HHMJPlayerCard::canEatCard(uint8_t nCard, uint8_t& nWithA, uint8_t& nWithB)
{
	auto eTypeC = card_Type(nCard);

	if (nWithA == 0 && nWithB == 0)
	{
		if (eTypeC != eCT_Tiao && eTypeC != eCT_Tong && eTypeC != eCT_Wan)
		{
			return false;
		}

		std::vector<uint8_t>::iterator it_a, it_b;
		it_a = it_b = m_vCards[eTypeC].end();
		
		it_a = std::find(m_vCards[eTypeC].begin(), m_vCards[eTypeC].end(), nCard + 1);
		it_b = std::find(m_vCards[eTypeC].begin(), m_vCards[eTypeC].end(), nCard + 2);
		if (it_a != m_vCards[eTypeC].end() && it_b != m_vCards[eTypeC].end())
		{
			return true;
		}
		
		it_a = it_b = m_vCards[eTypeC].end();
		it_a = std::find(m_vCards[eTypeC].begin(), m_vCards[eTypeC].end(), nCard - 1);
		it_b = std::find(m_vCards[eTypeC].begin(), m_vCards[eTypeC].end(), nCard - 2);
		if (it_a != m_vCards[eTypeC].end() && it_b != m_vCards[eTypeC].end())
		{
			return true;
		}

		it_b = m_vCards[eTypeC].end();
		it_b = std::find(m_vCards[eTypeC].begin(), m_vCards[eTypeC].end(), nCard + 1);
		if (it_a != m_vCards[eTypeC].end() && it_b != m_vCards[eTypeC].end())
		{
			return true;
		}
	}
	else
	{
		auto eTypeA = card_Type(nWithA);
		auto eTypeB = card_Type(nWithB);

		if (eTypeA != eTypeB || eTypeA != eTypeC || eTypeB != eTypeC)
		{
			return false;
		}

		if (eTypeA > eCT_Tiao || eTypeA < eCT_Wan)
		{
			return false;
		}

		std::vector<uint8_t>::iterator it_a = std::find(begin(m_vCards[eTypeA]), end(m_vCards[eTypeA]), nWithA);
		if (it_a == m_vCards[eTypeA].end())
		{
			return false;
		}

		std::vector<uint8_t>::iterator it_b = std::find(begin(m_vCards[eTypeA]), end(m_vCards[eTypeA]), nWithB);
		if (it_b == m_vCards[eTypeA].end())
		{
			return false;
		}

		uint8_t c = card_Value(nCard);
		uint8_t a = card_Value(nWithA);
		uint8_t b = card_Value(nWithB);

		if (a < 1 || a > 9 || b < 1 || b > 9 || c < 1 || c > 9)
		{
			return false;
		}

		std::vector<uint8_t> vSort;
		vSort.push_back(a);
		vSort.push_back(b);
		vSort.push_back(c);
		std::sort(vSort.begin(), vSort.end());
		if (vSort.at(0) + 1 == vSort.at(1) && vSort.at(0) + 2 == vSort.at(2))
		{
			return true;
		}
	}
	return false;
}

bool HHMJPlayerCard::canPengWithCard(uint8_t nCard)
{
	if (m_vLouPeng.end() != std::find(m_vLouPeng.begin(), m_vLouPeng.end(), nCard))
	{
		return false;
	} 

	return MJPlayerCard::canPengWithCard(nCard);
}

void HHMJPlayerCard::reset()
{
	MJPlayerCard::reset();
	m_vLouPeng.clear();
	m_isLouHu = false;
	m_vMingGangSongIdx.clear();
	m_vPengSongIdx.clear();
	m_vGangTimes.clear();
	m_isChiPengGang = false;
	m_vChuedCardTangZhi.clear();
}

bool HHMJPlayerCard::getCardInfo(Json::Value& jsPeerCards)
{
	// svr: { idx : 2 , anPai : [2,3,4,34] , chuPai: [2,34,4] , huaPai: [23,23,23] , anGangPai : [23,24],buGang : [23,45], pengCard : [23,45], isTing : 1, baoCard : 23 }
	IMJPlayerCard::VEC_CARD vAnPai, vChuPai, vAnGangedCard, vBuGang, vPenged, vEated;
	Json::Value jsAnPai, jsChuPai, jsAngangedPai, jsBuGang, jsPenged, jsEated;

	getHoldCard(vAnPai);
	getChuedCard(vChuPai);
	getAnGangedCard(vAnGangedCard);
	getMingGangedCard(vBuGang);
	getPengedCard(vPenged);
	getEatedCard(vEated);

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
	toJs(vEated, jsEated);

	jsPeerCards["anPai"] = jsAnPai; 
	jsPeerCards["chuPai"] = jsChuPai; 
	jsPeerCards["anGangPai"] = jsAngangedPai; 
	jsPeerCards["buGang"] = jsBuGang;
	jsPeerCards["pengCard"] = jsPenged;
	jsPeerCards["eatCard"] = jsEated;

	return true;
}

bool HHMJPlayerCard::onDoHu(bool isZiMo, uint8_t nCard, bool bLanPai, bool haveYiTiaoLong, bool haveLuoDiLong, bool haveShouDaiLong, bool bShouDaiLong, std::vector<uint16_t>& vHuTypes, int32_t& nFanCnt)
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

	if (isHoldCardCanHu() == false)
	{
		LOGFMTE("do hu act , but can not hu ? why ? bug card = %u ", nCard);
		debugCardInfo();
		if (!isZiMo)
		{
			funRemoveAddToCard(nCard);
		}
		return false;
	}

	bool b7Stair = check7Stair();
	bool bYiBanGao = checkYiBanGao();
	if (b7Stair && bYiBanGao)
	{
		vHuTypes.push_back(eFanxing_7StairYiBanGao);
		nFanCnt += 3;
	}
	else if (b7Stair)
	{
		vHuTypes.push_back(eFanxing_7Stair);
		nFanCnt += 2;
	}
	else if (bYiBanGao)
	{
		vHuTypes.push_back(eFanxing_YiBanGao);
		nFanCnt += 2;
	}
	else
	{
		if (bLanPai)
		{
			if (checkZhengGuiLan())
			{
				vHuTypes.push_back(eFanxing_ZhengGuiLan);
				nFanCnt++;
			}
		}
		//else
		//{
		//	if (checkRuanLan())
		//	{
		//		vHuTypes.push_back(eFanxing_RuanLan);
		//		nFanCnt++;
		//	}
		//}
	}

	if (checkDuiDuiHu())
	{
		std::vector<uint8_t> vHoldCards;
		getHoldCard(vHoldCards);
		getPengedCard(vHoldCards);
		getAnGangedCard(vHoldCards);
		getMingGangedCard(vHoldCards);
		if (isAllZiPai(vHoldCards))
		{
			vHuTypes.push_back(eFanxing_ZiDaDui);
			nFanCnt += 4;
		}
		else
		{
			vHuTypes.push_back(eFanxing_DuiDuiHu);
			nFanCnt++;
		}
	}

	if (haveYiTiaoLong)
	{
		bool bShouDai_ = false;
		//if (!isZiMo)
		//{
		//	pickoutHoldCard(nCard);
		//	bShouDai_ = checkShouDaiLong();
		//	addDistributeCard(nCard);
		//}
		//else
		//{
			bShouDai_ = checkShouDaiLong();
		//}

		if (haveShouDaiLong && bShouDai_)
		{
			vHuTypes.push_back(eFanxing_ShouDaiLong);
			nFanCnt++;
			if (bShouDaiLong)
			{
				nFanCnt++;
			}
		}
		else
		{
			if (haveLuoDiLong && checkLuoDiLong())
			{
				vHuTypes.push_back(eFanxing_LuoDiLong);
				nFanCnt++;
			}
		}
	}
	
	if (checkQingYiSe())
	{
		vHuTypes.push_back(eFanxing_QingYiSe);
		nFanCnt += 2;
	} 
	else
	{
		if (checkHunYiSe())
		{
			vHuTypes.push_back(eFanxing_HunYiSe);
			nFanCnt++;
		}
	}

	if (vHuTypes.empty())
	{
		vHuTypes.push_back(eFanxing_PingHu);
	}

	if (!isZiMo)
	{
		funRemoveAddToCard(nCard);
	}
	return true;
}

bool HHMJPlayerCard::onDoTangZhiHu(std::vector<uint16_t>& vHuTypes, int32_t& nFanCnt, bool is7Pair)
{
	vHuTypes.clear();

	bool is13 = false;
	if (m_vChuedCardTangZhi.size() == 13)
	{
		is13 = true;
	}

	auto nLongNum = checkTZ_6Pair(is13);
	if (nLongNum != uint8_t(-1))
	{
		if (!is13)
		{
			nFanCnt++;

			if (is7Pair)
			{
				nFanCnt++;
			}
			if (isAllZiPai(m_vChuedCardTangZhi))
			{
				nFanCnt += 3;
				vHuTypes.push_back(eFanxing_TZ_Zi6Pair);
			}
			else
			{
				nFanCnt += nLongNum;
				vHuTypes.push_back(eFanxing_TZ_6Pair);
			}
		}
		else
		{
			nFanCnt++;
			vHuTypes.push_back(eFanxing_TZ_6Pair);
		}
	}
	if (checkTZ_10Old())
	{
		vHuTypes.push_back(eFanxing_TZ_10Old);
		nFanCnt++;
	}
	if (checkTZ_13())
	{
		vHuTypes.push_back(eFanxing_TZ_13);
		nFanCnt++;
	}
	
	if (checkTZ_7StairYiBanGao(is13))
	{
		vHuTypes.push_back(eFanxing_TZ_7StairYiBanGao);
		if (is13)
		{
			nFanCnt++;
		}
		else
		{
			nFanCnt += 3;
		}
	}
	else if (checkTZ_YiBanGao())
	{
		vHuTypes.push_back(eFanxing_TZ_YiBanGao);
		if (is13)
		{
			nFanCnt++;
		}
		else
		{
			nFanCnt += 2;
		}
	}
	else if (checkTZ_7Stair(is13))
	{
		vHuTypes.push_back(eFanxing_TZ_7Stair);
		if (is13)
		{
			nFanCnt++;
		}
		else
		{
			nFanCnt += 2;
		}
	}
	else if (checkTZ_BadCard(is13))
	{
		vHuTypes.push_back(eFanxing_TZ_BadCard);
		nFanCnt++;
	}

	if (vHuTypes.empty())
	{
		vHuTypes.push_back(eFanxing_PingHu);
	}

	return true;
}

bool HHMJPlayerCard::canHuWitCard(uint8_t nCard)
{
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

bool HHMJPlayerCard::isHoldCardCanHu()
{
	return checkHu(getNewestFetchedCard());
}

bool HHMJPlayerCard::checkDuiDuiHu()
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

//烂牌 - 正规烂（1番）
//1、4、7 / 2、5、8 / 3、6、9牌加字牌的胡牌(不能重复)。
bool HHMJPlayerCard::checkZhengGuiLan()
{
	std::vector<uint8_t> vHoldCards;
	getHoldCard(vHoldCards);

	if (vHoldCards.size() != 14)
	{
		return false;
	}

	std::vector<uint8_t> vZiCard;
	auto nCount = get147258369(vHoldCards, vZiCard);
	if (nCount == 0)
	{
		return false;
	}

	std::sort(vZiCard.begin(), vZiCard.end());

	for (int i = 1; i < vZiCard.size(); ++i)
	{
		if (vZiCard.at(i - 1) == vZiCard.at(i))
		{
			return false;
		}
	}

	return true;
}

////烂牌 - 软烂（1番）
////筒、条、万每门牌的牌张数字需要间隔二以上，再加上不重复的字牌的胡牌(不能重复)。
//bool HHMJPlayerCard::checkRuanLan()
//{
//	std::vector<uint8_t> vHoldCards;
//	getHoldCard(vHoldCards);
//
//	if (vHoldCards.size() != 14)
//	{
//		return false;
//	}
//
//	std::vector<uint8_t> vZiCards;
//	auto nCount = getRuanLan(vHoldCards, vZiCards);
//
//	if (nCount < 7)
//	{
//		return false;
//	}
//
//	std::sort(vZiCards.begin(), vZiCards.end());
//
//	for (int i = 1; i < vZiCards.size(); ++i)
//	{
//		if (vZiCards.at(i - 1) == vZiCards.at(i))
//		{
//			return false;
//		}
//	}
//
//	return true;
//}

//七星（2番）
//东南西北中发白全在的烂牌。
bool HHMJPlayerCard::check7Stair()
{
	std::vector<uint8_t> vHoldCards;
	getHoldCard(vHoldCards);

	if (vHoldCards.size() != 14)
	{
		return false;
	}

	std::vector<uint8_t> vZiCards;
	auto nCount = get147258369(vHoldCards, vZiCards);

	if (nCount != 7 || vZiCards.size() != 7)
	{
		vZiCards.clear();
		nCount = getYiBanGao(vHoldCards, vZiCards);

		if (nCount != 7 || vZiCards.size() != 7)
		{
			return false;
		}
	}

	std::sort(vZiCards.begin(), vZiCards.end());

	for (int i = 1; i < vZiCards.size(); ++i)
	{
		if (vZiCards.at(i - 1) == vZiCards.at(i))
		{
			return false;
		}
	}
	return true;
}


//一般高（2番）
//无将对，每种花色最多三张，且必须全部是147或全部是258或全部是369的组合 + 5张不重复的字牌。
bool HHMJPlayerCard::checkYiBanGao()
{
	std::vector<uint8_t> vHoldCards;
	getHoldCard(vHoldCards);

	if (vHoldCards.size() != 14)
	{
		return false;
	}

	std::vector<uint8_t> vZiCards;
	auto nCount = getYiBanGao(vHoldCards, vZiCards);

	if (nCount < 7)
	{
		return false;
	}

	std::sort(vZiCards.begin(), vZiCards.end());

	for (int i = 1; i < vZiCards.size(); ++i)
	{
		if (vZiCards.at(i - 1) == vZiCards.at(i))
		{
			return false;
		}
	}

	return true;
}

//一条龙（1番 / 2番）
//胡牌时，有同一门花色123 456 789的顺子。有吃 / 碰 / 杠的情况下叫做“落地龙”，计1番。没有吃 / 碰 / 杠的情况下叫做“手逮龙”，计1番或者2番（勾选手逮龙加番）。
bool HHMJPlayerCard::checkLuoDiLong()
{
	std::vector<uint8_t> vCards, vHoldCards, vEatedCards;
	getHoldCard(vHoldCards);

	if (vHoldCards.size() == 14) // 不能是手逮龙
	{
		return false;
	}

	vCards.insert(vCards.end(), vHoldCards.begin(), vHoldCards.end());
	getEatedCard(vEatedCards);
	vCards.insert(vCards.end(), vEatedCards.begin(), vEatedCards.end());

	auto eType = (eMJCardType)haveYiTiaoLong(vCards);
	if (eType != eCT_Wan && eType != eCT_Tong && eType != eCT_Tiao)
	{
		return false;
	}

	bool canHu = false;

	bool haveShun[3];
	for (int i = 0; i < 3; ++i) haveShun[i] = false;
	for (int i = 2; i < vEatedCards.size(); i += 3)
	{
		auto nVal = card_Value(vEatedCards.at(i));
		if (nVal % 3 == 0)
		{
			haveShun[nVal / 3 - 1] = true;
		}
	}

	for (int j = 0; j < 9; j += 3)
	{
		auto nCard = CMJCard::makeCardNumber(eType, j + 1);

		if (haveShun[j])
		{
			continue;
		}

		pickoutHoldCard(nCard);
		pickoutHoldCard(nCard + 1);
		pickoutHoldCard(nCard + 2);
	}

	if (MJPlayerCard::isHoldCardCanHu())
	{
		canHu = true;
	}

	for (int j = 0; j < 9; j+=3)
	{
		auto nCard = CMJCard::makeCardNumber(eType, j + 1);

		if (haveShun[j])
		{
			continue;
		}
		addDistributeCard(nCard);
		addDistributeCard(nCard + 1);
		addDistributeCard(nCard + 2);
	}

	return canHu;
}

bool HHMJPlayerCard::checkShouDaiLong()
{
	std::vector<uint8_t> vHoldCards;
	getHoldCard(vHoldCards);

	//if (vHoldCards.size() != 14)
	//{
	//	return false;
	//}

	auto eType = (eMJCardType)haveYiTiaoLong(vHoldCards);
	if (eType != eCT_Wan && eType != eCT_Tong && eType != eCT_Tiao)
	{
		return false;
	}

	bool canHu = false;

	for (int j = 1; j <= 9; ++j)
	{
		auto nCard = CMJCard::makeCardNumber(eType, j);
		pickoutHoldCard(nCard);
	}

	if (MJPlayerCard::isHoldCardCanHu())
	{
		canHu = true;
	}

	for (int j = 1; j <= 9; ++j)
	{
		auto nCard = CMJCard::makeCardNumber(eType, j);
		addDistributeCard(nCard);
	}

	return canHu;
}

//混一色（1番）
//全部是一种花色和字牌的平胡。
bool HHMJPlayerCard::checkHunYiSe()
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

	vTemp.clear();
	getEatedCard(vTemp);
	vAllCard.insert(vAllCard.end(), vTemp.begin(), vTemp.end());

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

	if (nType != eCT_None)
	{
		for (auto& ref : vAllCard)
		{
			auto tt = card_Type(ref);
			if (tt == eCT_Feng || tt == eCT_Jian)
			{
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
		//for (auto& ref : vAllCard)
		//{
		//	auto tt = card_Type(ref);
		//	if (tt != eCT_Feng && tt != eCT_Jian)
		//	{
		//		return false;
		//	}
		//}
		return false;
	}

	return true;
}

//清一色（2番）
//全部是一种花色的平胡。
bool HHMJPlayerCard::checkQingYiSe()
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

	vTemp.clear();
	getEatedCard(vTemp);
	vAllCard.insert(vAllCard.end(), vTemp.begin(), vTemp.end());

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

//七对（1番 / x5）
//玩家手牌都是对子，没有碰和刮风下雨。
int HHMJPlayerCard::check7Pair(bool isZiMo, uint8_t nHuCard)
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

	if (!isZiMo)
	{
		auto type = card_Type(nHuCard);
		if (type >= eCT_Max)
		{
			LOGFMTE("invalid card type for card = %u", nHuCard);
			return -1;
		}
		addCardToVecAsc(m_vCards[type], nHuCard);
	}

	if (canHoldCard7PairHu() == false)
	{
		if (!isZiMo)
		{
			funRemoveAddToCard(nHuCard);
		}
		return -1;
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

	if (!isZiMo)
	{
		funRemoveAddToCard(nHuCard);
	}

	return nCount;
}

void HHMJPlayerCard::pickoutHoldCard(uint8_t nCard)
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

bool HHMJPlayerCard::checkHu(uint8_t nCard)
{
	if (m_isLouHu)
	{
		return false;
	}

	bool bSelfHu = MJPlayerCard::isHoldCardCanHu();
	if (bSelfHu)
	{
		return true;
	}
	
	if (checkZhengGuiLan() /*|| checkRuanLan()*/ || check7Stair() || checkYiBanGao())
	{
		return true;
	}

	//debugCardInfo();
	return false;
}

void HHMJPlayerCard::addSongGangIdx(uint8_t nCard, uint8_t nIdx, uint8_t eAct)
{
	bool bFind = false;
	for (auto idx : m_vMingGangSongIdx)
	{
		if (idx.nGangCard == nCard && idx.nIdx == nIdx)
		{
			bFind = true;
			break;
		}
	}
	if (bFind)
		return;

	tSongGangIdx tSong;
	tSong.eAct = eAct;
	tSong.nGangCard = nCard;
	tSong.nIdx = nIdx;
	m_vMingGangSongIdx.push_back(tSong);
}

void HHMJPlayerCard::addSongPengIdx(uint8_t nCard, uint8_t nIdx)
{
	tSongGangIdx tSong;
	tSong.eAct = eMJAct_Peng;
	tSong.nGangCard = nCard;
	tSong.nIdx = nIdx;
	m_vPengSongIdx.push_back(tSong);
}

bool HHMJPlayerCard::isTangZhiHu()
{
	if (checkTZ_10Old() || checkTZ_13())
	{
		return true;
	}

	return false;
}

bool HHMJPlayerCard::isTangZhiHu13()
{
	if (m_vChuedCardTangZhi.size() != 13)
	{
		return false;
	}

	if (checkTZ_6Pair(true) != uint8_t(-1) || checkTZ_BadCard(true) || checkTZ_7Stair(true) || checkTZ_YiBanGao() || checkTZ_7StairYiBanGao(true))
	{
		return true;
	}

	return false;
}

bool HHMJPlayerCard::isTangZhiHu14()
{
	if (m_vChuedCardTangZhi.size() != 14)
	{
		return false;
	}

	if (checkTZ_6Pair(false) != uint8_t(-1) || checkTZ_BadCard(false) || checkTZ_7Stair(false) || checkTZ_YiBanGao() || checkTZ_7StairYiBanGao(false))
	{
		return true;
	}

	return false;
}

uint8_t HHMJPlayerCard::checkTZ_6Pair(bool is13)
{
	std::sort(m_vChuedCardTangZhi.begin(), m_vChuedCardTangZhi.end());
	
	auto nLess = 6;
	if (!is13)
	{
		nLess = 7;
	}

	int nCount = 0;
	for (VEC_CARD::iterator it = m_vChuedCardTangZhi.begin(); it != m_vChuedCardTangZhi.end(); ++it)
	{
		auto nCard1 = *it;
		VEC_CARD::iterator it_ = it + 1;
		if (it_ == m_vChuedCardTangZhi.end())
		{
			break;
		}
		auto nCard2 = *it_;
		if (nCard1 != nCard2)
		{
			continue;
		}
		else
		{
			it++;
			nCount++;
		}
	}
	if (nCount != nLess)
	{
		return uint8_t(-1);
	}

	int nCount_ = 0;
	for (int j = 0; j < m_vChuedCardTangZhi.size(); ++j)
	{
		if (j + 3 < m_vChuedCardTangZhi.size())
		{
			if (m_vChuedCardTangZhi.at(j) == m_vChuedCardTangZhi.at(j + 3))
			{
				nCount_++;
				j += 3;
			}
		}
		else
		{
			break;
		}
	}
	assert(nCount_ >= 0 && nCount_ <= 3);
	return nCount_;
}

bool HHMJPlayerCard::checkTZ_10Old()
{
	if (m_vChuedCardTangZhi.size() != 10)
	{
		return false;
	}
	for (auto nCard : m_vChuedCardTangZhi)
	{
		auto eType = card_Type(nCard);
		if (eType != eCT_Feng && eType != eCT_Jian)
		{
			return false;
		}
	}
	return true;
}

bool HHMJPlayerCard::checkTZ_BadCard(bool is13)
{
	auto nLess = 7;
	if (!is13)
	{
		nLess = 8;
	}

	std::vector<uint8_t> vZiCard;
	auto nCount = get147258369(m_vChuedCardTangZhi, vZiCard);
	if (nCount < nLess)
	{
		return false;
	}

	std::sort(vZiCard.begin(), vZiCard.end());

	for (int i = 1; i < vZiCard.size(); ++i)
	{
		if (vZiCard.at(i - 1) == vZiCard.at(i))
		{
			return false;
		}
	}

	return true;
}

bool HHMJPlayerCard::checkTZ_13()
{
	if (m_vChuedCardTangZhi.size() != 13)
	{
		return false;
	}
	for (auto nCard : m_vChuedCardTangZhi)
	{
		auto eType = card_Type(nCard);
		if (eType == eCT_Wan || eType == eCT_Tong || eType == eCT_Tiao)
		{
			auto nVal = card_Value(nCard);
			if (nVal != 1 && nVal != 9)
			{
				return false;
			}
		}
	}
	return true;
}

bool HHMJPlayerCard::checkTZ_7Stair(bool is13)
{
	auto nLess = 6;
	if (!is13)
	{
		nLess = 7;
	}

	std::vector<uint8_t> vZiCard;
	auto nCount = get147258369(m_vChuedCardTangZhi, vZiCard);
	if (nCount != nLess)
	{
		return false;
	}

	std::sort(vZiCard.begin(), vZiCard.end());

	for (int i = 1; i < vZiCard.size(); ++i)
	{
		if (vZiCard.at(i - 1) == vZiCard.at(i))
		{
			return false;
		}
	}

	return true;
}

bool HHMJPlayerCard::checkTZ_YiBanGao()
{
	std::vector<uint8_t> vZiCard;
	auto nCount = getYiBanGao(m_vChuedCardTangZhi, vZiCard);
	if (nCount == 0)
	{
		return false;
	}

	std::sort(vZiCard.begin(), vZiCard.end());

	for (int i = 1; i < vZiCard.size(); ++i)
	{
		if (vZiCard.at(i - 1) == vZiCard.at(i))
		{
			return false;
		}
	}
	return true;
}

bool HHMJPlayerCard::checkTZ_7StairYiBanGao(bool is13)
{
	auto nLess = 6;
	if (!is13)
	{
		nLess = 7;
	}

	std::vector<uint8_t> vZiCard;
	auto nCount = getYiBanGao(m_vChuedCardTangZhi, vZiCard);
	if (nCount != nLess)
	{
		return false;
	}

	std::sort(vZiCard.begin(), vZiCard.end());

	for (int i = 1; i < vZiCard.size(); ++i)
	{
		if (vZiCard.at(i - 1) == vZiCard.at(i))
		{
			return false;
		}
	}
	return true;
}

//uint8_t HHMJPlayerCard::get147258369(std::vector<uint8_t> vCard, std::vector<uint8_t>& vZiCards)
//{
//	vZiCards.clear();
//
//	std::vector<std::vector<uint8_t>> vTypeCards;
//	vTypeCards.resize(eCT_Tiao + 1);
//	
//	uint8_t ect_type[3];
//	memset(ect_type, eCT_Max, 3);
//
//	for (auto nCard : vCard)
//	{
//		auto eType = card_Type(nCard);
//		if (eType == eCT_Wan)
//		{
//			vTypeCards[eCT_Wan].push_back(nCard);
//		}
//		else if (eType == eCT_Tong)
//		{
//			vTypeCards[eCT_Tong].push_back(nCard);
//		}
//		else if (eType == eCT_Tiao)
//		{
//			vTypeCards[eCT_Tiao].push_back(nCard);
//		}
//		else
//		{
//			vZiCards.push_back(nCard);
//			continue;
//		}
//
//		auto nVal = card_Value(nCard);
//		if (ect_type[0] == eCT_Max && (nVal == 1 || nVal == 4 || nVal == 7))
//		{
//			ect_type[0] = eType;
//		}
//		if (ect_type[1] == eCT_Max && (nVal == 2 || nVal == 5 || nVal == 8))
//		{
//			ect_type[1] = eType;
//		}
//		if (ect_type[2] == eCT_Max && (nVal == 3 || nVal == 6 || nVal == 9))
//		{
//			ect_type[2] = eType;
//		}
//	}
//
//	if (ect_type[0] == eCT_Max || ect_type[1] == eCT_Max || ect_type[2] == eCT_Max || 
//		ect_type[0] == ect_type[1] || ect_type[0] == ect_type[2] || ect_type[1] == ect_type[2] ||
//		vTypeCards[ect_type[0]].size() > 3 || vTypeCards[ect_type[0]].empty() ||
//		vTypeCards[ect_type[1]].size() > 3 || vTypeCards[ect_type[1]].empty() ||
//		vTypeCards[ect_type[2]].size() > 3 || vTypeCards[ect_type[2]].empty())
//	{
//		return 0;
//	}
//
//	for (int i = 0; i < 3; ++i)
//	{
//		auto eType = ect_type[i];
//		std::sort(vTypeCards[eType].begin(), vTypeCards[eType].end());
//	}
//	
//	for (int i = 0; i < 3; ++i)
//	{
//		auto eType = ect_type[i];
//		for (int j = 1; j < vTypeCards[eType].size(); ++j)
//		{
//			if (vTypeCards[eType].at(j - 1) + 3 != vTypeCards[eType].at(j) && 
//				vTypeCards[eType].at(j - 1) + 6 != vTypeCards[eType].at(j))
//			{
//				return 0;
//			}
//		}
//	}
//
//	return vTypeCards[eCT_Wan].size() + vTypeCards[eCT_Tong].size() + vTypeCards[eCT_Tiao].size();
//}

uint8_t HHMJPlayerCard::get147258369(std::vector<uint8_t> vCard, std::vector<uint8_t>& vZiCards)
{
	vZiCards.clear();

	std::vector<std::vector<uint8_t>> vTypeCards;
	vTypeCards.resize(eCT_Tiao + 1);

	uint8_t ect_type[3];
	memset(ect_type, eCT_Max, 3);

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
		else
		{
			vZiCards.push_back(nCard);
			continue;
		}
	}

	if (vTypeCards[eCT_Wan].empty() || vTypeCards[eCT_Tong].empty() || vTypeCards[eCT_Tiao].empty())
	{
		return 0;
	}

	std::vector<uint8_t> vStartVal;
	for (uint8_t i = eCT_Wan; i <= eCT_Tiao; ++i)
	{
		std::sort(vTypeCards[i].begin(), vTypeCards[i].end());

		auto nStart = card_Value(vTypeCards[i].at(0)) % 3;
		vStartVal.push_back(nStart);

		for (int j = 1; j < vTypeCards[i].size(); ++j)
		{
			auto nCardVal = card_Value(vTypeCards[i].at(j)) % 3;
			if (nCardVal != nStart)
			{
				return 0;
			}
			if (vTypeCards[i].at(j) == vTypeCards[i].at(j - 1))
			{
				return 0;
			}
		}
	}

	bool isSame = true;
	for (int i = 1; i < vStartVal.size(); ++i)
	{
		if (vStartVal.at(i) != vStartVal.at(i - 1))
		{
			isSame = false;
			break;
		}
	}

	if (isSame)
	{
		return 0; // 一般高
	}

	return vTypeCards[eCT_Wan].size() + vTypeCards[eCT_Tong].size() + vTypeCards[eCT_Tiao].size();
}

uint8_t HHMJPlayerCard::getRuanLan(std::vector<uint8_t> vCard, std::vector<uint8_t>& vZiCards)
{
	vZiCards.clear();

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
		else
		{
			vZiCards.push_back(nCard);
		}
	}

	for (uint8_t i = eCT_Wan; i <= eCT_Tiao; ++i)
	{
		std::sort(vTypeCards[i].begin(), vTypeCards[i].end());
	}

	for (uint8_t i = eCT_Wan; i <= eCT_Tiao; ++i)
	{
		for (int j = 1; j < vTypeCards[i].size(); ++j)
		{
			if (vTypeCards[i].at(j) - vTypeCards[i].at(j - 1) < 3)
			{
				return 0;
			}
		}
	}

	return vTypeCards[eCT_Wan].size() + vTypeCards[eCT_Tong].size() + vTypeCards[eCT_Tiao].size();
}

uint8_t HHMJPlayerCard::getYiBanGao(std::vector<uint8_t> vCard, std::vector<uint8_t>& vZiCards)
{
	vZiCards.clear();

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
		else
		{
			vZiCards.push_back(nCard);
		}
	}

	//if (vTypeCards[eCT_Wan].empty() || vTypeCards[eCT_Tong].empty() || vTypeCards[eCT_Tiao].empty())
	//{
	//	return 0;
	//}
	auto nVal = 0;
	if (vTypeCards[eCT_Wan].empty() == false)
	{
		nVal = card_Value(vTypeCards[eCT_Wan].at(0));
	}
	else if (vTypeCards[eCT_Tong].empty() == false)
	{
		nVal = card_Value(vTypeCards[eCT_Tong].at(0));
	}
	else if (vTypeCards[eCT_Tiao].empty() == false)
	{
		nVal = card_Value(vTypeCards[eCT_Tiao].at(0));
	}
	auto nStart = nVal % 3;

	for (uint8_t i = eCT_Wan; i <= eCT_Tiao; ++i)
	{
		if (vTypeCards[i].empty())
		{
			continue;
		}

		std::sort(vTypeCards[i].begin(), vTypeCards[i].end());

		for (int j = 0; j < vTypeCards[i].size(); ++j)
		{
			auto nCardVal = card_Value(vTypeCards[i].at(j)) % 3;
			if (nCardVal != nStart)
			{
				return 0;
			}
		}

		for (int j = 1; j < vTypeCards[i].size(); ++j)
		{
			if (vTypeCards[i].at(j) == vTypeCards[i].at(j - 1))
			{
				return 0;
			}
		}
	}

	return vTypeCards[eCT_Wan].size() + vTypeCards[eCT_Tong].size() + vTypeCards[eCT_Tiao].size();
}

uint8_t HHMJPlayerCard::haveYiTiaoLong(std::vector<uint8_t> vCard)
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

void HHMJPlayerCard::addGangTimes(uint8_t nCard, uint8_t nIdx)
{
	bool bFind = false;
	for (auto t : m_vGangTimes)
	{
		if (t.nGangCard == nCard && t.nIdx == nIdx)
		{
			bFind = true;
			break;
		}
	}

	if (bFind)
	{
		return;
	}

	tSongGangIdx songGang;
	songGang.nGangCard = nCard;
	songGang.nIdx = nIdx;
	m_vGangTimes.push_back(songGang);
}

bool HHMJPlayerCard::isAllZiPai(VEC_CARD vCards)
{
	for (auto c : vCards)
	{
		auto t = card_Type(c);
		if (t != eCT_Feng && t != eCT_Jian)
		{
			return false;
		}
	}
	return true;
}

void HHMJPlayerCard::getTangZhiCard(VEC_CARD& vCards)
{
	vCards.clear();
	vCards.insert(vCards.begin(), m_vChuedCardTangZhi.begin(), m_vChuedCardTangZhi.end());
}