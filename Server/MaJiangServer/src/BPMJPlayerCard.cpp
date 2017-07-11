#include "BPMJPlayerCard.h"
#include "log4z.h"
#include "MJCard.h"
#include "assert.h"
#include "TestHuiModule.h"

#define MJPAI_HUNMAX 4

bool BPMJPlayerCard::canEatCard(uint8_t nCard, uint8_t& nWithA, uint8_t& nWithB)
{
	if (nCard == m_nHuiCard)
	{
		return false;
	}

	auto eTypeC = card_Type(nCard);

	if (nWithA == 0 && nWithB == 0)
	{
		if (eTypeC != eCT_Tiao && eTypeC != eCT_Tong && eTypeC != eCT_Wan)
		{
			return false;
		}

		std::vector<uint8_t>::iterator it_a, it_b;
		it_a = it_b = m_vCards[eTypeC].end();
		
		if (nCard + 1 != m_nHuiCard)
			it_a = std::find(m_vCards[eTypeC].begin(), m_vCards[eTypeC].end(), nCard + 1);
		if (nCard + 2 != m_nHuiCard)
			it_b = std::find(m_vCards[eTypeC].begin(), m_vCards[eTypeC].end(), nCard + 2);
		if (it_a != m_vCards[eTypeC].end() && it_b != m_vCards[eTypeC].end())
		{
			return true;
		}
		
		it_a = it_b = m_vCards[eTypeC].end();
		if (nCard - 1 != m_nHuiCard)
			it_a = std::find(m_vCards[eTypeC].begin(), m_vCards[eTypeC].end(), nCard - 1);
		if (nCard - 2 != m_nHuiCard)
			it_b = std::find(m_vCards[eTypeC].begin(), m_vCards[eTypeC].end(), nCard - 2);
		if (it_a != m_vCards[eTypeC].end() && it_b != m_vCards[eTypeC].end())
		{
			return true;
		}

		it_b = m_vCards[eTypeC].end();
		if (nCard + 1 != m_nHuiCard)
			it_b = std::find(m_vCards[eTypeC].begin(), m_vCards[eTypeC].end(), nCard + 1);
		if (it_a != m_vCards[eTypeC].end() && it_b != m_vCards[eTypeC].end())
		{
			return true;
		}
	}
	else
	{
		if (nWithA == m_nHuiCard || nWithB == m_nHuiCard)
		{
			return false;
		}

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

bool BPMJPlayerCard::canPengWithCard(uint8_t nCard)
{
	if (m_vLouPeng.end() != std::find(m_vLouPeng.begin(), m_vLouPeng.end(), nCard))
	{
		return false;
	} 

	return MJPlayerCard::canPengWithCard(nCard);
}

void BPMJPlayerCard::reset()
{
	MJPlayerCard::reset();
	m_nSongZhiGangIdx = -1;
	m_vLouPeng.clear();
	m_isLouHu = false;
	m_vMingGangSongIdx.clear();
	m_vPengSongIdx.clear();

	m_isBiMenHu = false;
	m_isHuiPai = false;
	m_is7Pair = false;
	m_nHuiCard = uint8_t(-1);
}

bool BPMJPlayerCard::getCardInfo(Json::Value& jsPeerCards)
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

bool BPMJPlayerCard::onDoHu(bool isZiMo, bool isHaiDiLoaYue, uint8_t nCard, std::vector<uint16_t>& vHuTypes, int32_t& nFanCnt)
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

	//if (checkHu(nCard)/*isHoldCardCanHu()*/ == false && nCard != m_nBaoCard)
	//{
	//	LOGFMTE("do hu act , but can not hu ? why ? bug card = %u ", nCard);
	//	debugCardInfo();
	//	if (!isZiMo)
	//	{
	//		funRemoveAddToCard(nCard);
	//	}
	//	return false;
	//}

	// check fanxing and bei shu 
	if (isZiMo && isHaiDiLoaYue)
	{
		vHuTypes.push_back(eFanxing_HaiDiLaoYue);
		nFanCnt++;
	}

	VEC_CARD vHoldCards;
	getHoldCard(vHoldCards);

	auto it = std::find(vHoldCards.begin(), vHoldCards.end(), m_nHuiCard);
	if (it == vHoldCards.end())
	{
		//没有会牌，穷胡加倍
		nFanCnt++;
	}

	if (checkDuiDuiHu())
	{
		if (m_nJIang == nCard && vHoldCards.size() < 3)
		{
			vHuTypes.push_back(eFanxing_MingPiao);
			nFanCnt += 3;
		}
		else
		{
			vHuTypes.push_back(eFanxing_DuiDuiHu);
			nFanCnt++;
		}
	}

	if (check7Pair() && m_is7Pair)
	{
		vHuTypes.push_back(eFanxing_QiDui);
		nFanCnt++;
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

bool BPMJPlayerCard::check7Pair()
{
	VEC_CARD vHoldCard, vEated, vPengGang;
	getHoldCard(vHoldCard);

	getEatedCard(vEated);
	getPengedCard(vPengGang);
	getMingGangedCard(vPengGang);
	getAnGangedCard(vPengGang);

	if (vHoldCard.size() == 14)
	{
		if (!m_isHuiPai && MJPlayerCard::canHoldCard7PairHu())
		{
			return true;
		}

		if (m_isHuiPai)
		{
			TestHuiModule huiModule(m_nHuiCard, m_is7Pair, vEated, vPengGang);
			bool bRet = huiModule.vecHu7Pair(m_nHuiCard, vHoldCard);
			if (bRet)
			{
				return true;
			}
		}
	}
	return false;
}

uint8_t BPMJPlayerCard::isMingPiao(uint8_t nHuCard)
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

	auto type = card_Type(nHuCard);
	if (type >= eCT_Max)
	{
		LOGFMTE("invalid card type for card = %u", nHuCard);
		return false;
	}
	addCardToVecAsc(m_vCards[type], nHuCard);

	if (checkDuiDuiHu())
	{
		VEC_CARD vHoldCards;
		getHoldCard(vHoldCards);

		funRemoveAddToCard(nHuCard);

		if (m_nJIang == nHuCard && vHoldCards.size() < 3)
		{
			return eFanxing_MingPiao;
		}
		else
		{
			return eFanxing_DuiDuiHu;
		}
	}
	funRemoveAddToCard(nHuCard);
	return 0;
}

uint8_t BPMJPlayerCard::getSongGangIdx()
{
	return m_nSongZhiGangIdx;
}

void BPMJPlayerCard::setSongGangIdx(uint8_t nIdx)
{
	m_nSongZhiGangIdx = nIdx;
}

bool BPMJPlayerCard::canHuWitCard(uint8_t nCard)
{
	auto eType = card_Type(nCard);
	if (eType >= eCT_Max)
	{
		LOGFMTE("parse card type error ,canHuWitCard have this card = %u", nCard);
		return false;
	}

	addCardToVecAsc(m_vCards[eType], nCard);

	bool bSelfHu = false;

	if (m_isHuiPai)
	{
		VEC_CARD vHoldCard, vEated, vPengGang;
		getHoldCard(vHoldCard);

		getEatedCard(vEated);
		getPengedCard(vPengGang);
		getMingGangedCard(vPengGang);
		getAnGangedCard(vPengGang);

		TestHuiModule huiModule(m_nHuiCard, m_is7Pair, vEated, vPengGang);
		bSelfHu = huiModule.testHuPai(vHoldCard);
		//if (bSelfHu)
		//{
		//	bSelfHu = checkHu(true, nCard);
		//}
		if (!m_isBiMenHu && m_vPenged.size() == 0 && m_vGanged.size() == 0 && m_vEated.size() == 0)
		{
			bSelfHu = false;
		}
	}
	else
	{
		bSelfHu = checkHu(false, nCard);
	}
	
	auto iter = std::find(m_vCards[eType].begin(), m_vCards[eType].end(), nCard);
	m_vCards[eType].erase(iter);
	//debugCardInfo();
	return bSelfHu;
}

bool BPMJPlayerCard::isHoldCardCanHu()
{
	if (m_isHuiPai)
	{
		VEC_CARD vHoldCard, vEated, vPengGang;
		getHoldCard(vHoldCard);

		getEatedCard(vEated);
		getPengedCard(vPengGang);
		getMingGangedCard(vPengGang);
		getAnGangedCard(vPengGang);

		TestHuiModule huiModule(m_nHuiCard, m_is7Pair, vEated, vPengGang);
		bool bSelfHu = huiModule.testHuPai(vHoldCard);
		//if (bSelfHu)
		//{
		//	return checkHu(true, getNewestFetchedCard());
		//}
		if (!m_isBiMenHu && m_vPenged.size() == 0 && m_vGanged.size() == 0 && m_vEated.size() == 0)
		{
			bSelfHu = false;
		}
		return bSelfHu;
	}
	else
	{
		bool bSelfHu = MJPlayerCard::isHoldCardCanHu();
		if (bSelfHu)
		{
			return checkHu(true, getNewestFetchedCard());
		}
	}
	return false;
}

bool BPMJPlayerCard::checkDuiDuiHu()
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

bool BPMJPlayerCard::pickoutHoldCard(uint8_t nCard)
{
	auto eType = card_Type(nCard);
	
	std::vector<uint8_t>::iterator it = std::find(begin(m_vCards[eType]), end(m_vCards[eType]), nCard);
	if (it != m_vCards[eType].end())
	{
		m_vCards[eType].erase(it);
		return true;
	}
	return false;
}

bool BPMJPlayerCard::checkKeZhi(bool isHold, uint8_t nCard)
{
	if (m_vPenged.size() > 0 || m_vGanged.size() > 0 || m_vAnGanged.size() > 0)
	{
		return true;
	}

	VEC_CARD vCards;
	getHoldCard(vCards);

	VEC_CARD vFoundCard;
	for (auto nCard_ : vCards)
	{
		auto type_ = card_Type(nCard_);
		if (type_ == eCT_Jian && std::count(vCards.begin(), vCards.end(), nCard_) >= 2)
		{
			return true;
		}
		else if (type_ == eCT_Feng)
		{
			if (std::count(vCards.begin(), vCards.end(), nCard_) >= 3)
			{
				return true;
			}
		}
		else if (type_ == eCT_Tiao || type_ == eCT_Tong || type_ == eCT_Wan)
		{
			if (std::count(vCards.begin(), vCards.end(), nCard_) >= 3)
			{
				VEC_CARD::iterator it = std::find(vFoundCard.begin(), vFoundCard.end(), nCard_);
				if (it == vFoundCard.end())
				{
					vFoundCard.push_back(nCard_);
				}
			}
		}
	}

	auto nCount = vFoundCard.size();
	bool bHu = false;

	if (nCount == 1)
	{
		VEC_CARD vCardTemp;
		vCardTemp.push_back(vFoundCard.at(0));
		bHu = takeKeZhi(vCardTemp);
	}
	else if (nCount == 2)
	{
		do 
		{
			VEC_CARD vCardTemp;
			vCardTemp.push_back(vFoundCard.at(0));
			bHu = takeKeZhi(vCardTemp);
			if (bHu) break;

			vCardTemp.clear();
			vCardTemp.push_back(vFoundCard.at(1));
			bHu = takeKeZhi(vCardTemp);
			if (bHu) break;

			vCardTemp.clear();
			vCardTemp.push_back(vFoundCard.at(0));
			vCardTemp.push_back(vFoundCard.at(1));
			bHu = takeKeZhi(vCardTemp);
		} while (0);
	}
	else if (nCount == 3)
	{
		do 
		{
			VEC_CARD vCardTemp;
			vCardTemp.push_back(vFoundCard.at(0));
			bHu = takeKeZhi(vCardTemp);
			if (bHu) break;

			vCardTemp.clear();
			vCardTemp.push_back(vFoundCard.at(1));
			bHu = takeKeZhi(vCardTemp);
			if (bHu) break;

			vCardTemp.clear();
			vCardTemp.push_back(vFoundCard.at(2));
			bHu = takeKeZhi(vCardTemp);
			if (bHu) break;

			vCardTemp.clear();
			vCardTemp.push_back(vFoundCard.at(0));
			vCardTemp.push_back(vFoundCard.at(1));
			bHu = takeKeZhi(vCardTemp);
			if (bHu) break;

			vCardTemp.clear();
			vCardTemp.push_back(vFoundCard.at(0));
			vCardTemp.push_back(vFoundCard.at(2));
			bHu = takeKeZhi(vCardTemp);
			if (bHu) break;

			vCardTemp.clear();
			vCardTemp.push_back(vFoundCard.at(1));
			vCardTemp.push_back(vFoundCard.at(2));
			bHu = takeKeZhi(vCardTemp);
			if (bHu) break;

			vCardTemp.clear();
			vCardTemp.push_back(vFoundCard.at(0));
			vCardTemp.push_back(vFoundCard.at(1));
			vCardTemp.push_back(vFoundCard.at(2));
			bHu = takeKeZhi(vCardTemp);
		} while (0);
	}
	else if (nCount == 4)
	{
		bHu = true;
	}
	return bHu;
}

bool BPMJPlayerCard::takeKeZhi(VEC_CARD vCards)
{
	bool bRet = false;

	for (auto nCardTemp_ : vCards)
	{
		for (int i = 0; i < 3; ++i)
		{
			if (pickoutHoldCard(nCardTemp_) == false)
			{
				assert(0);
			}
		}
	}

	bRet = MJPlayerCard::isHoldCardCanHu();

	for (auto nCardTemp_ : vCards)
	{
		for (int i = 0; i < 3; ++i) addDistributeCard(nCardTemp_);
	}

	return bRet;
}

uint8_t BPMJPlayerCard::checkJiaHu(uint8_t nCard, bool isZiMo)
{
	auto eType = card_Type(nCard);

	if (eType != eCT_Wan && eType != eCT_Tong && eType != eCT_Tiao)
	{
		return 0;
	}

	if (isZiMo)
	{
		std::vector<uint8_t>::iterator it_2 = std::find(begin(m_vCards[eType]), end(m_vCards[eType]), nCard);
		if (it_2 == m_vCards[eType].end())
		{
			return 0;
		}
	}

	bool bMiddle = true;
	bool bRight = true;
	bool bLeft = true;
	std::vector<uint8_t>::iterator it_11 = std::find(begin(m_vCards[eType]), end(m_vCards[eType]), nCard + 2);
	if (it_11 == m_vCards[eType].end())
	{
		bRight = false;
	}

	std::vector<uint8_t>::iterator it_1 = std::find(begin(m_vCards[eType]), end(m_vCards[eType]), nCard + 1);
	if (it_1 == m_vCards[eType].end())
	{
		bMiddle = false;
		bRight = false;
	}

	std::vector<uint8_t>::iterator it_3 = std::find(begin(m_vCards[eType]), end(m_vCards[eType]), nCard - 1);
	if (it_3 == m_vCards[eType].end())
	{
		bMiddle = false;
		bLeft = false;
	}

	std::vector<uint8_t>::iterator it_33 = std::find(begin(m_vCards[eType]), end(m_vCards[eType]), nCard - 2);
	if (it_33 == m_vCards[eType].end())
	{
		bLeft = false;
	}

	if (isZiMo)
	{
		pickoutHoldCard(nCard);
	}

	std::set<uint8_t> vHuCards;
	getCanHuCards_1(vHuCards);

	std::vector<uint8_t> vHuCards_;
	for (auto c : vHuCards)
	{
		addDistributeCard(c);
		bool bSelfHu = MJPlayerCard::isHoldCardCanHu();
		pickoutHoldCard(c);

		if (bSelfHu)
		{
			vHuCards_.push_back(c);
		}
	}

	if (isZiMo)
	{
		addDistributeCard(nCard);
	}

	//---------------------------------------胡多张，有一个夹，但不能多夹
	if (vHuCards_.size() > 1) 
	{
		return 0;
	}

	auto nVal = card_Value(nCard);
	if (bLeft)
	{
		if (nVal == 3)
		{
			pickoutHoldCard(nCard - 2);
			pickoutHoldCard(nCard - 1);
			if (isZiMo)
			{
				pickoutHoldCard(nCard);
			}

			bLeft = MJPlayerCard::isHoldCardCanHu();

			addDistributeCard(nCard - 2);
			addDistributeCard(nCard - 1);
			if (isZiMo)
			{
				addDistributeCard(nCard);
			}
		} 
		else
		{
			bLeft = false;
		}
	}
	
	if (bMiddle)
	{
		pickoutHoldCard(nCard - 1);
		if (isZiMo)
		{
			pickoutHoldCard(nCard);
		}
		pickoutHoldCard(nCard + 1);

		bMiddle = MJPlayerCard::isHoldCardCanHu();

		addDistributeCard(nCard - 1);
		if (isZiMo)
		{
			addDistributeCard(nCard);
		}
		addDistributeCard(nCard + 1);
	}

	if (bRight)
	{
		if (nVal == 7)
		{
			pickoutHoldCard(nCard + 1);
			pickoutHoldCard(nCard + 2);
			if (isZiMo)
			{
				pickoutHoldCard(nCard);
			}

			bRight = MJPlayerCard::isHoldCardCanHu();

			addDistributeCard(nCard + 1);
			addDistributeCard(nCard + 2);
			if (isZiMo)
			{
				addDistributeCard(nCard);
			}
		} 
		else
		{
			bRight = false;
		}
	}

	if (bLeft)
	{
		return 1;
	}
	else if (bMiddle)
	{
		return 2;
	}
	else if (bRight)
	{
		return 3;
	}
	return 0;
}

bool BPMJPlayerCard::checkHu(bool isHold, uint8_t nCard)
{
	auto pfunCheck19 = [](VEC_CARD vCards)->bool {
		for (auto nCard_ : vCards)
		{
			auto type_ = card_Type(nCard_);
			if (type_ == eCT_Feng || type_ == eCT_Jian)
			{
				return true;
			}
			else if (type_ == eCT_Tiao || type_ == eCT_Tong || type_ == eCT_Wan)
			{
				auto value_ = card_Value(nCard_);
				if (value_ == 1 || value_ == 9)
				{
					return true;
				}
			}
		}
		return false;
	};

	auto pfunCheckJianGan = [](VEC_CARD vMingGang, VEC_CARD vAnGang)->bool {
		VEC_CARD vCards;
		vCards.insert(vCards.end(), vMingGang.begin(), vMingGang.end());
		vCards.insert(vCards.end(), vAnGang.begin(), vAnGang.end());

		VEC_CARD vJian;
		for (auto nCard_ : vCards)
		{
			auto type_ = card_Type(nCard_);
			if (type_ == eCT_Jian && vJian.end() == std::find(vJian.begin(), vJian.end(), nCard_))
			{
				vJian.push_back(nCard_);
			}
		}
		if (vJian.size() == 3)
		{
			return true;
		}
		return false;
	};

	auto pfunCheck3Men = [](VEC_CARD vCardType, VEC_CARD vPenged, VEC_CARD vGanged, VEC_CARD vAnGanged, VEC_CARD vEated)->bool {
		if (vCardType.size() >= 3)
		{
			return true;
		}

		VEC_CARD vCards;
		vCards.insert(vCards.end(), vPenged.begin(), vPenged.end());
		vCards.insert(vCards.end(), vGanged.begin(), vGanged.end());
		vCards.insert(vCards.end(), vAnGanged.begin(), vAnGanged.end());
		vCards.insert(vCards.end(), vEated.begin(), vEated.end());

		for (auto nCard_ : vCards)
		{
			auto type_ = card_Type(nCard_);
			if (type_ != eCT_Tiao && type_ != eCT_Tong && type_ != eCT_Wan)
			{
				continue;
			}
			if (vCardType.end() == std::find(vCardType.begin(), vCardType.end(), type_))
			{
				vCardType.push_back(type_);
			}
			if (vCardType.size() >= 3)
			{
				return true;
			}
		}
		return false;
	};

	VEC_CARD vHoldCard, vEated, vPengGang;
	getHoldCard(vHoldCard);

	getEatedCard(vEated);
	getPengedCard(vPengGang);
	getMingGangedCard(vPengGang);
	getAnGangedCard(vPengGang);

	auto nHunNum = std::count(vHoldCard.begin(), vHoldCard.end(), m_nHuiCard);

	if (vHoldCard.size() == 14)
	{
		if (!m_isHuiPai && MJPlayerCard::canHoldCard7PairHu())
		{
			return true;
		}

		if (m_isHuiPai)
		{
			TestHuiModule huiModule(m_nHuiCard, m_is7Pair, vEated, vPengGang);
			bool bRet = huiModule.vecHu7Pair(m_nHuiCard, vHoldCard);
			if (bRet)
			{
				return true;
			}
		}
	}

	if (m_isLouHu)
	{
		return false;
	}

	bool bSelfHu = MJPlayerCard::isHoldCardCanHu();
	while (bSelfHu)
	{
		//4、已开门（暗杠不算开门）
		if (!m_isBiMenHu && m_vPenged.size() == 0 && m_vGanged.size() == 0 && m_vEated.size() == 0)
		{
			bSelfHu = false;
			break;
		}
		//2、有幺九（宝牌不顶幺九）：胡牌时至少有一张幺九牌，东南西北中发白也可以算作幺九牌
		bool has19 = false;
		if (!pfunCheck19(m_vPenged) && !pfunCheck19(m_vGanged) && !pfunCheck19(m_vAnGanged) && !pfunCheck19(m_vEated) && !pfunCheck19(m_vCards[eCT_Tiao]) && 
			!pfunCheck19(m_vCards[eCT_Tong]) && !pfunCheck19(m_vCards[eCT_Wan]) && m_vCards[eCT_Feng].size() == 0 && m_vCards[eCT_Jian].size() == 0)
		{
			bSelfHu = false;
			break;
		}
		//3、有平胡：胡牌时至少有一刻子或杠，或有中发白其中一对。（刻子：三张一样的牌）
		if (!checkKeZhi(isHold, nCard))
		{
			bSelfHu = false;
			break;
		}
		//6、中发白在扛后也可正常胡牌，没有飘胡限制。
		if (pfunCheckJianGan(m_vGanged, m_vAnGanged))
		{
			break;
		}
		//1、三门齐：有万牌、条牌、筒牌才能胡牌（清一色不需要三门齐）
		VEC_CARD vCardType;
		if (m_vCards[eCT_Tong].size() > 0) vCardType.push_back(eCT_Tong);
		if (m_vCards[eCT_Tiao].size() > 0) vCardType.push_back(eCT_Tiao);
		if (m_vCards[eCT_Wan].size() > 0) vCardType.push_back(eCT_Wan);
		if (!pfunCheck3Men(vCardType, m_vPenged, m_vGanged, m_vAnGanged, m_vEated) && nHunNum < 2)
		{
			bSelfHu = false;
			break;
		}
		break;
	}
	//debugCardInfo();
	return bSelfHu;
}

void BPMJPlayerCard::addSongGangIdx(uint8_t nCard, uint8_t nIdx, uint8_t eAct)
{
	tSongGangIdx tSong;
	tSong.eAct = eAct;
	tSong.nGangCard = nCard;
	tSong.nIdx = nIdx;
	m_vMingGangSongIdx.push_back(tSong);
}

void BPMJPlayerCard::addSongPengIdx(uint8_t nCard, uint8_t nIdx)
{
	tSongGangIdx tSong;
	tSong.eAct = eMJAct_Peng;
	tSong.nGangCard = nCard;
	tSong.nIdx = nIdx;
	m_vPengSongIdx.push_back(tSong);
}

// 只有4张牌的时候检查，如果吃过就不能吃碰杠，否则不能吃
// 返回值：0-不能吃碰杠；1-可以吃碰杠；2-不能吃
uint8_t BPMJPlayerCard::checkMingPiao()
{
	VEC_CARD vHoldCard;
	getHoldCard(vHoldCard);

	if (vHoldCard.size() != 4)
	{
		return 1;
	}
	
	if (m_vEated.empty() == false)
	{
		return 0;
	}

	return 2;
}

bool BPMJPlayerCard::getCanHuCards_1(std::set<uint8_t>& vCanHuCards)
{
	//if (is7PairTing())
	//{
	//	// when 7 pair ting , then must dan diao ;
	//	if (m_nDanDiao == 0)
	//	{
	//		LOGFMTE("why 7 pair ting , is not  dan diao ? bug ?????");
	//		return false;
	//	}
	//	vCanHuCards.insert(m_nDanDiao);
	//	return true;
	//}

	if (isTingPai() == false)
	{
		vCanHuCards.clear();
		return false;
	}

	// copy card for use ;
	SET_NOT_SHUN vNotShun[eCT_Max];
	VEC_CARD vCards[eCT_Max];
	std::vector<uint8_t> vNotEmptyShunIdx;
	for (uint8_t nIdx = 0; nIdx < eCT_Max; ++nIdx)
	{
		vCards[nIdx] = m_vCards[nIdx];
		getNotShuns(vCards[nIdx], vNotShun[nIdx], eCT_Feng == nIdx || eCT_Jian == nIdx);
		if (vNotShun[nIdx].empty() == false)
		{
			vNotEmptyShunIdx.push_back(nIdx);
		}
	}

	if (vNotEmptyShunIdx.size() > 2 || vNotEmptyShunIdx.empty())
	{
		LOGFMTE("already ting pai ,why no que card = %u", vNotEmptyShunIdx.size());
		return false;
	}

	// already 
	auto pfnGetCanHuCardIgnoreJiang = [this](VEC_CARD& vCardToFind, std::set<uint8_t>& vCanHuCards)
	{
		if (vCardToFind.empty())
		{
			return;
		}
		SET_NOT_SHUN vNotShun;
		auto type = card_Type(vCardToFind.front());
		auto bMustKe = (type == eCT_Feng || eCT_Jian == type);
		getNotShuns(vCardToFind, vNotShun, bMustKe);

		// pare can hu cards ;
		auto iter = vNotShun.begin();
		for (; iter != vNotShun.end(); ++iter)
		{
			if ((*iter).getSize() != 2)
			{
				continue;
			}
			auto vCheckCard = (*iter).vCards;
			auto aValue = vCheckCard[0];
			auto bValue = vCheckCard[1];

			if (aValue == bValue)
			{
				vCanHuCards.insert(aValue);
				continue;
			}

			if (bMustKe)
			{
				continue;
			}

			// when not must ke zi ;
			if (aValue > bValue)
			{
				aValue += bValue;
				bValue = aValue - bValue;
				aValue = aValue - bValue;
			}

			if (aValue + 2 == bValue)
			{
				vCanHuCards.insert((aValue + 1));
				continue;
			}

			if (aValue + 1u == bValue)
			{
				auto a = aValue - 1;
				auto b = bValue + 1;
				if (card_Value(a) >= 1)
				{
					vCanHuCards.insert(a);
				}

				if (card_Value(b) <= 9)
				{
					vCanHuCards.insert(b);
				}
			}
		}
	};
	if (vNotEmptyShunIdx.size() == 2)
	{
		// must have jiang 
		// asume idx 0 have jiang 
		SET_NOT_SHUN& vFirst = vNotShun[vNotEmptyShunIdx[0]];
		SET_NOT_SHUN& vSecond = vNotShun[vNotEmptyShunIdx[1]];
		for (auto& v : vFirst)
		{
			if (v.getSize() == 2 && v.vCards[0] == v.vCards[1])
			{
				// other type shound have hu card ;
				pfnGetCanHuCardIgnoreJiang(vCards[vNotEmptyShunIdx[1]], vCanHuCards);
				break;
			}
		}

		// asume idx 1 have jiang ;
		for (auto& v : vSecond)
		{
			if (v.getSize() == 2 && v.vCards[0] == v.vCards[1])
			{
				// other type shound have hu card ;
				pfnGetCanHuCardIgnoreJiang(vCards[vNotEmptyShunIdx[0]], vCanHuCards);
				break;
			}
		}

		return vCanHuCards.size() > 0;
	}

	// only one que type 
	// check 9 card ;
	for (uint8_t nValue = 1; nValue <= 9; ++nValue)
	{
		uint8_t card = make_Card_Num((eMJCardType)vNotEmptyShunIdx[0], nValue);
		if (canHuWitCard_1(card))
		{
			vCanHuCards.insert(card);
		}
	}
	return vCanHuCards.size() > 0;
}

bool BPMJPlayerCard::canHuWitCard_1(uint8_t nCard)
{
	auto eType = card_Type(nCard);
	if (eType >= eCT_Max)
	{
		LOGFMTE("parse card type error ,canHuWitCard have this card = %u", nCard);
		return false;
	}

	addCardToVecAsc(m_vCards[eType], nCard);
	bool bSelfHu = MJPlayerCard::isHoldCardCanHu();
	auto iter = std::find(m_vCards[eType].begin(), m_vCards[eType].end(), nCard);
	m_vCards[eType].erase(iter);
	//debugCardInfo();
	return bSelfHu;
}

bool BPMJPlayerCard::isTingPai()
{
	//if (is7PairTing())
	//{
	//	return true;
	//}

	auto nct = getMiniQueCnt(m_vCards);
	if (nct <= 1)
	{
		if (0 == nct)
		{
			LOGFMTE("this is already hu ? why you check ting ?");
		}
		return true;
	}
	return false;
}

bool BPMJPlayerCard::getCanHuCards(std::set<uint8_t>& vCanHuCards)
{
	//if (is7PairTing())
	//{
	//	// when 7 pair ting , then must dan diao ;
	//	if (m_nDanDiao == 0)
	//	{
	//		LOGFMTE("why 7 pair ting , is not  dan diao ? bug ?????");
	//		return false;
	//	}
	//	vCanHuCards.insert(m_nDanDiao);
	//	return true;
	//}

	if (isTingPai() == false)
	{
		vCanHuCards.clear();
		return false;
	}

	// copy card for use ;
	SET_NOT_SHUN vNotShun[eCT_Max];
	VEC_CARD vCards[eCT_Max];
	std::vector<uint8_t> vNotEmptyShunIdx;
	for (uint8_t nIdx = 0; nIdx < eCT_Max; ++nIdx)
	{
		vCards[nIdx] = m_vCards[nIdx];
		getNotShuns(vCards[nIdx], vNotShun[nIdx], eCT_Feng == nIdx || eCT_Jian == nIdx);
		if (vNotShun[nIdx].empty() == false)
		{
			vNotEmptyShunIdx.push_back(nIdx);
		}
	}

	if (vNotEmptyShunIdx.size() > 2 || vNotEmptyShunIdx.empty())
	{
		LOGFMTE("already ting pai ,why no que card = %u", vNotEmptyShunIdx.size());
		return false;
	}

	// already 
	auto pfnGetCanHuCardIgnoreJiang = [this](VEC_CARD& vCardToFind, std::set<uint8_t>& vCanHuCards)
	{
		if (vCardToFind.empty())
		{
			return;
		}
		SET_NOT_SHUN vNotShun;
		auto type = card_Type(vCardToFind.front());
		auto bMustKe = (type == eCT_Feng || eCT_Jian == type);
		getNotShuns(vCardToFind, vNotShun, bMustKe);

		// pare can hu cards ;
		auto iter = vNotShun.begin();
		for (; iter != vNotShun.end(); ++iter)
		{
			if ((*iter).getSize() != 2)
			{
				continue;
			}
			auto vCheckCard = (*iter).vCards;
			auto aValue = vCheckCard[0];
			auto bValue = vCheckCard[1];

			if (aValue == bValue)
			{
				vCanHuCards.insert(aValue);
				continue;
			}

			if (bMustKe)
			{
				continue;
			}

			// when not must ke zi ;
			if (aValue > bValue)
			{
				aValue += bValue;
				bValue = aValue - bValue;
				aValue = aValue - bValue;
			}

			if (aValue + 2 == bValue)
			{
				vCanHuCards.insert((aValue + 1));
				continue;
			}

			if (aValue + 1u == bValue)
			{
				auto a = aValue - 1;
				auto b = bValue + 1;
				if (card_Value(a) >= 1)
				{
					vCanHuCards.insert(a);
				}

				if (card_Value(b) <= 9)
				{
					vCanHuCards.insert(b);
				}
			}
		}
	};
	if (vNotEmptyShunIdx.size() == 2)
	{
		// must have jiang 
		// asume idx 0 have jiang 
		SET_NOT_SHUN& vFirst = vNotShun[vNotEmptyShunIdx[0]];
		SET_NOT_SHUN& vSecond = vNotShun[vNotEmptyShunIdx[1]];
		for (auto& v : vFirst)
		{
			if (v.getSize() == 2 && v.vCards[0] == v.vCards[1])
			{
				// other type shound have hu card ;
				pfnGetCanHuCardIgnoreJiang(vCards[vNotEmptyShunIdx[1]], vCanHuCards);
				break;
			}
		}

		// asume idx 1 have jiang ;
		for (auto& v : vSecond)
		{
			if (v.getSize() == 2 && v.vCards[0] == v.vCards[1])
			{
				// other type shound have hu card ;
				pfnGetCanHuCardIgnoreJiang(vCards[vNotEmptyShunIdx[0]], vCanHuCards);
				break;
			}
		}

		return vCanHuCards.size() > 0;
	}

	// only one que type 
	// check 9 card ;
	for (uint8_t nValue = 1; nValue <= 9; ++nValue)
	{
		uint8_t card = make_Card_Num((eMJCardType)vNotEmptyShunIdx[0], nValue);
		if (canHuWitCard(card))
		{
			vCanHuCards.insert(card);
		}
	}
	return vCanHuCards.size() > 0;
}

bool BPMJPlayerCard::canHoldCard7PairHu()
{
	if (m_is7Pair)
	{
		return MJPlayerCard::canHoldCard7PairHu();
	}
	else
	{
		return false;
	}
}
