#include "InsuranceCheck.h"
#include "log4z.h"
#include <cassert>
#include <algorithm>
CInsuranceCheck::CInsuranceCheck()
{
	m_tCheckPoker.InitTaxasPoker();
	reset() ;
}

void CInsuranceCheck::reset()
{
	m_nBuyerIdx = -1 ;
	m_vAllPeers.clear() ;
	m_isDataDirty = false ;
	m_vExincludeCardNums.clear();
}

void CInsuranceCheck::addCheckPeer(uint16_t nIdx , uint16_t nPeerCard0, uint16_t nPeerCard1 )
{
	auto iter = m_vAllPeers.find(nIdx) ;
	if ( iter != m_vAllPeers.end() )
	{
		LOGFMTE("nidx = %u alredy add to insurance check",nIdx) ;
		return ;
	}

	stCheckPeer t ;
	t.m_pPeerCard->AddCardByCompsiteNum(nPeerCard0);
	t.m_pPeerCard->AddCardByCompsiteNum(nPeerCard1) ;
	t.nIdx = nIdx ;
	t.vOuts.clear() ;
	m_vAllPeers[t.nIdx] = t ;
	m_isDataDirty = true ;

	m_vExincludeCardNums.push_back(nPeerCard1);
	m_vExincludeCardNums.push_back(nPeerCard0);
}

void CInsuranceCheck::addPublicCard(uint32_t nPublicCard )
{
	for ( auto& ref : m_vAllPeers )
	{
		//auto p = ref.second ;
		ref.second.m_pPeerCard->AddCardByCompsiteNum(nPublicCard);
	}
	m_isDataDirty = true ;
	m_vExincludeCardNums.push_back(nPublicCard);
}

void CInsuranceCheck::addExincludeCard(uint32_t nCards )
{
	m_vExincludeCardNums.push_back(nCards);
}

uint8_t CInsuranceCheck::getOuts( uint16_t& nInsuranceBuyerIdx, std::vector<uint8_t>& allOuts )
{
	caculateResult() ;
	nInsuranceBuyerIdx = m_nBuyerIdx ;
	if ( (uint16_t)-1 == m_nBuyerIdx )
	{
		return 0 ;
	}

	for ( auto ref : m_vAllPeers )
	{
		if ( ref.first == m_nBuyerIdx )
		{
			continue;
		}
		allOuts.insert(allOuts.end(),ref.second.vOuts.begin(),ref.second.vOuts.end() ) ;
	}

	return allOuts.size() ;
}

uint8_t CInsuranceCheck::getOutsForPlayer( uint16_t nIdx ,std::vector<uint8_t>* allOuts )
{
	caculateResult();
	auto iter = m_vAllPeers.find(nIdx) ;
	if ( iter == m_vAllPeers.end() )
	{
		LOGFMTE("idx = %u not in check outs player vec", nIdx ) ;
		return 0 ;
	}

	if ( allOuts != nullptr )
	{
		allOuts->assign(iter->second.vOuts.begin(),iter->second.vOuts.end() ) ;
	}

	return iter->second.vOuts.size();
}

bool compareCheckPeerCard( CInsuranceCheck::stCheckPeer* left , CInsuranceCheck::stCheckPeer* right )
{
	return left->m_pPeerCard->PK(right->m_pPeerCard.get()) == 1 ;
}

void CInsuranceCheck::caculateResult()
{
	if ( m_isDataDirty == false )
	{
		return ; 
	}

	m_isDataDirty = false ;

	if ( m_vAllPeers.size() < 2 )
	{
		LOGFMTE("check peer is too few ") ;
		return  ;
	}
	// check all player card state ;
#ifdef _DEBUG
	for ( auto ref : m_vAllPeers )
	{
		auto refValue = ref.second ;
		if ( refValue.m_pPeerCard->getAllCardCnt() < 5 )
		{
			LOGFMTE("why this player do not have 5 cards") ;
			assert(0 && "why this player do not have 5 cards");
		}
	}
#endif
	// put player into vec 
	std::vector<stCheckPeer*> vPlayers ;
	for ( auto& pp : m_vAllPeers )
	{
		vPlayers.push_back(&pp.second);
	}

	// decide buyer idx ;
	std::sort(vPlayers.begin(),vPlayers.end(),compareCheckPeerCard);
	if ( vPlayers[0]->m_pPeerCard->PK(vPlayers[1]->m_pPeerCard.get()) == 0  ) // have more than 0ne potential winner , can not buy insurance 
	{
		return ;
	}

	m_nBuyerIdx = vPlayers[0]->nIdx ;
	// caculate outs ;
	for ( uint8_t nIdx = 0 ; nIdx < m_tCheckPoker.GetAllCard(); ++nIdx )
	{
		auto nCardNum = (uint8_t)m_tCheckPoker.getCardNum(nIdx) ;
		auto iter = std::find(m_vExincludeCardNums.begin(),m_vExincludeCardNums.end(),nCardNum) ;
		if ( iter != m_vExincludeCardNums.end() )
		{
			continue;
		}

		// add this card to all player 
		for ( auto pPlayer : vPlayers )
		{
			pPlayer->m_pPeerCard->AddCardByCompsiteNum(nCardNum);
		}

		// sort ;
		std::sort(vPlayers.begin(),vPlayers.end(),compareCheckPeerCard);

		// find win buyer idx ;
		if ( vPlayers[0]->nIdx != m_nBuyerIdx )  // find outs ;  
		{
			vPlayers[0]->vOuts.push_back(nCardNum) ;
		}
		else if ( vPlayers[0]->m_pPeerCard->PK(vPlayers[1]->m_pPeerCard.get()) == 0 )  // if the first is buyer idx . but the same as second , this regard as outs 
		{
			vPlayers[1]->vOuts.push_back(nCardNum) ;
		}

		// remove this card from all players ;
		for ( auto pPlayer : vPlayers )
		{
			pPlayer->m_pPeerCard->removePublicCompsiteNum(nCardNum);
		}
	}
}

uint32_t CInsuranceCheck::getInsuredProfit( uint32_t nInsuredAmount )
{
	uint16_t nidx = 0 ;
	std::vector<uint8_t> vOuts ;
	auto nOutsCnt = getOuts(nidx,vOuts) ;
	static float s_fInsuranceRate[] = {30,16,10,8,6,5,4,3.5,3,2.5,2.2,2,1.8,1.6,1.4,1.2,1} ;
	float fRate = 0 ;
	if ( nOutsCnt <= 17 )
	{
		fRate = s_fInsuranceRate[nOutsCnt] ;
	}

	return (float)nInsuredAmount * fRate ;
}

uint32_t CInsuranceCheck::getAmountNeedForPofit( uint32_t nProfit )
{
	if ( nProfit == 0 )
	{
		return 0 ;
	}

	uint16_t nidx = 0 ;
	std::vector<uint8_t> vOuts ;
	auto nOutsCnt = getOuts(nidx,vOuts) ;
	static float s_fInsuranceRate[] = {30,16,10,8,6,5,4,3.5,3,2.5,2.2,2,1.8,1.6,1.4,1.2,1} ;
	float fRate = 0 ;
	if ( nOutsCnt <= 17 )
	{
		fRate = s_fInsuranceRate[nOutsCnt] ;
	}
	return (float(nProfit) / fRate) + 1 ;
}