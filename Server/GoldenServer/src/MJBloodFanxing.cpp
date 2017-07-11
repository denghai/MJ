#include "MJBloodFanxing.h"
#include <cassert>
#include "MJCard.h"
CBloodFanxingPingHu::CBloodFanxingPingHu()
{
	auto pp = new CBloodQingYiSe ;
	addChildFanXing(pp);

	auto ppp = new CBloodFanxingDaiYaoJiu;
	addChildFanXing(ppp) ;
}

bool CBloodFanxingPingHu::checkFanXingWantedCards(CMJPeerCard& peerCard, LIST_WANTED_CARD& vWanted)
{
	uint8_t nWantCount = vWanted.size() ;
	if ( peerCard.isContainMustQue() )
	{
		return false ;
	}

	std::vector<stCardTypeCheckResult> vecResult;
	for ( auto ref : peerCard.m_vSubCollectionCards )
	{
		VEC_CARD_ITEM vCardAllNum ;
		
		stCarItem stItem ;
		stItem.nCardValue = 0  ;
		stItem.nCount = 0 ;
		for ( auto singlCard : ref.second.m_vAnCards )
		{
			if ( stItem.nCardValue == 0 && stItem.nCount == 0 )
			{
				stItem.nCardValue = singlCard.nCardNumber ;
				stItem.nCount = 1 ;
				continue;
			}

			if ( stItem.nCardValue = singlCard.nCardNumber )
			{
				++stItem.nCount;
				continue;
			}

			vCardAllNum.push_back(stItem) ;
			stItem.nCardValue = singlCard.nCardNumber ;
			stItem.nCount = 1 ;
		}

		if ( vCardAllNum.size() == 0 )
		{
			continue;
		}

		if ( checkChunShun(vCardAllNum) )
		{
			continue;
		}

		stCardTypeCheckResult vtypeResult ;
		vtypeResult.eType = ref.first ;

		stCheckResult result ;
		if ( checkZhiYouJiang(vCardAllNum,result.vQueCardValue) )
		{
			result.eType = eResult_ZhiYouJiang ;
			vtypeResult.vResults.push_back(result) ;
		}

		result.vQueCardValue.clear() ;
		if ( checkYouJiangQueShun(vCardAllNum,result.vQueCardValue) )
		{
			result.eType = eResult_YouJiangQueShun ;
			vtypeResult.vResults.push_back(result) ;
		}

		result.vQueCardValue.clear() ;
		if ( checkZhiQueJiang(vCardAllNum,result.vQueCardValue) )
		{
			result.eType = eResult_ZhiQueJiang ;
			vtypeResult.vResults.push_back(result) ;
		}

		result.vQueCardValue.clear() ;
		if ( checkWuJiangQueShun(vCardAllNum,result.vQueCardValue) )
		{
			result.eType = eResult_WuJiangQueShun ;
			vtypeResult.vResults.push_back(result) ;
		}

		if ( vtypeResult.vResults.empty() == false )
		{
			vecResult.push_back(vtypeResult);
		}
		else
		{
			printf("have card type is none of result type = %u expected so , can not hu \n",vtypeResult.eType);
			return false ;
		}
	}

	if ( vecResult.size() == 0 || vecResult.size() > 2 )
	{
		return false ;
	}

	// wanted card mo ban ;
	stWantedCard stWant;
	stWant.eCanInvokeAct = eMJAct_Hu ;
	stWant.eFanxing = getType() ;
	stWant.eWanteddCardFrom = ePos_Any ;
	stWant.nFanRate = getFanRate();

	// only one type 
	if ( vecResult.size() == 1 )
	{
		stCardTypeCheckResult& tR = vecResult.front() ;
		for ( auto& tt : tR.vResults )
		{
			if ( tt.eType == eResult_YouJiangQueShun || tt.eType == eResult_ZhiQueJiang )
			{
				for ( auto & wanted : tt.vQueCardValue )
				{
					 stWant.nNumber = wanted ;
					 vWanted.push_back(stWant) ;
				}
			}
		}
	}
	else if ( vecResult.size() == 2 )
	{
		stCardTypeCheckResult& tR = vecResult.front() ;
		stCardTypeCheckResult& tR2 = vecResult[1] ;
		
		for ( auto& tt : tR.vResults )
		{
			if ( tt.eType == eResult_ZhiYouJiang  )
			{
				for ( auto tt2 : tR2.vResults )
				{
					if ( tt2.eType == eResult_ZhiYouJiang )  // when liang zu dou shi , si lian dui . wei yi de jiang , hui you duo ge ;
					{
						for ( auto & wanted : tt.vQueCardValue )
						{
							stWant.nNumber = wanted ;
							vWanted.push_back(stWant) ;
						}

						for ( auto & wanted : tt2.vQueCardValue )
						{
							stWant.nNumber = wanted ;
							vWanted.push_back(stWant) ;
						}
					}
					else if ( eResult_WuJiangQueShun == tt2.eType )
					{
						for ( auto & wanted : tt2.vQueCardValue )
						{
							stWant.nNumber = wanted ;
							vWanted.push_back(stWant) ;
						}
					}
				}
			}
			else if ( eResult_WuJiangQueShun == tt.eType )
			{
				for ( auto tt2 : tR2.vResults )
				{
					if ( tt2.eType == eResult_ZhiYouJiang )
					{
						for ( auto & wanted : tt.vQueCardValue )
						{
							stWant.nNumber = wanted ;
							vWanted.push_back(stWant) ;
						}
						break;
					}
				}
			}
		}
	}

	return nWantCount < vWanted.size() ;
}

