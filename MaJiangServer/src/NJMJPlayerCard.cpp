#pragma once
#include "NJMJPlayerCard.h"
#include "log4z.h"
#include "MJCard.h"
#include "NJMJRoom.h"
void NJMJPlayerCard::reset()
{
	MJPlayerCard::reset();
	m_vActCardSign.clear();
	m_vBuHuaCard.clear();
	m_pCurRoom = nullptr;
	setSongGangIdx(-1);
	m_vAllActCardSign.clear();
	m_isHaveAnGangFlag = false;
	m_isHaveZhiGangFlag = false;
}

bool NJMJPlayerCard::canEatCard(uint8_t nCard, uint8_t& nWithA, uint8_t& withB)
{
	return false;
}

void NJMJPlayerCard::addActSign(uint8_t nCard, uint8_t nInvokerIdx, eMJActType eAct)
{
	stActCardSign st;
	st.eAct = eAct;
	st.InvokerIdx = nInvokerIdx;
	st.nCard = nCard;
	m_vAllActCardSign.push_back(st);
	if ( eMJAct_AnGang != eAct )
	{
		m_vActCardSign.push_back(st);
	}
}

bool NJMJPlayerCard::isChued4Card(uint8_t nCard)
{
	auto nCnt = std::count(m_vChuedCard.begin(),m_vChuedCard.end(),nCard );
	return nCnt == 4;
}

bool NJMJPlayerCard::isChued4Feng()
{
	if ( m_vChuedCard.size() != 4 )
	{
		return false;
	}

	auto nFeng = make_Card_Num(eCT_Feng, 1);
	uint8_t nCnt = 4;
	while (nCnt--)
	{
		auto iter = std::find(m_vChuedCard.begin(),m_vChuedCard.end(),nFeng);
		if (iter == m_vChuedCard.end())
		{
			return false;
		}
		++nFeng;
	}
	return true;
}

uint8_t NJMJPlayerCard::getInvokerPengIdx(uint8_t nCard)
{
	for (auto& ref : m_vActCardSign)
	{
		if (ref.nCard == nCard && ref.eAct == eMJAct_Peng)
		{
			return ref.InvokerIdx;
		}
	}

	return -1;
}

uint8_t NJMJPlayerCard::getInvokerGangIdx(uint8_t nCard)
{
	for (auto& ref : m_vActCardSign)
	{
		if (ref.nCard == nCard && ref.eAct == eMJAct_MingGang)
		{
			return ref.InvokerIdx;
		}
	}

	return -1;
}

void NJMJPlayerCard::onHuaGang(uint8_t nCard, uint8_t nNewCard)
{
	onBuHua(nCard,nNewCard);
}

void NJMJPlayerCard::onBuHua(uint8_t nHuaCard, uint8_t nCard)
{
	auto eType = card_Type(nHuaCard);
	if (eType >= eCT_Max)
	{
		LOGFMTE("onBuHua parse card type error so do not have this card = %u", nHuaCard);
		return ;
	}

	auto& vCard = m_vCards[eType];
	auto iter = std::find(vCard.begin(), vCard.end(), nHuaCard);
	vCard.erase(iter);

	onMoCard(nCard);
	m_vBuHuaCard.push_back(nHuaCard);
}

