#include "MJPlayerCard.h"
#include "MJCard.h"
#include "log4z.h"

MJPlayerCard::stNotShunCard::stNotShunCard(){ vCards.clear(); }
bool MJPlayerCard::stNotShunCard::operator != (const stNotShunCard& v)
{
	if (*this == v)
	{
		return false;
	}
	return true;
}

MJPlayerCard::stNotShunCard& MJPlayerCard::stNotShunCard::operator = (const stNotShunCard& v)
{
	vCards.clear();
	vCards.assign(v.vCards.begin(), v.vCards.end());
	return *this;
}

bool MJPlayerCard::stNotShunCard::operator == (const stNotShunCard& v)
{
	if (vCards.size() != v.vCards.size())
	{
		return false;
	}

	if (vCards.empty())
	{
		// will never come to here ;
		return true;
	}
	stNotShunCard self = *this;
	stNotShunCard nonConstObj = v;
	std::sort(self.vCards.begin(), self.vCards.end());
	std::sort(nonConstObj.vCards.begin(), nonConstObj.vCards.end());
	for (uint8_t nIdx = 0; nIdx < self.vCards.size(); ++nIdx)
	{
		if (self.vCards[nIdx] != nonConstObj.vCards[nIdx])
		{
			return false;
		}
	}
	return true;
}

bool MJPlayerCard::stNotShunCard::operator < (const stNotShunCard& v)const
{
	if (getSize() < v.getSize())
	{
		return true;
	}

	if (getSize() > v.getSize())
	{
		return false;
	}

	if (vCards.empty())
	{
		return false;
	}

	stNotShunCard self = *this;
	stNotShunCard nonConstObj = v;
	std::sort(self.vCards.begin(), self.vCards.end());
	std::sort(nonConstObj.vCards.begin(), nonConstObj.vCards.end());
	for (uint8_t nIdx = 0; nIdx < self.vCards.size(); ++nIdx)
	{
		if (self.vCards[nIdx] < nonConstObj.vCards[nIdx])
		{
			return true;
		}
		else if (self.vCards[nIdx] > nonConstObj.vCards[nIdx] )
		{
			return false;
		}
	}

	return false;
}

uint8_t MJPlayerCard::stNotShunCard::getLackCardCntForShun()
{
	if (vCards.empty())
	{
		LOGFMTE("not shun must not be empty ? error ");
		return 0;
	}

	auto type = card_Type(vCards.front());
	auto bMustKe = ( type == eCT_Feng || eCT_Jian == type );

	uint8_t nLackCnt = 0;
	std::sort(vCards.begin(),vCards.end());
	for (uint8_t nIdx = 0; nIdx < vCards.size(); )
	{
		uint8_t nValue = vCards[nIdx];
		if ((uint8_t)(nIdx + 1) >= vCards.size())
		{
			nLackCnt += 2;
			break;
		}

		uint8_t nNextV = vCards[nIdx + 1];
		if (nNextV == nValue)
		{
			nLackCnt += 1;
			nIdx += 2;
			continue;
		}

		if ( bMustKe == false && (nNextV - nValue <= 2))
		{
			nLackCnt += 1;
			nIdx += 2;
			continue;
		}

		nLackCnt += 2;
		nIdx += 1;
	}
	return nLackCnt;
}


// mj player card ;
void MJPlayerCard::reset()
{
	for (auto& vC : m_vCards)
	{
		vC.clear();
	}
	m_vChuedCard.clear();
	m_vPenged.clear();
	m_vGanged.clear();
	m_vEated.clear();
	m_vAnGanged.clear();
	m_nNesetFetchedCard = 0 ;
	m_nJIang = 0;
	m_nDanDiao = 0;
}

void MJPlayerCard::addDistributeCard(uint8_t nCardNum)
{
	auto eType = card_Type(nCardNum);
	addCardToVecAsc(m_vCards[eType],nCardNum);
}

bool MJPlayerCard::onGangCardBeRobot(uint8_t nCardNum)
{
	// must test vCard is orig vector ;
	auto eType = card_Type(nCardNum);
	if (eType >= eCT_Max)
	{
		LOGFMTE("parse card type error ,gang be robot have this card = %u", nCardNum);
		return false;
	}
	auto& vCard = m_vCards[eType];
	auto iter = std::find(vCard.begin(),vCard.end(),nCardNum);
	if (iter != vCard.end())
	{
		vCard.erase(iter);
		return true;
	}
	LOGFMTE("robot the gang card but player do not have card = %u",nCardNum);
	return false;
}

bool MJPlayerCard::onCardBeGangPengEat(uint8_t nCardNum)
{
	auto eType = card_Type(nCardNum);
	auto& vCard = m_vChuedCard;
	auto iter = std::find(vCard.begin(), vCard.end(), nCardNum);
	if (iter != vCard.end())
	{
		vCard.erase(iter);
		return true;
	}
	LOGFMTE("gang eat peng card but player do not have card = %u", nCardNum);
	return false;
}

bool MJPlayerCard::isHaveCard(uint8_t nCardNum)
{
	auto eType = card_Type(nCardNum);
	if (eType >= eCT_Max)
	{
		LOGFMTE("parse card type error so do not have this card = %u",nCardNum);
		return false;
	}

	auto& vCard = m_vCards[eType];
	auto iter = std::find(vCard.begin(), vCard.end(), nCardNum);
	return iter != vCard.end();
}

bool MJPlayerCard::canMingGangWithCard(uint8_t nCard)
{
	auto eType = card_Type(nCard);
	if (eType >= eCT_Max)
	{
		LOGFMTE("canMingGangWithCard parse card type error so do not have this card = %u", nCard);
		return false;
	}
	auto& vCard = m_vCards[eType];
	auto nCnt = std::count(vCard.begin(), vCard.end(), nCard);
	return nCnt == 3;
}

bool MJPlayerCard::canPengWithCard(uint8_t nCard)
{
	auto eType = card_Type(nCard);
	if (eType >= eCT_Max)
	{
		LOGFMTE("canPengWithCard parse card type error so do not have this card = %u", nCard);
		return false;
	}
	auto& vCard = m_vCards[eType];
	auto nCnt = std::count(vCard.begin(), vCard.end(), nCard);
	return nCnt >= 2;
}

bool MJPlayerCard::canEatCard(uint8_t nCard, uint8_t& nWithA, uint8_t& withB)
{
	auto eType = card_Type(nCard);
	if (eType >= eCT_Max)
	{
		LOGFMTE("canEatCard parse card type error so do not have this card = %u", nCard);
		return false;
	}

	if (eType != eCT_Tiao && eCT_Tong != eType && eCT_Wan != eType)
	{
		LOGFMTD("only wan , tiao , tong can do eat act");
		return false;
	}

	// ABX ;
	nWithA = nCard - 2;
	withB = nCard - 1;
	if (isHaveCard(nWithA) && isHaveCard(withB))
	{
		return true;
	}
	// AXB ;
	nWithA = nCard - 1;
	withB = nCard + 1;
	if (isHaveCard(nWithA) && isHaveCard(withB))
	{
		return true;
	}
	// XAB
	nWithA = nCard + 1;
	withB = nCard + 2;
	if (isHaveCard(nWithA) && isHaveCard(withB))
	{
		return true;
	}
	return false;
}