bool CBloodFanxingPingHu::checkType(CMJPeerCard& peerCard)
{
	if ( peerCard.isContainMustQue() )
	{
		return false ;
	}

	bool bFindJiang = false ;
	for ( auto ref : peerCard.m_vSubCollectionCards )
	{
		VEC_CARD_ITEM vCardAllNum ;

		stCarItem stItem ;
		stItem.nCardValue = 0  ;
		stItem.nCount = 0 ;
		for ( auto singlCard : ref.second.m_vAnCards )
		{
			if ( stItem.nCardValue == 0 && stItem.nCount == 0 )
			{
				stItem.nCardValue = singlCard.nCardNumber ;
				stItem.nCount = 1 ;
				continue;
			}

			if ( stItem.nCardValue = singlCard.nCardNumber )
			{
				++stItem.nCount;
				continue;
			}

			vCardAllNum.push_back(stItem) ;
			stItem.nCardValue = singlCard.nCardNumber ;
			stItem.nCount = 1 ;
		}

		if ( checkChunShun(vCardAllNum) )
		{
			continue;
		}
		else if ( checkZhiYouJiang(vCardAllNum,result.vQueCardValue) )
		{
			if ( bFindJiang == false )
			{
				bFindJiang = true ;
			}
			else
			{
				return false ;
			}
		}
		else
		{
			return false ;
		}
	}

	return bFindJiang ;
}

bool CBloodFanxingPingHu::checkChunShun(VEC_CARD_ITEM& vec)
{
	uint8_t nCount = 0 ;
	for ( auto refCItem : vec )
	{
		nCount += refCItem.nCount ;
	}

	if ( nCount % 3 != 0 )
	{
		return false ;
	}

	assert(nCount > 0 && "why count < 0 ,check shun");

	std::vector<uint8_t> vQue ;
	if ( checkQueShun(vec,vQue) ) 
	{
		if ( vQue.empty() )
		{
			return true ;
		}
	}
	return false ;
}

bool CBloodFanxingPingHu::checkZhiYouJiang(VEC_CARD_ITEM& vec, std::vector<uint8_t>& vecOutQue)
{
	uint8_t nOutCnt = vecOutQue.size() ;
	uint8_t nCount = 0 ;
	for ( auto refCItem : vec )
	{
		nCount += refCItem.nCount ;
	}

	if ( nCount % 3 != 2 )
	{
		return false ;
	}

	// find may be jiang 
	std::vector<uint8_t> vMayBeJiang ;
	for ( auto refCItem : vec )
	{
		if ( refCItem.nCount >= 2 )
		{
			vMayBeJiang.push_back(refCItem.nCardValue) ;
		}
	}

	if ( vMayBeJiang.empty() )
	{
		return false ;
	}

	// just two card and be pair 
	if ( nCount == 2 )
	{
		if ( vMayBeJiang.size() == 1 )
		{
			vecOutQue.push_back(vMayBeJiang[0]) ;
			return true ;
		}
		return false ;
	}

	// check let jiang is  chun shun 
	for ( auto& nJiang : vMayBeJiang )
	{
		VEC_CARD_ITEM vCheck ;
		vCheck.assign(vec.begin(),vec.end());
		// remove Jiang From vCheck ;
		auto iter = vCheck.begin() ;
		for ( ; iter != vCheck.end() ; ++iter )
		{
			if ( (*iter).nCardValue == nJiang )
			{
				(*iter).nCount -= 2 ;

				if ( (*iter).nCount <= 0 )
				{
					vCheck.erase(iter) ;
					break;
				}
			}
		}

		// left must be chun shun 
		if ( checkChunShun(vCheck) )
		{
			vecOutQue.push_back(nJiang) ;
		}
	}

	return nOutCnt < vecOutQue.size() ;
}