bool NJMJPlayerCard::onDoHu(bool isZiMo, uint8_t nCard, bool isBePenged, std::vector<uint16_t>& vHuTypes, uint16_t& nHuHuaCnt, uint16_t& nHardAndSoftHua, bool& isSpecailHuPai, uint8_t nInvokerIdx )
{
	nHuHuaCnt = 0;
	vHuTypes.clear();
	bool bisSpecailHu = false;
	// if not zi mo , must add to fo check hu ;
	if (!isZiMo)
	{
		auto type = card_Type(nCard);
		if (type >= eCT_Max)
		{
			LOGFMTE("invalid card type for card = %u", nCard);
			return false;
		}
		bisSpecailHu = getIsSpecailHu(nCard);
		addCardToVecAsc(m_vCards[type], nCard);
	}
	else
	{
		bisSpecailHu = getIsZiMoSpecailHu();
	}
	isSpecailHuPai = bisSpecailHu;

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

	if ( false == bisSpecailHu && MJPlayerCard::isHoldCardCanHu() == false)
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
	auto bHunYise = checkHunYiSe(vHuTypes, nHuHuaCnt);
	if (!bHunYise)
	{
		checkQingYiSe(vHuTypes, nHuHuaCnt);
	}
	
	auto bQiDui = checkQiDui(nCard, vHuTypes, nHuHuaCnt);
	if (!bQiDui)
	{
		checkDuiDuiHu(vHuTypes, nHuHuaCnt,bisSpecailHu );
		checkMenQing(vHuTypes, nHuHuaCnt);
	}
	
	checkQuanQiuDuDiao(nCard, vHuTypes, nHuHuaCnt, bisSpecailHu, nInvokerIdx,isZiMo);
	if (bisSpecailHu == false)
	{
		checkYaJue(nCard, isBePenged, vHuTypes, nHuHuaCnt);
	}
	checkWuHuaGuo(vHuTypes, nHuHuaCnt);

	if (vHuTypes.empty())
	{
		vHuTypes.push_back(eFanxing_PingHu);
	}
	// default add 10 hua , pao 10 ; 
	nHuHuaCnt += 10;
	nHardAndSoftHua = getAllHuaCnt(nCard); // all soft and hard hua 
	
	// if have ya jue , must erase one soft hua 
	auto niterYaJue = std::find(vHuTypes.begin(),vHuTypes.end(),eFanxing_YaJue);
	if (niterYaJue != vHuTypes.end())
	{
		--nHardAndSoftHua;
	}

	if (!isZiMo)
	{
		funRemoveAddToCard(nCard);
	}
	return true;
}

bool NJMJPlayerCard::canHuWitCard(uint8_t nCard)
{
	auto eType = card_Type(nCard);
	if (eType >= eCT_Max)
	{
		LOGFMTE("parse card type error ,canHuWitCard have this card = %u", nCard);
		return false;
	}

	if (getIsSpecailHu(nCard)) // specail hu must be da hu ;
	{
		return true;
	}

	std::vector<uint16_t> vType;
	uint16_t nHuHuaCnt = 0;
	uint16_t nHardSoftHua = 0;
	bool isSpecialHu = false;
	auto bRet = onDoHu(false, nCard, m_pCurRoom->isCardByPenged(nCard), vType, nHuHuaCnt, nHardSoftHua, isSpecialHu);
	
	do
	{
		if (bRet == false)
		{
			break;
		}

		if ((vType.size() == 1 && vType.front() == eFanxing_PingHu) && m_vBuHuaCard.size() < 4 )
		{
			bRet = false;
			break;
		}

	} while (0);
	return bRet;
}