bool MJPlayerCard::canHuWitCard(uint8_t nCard)
{
	auto eType = card_Type(nCard);
	if (eType >= eCT_Max)
	{
		LOGFMTE("parse card type error ,canHuWitCard have this card = %u", nCard);
		return false;
	}

	addCardToVecAsc(m_vCards[eType], nCard);
	bool bSelfHu = isHoldCardCanHu();
	auto iter = std::find(m_vCards[eType].begin(), m_vCards[eType].end(),nCard);
	m_vCards[eType].erase(iter);
	//debugCardInfo();
	return bSelfHu;
}

bool MJPlayerCard::canAnGangWithCard(uint8_t nCard)
{
	auto eType = card_Type(nCard);
	if (eType >= eCT_Max)
	{
		LOGFMTE("canAnGangWithCard parse card type error so do not have this card = %u", nCard);
		return false;
	}
	auto& vCard = m_vCards[eType];
	auto nCnt = std::count(vCard.begin(), vCard.end(), nCard);
	return nCnt == 4;
}

bool MJPlayerCard::canBuGangWithCard(uint8_t nCard)
{
	auto eType = card_Type(nCard);
	if (eType >= eCT_Max)
	{
		LOGFMTE("canAnGangWithCard parse card type error so do not have this card = %u", nCard);
		return false;
	}
	auto& vCard = m_vCards[eType];
	auto nCnt = std::count(vCard.begin(), vCard.end(), nCard);
	auto npeng = std::count(m_vPenged.begin(), m_vPenged.end(), nCard);
	return nCnt == 1 && 0 < npeng;
}

//bool MJPlayerCard::isTingPai()
//{
//	if (is7PairTing())
//	{
//		return true;
//	}
//
//	SET_NOT_SHUN vNotShun[eCT_Max];
//	uint8_t nTingType = eCT_None;
//	uint8_t nTingTypeB = eCT_None;
//	for (uint8_t nType = eCT_None; nType < eCT_Max; ++nType)
//	{
//		auto& vCard = m_vCards[nType];
//		if (vCard.empty())
//		{
//			continue;
//		}
//
//		if (!getNotShuns(vCard, vNotShun[nType], eCT_Feng == nType || eCT_Jian == nType))
//		{
//			if (eCT_None != nTingType && nTingTypeB != eCT_None )
//			{
//				return false;
//			}
//
//			if (eCT_None != nTingType)
//			{
//				nTingType = nType;
//			}
//
//			if (eCT_None != nTingTypeB)
//			{
//				nTingTypeB = nType;
//			}
//		}
//	}
//
//	if (eCT_None == nTingType && eCT_None == nTingTypeB )
//	{
//		LOGFMTE( "why all type hold card is shun zi ?" );
//	}
//
//	auto pfnIsJiang = [](SET_NOT_SHUN& vNot)->bool
//	{
//		auto iter = vNot.begin();
//		for (; iter != vNot.end(); ++iter)
//		{
//			auto refNot = *iter;
//			if (refNot.getLackCardCntForShun() == 1)
//			{
//				if (refNot.vCards[0] == refNot.vCards[1])
//				{
//					return true;
//				}
//			}
//		}
//		return false;
//	};
//
//	auto pfnIsNormalQue = [](SET_NOT_SHUN& vNot)->bool
//	{
//		auto iter = vNot.begin();
//		for (; iter != vNot.end(); ++iter)
//		{
//			auto refNot = *iter;
//			if (refNot.getLackCardCntForShun() == 1)
//			{
//				return true;
//			}
//		}
//		return false;
//	};
//
//	auto pfnIsNormalQueSelfContainJiang = [](SET_NOT_SHUN& vNot)->bool
//	{
//		auto iter = vNot.begin();
//		for (; iter != vNot.end(); ++iter)
//		{
//			auto refNot = *iter;
//			if (refNot.getLackCardCntForShun() == 2 && refNot.getSize() == 1 )
//			{
//				return true;
//			}
//
//			if (refNot.getLackCardCntForShun() == 2 && refNot.getSize() == 4 )
//			{
//				std::sort(refNot.vCards.begin(),refNot.vCards.end());
//				if ((refNot.vCards[0] == refNot.vCards[1]) && (refNot.vCards[3] - refNot.vCards[2] <= 2))
//				{
//					return true;
//				}
//
//				if ((refNot.vCards[2] == refNot.vCards[3]) && (refNot.vCards[1] - refNot.vCards[0] <= 2))
//				{
//					return true;
//				}
//			}
//		}
//		return false;
//	};
//
//	if (eCT_None != nTingType && eCT_None == nTingTypeB)
//	{
//		return pfnIsNormalQueSelfContainJiang(vNotShun[nTingType]);
//	}
//	else if (eCT_None == nTingType && eCT_None != nTingTypeB)
//	{
//		return pfnIsNormalQueSelfContainJiang(vNotShun[nTingTypeB]);
//	}
//	else if (eCT_None != nTingType && eCT_None != nTingTypeB)
//	{
//		return (pfnIsNormalQue(vNotShun[nTingTypeB]) && pfnIsJiang(vNotShun[nTingType])) || (pfnIsNormalQue(vNotShun[nTingType]) && pfnIsJiang(vNotShun[nTingTypeB]));
//	}
//		
//	return false;
//}
bool MJPlayerCard::isTingPai()
{
	if (is7PairTing())
	{
		return true;
	}

	auto nct = getMiniQueCnt(m_vCards);
	if ( nct <= 1)
	{
		if (0 == nct)
		{
			LOGFMTE("this is already hu ? why you check ting ?");
		}
		return true;
	}
	return false;
}

bool MJPlayerCard::getHoldCardThatCanAnGang(VEC_CARD& vGangCards)
{
	for (auto& vCard : m_vCards)
	{
		if ( vCard.size() < 4 )
		{
			continue;
		}

		for (uint8_t nIdx = 0; (uint8_t)(nIdx + 3) < vCard.size();)
		{
			if (vCard[nIdx] == vCard[nIdx + 3])
			{
				vGangCards.push_back(vCard[nIdx]);
				nIdx += 4;
			}
			else
			{
				++nIdx;
			}
		}
	}
	return !vGangCards.empty();
}

bool MJPlayerCard::getHoldCardThatCanBuGang(VEC_CARD& vGangCards)
{
	for (auto& ref : m_vPenged)
	{
		if (isHaveCard(ref))
		{
			vGangCards.push_back(ref);
		}
	}

	return !vGangCards.empty();
}