bool CBloodFanxingPingHu::checkYouJiangQueShun(VEC_CARD_ITEM& vec , std::vector<uint8_t>& vecOutQue )
{
	uint8_t nOutCnt = vecOutQue.size() ;
	uint8_t nCount = 0 ;
	for ( auto refCItem : vec )
	{
		nCount += refCItem.nCount ;
	}

	if ( nCount < 4 )
	{
		return false ;
	}

	if ( ( nCount - 2 ) % 3 != 2  )
	{
		return false ;
	}

	// find may be jiang 
	std::vector<uint8_t> vMayBeJiang ;
	for ( auto refCItem : vec )
	{
		if ( refCItem.nCount >= 2 )
		{
			vMayBeJiang.push_back(refCItem.nCardValue) ;
		}
	}

	if ( vMayBeJiang.empty() )
	{
		return false ;
	}

	// check left jiang,  is  que  shun 
	for ( auto& nJiang : vMayBeJiang )
	{
		VEC_CARD_ITEM vCheck ;
		vCheck.assign(vec.begin(),vec.end());
		// remove Jiang From vCheck ;
		auto iter = vCheck.begin() ;
		for ( ; iter != vCheck.end() ; ++iter )
		{
			if ( (*iter).nCardValue == nJiang )
			{
				(*iter).nCount -= 2 ;

				if ( (*iter).nCount <= 0 )
				{
					vCheck.erase(iter) ;
					break;
				}
			}
		}

		// left must be que shun 
		std::vector<uint8_t> vecCheckQue ;
		if ( checkQueShun(vCheck,vecCheckQue) )
		{
			if (vecCheckQue.empty() == false )
			{
				vecOutQue.insert(vecOutQue.begin(),vecCheckQue.begin(),vecCheckQue.end()) ;
			}
		}
	}

	return nOutCnt < vecOutQue.size() ;
}

bool CBloodFanxingPingHu::checkZhiQueJiang(VEC_CARD_ITEM& vec  , std::vector<uint8_t>& vecOutQue )
{
	uint8_t nOutCnt = vecOutQue.size() ;
	uint8_t nCount = 0 ;
	for ( auto refCItem : vec )
	{
		nCount += refCItem.nCount ;
	}

	if ( nCount % 3 != 1 )
	{
		return false ;
	}

	std::vector<uint8_t> vMayBeQueJiang ;
	for ( auto refCItem : vec )
	{
		vMayBeQueJiang.push_back(refCItem.nCardValue) ;
	}

	if ( nCount == 1 )
	{
		vecOutQue.push_back(vMayBeQueJiang[0]) ;
		return true ;
	}
	// check let jiang is  chun shun 
	for ( auto& nQueJiang : vMayBeQueJiang )
	{
		VEC_CARD_ITEM vCheck ;
		vCheck.assign(vec.begin(),vec.end());
		// remove Jiang From vCheck ;
		auto iter = vCheck.begin() ;
		for ( ; iter != vCheck.end() ; ++iter )
		{
			if ( (*iter).nCardValue == nQueJiang )
			{
				(*iter).nCount -= 1 ;

				if ( (*iter).nCount <= 0 )
				{
					vCheck.erase(iter) ;
					break;
				}
			}
		}

		// left must be chun shun 
		if ( checkChunShun(vCheck) )
		{
			vecOutQue.push_back(nQueJiang) ;
		}
	}

	return nOutCnt < vecOutQue.size() ;
}

bool CBloodFanxingPingHu::checkWuJiangQueShun( VEC_CARD_ITEM& vec  , std::vector<uint8_t>& vecOutQue )
{
	uint8_t nCount = 0 ;
	for ( auto refCItem : vec )
	{
		nCount += refCItem.nCount ;
	}

	if ( nCount % 3 != 2  )
	{
		return false ;
	}

	std::vector<uint8_t> vecCheckQue ;
	if ( checkQueShun(vec,vecCheckQue) )
	{
		if (vecCheckQue.empty() == false )
		{
			vecOutQue.insert(vecOutQue.begin(),vecCheckQue.begin(),vecCheckQue.end()) ;
			return true ;
		}
	}

	return false ;
}

