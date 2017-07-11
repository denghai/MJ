#include "SZMJPlayerCard.h"
#include "log4z.h"
#include "MJCard.h"
void SZMJPlayerCard::reset()
{
	MJPlayerCard::reset();
	m_vBuHuaCard.clear();
	m_nSongZhiGangIdx = -1;
}

void SZMJPlayerCard::onBuHua(uint8_t nHuaCard, uint8_t nCard)
{
	auto eType = card_Type(nHuaCard);
	if (eType >= eCT_Max)
	{
		LOGFMTE("onBuHua parse card type error so do not have this card = %u", nHuaCard);
		return;
	}

	auto& vCard = m_vCards[eType];
	auto iter = std::find(vCard.begin(), vCard.end(), nHuaCard);
	vCard.erase(iter);

	onMoCard(nCard);
	m_vBuHuaCard.push_back(nHuaCard);
}

bool SZMJPlayerCard::getCardInfo(Json::Value& jsPeerCards)
{
	// svr: { idx : 2 , newMoCard : 2, anPai : [2,3,4,34] , chuPai: [2,34,4] , huaPai: [23,23,23] , anGangPai : [23,24],buGang : [23,45], pengCard : [23,45] }
	IMJPlayerCard::VEC_CARD vAnPai, vChuPai, vAnGangedCard, vBuGang, vPenged;
	Json::Value jsAnPai, jsChuPai, jsAngangedPai, jsBuGang, jsHuaPai , jsPenged;

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

	toJs(vAnPai, jsAnPai); toJs(vChuPai, jsChuPai); toJs(vAnGangedCard, jsAngangedPai); toJs(vBuGang, jsBuGang); toJs(m_vBuHuaCard, jsHuaPai), toJs(vPenged, jsPenged);;
	jsPeerCards["anPai"] = jsAnPai; jsPeerCards["chuPai"] = jsChuPai; jsPeerCards["anGangPai"] = jsAngangedPai; jsPeerCards["buGang"] = jsBuGang; jsPeerCards["huaPai"] = jsHuaPai;
	jsPeerCards["pengCard"] = jsPenged;
	return true;
}