//bool MJPlayerCard::isHoldCardCanHu()
//{
//	if (canHoldCard7PairHu())
//	{
//		return true;
//	}
//
//	SET_NOT_SHUN vNotShun[eCT_Max];
//	uint8_t nTingType = eCT_None;
//	for (uint8_t nType = eCT_None; nType < eCT_Max; ++nType)
//	{
//		auto& vCard = m_vCards[nType];
//		if (vCard.empty())
//		{
//			continue;
//		}
//
//		if (!getNotShuns(vCard, vNotShun[nType], eCT_Feng == nType || eCT_Jian == nType))
//		{
//			if (eCT_None != nTingType )
//			{
//				return false;
//			}
//			nTingType = nType;
//		}
//	}
//
//	if (eCT_None == nTingType )
//	{
//		LOGFMTE("why all type hold card is shun zi no jiang ? how to hu ?");
//		return false;
//	}
//
//	auto pfnIsJiang = [](SET_NOT_SHUN& vNot)->bool
//	{
//		auto iter = vNot.begin();
//		for (; iter != vNot.end(); ++iter)
//		{
//			auto refNot = *iter;
//			if (refNot.getLackCardCntForShun() == 1)
//			{
//				if (refNot.vCards[0] == refNot.vCards[1])
//				{
//					return true;
//				}
//			}
//		}
//		return false;
//	};
//
//	return pfnIsJiang(vNotShun[nTingType]);
//}
bool MJPlayerCard::isHoldCardCanHu()
{
	if (canHoldCard7PairHu())
	{
		return true;
	}

	if (getMiniQueCnt(m_vCards) == 0 )
	{
		return true;
	}
	return false;
}

void MJPlayerCard::onMoCard(uint8_t nMoCard)
{
	auto eType = card_Type(nMoCard);
	if (eType >= eCT_Max)
	{
		LOGFMTE("onMoCard parse card type error so do not have this card = %u", nMoCard);
		return ;
	}
	addCardToVecAsc(m_vCards[eType], nMoCard);
	m_nNesetFetchedCard = nMoCard;
	debugCardInfo();
}

bool MJPlayerCard::onPeng(uint8_t nCard)
{
	auto eType = card_Type(nCard);
	if (eType >= eCT_Max)
	{
		LOGFMTE("onPeng parse card type error so do not have this card = %u", nCard);
		return false;
	}
	
	auto& vCard = m_vCards[eType];
	auto nEraseCnt = 2;
	while (nEraseCnt-- > 0)
	{
		auto iter = std::find(vCard.begin(), vCard.end(), nCard);
		vCard.erase(iter);
	}

	addCardToVecAsc(m_vPenged,nCard);
	//debugCardInfo();
	return true;
}

bool MJPlayerCard::onMingGang(uint8_t nCard, uint8_t nGangGetCard)
{
	auto eType = card_Type(nCard);
	if (eType >= eCT_Max)
	{
		LOGFMTE("onMingGang parse card type error so do not have this card = %u", nCard);
		return false;
	}

	auto& vCard = m_vCards[eType];
	auto nEraseCnt = 3;
	while (nEraseCnt-- > 0)
	{
		auto iter = std::find(vCard.begin(), vCard.end(), nCard);
		vCard.erase(iter);
	}

	addCardToVecAsc(m_vGanged, nCard);
	
	// new get card ;
	auto eGetType = card_Type(nGangGetCard);
	addCardToVecAsc(m_vCards[eGetType],nGangGetCard);
	m_nNesetFetchedCard = nGangGetCard;
	//debugCardInfo();
	return true;
}

bool MJPlayerCard::onAnGang(uint8_t nCard, uint8_t nGangGetCard)
{
	auto eType = card_Type(nCard);
	if (eType >= eCT_Max)
	{
		LOGFMTE("onAnGang parse card type error so do not have this card = %u", nCard);
		return false;
	}

	auto& vCard = m_vCards[eType];
	auto nEraseCnt = 4;
	while (nEraseCnt-- > 0)
	{
		auto iter = std::find(vCard.begin(), vCard.end(), nCard);
		vCard.erase(iter);
	}

	//addCardToVecAsc(m_vGanged, nCard); 
	addCardToVecAsc(m_vAnGanged, nCard);
	// new get card ;
	auto eGetType = card_Type(nGangGetCard);
	addCardToVecAsc(m_vCards[eGetType], nGangGetCard);
	m_nNesetFetchedCard = nGangGetCard;
	//debugCardInfo();
	return true;
}

bool MJPlayerCard::onBuGang(uint8_t nCard, uint8_t nGangGetCard)
{
	auto eType = card_Type(nCard);
	if (eType >= eCT_Max)
	{
		LOGFMTE("onMingGang parse card type error so do not have this card = %u", nCard);
		return false;
	}

	// remove hold card 
	auto& vCard = m_vCards[eType];
	auto iter = std::find(vCard.begin(), vCard.end(), nCard);
	vCard.erase(iter);

	// remove peng 
	auto iterPeng = std::find(m_vPenged.begin(), m_vPenged.end(), nCard);
	if (iterPeng == m_vPenged.end())
	{
		LOGFMTE("not peng , hao to bu gang ? %u ",nCard); 
		return false;
	}
	m_vPenged.erase(iterPeng);

	// add to gang ;
	addCardToVecAsc(m_vGanged, nCard);

	// new get card ;
	auto eGetType = card_Type(nGangGetCard);
	addCardToVecAsc(m_vCards[eGetType], nGangGetCard);
	m_nNesetFetchedCard = nGangGetCard;
	//debugCardInfo();
	return true;
}

bool MJPlayerCard::onEat(uint8_t nCard, uint8_t nWithA, uint8_t withB)
{
	if (!isHaveCard(nWithA) || !isHaveCard(withB))
	{
		LOGFMTE("do not have with card , can not eat a = %u , b = %u, c = %u",nCard,nWithA,withB);
		return false;
	}

	auto eT = card_Type(nCard);
	if (card_Type(nCard) != card_Type(nWithA))
	{
		LOGFMTE("not the same type , can not eat a = %u , b = %u, c = %u", nCard, nWithA, withB);
		return false;
	}

	auto iter = std::find(m_vCards[eT].begin(), m_vCards[eT].end(), nWithA);
	m_vCards[eT].erase(iter);
	iter = std::find(m_vCards[eT].begin(), m_vCards[eT].end(), withB);
	m_vCards[eT].erase(iter);

	// add to eat , should not sort 
	m_vEated.push_back(nCard);
	m_vEated.push_back(nWithA);
	m_vEated.push_back(withB);
	return true;
}

bool MJPlayerCard::onChuCard(uint8_t nChuCard)
{
	if (!isHaveCard(nChuCard))
	{
		LOGFMTE("you don't have this card , how can chu ?  = %u",nChuCard);
		return false;
	}
	auto eT = card_Type(nChuCard);
	auto iter = std::find(m_vCards[eT].begin(), m_vCards[eT].end(), nChuCard);
	m_vCards[eT].erase(iter);
	m_vChuedCard.push_back(nChuCard);

	//debugCardInfo();
	return true;
}