// help function 
bool CBloodFanxingPingHu::checkQueShun( VEC_CARD_ITEM& vec , std::vector<uint8_t>& vecOutQue )
{
	if ( vec.empty() )
	{
		printf("why check count = 0 checkQueShun \n") ;
		return true ;
	}
	// check que shun ;
	std::vector<uint8_t> arrayCheck ;
	for ( auto checkItem : vec )
	{
		while ( checkItem.nCount-- )
		{
			arrayCheck.push_back(checkItem.nCardValue) ;
		}
	}

	if ( arrayCheck.size() % 3 == 1 )
	{
		return false ;
	}

	if ( arrayCheck.size() % 3 == 0 )  // not que ;
	{
		std::vector<std::vector<uint8_t>> vecOutShunzi ;
		return findAllShun( arrayCheck ,vecOutShunzi ) ;
	}

	uint8_t nQueStarIdx = 0 ;
	if ( findQueShun(arrayCheck,nQueStarIdx ) == false )
	{
		return false ;
	}

	uint8_t nVal = arrayCheck[nQueStarIdx] ;
	uint8_t nVal2 = arrayCheck[nQueStarIdx+1] ;
	if ( nVal2 == nVal )
	{
		vecOutQue.push_back(nVal);
	}
	else if ( nVal + 2 == nVal2 )
	{
		vecOutQue.push_back(nVal + 1 ) ;
	}
	else
	{
		uint8_t nRealValue = CMJCard::parseCardValue(nVal);
		uint8_t nRealValue2 = CMJCard::parseCardValue(nVal2);
		if (nRealValue > 1 )
		{
			vecOutQue.push_back(nVal - 1 ) ;
		}

		if ( nRealValue2 < 9 )
		{
			vecOutQue.push_back(nVal2 + 1 ) ;
		}

		// may another que type 
		uint8_t nNextFind = nQueStarIdx + 2;
		if ( findQueShun(arrayCheck,nNextFind ) )
		{
			uint8_t nVal = arrayCheck[nNextFind] ;
			uint8_t nVal2 = arrayCheck[nNextFind+1] ;

			uint8_t nRealValue = CMJCard::parseCardValue(nVal);
			uint8_t nRealValue2 = CMJCard::parseCardValue(nVal2);
			if (nRealValue > 1 )
			{
				vecOutQue.push_back(nVal - 1 ) ;
			}

			if ( nRealValue2 < 9 )
			{
				vecOutQue.push_back(nVal2 + 1 ) ;
			}

		}
	}
	
	return true ;
}

bool CBloodFanxingPingHu::findQueShun( std::vector<uint8_t>& vec ,uint8_t& nQueIdx )
{
	uint8_t nNextQue = nQueIdx + 1 ;
	if ( nNextQue >= vec.size() ) 
	{
		return false ;
	}

	uint8_t nVal = vec[nQueIdx];
	uint8_t nVal2 = vec[nNextQue] ;
	if ( nVal2 - nVal <= 2 )
	{
		// make a vec for check 
		std::vector<uint8_t> vecCheck ;
		for ( uint8_t nIdx = 0 ; nIdx < vec.size() ; ++nIdx )
		{
			if ( nIdx == nNextQue || nQueIdx == nIdx )
			{
				continue;
			}
			vecCheck.push_back(vec[nIdx]) ;
		}

		std::vector<std::vector<uint8_t>> vecOutShunzi ;
		if ( findAllShun(vecCheck,vecOutShunzi) )
		{
			return true ;
		}
	}
	

	if ( findQueShun(vec,nNextQue) )
	{
		nQueIdx = nNextQue ;
		return true ;
	}
	return false ;
}