bool NJMJPlayerCard::getIsSpecailHu(uint8_t nTargetCard)
{
	VEC_CARD vHoldCard;
	getHoldCard(vHoldCard);
	if (vHoldCard.size() != 4)
	{
		return false;
	}

	if (!canPengWithCard(nTargetCard))
	{
		return false;
	}

	// dui tong yi ren san zui 
	auto nBaoPaiidx = getSpecailHuBaoPaiKuaiZhaoIdx();
	if (nBaoPaiidx != (uint8_t)-1 )
	{
		return true;
	}

	// qing yi se qingkuang , dui yi zui ;
	VEC_CARD vAllCard;
	getHoldCard(vAllCard);
	if (card_Type(nTargetCard) != card_Type(vAllCard.front()))
	{
		return false;
	}

	VEC_CARD vTemp;
	getAnGangedCard(vTemp);
	vAllCard.insert(vAllCard.end(), vTemp.begin(), vTemp.end());

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

bool NJMJPlayerCard::getIsZiMoSpecailHu()
{
	VEC_CARD vHoldCard;
	getHoldCard(vHoldCard);
	if (vHoldCard.size() != 2 )
	{
		return false;
	}

	// have gang flag ;
	if ( ( false == m_isHaveAnGangFlag ) && ( false == m_isHaveZhiGangFlag ) )
	{
		return false;
	}

	// dui tong yi ren san zui 
	auto nBaoPaiidx = getSpecailHuBaoPaiKuaiZhaoIdx();
	if (nBaoPaiidx != (uint8_t)-1)
	{
		return true;
	}

	// qing yi se qingkuang , dui yi zui ;
	VEC_CARD vAllCard;
	getHoldCard(vAllCard);
	if (card_Type(vAllCard.back()) != card_Type(vAllCard.front()))
	{
		return false;
	}

	VEC_CARD vTemp;
	getAnGangedCard(vTemp);
	vAllCard.insert(vAllCard.end(), vTemp.begin(), vTemp.end());

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

bool NJMJPlayerCard::onChuCard(uint8_t nChuCard)
{
	m_isHaveAnGangFlag = false;
	m_isHaveZhiGangFlag = false;
	return MJPlayerCard::onChuCard(nChuCard);
}

bool NJMJPlayerCard::onMingGang(uint8_t nCard, uint8_t nGangGetCard)
{
	m_isHaveZhiGangFlag = true;
	return MJPlayerCard::onMingGang(nCard, nGangGetCard);
}

bool NJMJPlayerCard::onAnGang(uint8_t nCard, uint8_t nGangGetCard)
{
	m_isHaveAnGangFlag = true;
	return MJPlayerCard::onAnGang(nCard, nGangGetCard);
}

bool NJMJPlayerCard::getCanHuCards(std::set<uint8_t>& vCanHuCards)
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

				if (card_Value(a) <= 9)
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
		if (canHuWitCardLocal(card))
		{
			vCanHuCards.insert(card);
		}
	}
	return vCanHuCards.size() > 0;
}

bool NJMJPlayerCard::canHuWitCardLocal(uint8_t nCard)
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

void NJMJPlayerCard::bindRoom(NJMJRoom* pRoom)
{
	m_pCurRoom = pRoom;
}

uint8_t NJMJPlayerCard::getKuaiZhaoBaoPaiIdx()
{
	// peng or gang 1 time , hanve 2 an gang
	if (m_vAnGanged.size() == 2)
	{
		for (auto& ref : m_vActCardSign)
		{
			return ref.InvokerIdx;
		}
	}

	// find invoke times ;
	std::map<uint8_t, uint8_t> mapIdxCnt;
	for (auto& ref : m_vActCardSign)
	{
		auto iter = mapIdxCnt.find(ref.InvokerIdx);
		if (iter == mapIdxCnt.end())
		{
			mapIdxCnt[ref.InvokerIdx] = 1;
		}
		else
		{
			++mapIdxCnt[ref.InvokerIdx];
		}
	}

	// peng or gang  2 time, have an gang
	if (m_vAnGanged.empty() == false)
	{
		for (auto& ref : mapIdxCnt)
		{
			if (ref.second >= 2)
			{
				return ref.first;
			}
		}
	}

	// peng or gang 3 times  
	for (auto& ref : mapIdxCnt)
	{
		if (ref.second >= 3)
		{
			return ref.first;
		}
	}

	return -1;
}

uint8_t NJMJPlayerCard::getLastActSignIdx()
{
	if (m_vActCardSign.empty() || m_vActCardSign.size() < 4 )
	{
		return -1;
	}

	return m_vActCardSign.back().InvokerIdx;
}