bool MJPlayerCard::getHoldCard(VEC_CARD& vHoldCard)
{
	for (auto& vCards : m_vCards)
	{
		if (vCards.empty())
		{
			continue;
		}
		vHoldCard.insert(vHoldCard.end(),vCards.begin(),vCards.end());
	}
	return !vHoldCard.empty();
}

bool MJPlayerCard::getChuedCard(VEC_CARD& vChuedCard)
{
	vChuedCard.insert(vChuedCard.end(),m_vChuedCard.begin(),m_vChuedCard.end());
	return vChuedCard.empty() == false;
}

bool MJPlayerCard::getAnGangedCard(VEC_CARD& vAnGanged)
{
	vAnGanged.insert(vAnGanged.end(), m_vAnGanged.begin(), m_vAnGanged.end());
	return vAnGanged.empty() == false;
}

bool MJPlayerCard::getMingGangedCard(VEC_CARD& vGangCard)
{
	vGangCard.insert(vGangCard.end(), m_vGanged.begin(), m_vGanged.end());
	return false == vGangCard.empty();
}

bool MJPlayerCard::getPengedCard(VEC_CARD& vPengedCard)
{
	vPengedCard.insert(vPengedCard.end(), m_vPenged.begin(), m_vPenged.end());
	return false == vPengedCard.empty();
}

bool MJPlayerCard::getEatedCard(VEC_CARD& vEatedCard)
{
	vEatedCard.insert(vEatedCard.end(), m_vEated.begin(), m_vEated.end());
	return false == vEatedCard.empty();
}

uint32_t MJPlayerCard::getNewestFetchedCard()
{
	return m_nNesetFetchedCard;
}

void MJPlayerCard::addCardToVecAsc(VEC_CARD& vec, uint8_t nCard)
{
	auto iter = vec.begin();
	for (; iter < vec.end(); ++iter)
	{
		if ((*iter) >= nCard)
		{
			vec.insert(iter, nCard);
			return;
		}
	}
	vec.push_back(nCard);
}

bool MJPlayerCard::getNotShuns(VEC_CARD vCard, SET_NOT_SHUN& vNotShun, bool bMustKeZiShun )
{
	/// temp unsed ;
	if (vCard.empty())
	{
		vNotShun.clear();
		return true;
	}

	auto nCnt = tryBestFindLeastNotShun(vCard, vNotShun, bMustKeZiShun );
	if (nCnt == 0)
	{
		return true;
	}

	return false;



	// below will not used 
	SET_NOT_SHUN vMyNotShun;
	uint8_t nMyCnt = 100;

	auto pCheckShun = [this]( bool bMustKeZi, uint8_t& nMyCnt, SET_NOT_SHUN& vMyNotShun, VEC_CARD& vCheckCard )
	{
		SET_NOT_SHUN vTemp;
		auto nCnt = tryBestFindLeastNotShun(vCheckCard, vTemp, bMustKeZi);
		if (nCnt == 0)
		{
			nMyCnt = 0;
			vMyNotShun.clear();
			return true;
		}

		if (nCnt <= 2 || nCnt <= nMyCnt )
		{
			if (nMyCnt > 2)
			{
				// just wap ;
				vMyNotShun.swap(vTemp);
			}
			else
			{
				vMyNotShun.insert(vTemp.begin(), vTemp.end());
			}

			if (nCnt < nMyCnt )
			{
				nMyCnt = nCnt;
			}
		}
		return false;
	};
	
	VEC_CARD vIgnoreKezi;
	vIgnoreKezi.assign(vCard.begin(),vCard.end());
	if (pCheckShun(bMustKeZiShun, nMyCnt, vMyNotShun, vIgnoreKezi))
	{
		return true;
	}

	// not ignore ke zi 
	VEC_CARD vKeZi;
	VEC_CARD vLeftCard;
	pickKeZiOut(vCard,vKeZi,vLeftCard);
	if (vLeftCard.empty())
	{
		vNotShun.clear();
		vMyNotShun.clear();
		nMyCnt = 0;
		return true;
	}

	// without kezi ,Left card , that not shun . ignore part ke zi , means some kezi not represent ke zi ;
	if ( vKeZi.size() > 0 )
	{
		// without ke zi situation 
		if (pCheckShun(bMustKeZiShun, nMyCnt, vMyNotShun, vLeftCard))
		{
			return true;
		}
	}
	
	// take part keZi into construct shun ;
	if (vKeZi.size() > 1 )
	{
		VEC_CARD vCheck;
		vCheck.assign(vLeftCard.begin(), vLeftCard.end());
		vCheck.push_back(vKeZi[0]);
		vCheck.push_back(vKeZi[0]);
		vCheck.push_back(vKeZi[0]);
		if (pCheckShun(bMustKeZiShun, nMyCnt, vMyNotShun, vCheck))
		{
			return true;
		}
	}

	if (vKeZi.size() >= 2 )
	{
		VEC_CARD vCheck;
		vCheck.assign(vLeftCard.begin(), vLeftCard.end());
		vCheck.push_back(vKeZi[1]);
		vCheck.push_back(vKeZi[1]);
		vCheck.push_back(vKeZi[1]);

		if (pCheckShun(bMustKeZiShun, nMyCnt, vMyNotShun, vCheck))
		{
			return true;
		}
	}

	 if ( vKeZi.size() >= 3 )
	 {
		 VEC_CARD vCheck;
		 vCheck.assign(vLeftCard.begin(), vLeftCard.end());
		 vCheck.push_back(vKeZi[2]);
		 vCheck.push_back(vKeZi[2]);
		 vCheck.push_back(vKeZi[2]);
		 if (pCheckShun(bMustKeZiShun, nMyCnt, vMyNotShun, vCheck))
		 {
			 return true;
		 }

		 // 0,1 ; 1,2; 0,2 ;
		 vCheck.clear();
		 vCheck.assign(vLeftCard.begin(), vLeftCard.end());
		 vCheck.push_back(vKeZi[2]);
		 vCheck.push_back(vKeZi[2]);
		 vCheck.push_back(vKeZi[2]);

		 vCheck.push_back(vKeZi[0]);
		 vCheck.push_back(vKeZi[0]);
		 vCheck.push_back(vKeZi[0]);
		 if (pCheckShun(bMustKeZiShun, nMyCnt, vMyNotShun, vCheck))
		 {
			 return true;
		 }

		 // 0,1 ; 1,2; 0,2 ;
		 vCheck.clear();
		 vCheck.assign(vLeftCard.begin(), vLeftCard.end());
		 vCheck.push_back(vKeZi[2]);
		 vCheck.push_back(vKeZi[2]);
		 vCheck.push_back(vKeZi[2]);

		 vCheck.push_back(vKeZi[1]);
		 vCheck.push_back(vKeZi[1]);
		 vCheck.push_back(vKeZi[1]);
		 if (pCheckShun(bMustKeZiShun, nMyCnt, vMyNotShun, vCheck))
		 {
			 return true;
		 }

		 vCheck.clear();
		 vCheck.assign(vLeftCard.begin(), vLeftCard.end());
		 vCheck.push_back(vKeZi[0]);
		 vCheck.push_back(vKeZi[0]);
		 vCheck.push_back(vKeZi[0]);

		 vCheck.push_back(vKeZi[1]);
		 vCheck.push_back(vKeZi[1]);
		 vCheck.push_back(vKeZi[1]);
		 if (pCheckShun(bMustKeZiShun, nMyCnt, vMyNotShun, vCheck))
		 {
			 return true;
		 }
	 }

	vNotShun.insert(vMyNotShun.begin(),vMyNotShun.end());
	return false;
}

