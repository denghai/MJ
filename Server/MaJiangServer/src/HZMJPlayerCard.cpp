#include "HZMJPlayerCard.h"
#include "MJCard.h"
#include "log4z.h"
bool HZMJPlayerCard::canEatCard(uint8_t nCard, uint8_t& nWithA, uint8_t& withB)
{
	if (m_vEated.size() >= 6)
	{
		return false;
	}

	return MJPlayerCard::canEatCard(nCard,nWithA,withB);
}

//bool HZMJPlayerCard::is7PairTing()
//{
//	if (m_vPenged.empty() == false || false == m_vGanged.empty() || false == m_vEated.empty())
//	{
//		return false;
//	}
//
//	if (get7PairQueCnt() <= getCaiShenCnt() + 1 )
//	{
//		return true;
//	}
//	return false;
//}
//
//bool HZMJPlayerCard::canHoldCard7PairHu()
//{
//	if (m_vPenged.empty() == false || false == m_vGanged.empty() || false == m_vEated.empty())
//	{
//		return false;
//	}
//
//	if (get7PairQueCnt() == getCaiShenCnt() )
//	{
//		return true;
//	}
//	return false;
//}

//bool HZMJPlayerCard::isTingPai()
//{
//	if (getMiniQueCnt() <= getCaiShenCnt() + 1)
//	{
//		return true;
//	}
//
//	return false;
//}

//bool HZMJPlayerCard::isHoldCardCanHu()
//{
//	if (getMiniQueCnt() == getCaiShenCnt() )
//	{
//		return true;
//	}
//
//	return false;
//}

uint8_t HZMJPlayerCard::getMiniQueCnt( VEC_CARD vCards[eCT_Max] )
{
	VEC_CARD vBackUpJian;
	vBackUpJian.assign(vCards[eCT_Jian].begin(), vCards[eCT_Jian].end());
	auto caiShen = make_Card_Num(eCT_Jian, 3);
	auto iter = std::find(vCards[eCT_Jian].begin(), vCards[eCT_Jian].end(), caiShen);
	while (iter != vCards[eCT_Jian].end())
	{
		vCards[eCT_Jian].erase(iter);
		iter = std::find(vCards[eCT_Jian].begin(), vCards[eCT_Jian].end(), caiShen);
	}

	uint8_t nCnt = MJPlayerCard::getMiniQueCnt(vCards);
	// rollback ;
	vCards[eCT_Jian].swap(vBackUpJian);

	uint8_t nCaishenCnt = getCaiShenCnt();
	if (nCaishenCnt >= nCnt) // hu le 
	{
		if (m_nJIang == 0)
		{
			m_nJIang = caiShen;
			m_nDanDiao = caiShen;
		}
		nCaishenCnt -= nCnt;
		if (nCaishenCnt != 0 && nCaishenCnt != 3)
		{
			LOGFMTE("can not be here !! nCaishenCnt != 0 && nCaishenCnt != 3 ");
		}
		return 0;
	}
	nCnt -= nCaishenCnt;
	return nCnt;
}

//uint8_t HZMJPlayerCard::getLestQue(SET_NOT_SHUN& vNotShun, bool bFindJiang, bool bFindDanDiao, uint8_t& nFiandJiang, uint8_t& nFindDanDiao)
//{
//	if (vNotShun.empty())
//	{
//		return 0;
//	}
//
//	auto pfunFindQueCnt = [](stNotShunCard& stNotShun, bool bFindJiang, bool bFindDanDiao, uint8_t& nFiandJiang, uint8_t& nFindDanDiao)->uint8_t 
//	{
//		auto& vCards = stNotShun.vCards;
//		if (vCards.empty())
//		{
//			LOGFMTE("not shun must not be empty ? error ");
//			return 0;
//		}
//
//		auto type = card_Type(vCards.front());
//		auto bMustKe = (type == eCT_Feng || eCT_Jian == type);
//
//		uint8_t nLackCnt = 0;
//		std::sort(vCards.begin(), vCards.end());
//		for (uint8_t nIdx = 0; nIdx < vCards.size();)
//		{
//			uint8_t nValue = vCards[nIdx];
//			if ((uint8_t)(nIdx + 1) >= vCards.size())  // last single card ;
//			{
//				nLackCnt += 2;
//				if (bFindDanDiao)
//				{
//					nLackCnt -= 1;
//					bFindDanDiao = false;
//					nFindDanDiao = nValue;
//				}
//
//				break;
//			}
//
//			uint8_t nNextV = vCards[nIdx + 1];
//			if (nNextV == nValue)
//			{
//				nLackCnt += 1;
//				nIdx += 2;
//				if (bFindJiang)
//				{
//					nLackCnt -= 1;
//					bFindJiang = false;
//					nFiandJiang = nValue;
//				}
//				continue;
//			}
//
//			if (bMustKe == false && (nNextV - nValue <= 2))
//			{
//				nLackCnt += 1;
//				nIdx += 2;
//				continue;
//			}
//
//			nLackCnt += 2;
//			nIdx += 1;
//			if (bFindDanDiao)
//			{
//				nLackCnt -= 1;
//				bFindDanDiao = false;
//				nFindDanDiao = nValue;
//			}
//		}
//		return nLackCnt;
//	};
//
//	uint8_t nLesetQue = 100;
//	uint8_t nFedJIangResult = 0;
//	uint8_t nFedDanResult = 0;
//
//	auto iter = vNotShun.begin();
//	for (; iter != vNotShun.end(); ++iter)
//	{
//		uint8_t nFedJIang = 0;
//		uint8_t nFedDan = 0;
//		stNotShunCard stQ = *iter;
//		auto nQuenCnt = pfunFindQueCnt(stQ, bFindJiang, bFindDanDiao, nFedJIang, nFedDan);
//		if (nQuenCnt < nLesetQue)
//		{
//			nLesetQue = nQuenCnt;
//			nFedJIangResult = nFedJIang;
//			nFedDanResult = nFedDan;
//		}
//	}
//
//	nFiandJiang = nFedJIangResult;
//	nFindDanDiao = nFedDanResult;
//	return nLesetQue;
//}