bool NJMJPlayerCard::isHoldCardCanHu()
{
	if (getIsZiMoSpecailHu())
	{
		return true;
	}

	std::vector<uint16_t> vType;
	uint16_t nHuHuaCnt = 0;
	uint16_t nHardSoftHua = 0;
	bool bIsSpecialHu = false;
	auto bRet = onDoHu(true, getNewestFetchedCard(), false, vType, nHuHuaCnt, nHardSoftHua, bIsSpecialHu);
	if (bRet == false)
	{
		return false;
	}

	if ((vType.size() == 1 && vType.front() == eFanxing_PingHu) && m_vBuHuaCard.size() < 4 )
	{
		return false;
	}
	return true;
}

bool NJMJPlayerCard::canCardHuaGang(uint8_t nCard)
{
	auto eType = card_Type(nCard);
	if (eType == eCT_Jian)
	{
		auto cnt = std::count(m_vBuHuaCard.begin(),m_vBuHuaCard.end(),nCard);
		return cnt >= 3;
	}
	else if (eCT_Hua == eType)
	{
		bool isXiaoYuSi = card_Value(nCard) <= 4;
		uint8_t nHuaCnt = 0;
		for (auto& ref : m_vBuHuaCard)
		{
			auto nt = card_Type(ref);
			auto nCheckV = card_Value(ref);
			if (nt != eCT_Hua)
			{
				continue;
			}
			
			if (isXiaoYuSi)
			{
				if (nCheckV <= 4 )
				{
					++nHuaCnt;
				}
			}
			else
			{
				if (nCheckV > 4)
				{
					++nHuaCnt;
				}
			}
		}

		return nHuaCnt % 4 >= 3;
	}

	LOGFMTE("not hua type check hua gang ? card = %u",nCard);
	return false;
}

bool NJMJPlayerCard::getCardInfo(Json::Value& jsPeerCards)
{
	// svr: { idx : 2 , anPai : [2,3,4,34] , huaPai : [23,23,12] ,chuPai: [2,34,4] , anGangPai : [23,24],buGang : [23,45] ,pengGangInfo : [ { targetIdx : 23 , actType : 23 , card : 23 } , .... ]  }
	IMJPlayerCard::VEC_CARD vAnPai, vChuPai, vAnGangedCard , vBuGang;
	Json::Value jsAnPai, jsChuPai,  jsAngangedPai, jsBuGang , jsHuaPai;

	getHoldCard(vAnPai);
	getChuedCard(vChuPai);
	getAnGangedCard(vAnGangedCard);

	for (auto& ref : m_vGanged)
	{
		if ((uint8_t)-1 != getInvokerPengIdx(ref))
		{
			vBuGang.push_back(ref);
		}
	}

	auto toJs = [](IMJPlayerCard::VEC_CARD& vCards, Json::Value& js)
	{
		for (auto& c : vCards)
		{
			js[js.size()] = c;
		}
	};

	toJs(vAnPai, jsAnPai); toJs(vChuPai, jsChuPai); toJs(vAnGangedCard, jsAngangedPai); toJs(vBuGang, jsBuGang); toJs(m_vBuHuaCard,jsHuaPai);
	jsPeerCards["anPai"] = jsAnPai; jsPeerCards["chuPai"] = jsChuPai; jsPeerCards["anGangPai"] = jsAngangedPai; jsPeerCards["buGang"] = jsBuGang; jsPeerCards["huaPai"] = jsHuaPai;

	Json::Value jsPengGangInfo;
	for (auto& ref : m_vActCardSign)
	{
		Json::Value js;
		js["targetIdx"] = ref.InvokerIdx;
		js["actType"] = ref.eAct;
		js["card"] = ref.nCard;
		jsPengGangInfo[jsPengGangInfo.size()] = js;
	}

	jsPeerCards["pengGangInfo"] = jsPengGangInfo;
	return true;
}

uint8_t NJMJPlayerCard::getHuaCardToBuHua()
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