bool MJPlayerCard::pickKeZiOut(VEC_CARD vCards, VEC_CARD& vKeZi, VEC_CARD& vLeftCard)
{
	vKeZi.clear();
	for (uint8_t nIdx = 0; (uint8_t)(nIdx + 2) < vCards.size();)
	{
		if (vCards[nIdx] == vCards[nIdx + 2])
		{
			vKeZi.push_back(vCards[nIdx]);
			// mark find result to 0 , for remove later 
			vCards[nIdx] = 0;
			vCards[nIdx + 1] = 0;
			vCards[nIdx + 2] = 0;

			nIdx = nIdx + 3;
		}
		else
		{
			++nIdx;
		}
	}

	if (vKeZi.empty())
	{
		vLeftCard.swap(vCards);
		return false;
	}

	auto iter = std::find(vCards.begin(), vCards.end(), 0);
	while (iter != vCards.end())
	{
		vCards.erase(iter);
		iter = std::find(vCards.begin(), vCards.end(), 0);
	}
	vLeftCard.clear();
	vLeftCard.swap(vCards);
	return true;
}

//bool MJPlayerCard::pickNotShunZiOutIgnoreKeZi(VEC_CARD vCardIgnorKeZi, SET_NOT_SHUN& vNotShun)
//{
//	VEC_CARD vAscendSort, vDescendSort;
//	vAscendSort.assign(vCardIgnorKeZi.begin(),vCardIgnorKeZi.end());
//	std::sort(vAscendSort.begin(), vAscendSort.end());   // < asc
//
//	vDescendSort.swap(vCardIgnorKeZi);  //desc
//	std::sort(vDescendSort.begin(), vDescendSort.end(), [](const uint8_t& refLeft, const uint8_t& refLeftRight)->bool{ return refLeft > refLeftRight; });   // < asc
//
//	auto pfn = [](VEC_CARD& vec, uint8_t nSeekValue, uint8_t& nFindIdx, uint8_t nIdxStart )->bool{
//		for (uint8_t nIdx = nIdxStart; nIdx < vec.size(); ++nIdx)
//		{
//			if (vec[nIdx] == nSeekValue)
//			{
//				nFindIdx = nIdx;
//				return true;
//			}
//		}
//		return false;
//	};
//
//
//	auto pfnErase = [pfn](VEC_CARD& vec,bool bAsc )
//	{
//		for (uint8_t nIdx = 0; nIdx < vec.size(); ++nIdx)
//		{
//			uint8_t nValue = vec[nIdx];
//			if (0 == nValue)
//			{
//				continue;
//			}
//			uint8_t nValue1 = nValue + 1; uint8_t nIdx1 = 0;
//			uint8_t nValue2 = nValue1 + 1; uint8_t nIdx2 = 0;
//			if (!bAsc)
//			{
//				nValue1 = nValue - 1;
//				nValue2 = nValue1 - 1;
//			}
//			if (pfn(vec, nValue1, nIdx1,nIdx + 1 ) && pfn(vec, nValue2, nIdx2,nIdx + 1 ))
//			{
//				vec[nIdx] = 0;
//				vec[nIdx1] = 0;
//				vec[nIdx2] = 0;
//			}
//		}
//
//		// erase zero ;
//		auto iter = std::find(vec.begin(), vec.end(), 0);
//		while (iter != vec.end())
//		{
//			vec.erase(iter);
//			iter = std::find(vec.begin(), vec.end(), 0);
//		}
//	};
//
//	//-------left to right
//	pfnErase(vAscendSort,true);
//	if (vAscendSort.empty())
//	{
//		return true;
//	}
//
//	pfnErase(vDescendSort,false);
//	if ( vDescendSort.empty())
//	{
//		return true;
//	}
//
//	//---right to left 
//	if (vAscendSort.empty() == false)
//	{
//		stNotShunCard st;
//		st.vCards.clear();
//		st.vCards.swap(vAscendSort);
//		vNotShun.insert(st);
//	}
//
//	if (vDescendSort.empty() == false)
//	{
//		stNotShunCard st;
//		st.vCards.clear();
//		st.vCards.swap(vDescendSort);
//		vNotShun.insert(st);
//	}
//
//	return false;
//}

bool MJPlayerCard::pickNotShunZiOutIgnoreKeZi(VEC_CARD vCardIgnorKeZi, SET_NOT_SHUN& vNotShun)
{
	//VEC_CARD vCheckCard;
	//vCheckCard.assign(vCardIgnorKeZi.begin(), vCardIgnorKeZi.end());
	//std::sort(vCheckCard.begin(), vCheckCard.end());   

	//auto findNotShun = [](VEC_CARD& vCheck, SET_NOT_SHUN& vNotShun )
	//{
	//	if (vCheck.size() % 3 != 0)
	//	{
	//		LOGFMTE("when check here must 3 beishu ");
	//	}
	//};

	////vDescendSort.swap(vCardIgnorKeZi);  //desc
	////std::sort(vDescendSort.begin(), vDescendSort.end(), [](const uint8_t& refLeft, const uint8_t& refLeftRight)->bool{ return refLeft > refLeftRight; });   // < asc

	//auto pfn = [](VEC_CARD& vec, uint8_t nSeekValue, uint8_t& nFindIdx, uint8_t nIdxStart)->bool{
	//	for (uint8_t nIdx = nIdxStart; nIdx < vec.size(); ++nIdx)
	//	{
	//		if (vec[nIdx] == nSeekValue)
	//		{
	//			nFindIdx = nIdx;
	//			return true;
	//		}
	//	}
	//	return false;
	//};


	//auto pfnErase = [pfn](VEC_CARD& vec, bool bAsc)
	//{
	//	for (uint8_t nIdx = 0; nIdx < vec.size(); ++nIdx)
	//	{
	//		uint8_t nValue = vec[nIdx];
	//		if (0 == nValue)
	//		{
	//			continue;
	//		}
	//		uint8_t nValue1 = nValue + 1; uint8_t nIdx1 = 0;
	//		uint8_t nValue2 = nValue1 + 1; uint8_t nIdx2 = 0;
	//		if (!bAsc)
	//		{
	//			nValue1 = nValue - 1;
	//			nValue2 = nValue1 - 1;
	//		}
	//		if (pfn(vec, nValue1, nIdx1, nIdx + 1) && pfn(vec, nValue2, nIdx2, nIdx + 1))
	//		{
	//			vec[nIdx] = 0;
	//			vec[nIdx1] = 0;
	//			vec[nIdx2] = 0;
	//		}
	//	}

	//	// erase zero ;
	//	auto iter = std::find(vec.begin(), vec.end(), 0);
	//	while (iter != vec.end())
	//	{
	//		vec.erase(iter);
	//		iter = std::find(vec.begin(), vec.end(), 0);
	//	}
	//};

	////-------left to right
	//pfnErase(vAscendSort, true);
	//if (vAscendSort.empty())
	//{
	//	return true;
	//}

	//pfnErase(vDescendSort, false);
	//if (vDescendSort.empty())
	//{
	//	return true;
	//}

	////---right to left 
	//if (vAscendSort.empty() == false)
	//{
	//	stNotShunCard st;
	//	st.vCards.clear();
	//	st.vCards.swap(vAscendSort);
	//	vNotShun.insert(st);
	//}

	//if (vDescendSort.empty() == false)
	//{
	//	stNotShunCard st;
	//	st.vCards.clear();
	//	st.vCards.swap(vDescendSort);
	//	vNotShun.insert(st);
	//}

	return false;
}