uint8_t HZMJPlayerCard::getCaiShenCnt()
{
	auto& vCnt = m_vCards[eCT_Jian];
	uint8_t nCaiShenCnt = 0;
	for (auto& ref : vCnt)
	{
		if (card_Value(ref) == 3)  // here is cai shen ;
		{
			++nCaiShenCnt;
		}
	}
	return nCaiShenCnt;
}

uint8_t HZMJPlayerCard::get7PairQueCnt( VEC_CARD vCards[eCT_Max] )
{
	VEC_CARD vBackUpJian;
	vBackUpJian.assign(vCards[eCT_Jian].begin(), vCards[eCT_Jian].end());
	auto caiShen = make_Card_Num(eCT_Jian, 3);
	auto iter = std::find(vCards[eCT_Jian].begin(), vCards[eCT_Jian].end(), caiShen);
	while (iter != vCards[eCT_Jian].end())
	{
		vCards[eCT_Jian].erase(iter);
		iter = std::find(vCards[eCT_Jian].begin(), vCards[eCT_Jian].end(), caiShen);
	}
	auto nCnt = MJPlayerCard::get7PairQueCnt(vCards);
	// rollback ;
	vCards[eCT_Jian].swap(vBackUpJian);

	uint8_t nCaishenCnt = getCaiShenCnt();
	if (nCaishenCnt >= nCnt) // hu le 
	{
		if (m_nJIang == 0)
		{
			m_nJIang = caiShen;
			m_nDanDiao = caiShen;
		}
		nCaishenCnt -= nCnt;
		return nCaishenCnt % 2;
	}
	nCnt -= nCaishenCnt;
	return nCnt;
}

bool HZMJPlayerCard::canHuWitCard(uint8_t nCard)
{
	LOGFMTD("hangzhou Majiang only can zi mo ");
	return false;
}

uint8_t HZMJPlayerCard::get7PairHuHaoHuaCnt()
{
	if (canHoldCard7PairHu() == false)
	{
		return 0;
	}

	uint8_t nCaiCnt = getCaiShenCnt();
	uint8_t nReal4 = 0;
	uint8_t nReal3 = 0;
	uint8_t nReal2 = 0;
	for (uint8_t nType = eCT_None; nType < eCT_Max; ++nType)
	{
		VEC_CARD vCard = m_vCards[nType];
		if (nType == eCT_Jian)
		{
			// remove cai shen 
			auto caiShen = make_Card_Num(eCT_Jian, 3);
			auto iter = std::find(vCard.begin(), vCard.end(), caiShen);
			while (iter != vCard.end())
			{
				vCard.erase(iter);
				iter = std::find(vCard.begin(), vCard.end(), caiShen);
			}
		}

		// do check ;
		if (vCard.size() < 3 )
		{
			continue;
		}

		for (uint8_t nIdx = 0; (uint8_t)(nIdx) < vCard.size(); )
		{
			auto thirdValue = 0;
			auto forthValue = 0;
			auto second = 0;

			if ((uint8_t)(nIdx + 1) < vCard.size())
			{
				second = vCard[nIdx + 1];
			}

			if ((uint8_t)(nIdx + 2) < vCard.size())
			{
				thirdValue = vCard[nIdx + 2];
			}

			if ((uint8_t)(nIdx + 3) < vCard.size())
			{
				forthValue = vCard[nIdx + 3];
			}

			if ( vCard[nIdx] == forthValue )
			{
				++nReal4;
				nIdx += 4;
			}
			else if (vCard[nIdx] == thirdValue)
			{
				++nReal3;
				nIdx += 3;
				if (nCaiCnt > 0)
				{
					--nCaiCnt;
				}
				else
				{
					LOGFMTE("why cai sheng is not engough");
				}
			}
			else if (vCard[nIdx] == second)
			{
				++nReal2;
				nIdx += 2;
			}
			else
			{
				++nReal2;
				if (nCaiCnt > 0)
				{
					--nCaiCnt;
				}
				else
				{
					LOGFMTE("why cai sheng is not engough");
				}
				nIdx += 1;
			}
		}
	}

	uint8_t nCaiPair = nCaiCnt / 2;
	auto addtion = nCaiPair < nReal2 ? nCaiPair : nReal2;
	return (nReal3 + nReal4 + addtion);
}

bool HZMJPlayerCard::isBaoTou()
{
	auto caiShen = make_Card_Num(eCT_Jian, 3);
	return ( caiShen == m_nJIang || caiShen == m_nDanDiao );
}