void NJMJPlayerCard::setSongGangIdx(uint8_t nSongGangIdx)
{
	m_nSongGangIdx = nSongGangIdx;
}

uint8_t NJMJPlayerCard::getSongGangIdx()
{
	return m_nSongGangIdx;
}

// check pai xing 
bool NJMJPlayerCard::checkHunYiSe(std::vector<uint16_t>& vHuTypes, uint16_t& nHuaCnt)
{
	VEC_CARD vCheckCard;
	for (auto nIdx = 0; nIdx < eCT_Max; ++nIdx)
	{
		auto& vCard = m_vCards[nIdx];
		if (vCard.empty() == false)
		{
			vCheckCard.push_back(vCard.front());
		}
	}
	// check peng ,an gang , ming gang 
	for (auto& ref : m_vActCardSign)
	{
		vCheckCard.push_back(ref.nCard);
	}

	for (auto& ref : m_vAnGanged)
	{
		vCheckCard.push_back(ref);
	}

	// check 
	bool bFindFeng = false;
	uint8_t eMjtype  = eCT_Max;
	for (auto& ref : vCheckCard)
	{
		auto type = card_Type(ref);
		if (eCT_Feng == type)
		{
			bFindFeng = true;
			continue;
		}

		if (eCT_Wan == type || eCT_Tong == type || eCT_Tiao == type)
		{
			if (eCT_Max == eMjtype)
			{
				eMjtype = type;
				continue;
			}

			if (eMjtype != type)
			{
				return false;
			}
		}
	}

	if (bFindFeng)
	{
		vHuTypes.push_back(eFanxing_HunYiSe);
		nHuaCnt += 30;
	}

	return bFindFeng;
}

bool NJMJPlayerCard::checkMenQing(std::vector<uint16_t>& vHuTypes, uint16_t& nHuaCnt)
{
	std::vector<stActCardSign>::iterator iter = m_vActCardSign.begin();
	for (; iter != m_vActCardSign.end(); ++iter)
	{
		if ( (*iter).eAct == eMJAct_Peng )
		{
			return false;
		}
	}
	vHuTypes.push_back(eFanxing_MengQing);
	nHuaCnt += 10;
	return true;
}

bool NJMJPlayerCard::checkQingYiSe(std::vector<uint16_t>& vHuTypes, uint16_t& nHuaCnt)
{
	VEC_CARD vCheckCard;
	for (auto nIdx = 0; nIdx < eCT_Max; ++nIdx)
	{
		auto& vCard = m_vCards[nIdx];
		if (vCard.empty() == false)
		{
			vCheckCard.push_back(vCard.front());
		}
	}
	// check peng ,an gang , ming gang 
	for (auto& ref : m_vActCardSign)
	{
		vCheckCard.push_back(ref.nCard);
	}

	for (auto& ref : m_vAnGanged)
	{
		vCheckCard.push_back(ref);
	}

	// check 
	uint8_t eMjtype = eCT_Max;
	for (auto& ref : vCheckCard)
	{
		auto type = card_Type(ref);
		if (eCT_Wan == type || eCT_Tong == type || eCT_Tiao == type)
		{
			if (eCT_Max == eMjtype)
			{
				eMjtype = type;
				continue;
			}

			if (eMjtype != type)
			{
				return false;
			}
		}
	}

	vHuTypes.push_back(eFanxing_QingYiSe);
	nHuaCnt += 40;
	return true;
}