//bool MJPlayerCard::is7PairTing()
//{
//	if (m_vPenged.empty() == false || false == m_vGanged.empty() || false == m_vEated.empty() )
//	{
//		return false;
//	}
//
//	uint8_t nPairCnt = 0;
//	for (auto& vCards : m_vCards)
//	{
//		for (uint8_t nIdx = 0; (uint8_t)(nIdx + 1) < vCards.size(); )
//		{
//			if (vCards[nIdx] == vCards[1 + nIdx])
//			{
//				++nPairCnt;
//				nIdx += 2;
//			}
//			else
//			{
//				++nIdx;
//			}
//		}
//	}
//
//	return nPairCnt == 6;
//}
//
//bool MJPlayerCard::canHoldCard7PairHu()
//{
//	if (m_vPenged.empty() == false || false == m_vGanged.empty() || false == m_vEated.empty())
//	{
//		return false;
//	}
//
//	uint8_t nPairCnt = 0;
//	for (auto& vCards : m_vCards)
//	{
//		for (uint8_t nIdx = 0; (uint8_t)(nIdx + 1) < vCards.size(); ++nIdx)
//		{
//			if (vCards[nIdx] == vCards[1 + nIdx])
//			{
//				++nPairCnt;
//			}
//		}
//	}
//
//	return nPairCnt == 7;
//}

bool MJPlayerCard::is7PairTing()
{
	if (m_vPenged.empty() == false || false == m_vGanged.empty() || false == m_vEated.empty())
	{
		return false;
	}

	if (get7PairQueCnt(m_vCards) <= 1)
	{
		return true;
	}
	return false;
}

bool MJPlayerCard::canHoldCard7PairHu()
{
	if (m_vPenged.empty() == false || m_vAnGanged.empty() == false || false == m_vGanged.empty() || false == m_vEated.empty())
	{
		return false;
	}

	if (get7PairQueCnt(m_vCards) == 0)
	{
		if (m_nDanDiao && m_nJIang == 0)
		{
			m_nJIang = m_nDanDiao;
		}
		return true;
	}
	//debugCardInfo();
	return false;
}


bool MJPlayerCard::getCanHuCards(std::set<uint8_t>& vCanHuCards)
{
	if (is7PairTing())
	{
		// when 7 pair ting , then must dan diao ;
		if (m_nDanDiao == 0)
		{
			LOGFMTE("why 7 pair ting , is not  dan diao ? bug ?????");
			return false;
		}
		vCanHuCards.insert(m_nDanDiao);
		return true;
	}

	if ( isTingPai() == false )
	{
		vCanHuCards.clear();
		return false;
	}

	// copy card for use ;
	SET_NOT_SHUN vNotShun[eCT_Max];
	VEC_CARD vCards[eCT_Max];
	std::vector<uint8_t> vNotEmptyShunIdx;
	for (uint8_t nIdx = 0; nIdx < eCT_Max; ++nIdx )
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
		LOGFMTE("already ting pai ,why no que card = %u",vNotEmptyShunIdx.size());
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
	// check dan diao 
	//SET_NOT_SHUN& vFirst = vNotShun[vNotEmptyShunIdx[0]];
	//for (auto& v : vFirst)
	//{
	//	if (v.getSize() == 1 )
	//	{
	//		// other type shound have hu card ;
	//		vCanHuCards.insert(v.vCards[0]);
	//		continue;
	//	}

	//	if (v.getSize() == 4)
	//	{
	//		VEC_CARD vTempCard;
	//		if (v.vCards[0] == v.vCards[1])
	//		{
	//			vTempCard.push_back(v.vCards[2]); vTempCard.push_back(v.vCards[3]);
	//			pfnGetCanHuCardIgnoreJiang(vTempCard, vCanHuCards);
	//		}
	//		
	//		vTempCard.clear();
	//		if (v.vCards[2] == v.vCards[3])
	//		{
	//			vTempCard.push_back(v.vCards[0]); vTempCard.push_back(v.vCards[1]);
	//			pfnGetCanHuCardIgnoreJiang(vTempCard, vCanHuCards);
	//		}
	//	}
	//}
	//return vCanHuCards.size() > 0;
	// have jiang 
	
	///-----------------------------------------------
	//if ( m_nJIang )
	//{
	//	auto type = card_Type(m_nJIang);
	//	// remove jiang , then check ;
	//	auto iter = std::find(vCards[type].begin(), vCards[type].end(), m_nJIang);
	//	vCards[type].erase(iter);
	//	iter = std::find(vCards[type].begin(), vCards[type].end(), m_nJIang);
	//	vCards[type].erase(iter);

	//	for (uint8_t nIdx = 0; nIdx < eCT_Max; ++nIdx)
	//	{
	//		pfnGetCanHuCardIgnoreJiang(vCards[nIdx], vCanHuCards);
	//		if (vCanHuCards.empty() == false)
	//		{
	//			return true;
	//		}
	//	}

	//	return false;
	//}

	//// if no jiang , so must be dan diao ;
	//if (0 == m_nDanDiao)
	//{
	//	LOGFMTE("why is not dan diao , also have no jiang big bug ????? ");
	//	return false;
	//}
	//
	//auto type = card_Type(m_nDanDiao);
	//SET_NOT_SHUN vNotShun;
	//getNotShuns(vCards[type], vNotShun, eCT_Feng == type || eCT_Jian == type);
	//// parse dan diao card ;
	//auto iter = vNotShun.begin();
	//for (; iter != vNotShun.end(); ++iter)
	//{
	//	if ((*iter).getSize() == 1)
	//	{
	//		vCanHuCards.insert((*iter).vCards.front());
	//	}
	//}

	//// although  diandiao  , but also this can have already jiang , because when decide jiang or dandiao , we asume , dan diao first ;
	//if (std::count(vCards[type].begin(), vCards[type].end(), m_nDanDiao) >= 2)
	//{
	//	// remove as jiang 
	//	auto iter = std::find(vCards[type].begin(), vCards[type].end(), m_nDanDiao);
	//	vCards[type].erase(iter);
	//	iter = std::find(vCards[type].begin(), vCards[type].end(), m_nDanDiao);
	//	vCards[type].erase(iter);
	//	pfnGetCanHuCardIgnoreJiang(vCards[type], vCanHuCards);
	//}

	//return vCanHuCards.empty() == false;
}