bool CBloodFanxingPingHu::findAllShun( std::vector<uint8_t>& vec , std::vector<std::vector<uint8_t>>& vecOutShunzi )
{
	if ( vec.size() % 3 != 0 )
	{
		return false ;
	}

	if ( vec.empty() )
	{
		printf("why check count is 0 \n") ;
		return true ;
	}
	
	// find not 3 same shun 
	std::vector<uint8_t> vecShun ;
	std::vector<uint8_t> vecCheck(vec);
	while ( true )
	{
		vecShun.clear() ;
		for ( auto ref : vecCheck )
		{
			if ( ref == 0 )
			{
				continue;
			}

			if ( vecShun.empty() )
			{
				vecShun.push_back(ref) ;
				continue;
			}

			if ( ref == vecShun.back() )
			{
				continue;
			}

			if ( ref == vecShun.back() + 1 )
			{
				vecShun.push_back(ref) ;
				if ( vecShun.size() == 3 )
				{
					break;
				}
			}
			else
			{
				vecShun.clear() ;
				vecShun.push_back(ref) ;
			}
		}

		if ( vecShun.size() == 3 ) // find a shun 
		{
			vecOutShunzi.push_back(vecShun) ;

			// remove flag ;
			uint8_t nidx = 0 ;
			for ( auto reS : vecShun )
			{
				for (  ; nidx < vecCheck.size() ; ++nidx )
				{
					if ( reS == vecCheck[nidx] )
					{
						vecCheck[nidx] = 0 ;
						break;
					}
				}
			}

			vecShun.clear();
			continue;
		}

		break;
	}

	// left must be 3 the same or empty 
	vecShun.clear() ;
	for ( auto ref : vecCheck )
	{
		if ( ref == 0 )
		{
			continue;
		}

		if ( vecShun.empty() )
		{
			vecShun.push_back(ref) ;
			continue;
		}

		if ( vecShun.back() == ref )
		{
			vecShun.push_back(ref) ;

			if ( vecShun.size() == 3 )
			{
				vecOutShunzi.push_back(vecShun) ;
				vecShun.clear();
			}
			continue;
		}
		else
		{
			printf("why not the same 3 \n") ;
			vecOutShunzi.clear();
			return false ;
		}
	}

	if ( vecShun.size() % 3 != 0 )
	{
		assert("should not come to here\n");
		return false ;
	}

	return true ;
}

// qing yi se 
bool CBloodQingYiSe::checkType(CMJPeerCard& peerCard)
{
	auto fi = peerCard.m_vSubCollectionCards.begin() ;
	return peerCard.m_vSubCollectionCards.size() == 1 && fi->first != peerCard.getMustQueType() ;
}

// qi dui 
CBloodFanxingQiDui::CBloodFanxingQiDui()
{
	auto pp = new CBloodFanxingQingQiDui ;
	addChildFanXing(pp);

	auto ppp = new CBloodFanxingLongQiDui ;
	addChildFanXing(ppp);
}

bool CBloodFanxingQiDui::checkFanXingWantedCards(CMJPeerCard& peerCard, LIST_WANTED_CARD& vWaited)
{

}

bool CBloodFanxingQiDui::checkType(CMJPeerCard& peerCard)
{

}

// logn qi dui 
CBloodFanxingLongQiDui::CBloodFanxingLongQiDui()
{
	auto pp = new CBloodFanxingQingLongQiDui ;
	addChildFanXing(pp);
}

bool CBloodFanxingLongQiDui::checkType(CMJPeerCard& peerCard)
{

}

// dui dui hu 
CBloodFanxingDuiDuiHu::CBloodFanxingDuiDuiHu()
{
	auto p = new CBloodFanxingQingDuiDuiHu ;
	addChildFanXing(p) ;

	auto pp = new CBloodFanxingJinGouDiao ;
	addChildFanXing(pp) ;

	auto ppp = new CBloodFanxingJiangJinGouDiao ;
	addChildFanXing(ppp) ;

	auto pppp = new CBloodFanxingShiBaLuoHan ;
	addChildFanXing(pppp) ;
}

bool CBloodFanxingDuiDuiHu::checkFanXingWantedCards(CMJPeerCard& peerCard, LIST_WANTED_CARD& vWaited)
{

}

bool CBloodFanxingDuiDuiHu::checkType(CMJPeerCard& peerCard)
{

}

// jin gou gou 
CBloodFanxingJinGouDiao::CBloodFanxingJinGouDiao()
{
	auto pp = new CBloodFanxingQingJinGouDiao ;
	addChildFanXing(pp) ;
}

bool CBloodFanxingJinGouDiao::checkType(CMJPeerCard& peerCard)
{

}

// shi ba luo han 
bool CBloodFanxingShiBaLuoHan::checkType(CMJPeerCard& peerCard)
{

}

// jiang jin gou diao 
bool CBloodFanxingJiangJinGouDiao::checkType(CMJPeerCard& peerCard)
{

}

// dai yao jiu 
CBloodFanxingDaiYaoJiu::CBloodFanxingDaiYaoJiu()
{
	auto pp = new CBloodFanxingQingDaiYaoJiu ;
	addChildFanXing(pp) ;
}

bool CBloodFanxingDaiYaoJiu::checkFanXingWantedCards(CMJPeerCard& peerCard, LIST_WANTED_CARD& vWaited)
{

}

bool CBloodFanxingDaiYaoJiu::checkType(CMJPeerCard& peerCard)
{

}