bool NJMJPlayerCard::checkDuiDuiHu(std::vector<uint16_t>& vHuTypes, uint16_t& nHuaCnt, bool isSpecailHu)
{
	if (isSpecailHu)
	{
		vHuTypes.push_back(eFanxing_DuiDuiHu);
		nHuaCnt += 30;
		return true;
	}

	auto pfunCheckKeZi = [](VEC_CARD vCards, uint8_t nType, uint8_t nJiang)->bool
	{
		if (vCards.empty())
		{
			return true;
		}

		if (card_Type(nJiang) == nType)
		{
			// remove jiang
			auto cnt = std::count(vCards.begin(), vCards.end(), nJiang);
			if (cnt < 2)
			{
				LOGFMTE("check duidui hu fanxing jiang cnt < 2  %u", nJiang);
				return false;
			}

			auto iter = std::find(vCards.begin(), vCards.end(), nJiang);
			vCards.erase(iter);
			iter = std::find(vCards.begin(), vCards.end(), nJiang);
			vCards.erase(iter);
		}


		for (uint8_t nIdx = 0; nIdx + 2u < vCards.size(); nIdx += 3)
		{
			if (vCards[nIdx] != vCards[nIdx + 2u])
			{
				return false;
			}
		}
		return true;
	};

	auto nJiang = m_nJIang;
	if (pfunCheckKeZi(m_vCards[eCT_Tiao], eCT_Tiao, nJiang) == false)
	{
		return false;
	}

	if (pfunCheckKeZi(m_vCards[eCT_Wan], eCT_Wan, nJiang) == false)
	{
		return false;
	}

	if (pfunCheckKeZi(m_vCards[eCT_Tong], eCT_Tong, nJiang) == false)
	{
		return false;
	}

	if (pfunCheckKeZi(m_vCards[eCT_Feng], eCT_Feng, nJiang) == false)
	{
		return false;
	}

	vHuTypes.push_back(eFanxing_DuiDuiHu);
	nHuaCnt += 30;
	return true;
}

bool NJMJPlayerCard::checkQiDui(uint8_t nCard, std::vector<uint16_t>& vHuTypes, uint16_t& nHuaCnt)
{
	if ( !canHoldCard7PairHu())
	{
		return false;
	}

	auto nType = card_Type(nCard);
	auto& vCard = m_vCards[nType];
	auto nCnt = std::count(vCard.begin(),vCard.end(),nCard);
	if (nCnt == 4)
	{
		vHuTypes.push_back(eFanxing_ShuangQiDui);
		nHuaCnt += 100;
	}
	else
	{
		vHuTypes.push_back(eFanxing_QiDui);
		nHuaCnt += 50;
	}

	return true;
}

bool NJMJPlayerCard::checkQuanQiuDuDiao(uint8_t nCard, std::vector<uint16_t>& vHuTypes, uint16_t& nHuaCnt, bool isSpecailHu, uint8_t nInvokerIdx, bool isZiMo )
{
	if (isSpecailHu)
	{
		auto iter = std::find(vHuTypes.begin(),vHuTypes.end(),eFanxing_QingYiSe);
		bool isQingYISe = iter != vHuTypes.end();
		auto nSpecailZhaopIdx = getSpecailHuBaoPaiKuaiZhaoIdx();
		if ( (isZiMo && ( m_isHaveAnGangFlag || (m_isHaveZhiGangFlag && nSpecailZhaopIdx == getSongGangIdx() ) ) ) || isQingYISe || ( isZiMo == false && nSpecailZhaopIdx == nInvokerIdx && (uint8_t)-1 != nInvokerIdx ) )
		{
			vHuTypes.push_back(eFanxing_QuanQiuDuDiao);
			nHuaCnt += 40;
			return true;
		}
		return false;
	}

	uint8_t nCnt = 0;
	for (auto& vCard : m_vCards)
	{
		nCnt += vCard.size();
		if (nCnt > 2)
		{
			return false;
		}
	}

	vHuTypes.push_back(eFanxing_QuanQiuDuDiao);
	nHuaCnt += 40;
	return true;
}

bool NJMJPlayerCard::checkYaJue(uint8_t nCard, bool isBePenged, std::vector<uint16_t>& vHuTypes, uint16_t& nHuaCnt)
{
	if (isBePenged == false)
	{
		return false;
	}

	if (!checkYaDang(nCard))
	{
		return false;
	}

	vHuTypes.push_back(eFanxing_YaJue);
	nHuaCnt += 30;
	return true;
}