uint8_t MJPlayerCard::getMiniQueCnt( VEC_CARD vCards[eCT_Max] )
{
	// prepare cards without cai shen 
	SET_NOT_SHUN vNotShun[eCT_Max];
	for (uint8_t nType = eCT_None; nType < eCT_Max; ++nType)
	{
		auto& vCard = vCards[nType];
		if (vCard.empty())
		{
			continue;
		}

		getNotShuns(vCard, vNotShun[nType], eCT_Feng == nType || eCT_Jian == nType);
	}

	// when find dandiao 
	uint8_t nQueCnt = 0;
	m_nJIang = 0;
	m_nDanDiao = 0;
	for (auto& vRefNotShun : vNotShun)
	{
		if (vRefNotShun.empty())
		{
			continue;
		}
		uint8_t nTemp;
		nQueCnt += getLestQue(vRefNotShun, false, m_nDanDiao == 0, nTemp, m_nDanDiao);
	}
	LOGFMTI(" fand dan diao mode que cnt = %u value = %u", nQueCnt, m_nDanDiao);
	if (m_nDanDiao)
	{
		return nQueCnt;
	}

	if (m_nDanDiao == 0)  // can not dandiao 
	{
		uint8_t nJiang = 0;
		uint8_t nWhenJiangQueCnt = 0;
		nQueCnt = 0;
		for (auto& vRefNotShun : vNotShun)
		{
			uint8_t nTemp;
			nQueCnt += getLestQue(vRefNotShun, m_nJIang == 0, false, m_nJIang, nTemp);
		}

		LOGFMTI(" fand jiang mode que cnt = %u value = %u", nQueCnt, m_nJIang);
		if (m_nJIang)
		{
			return nQueCnt;
		}
	}

	LOGFMTI(" no jiang , no dandiao cnt = %u ", nQueCnt);
	// no jiang , no dandiao 
	nQueCnt += 2;
	return nQueCnt;
}

uint8_t MJPlayerCard::get7PairQueCnt(VEC_CARD vCards[eCT_Max])
{
	uint8_t nUnpairCnt = 0;
	uint8_t nDanDiao = 0;
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
				nDanDiao = vCard[nIdx];
				++nUnpairCnt;
				++nIdx;
			}
		}
	}

	if (nUnpairCnt == 1)
	{
		m_nDanDiao = nDanDiao;
	}

	return nUnpairCnt;
}

uint8_t MJPlayerCard::getLestQue(SET_NOT_SHUN& vNotShun, bool bFindJiang, bool bFindDanDiao, uint8_t& nFiandJiang, uint8_t& nFindDanDiao)
{
	if (vNotShun.empty())
	{
		return 0;
	}

	auto pfunFindQueCnt = [](stNotShunCard& stNotShun, bool bFindJiang, bool bFindDanDiao, uint8_t& nFiandJiang, uint8_t& nFindDanDiao)->uint8_t
	{
		auto& vCards = stNotShun.vCards;
		if (vCards.empty())
		{
			LOGFMTE("not shun must not be empty ? error ");
			return 0;
		}

		auto type = card_Type(vCards.front());
		auto bMustKe = (type == eCT_Feng || eCT_Jian == type);

		uint8_t nLackCnt = 0;
		std::sort(vCards.begin(), vCards.end());
		for (uint8_t nIdx = 0; nIdx < vCards.size();)
		{
			uint8_t nValue = vCards[nIdx];
			if ((uint8_t)(nIdx + 1) >= vCards.size())  // last single card ;
			{
				nLackCnt += 2;
				if (bFindDanDiao)
				{
					nLackCnt -= 1;
					bFindDanDiao = false;
					nFindDanDiao = nValue;
				}

				break;
			}

			uint8_t nNextV = vCards[nIdx + 1];
			if (nNextV == nValue)
			{
				nLackCnt += 1;
				nIdx += 2;
				if (bFindJiang)
				{
					nLackCnt -= 1;
					bFindJiang = false;
					nFiandJiang = nValue;
				}
				continue;
			}

			if (bMustKe == false && (nNextV - nValue <= 2))
			{
				nLackCnt += 1;
				nIdx += 2;
				continue;
			}

			nLackCnt += 2;
			nIdx += 1;
			if (bFindDanDiao)
			{
				nLackCnt -= 1;
				bFindDanDiao = false;
				nFindDanDiao = nValue;
			}
		}
		return nLackCnt;
	};

	uint8_t nLesetQue = 100;
	uint8_t nFedJIangResult = 0;
	uint8_t nFedDanResult = 0;

	auto iter = vNotShun.begin();
	for (; iter != vNotShun.end(); ++iter)
	{
		uint8_t nFedJIang = 0;
		uint8_t nFedDan = 0;
		stNotShunCard stQ = *iter;
		auto nQuenCnt = pfunFindQueCnt(stQ, bFindJiang, bFindDanDiao, nFedJIang, nFedDan);
		if (nQuenCnt < nLesetQue)
		{
			nLesetQue = nQuenCnt;
			nFedJIangResult = nFedJIang;
			nFedDanResult = nFedDan;

			if ( 0 == nLesetQue )
			{
				break;
			}
		}
	}

	if (nFiandJiang == 0)
	{
		nFiandJiang = nFedJIangResult;
	}
	
	if (nFindDanDiao == 0)
	{
		nFindDanDiao = nFedDanResult;
	}
	
	return nLesetQue;
}

void MJPlayerCard::debugCardInfo()
{
	LOGFMTD("card info start !");
	for (uint8_t eType = 0; eType < eCT_Max; ++eType)
	{
		VEC_CARD& vCard = m_vCards[eType];
		if (vCard.empty())
		{
			continue;
		}
		LOGFMTI("card info type : %u ", eType );
		for (auto & ref : vCard)
		{
			auto value = card_Value(ref);
			auto type = card_Type(ref);
			if (type != eType)
			{
				LOGFMTE("wrong add to card = %u",ref);
			}
			LOGFMTI("value = %u",value);
		}
	}

	LOGFMTD("card info end !");
}