bool SZMJPlayerCard::onDoHu(bool isZiMo, bool isHaiDiLoaYue,uint8_t nCard, std::vector<uint16_t>& vHuTypes, uint16_t& nHuHuaCnt, uint16_t& nHoldHuaCnt )
{
	nHuHuaCnt = 5;
	nHoldHuaCnt = getHuaCntWithoutHuTypeHuaCnt();
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

	if ( isHoldCardCanHu() == false)
	{
		LOGFMTE("do hu act , but can not hu ? why ? bug card = %u ", nCard);
		debugCardInfo();
		if (!isZiMo)
		{
			funRemoveAddToCard(nCard);
		}
		return false;
	}

	// check fanxing and bei shu 
	if (isZiMo && isHaiDiLoaYue)
	{
		vHuTypes.push_back(eFanxing_HaiDiLaoYue);
		nHuHuaCnt += 5;
	}

	if (checkHunYiSe())
	{
		vHuTypes.push_back(eFanxing_HunYiSe );
		nHuHuaCnt += 5;
	}
	else if ( checkQingYiSe() )
	{
		vHuTypes.push_back(eFanxing_QingYiSe );
		nHuHuaCnt += 10;
	}

	if (checkDuiDuiHu())
	{
		vHuTypes.push_back(eFanxing_DuiDuiHu);
		nHuHuaCnt += 5;
	}

	if (checkHaoHuaQiDui() )
	{
		vHuTypes.push_back(eFanxing_HaoHuaQiDui);
		nHuHuaCnt += 30;
	}
	else if ( checkQiDui() )
	{
		vHuTypes.push_back(eFanxing_QiDui);
		nHuHuaCnt += 15;
	}
	else if (checkDaMenQing())
	{
		vHuTypes.push_back(eFanxing_DaMenQing);
		nHuHuaCnt += 10;
	}
	else if (checkXiaoMenQing())
	{
		vHuTypes.push_back(eFanxing_XiaoMenQing);
		nHuHuaCnt += 5;
	}

	if (checkDaDiaoChe())
	{
		vHuTypes.push_back(eFanxing_DaDiaoChe );
		nHuHuaCnt += 10;
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

uint8_t SZMJPlayerCard::getSongGangIdx()
{
	return m_nSongZhiGangIdx;
}

void SZMJPlayerCard::setSongGangIdx(uint8_t nIdx)
{
	m_nSongZhiGangIdx = nIdx;
}

uint8_t SZMJPlayerCard::getHuaCardToBuHua()
{
	auto nNewCard = getNewestFetchedCard();
	if (0 == nNewCard)
	{

	}
	else
	{
		auto nType = card_Type(nNewCard);
		if (eCT_Jian == nType || eCT_Hua == nType)
		{
			return nNewCard;
		}
	}


	if (m_vCards[eCT_Jian].empty() == false)
	{
		return m_vCards[eCT_Jian].front();
	}

	if (m_vCards[eCT_Hua].empty() == false)
	{
		return m_vCards[eCT_Hua].front();
	}

	return -1;
}

bool SZMJPlayerCard::canHuWitCard( uint8_t nCard )
{
	auto eType = card_Type(nCard);
	if (eType >= eCT_Max)
	{
		LOGFMTE("parse card type error ,canHuWitCard have this card = %u", nCard);
		return false;
	}

	addCardToVecAsc(m_vCards[eType], nCard);
	bool bSelfHu = MJPlayerCard::isHoldCardCanHu();
	if ( bSelfHu )
	{
		auto nHuaCnt = getHuaCntWithoutHuTypeHuaCnt();
		if ( nHuaCnt >= 4 )
		{

		}
		else  // not enought hua cnt ;
		{
			auto isDaHu = checkDaMenQing() || checkXiaoMenQing() || checkHunYiSe() || checkQingYiSe() || checkDuiDuiHu() || checkQiDui() || checkDaDiaoChe();
			if (isDaHu == false )  // not da hua , then xiao hu need hua cnt ; so can not hu this card ;
			{
				bSelfHu = false;
			}
		}	
	}
	auto iter = std::find(m_vCards[eType].begin(), m_vCards[eType].end(), nCard);
	m_vCards[eType].erase(iter);
	//debugCardInfo();
	return bSelfHu;
}

bool SZMJPlayerCard::checkDaMenQing()
{
	VEC_CARD vAllCard;
	getHoldCard(vAllCard);
	if (vAllCard.size() != 14)
	{
		return false;
	}

	auto n = getHuaCntWithoutHuTypeHuaCnt();
	return n == 0;
}

bool SZMJPlayerCard::checkXiaoMenQing()
{
	VEC_CARD vCard;
	getMingGangedCard(vCard);
	if (vCard.empty() == false)
	{
		return false;
	}

	getPengedCard(vCard);
	if (vCard.empty() == false)
	{
		return false;
	}
	return true;
}

bool SZMJPlayerCard::checkHunYiSe()
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

	bool bFindFeng = false;
	do
	{
		auto iter = std::find_if(vAllCard.begin(), vAllCard.end(), [](uint8_t n) { return card_Type(n) == eCT_Feng; });
		if (iter != vAllCard.end() )
		{
			bFindFeng = true;
			vAllCard.erase(iter);
		}
		else
		{
			break;
		}
	} while ( 1 );

	if (bFindFeng == false || vAllCard.empty() )
	{
		return false;
	}

	auto nType = card_Type(vAllCard.front());
	for (auto& ref : vAllCard)
	{
		auto tt = card_Type(ref);
		if (nType != tt)
		{
			return false;
		}
	}
	return true;
}

bool SZMJPlayerCard::checkQingYiSe()
{
	VEC_CARD vAllCard;
	getHoldCard(vAllCard);

	VEC_CARD vTemp;
	getAnGangedCard(vTemp);
	vAllCard.insert(vAllCard.end(),vTemp.begin(),vTemp.end());

	vTemp.clear();
	getMingGangedCard(vTemp);
	vAllCard.insert(vAllCard.end(), vTemp.begin(), vTemp.end());

	vTemp.clear();
	getPengedCard(vTemp);
	vAllCard.insert(vAllCard.end(), vTemp.begin(), vTemp.end());
	
	auto nType = card_Type(vAllCard.front());
	for (auto& ref : vAllCard)
	{
		auto tt = card_Type(ref);
		if (nType != tt)
		{
			return false;
		}
	}
	return true;
}

bool SZMJPlayerCard::checkDuiDuiHu()
{
	VEC_CARD vAllCard;
	getHoldCard(vAllCard);
	std::sort(vAllCard.begin(),vAllCard.end());
	bool bFindJiang = false;
	for ( uint8_t nIdx = 0; ( nIdx + 1 ) < vAllCard.size(); )
	{
		auto nThirdIdx = nIdx + 2;
		if ( nThirdIdx < vAllCard.size() && vAllCard[nIdx] == vAllCard[nThirdIdx] )
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

bool SZMJPlayerCard::checkQiDui()
{
	if (canHoldCard7PairHu())
	{
		return true;
	}
	return false;
}

bool SZMJPlayerCard::checkHaoHuaQiDui()
{
	if (checkQiDui() == false)
	{
		return false;
	}

	VEC_CARD v;
	getHoldCard(v);
	for ( uint8_t nIdx = 0; (nIdx + 3) < v.size(); ++nIdx )
	{
		if (v[nIdx] == v[nIdx + 3])
		{
			return true;
		}
	}

	return false;
}

bool SZMJPlayerCard::checkDaDiaoChe()
{
	VEC_CARD v;
	getHoldCard(v);
	return (v.size() == 2) && (v[0] == v[1]);
}

uint8_t SZMJPlayerCard::getHuaCntWithoutHuTypeHuaCnt()
{
	VEC_CARD vCard;
	uint8_t nHuaCnt = m_vBuHuaCard.size();
	// check ming gang 
	getMingGangedCard(vCard);
	for ( auto& ref :vCard )
	{
		if (card_Type(ref) == eCT_Feng)
		{
			nHuaCnt += 3;
		}
		else
		{
			nHuaCnt += 1;
		}
	}
	// check an gang 
	vCard.clear();
	getAnGangedCard(vCard);
	for (auto& ref : vCard)
	{
		if ( card_Type(ref) == eCT_Feng )
		{
			nHuaCnt += 4;
		}
		else
		{
			nHuaCnt += 2;
		}
	}
	// check feng peng 
	vCard.clear();
	getPengedCard(vCard);
	for (auto& ref : vCard)
	{
		if ( card_Type(ref) == eCT_Feng )
		{
			nHuaCnt += 1;
		}
	}
	// check feng an ke 
	auto& vFeng = m_vCards[eCT_Feng];
	for (uint8_t nidx = 0; (nidx + 2) < vFeng.size(); )
	{
		if (vFeng[nidx] == vFeng[nidx + 2])
		{
			nHuaCnt += 1;
			nidx += 3;
			continue;
		}
		++nidx;
	}

	return nHuaCnt;
}