bool NJMJPlayerCard::checkWuHuaGuo(std::vector<uint16_t>& vHuTypes, uint16_t& nHuaCnt)
{
	if ( m_vBuHuaCard.empty() == false )
	{
		return true;
	}

	if (vHuTypes.empty()) // means ping hu 
	{
		for (auto& ref : m_vActCardSign)
		{
			if ( ref.eAct == eMJAct_Peng)
			{
				return false;
			}
		}
	}

	vHuTypes.push_back(eFanxing_WuHuaGuo);
	nHuaCnt += 40;
	return true;
}

// check fanxing helpe
uint16_t NJMJPlayerCard::getAllHuaCnt(uint8_t nHuCard)
{
	uint16_t nHuaCnt = m_vBuHuaCard.size();   // ying hua 

	// check feng peng 
	for (uint8_t nValue = 1; nValue <= 4; ++nValue)
	{
		auto nCard = make_Card_Num(eCT_Feng, nValue);
		auto& vHold = m_vCards[eCT_Feng];
		auto nCnt = std::count(vHold.begin(), vHold.end(), nCard);
		if (nCnt == 3)  // feng peng 
		{
			++nHuaCnt;
			continue;
		}

		auto iter = std::find(m_vPenged.begin(), m_vPenged.end(), nCard);
		if (iter != m_vPenged.end())  // feng peng 
		{
			++nHuaCnt;
			continue;
		}

		// check An gang 
		auto iterAn = std::find(m_vAnGanged.begin(), m_vAnGanged.end(), nCard);
		if (iterAn != m_vAnGanged.end())  // An gang 
		{
			nHuaCnt += 3;
			continue;
		}

		// check minggang gang 
		auto iterMing = std::find(m_vGanged.begin(), m_vGanged.end(), nCard);
		if (iterMing != m_vGanged.end())  // ming gang 
		{
			nHuaCnt += 2;
			continue;
		}
	}

	// check YaDang 
	if (checkYaDang(nHuCard))
	{
		++nHuaCnt;
	}

	// check du zhan 
	if (checkDuZhan(nHuCard))
	{
		++nHuaCnt;
	}

	// check bian zhi 
	if (checkBianZhi(nHuCard))
	{
		++nHuaCnt;
	}

	// check bian zhi 
	if (checkQueYi(nHuCard))
	{
		++nHuaCnt;
	}
	return nHuaCnt;
}

bool NJMJPlayerCard::checkYaDang(uint8_t nCard)
{
	auto nType = card_Type(nCard);
	if (nType != eCT_Wan && nType != eCT_Tong && nType != eCT_Tiao)
	{
		return false;
	}

	auto nPre = nCard - 1;
	auto nNext = nCard + 1;
	if (isHaveCard(nPre) == false || false == isHaveCard(nNext))
	{
		return false;
	}

	// remove still can holse card hu 
	auto& vCard = m_vCards[nType];
	auto iter = std::find(vCard.begin(),vCard.end(),nPre);
	vCard.erase(iter);

	iter = std::find(vCard.begin(), vCard.end(), nCard);
	vCard.erase(iter);

	iter = std::find(vCard.begin(), vCard.end(), nNext);
	vCard.erase(iter);

	auto isOk = MJPlayerCard::isHoldCardCanHu();
	addCardToVecAsc(vCard, nPre);
	addCardToVecAsc(vCard, nCard);
	addCardToVecAsc(vCard, nNext);
	return isOk;
}