uint8_t MJPlayerCard::tryBestFindLeastNotShun(VEC_CARD& vCard, SET_NOT_SHUN& vNotShun, bool bMustKeZi )
{
	if (vCard.empty())
	{
		return 0;
	}

	VEC_CARD vCheckCard;
	vCheckCard.assign(vCard.begin(), vCard.end());
	std::sort(vCheckCard.begin(), vCheckCard.end());
	
	if (vCheckCard.size() < 3 && bMustKeZi )  // this function not contai ke zi shun ; 
	{
		stNotShunCard st;
		st.vCards.swap(vCheckCard);
		vNotShun.insert(st);
		return st.getLackCardCntForShun();
	}

	// find shun from card ;
	SET_NOT_SHUN vMyNotShun;
	uint8_t nMyLeastCnt;
	nMyLeastCnt = 100;
	for (uint8_t nIdx = 0; nIdx < vCheckCard.size(); ++nIdx)
	{
		if (nIdx + 2 >= vCheckCard.size())
		{
			break;
		}

		auto pFuncSubCheck = [this](VEC_CARD& vSubCheckCard, bool bMustKeZi, SET_NOT_SHUN& vMyNotShun, uint8_t& nMyLeastCnt )
		{
			// remove zero ;
			auto iter = std::find(vSubCheckCard.begin(), vSubCheckCard.end(), 0);
			while (iter != vSubCheckCard.end())
			{
				vSubCheckCard.erase(iter);
				iter = std::find(vSubCheckCard.begin(), vSubCheckCard.end(), 0);
			}

			SET_NOT_SHUN vTemp;
			auto nCnt = tryBestFindLeastNotShun(vSubCheckCard, vTemp, bMustKeZi);
			if (nCnt == 0)
			{
				nMyLeastCnt = 0;
				vMyNotShun.clear();
				return true;
			}

			if (nCnt <= 2 || nCnt <= nMyLeastCnt)
			{
				if (nMyLeastCnt > 2)
				{
					// just wap ;
					vMyNotShun.swap(vTemp);
				}
				else
				{
					vMyNotShun.insert(vTemp.begin(), vTemp.end());
				}

				if (nCnt < nMyLeastCnt)
				{
					nMyLeastCnt = nCnt;
				}
			}

			return false;
		};

		// check ke zi shun zi 
		VEC_CARD vSubCheckCard;
		vSubCheckCard.assign(vCheckCard.begin(), vCheckCard.end());
		if (vSubCheckCard[nIdx] == vSubCheckCard[nIdx + 2])
		{
			// do may ke zi ;
			vSubCheckCard[nIdx] = vSubCheckCard[nIdx + 1] = vSubCheckCard[nIdx + 2] = 0;
			if (pFuncSubCheck(vSubCheckCard, bMustKeZi, vMyNotShun, nMyLeastCnt))
			{
				return 0;
			}
		}

		// pu tong shun zi case ;
		if (bMustKeZi)
		{
			continue;
		}
		vSubCheckCard.clear();
		vSubCheckCard.assign(vCheckCard.begin(), vCheckCard.end());
 
		bool bFindSecond = false , bFind3 = false;
		for (uint8_t sIdx = nIdx + 1; sIdx < vSubCheckCard.size(); ++sIdx)
		{
			if (vSubCheckCard[sIdx] > (vSubCheckCard[nIdx] + 2))
			{
				break;
			}

			if (!bFindSecond)
			{
				if (vSubCheckCard[sIdx] == (vSubCheckCard[nIdx] + 1))
				{
					vSubCheckCard[sIdx] = 0;
					bFindSecond = true;
				}
				continue;
			}

			// find 3 
			if (vSubCheckCard[sIdx] == (vSubCheckCard[nIdx] + 2))
			{
				vSubCheckCard[sIdx] = 0;
				bFind3 = true;
				break;
			}
		}

		if (!bFindSecond || !bFind3)
		{
			continue;
		}
		vSubCheckCard[nIdx] = 0; // clear self value ;

		// remove zero ;
		if (pFuncSubCheck(vSubCheckCard, bMustKeZi, vMyNotShun, nMyLeastCnt))
		{
			return 0;
		}

	}

	// no shun zi here 
	if (vMyNotShun.empty())
	{
		stNotShunCard st;
		st.vCards.swap(vCheckCard);
		vNotShun.insert(st);
		return st.getLackCardCntForShun();
	}

	vNotShun.insert(vMyNotShun.begin(),vMyNotShun.end());
	return nMyLeastCnt;
}

//uint8_t MJPlayerCard::tryBestFindLeastNotShunMustKeZi(VEC_CARD& vCard, SET_NOT_SHUN& vNotShun)
//{
//	if (vCard.empty())
//	{
//		return 0;
//	}
//
//	VEC_CARD vCheckCard;
//	vCheckCard.assign(vCard.begin(), vCard.end());
//	std::sort(vCheckCard.begin(), vCheckCard.end());
//
//	if (vCheckCard.size() < 3 ) 
//	{
//		stNotShunCard st;
//		st.vCards.swap(vCheckCard);
//		vNotShun.insert(st);
//		return st.getLackCardCntForShun();
//	}
//
//	// find shun from card ;
//	SET_NOT_SHUN vMyNotShun;
//	uint8_t nMyLeastCnt;
//	nMyLeastCnt = 100;
//	for (uint8_t nIdx = 0; nIdx < vCheckCard.size(); ++nIdx)
//	{
//		if (nIdx + 2 >= vCheckCard.size())
//		{
//			break;
//		}
//
//		VEC_CARD vSubCheckCard;
//		vSubCheckCard.assign(vCheckCard.begin(), vCheckCard.end());
//
//		auto iter = std::find(vSubCheckCard.begin(), vSubCheckCard.end(), vCheckCard[nIdx]);
//		if (vSubCheckCard.end() == iter)
//		{
//			continue;
//		}
//		vSubCheckCard.erase(iter);
//
//		iter = std::find(vSubCheckCard.begin(), vSubCheckCard.end(), vCheckCard[nIdx] + 1);
//		if (vSubCheckCard.end() == iter)
//		{
//			continue;
//		}
//		vSubCheckCard.erase(iter);
//
//		iter = std::find(vSubCheckCard.begin(), vSubCheckCard.end(), vCheckCard[nIdx] + 2);
//		if (vSubCheckCard.end() == iter)
//		{
//			continue;
//		}
//		vSubCheckCard.erase(iter);
//
//		// do subcheck 
//		SET_NOT_SHUN vTemp;
//		auto nCnt = tryBestFindLeastNotShun(vSubCheckCard, vTemp, bMustKeZi);
//		if (nCnt == 0)
//		{
//			vMyNotShun.clear();
//			return 0;
//		}
//
//		if (nCnt <= 2 || nCnt <= nMyLeastCnt)
//		{
//			if (nMyLeastCnt > 2)
//			{
//				// just wap ;
//				vMyNotShun.swap(vTemp);
//			}
//			else
//			{
//				vMyNotShun.insert(vTemp.begin(), vTemp.end());
//			}
//
//			if (nCnt < nMyLeastCnt)
//			{
//				nMyLeastCnt = nCnt;
//			}
//		}
//	}
//
//	// no shun zi here 
//	if (vMyNotShun.empty())
//	{
//		stNotShunCard st;
//		st.vCards.swap(vCheckCard);
//		vNotShun.insert(st);
//		return st.getLackCardCntForShun();
//	}
//
//	vNotShun.insert(vMyNotShun.begin(), vMyNotShun.end());
//	return nMyLeastCnt;
//}