bool NJMJPlayerCard::checkDuZhan(uint8_t nCard)
{
	if (nCard != m_nJIang)
	{
		return false;
	}

	auto nType = card_Type(nCard);
	auto& vCard = m_vCards[nType];
	auto iter = std::find(vCard.begin(), vCard.end(), nCard);
	vCard.erase(iter);

	SET_NOT_SHUN vNotShun[eCT_Max];
	for (uint8_t nIdx = 0; nIdx < eCT_Max; ++nIdx )
	{
		getNotShuns(m_vCards[nIdx], vNotShun[nIdx], eCT_Feng == nIdx || eCT_Jian == nIdx);
	}

	addCardToVecAsc(vCard, nCard);
 
	for (uint8_t nsidx = 0; nsidx < eCT_Max; ++nsidx)
	{
		if ( vNotShun[nsidx].empty())
		{
			continue;
		}

		if (vNotShun[nsidx].size() >= 2)
		{
			return false;
		}

		if (vNotShun[nsidx].size() >= 1 && nsidx != nType)
		{
			return false;
		}

		auto iter = vNotShun[nsidx].begin();
		if (iter->vCards.size() != 1)
		{
			return false;
		}
	}
	

	return true ;
}

bool NJMJPlayerCard::checkBianZhi(uint8_t nCard)
{
	auto nType = card_Type(nCard);
	if (nType != eCT_Wan && nType != eCT_Tong && nType != eCT_Tiao)
	{
		return false;
	}

	auto nValue = card_Value(nCard);
	if (nValue != 7 && 3 != nValue)
	{
		return false;
	}

	auto nPre = nCard;
	auto nNext = nCard;
	if (nValue == 7)
	{
		nPre = nCard + 1;
		nNext = nCard + 2;
	}
	else if (3 == nValue)
	{
		nPre = nCard - 1;
		nNext = nCard - 2;
	}
	else
	{
		return false;
	}

	if (isHaveCard(nPre) == false || false == isHaveCard(nNext))
	{
		return false;
	}

	// remove still can holse card hu 
	auto& vCard = m_vCards[nType];
	auto iter = std::find(vCard.begin(), vCard.end(), nPre);
	vCard.erase(iter);

	iter = std::find(vCard.begin(), vCard.end(), nCard);
	vCard.erase(iter);

	iter = std::find(vCard.begin(), vCard.end(), nNext);
	vCard.erase(iter);

	auto isOk = MJPlayerCard::isHoldCardCanHu();
	addCardToVecAsc(vCard, nPre);
	addCardToVecAsc(vCard, nCard);
	addCardToVecAsc(vCard, nNext);
	return isOk;
}

bool NJMJPlayerCard::checkQueYi(uint8_t nCard)
{
	std::map<uint8_t,uint8_t> vSign;
	for (uint8_t nIdx = 0; nIdx < eCT_Max; ++nIdx)
	{
		if (nIdx == eCT_Tiao || eCT_Wan == nIdx || eCT_Tong == nIdx)
		{
			if (m_vCards[nIdx].empty() == false)
			{
				vSign[nIdx] = 1;
			}
		}
	}
	
	for (auto& ref : m_vActCardSign)
	{
		auto eType = card_Type(ref.nCard);
		if (eType == eCT_Tiao || eCT_Wan == eType || eCT_Tong == eType)
		{
			vSign[eType] = 1;
		}
	}
	return vSign.size() == 2 ;
}

uint8_t NJMJPlayerCard::getSpecailHuBaoPaiKuaiZhaoIdx()
{
	if (m_vAllActCardSign.size() < 3)
	{
		return -1;
	 }

	uint8_t nBaoIdx = -1;
	for (uint8_t nIdx = 0; nIdx < 3 ; ++nIdx)
	{
		if ( m_vAllActCardSign[nIdx].eAct != eMJAct_AnGang)
		{
			if (nBaoIdx == (uint8_t)-1)
			{
				nBaoIdx = m_vAllActCardSign[nIdx].InvokerIdx;
				continue;
			}

			if ( nBaoIdx != m_vAllActCardSign[nIdx].InvokerIdx )
			{
				return -1;
			}
		}
	}
	return nBaoIdx;
}
