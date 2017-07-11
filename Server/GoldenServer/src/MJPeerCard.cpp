#include "MJPeerCard.h"
#include <cassert>
#include "MJCard.h"
bool CPeerCardSubCollect::removeCardNumber( uint8_t nNumber )
{
	auto iter = m_vAnCards.begin() ;
	for ( ; iter != m_vAnCards.end(); ++iter )
	{
		if ( (*iter).nCardNumber == nNumber )
		{
			m_vAnCards.erase(iter) ;
			return true ;
		}
	}

	assert(0&&"can not find remove number");
	return false ;
}

uint8_t CPeerCardSubCollect::getCardCount()
{
	return m_vAnCards.size() + m_vMingCards.size() ;
}

void CPeerCardSubCollect::doAction(eMJActType eType, uint8_t nNumber )
{
	switch ( eType )
	{
	case eMJAct_Mo:
		{
			stSinglePeerCard peer ;
			peer.nCardNumber = nNumber ;

			auto iter = m_vAnCards.begin() ;
			for ( ; iter != m_vAnCards.end(); ++iter )
			{
				if ( (*iter).nCardNumber >= nNumber )
				{
					m_vAnCards.insert(iter,peer) ;
					return;
				}
			}
			m_vAnCards.push_back(peer) ;
		}
		break;
	case eMJAct_Peng:
		{
			stSinglePeerCard peer ;
			peer.nCardNumber = nNumber ;
			peer.eState = eSinglePeerCard_Peng;

			uint8_t nEraseCount= 2 ;
			auto iter = m_vAnCards.begin() ;
			for ( ; iter != m_vAnCards.end() && nEraseCount > 0; ++iter )
			{
				if ( (*iter).nCardNumber == nNumber )
				{
					m_vAnCards.erase(iter) ;
					--nEraseCount ;
					iter = m_vAnCards.begin() ;
				}
			}
			assert(nEraseCount == 0 && "why can not peng" );

			// add to ming pai
			auto iterMing = m_vMingCards.begin() ;
			for ( ; iterMing != m_vMingCards.end(); ++iterMing )
			{
				if ( (*iterMing).nCardNumber >= nNumber )
				{
					iterMing = m_vMingCards.insert(iterMing,peer) ;
					iterMing = m_vMingCards.insert(iterMing,peer) ;
					iterMing = m_vMingCards.insert(iterMing,peer) ;
					return;
				}
			}
			m_vMingCards.push_back(peer) ;
			m_vMingCards.push_back(peer) ;
			m_vMingCards.push_back(peer) ;
		}
		break;
	case eMJAct_AnGang:
		{
			stSinglePeerCard peer ;
			peer.nCardNumber = nNumber ;
			peer.eState = eSinglePeerCard_AnGang;

			uint8_t nEraseCount= 4 ;
			auto iter = m_vAnCards.begin() ;
			for ( ; iter != m_vAnCards.end() && nEraseCount > 0; ++iter )
			{
				if ( (*iter).nCardNumber == nNumber )
				{
					m_vAnCards.erase(iter) ;
					--nEraseCount ;
					iter = m_vAnCards.begin() ;
				}
			}
			assert(nEraseCount == 0 && "why can not peng" );

			// add to ming pai
			auto iterMing = m_vMingCards.begin() ;
			for ( ; iterMing != m_vMingCards.end(); ++iterMing )
			{
				if ( (*iterMing).nCardNumber >= nNumber )
				{
					iterMing = m_vMingCards.insert(iterMing,peer) ;
					iterMing = m_vMingCards.insert(iterMing,peer) ;
					iterMing = m_vMingCards.insert(iterMing,peer) ;
					iterMing = m_vMingCards.insert(iterMing,peer) ;
					return;
				}
			}
			m_vMingCards.push_back(peer) ;
			m_vMingCards.push_back(peer) ;
			m_vMingCards.push_back(peer) ;
			m_vMingCards.push_back(peer) ;
		}
		break;
	case eMJAct_MingGang:
		{
			stSinglePeerCard peer ;
			peer.nCardNumber = nNumber ;
			peer.eState = eSinglePeerCard_MingGang;

			uint8_t nEraseCount= 3 ;
			auto iter = m_vAnCards.begin() ;
			for ( ; iter != m_vAnCards.end() && nEraseCount > 0; ++iter )
			{
				if ( (*iter).nCardNumber == nNumber )
				{
					m_vAnCards.erase(iter) ;
					--nEraseCount ;
					iter = m_vAnCards.begin() ;
				}
			}
			assert(nEraseCount == 0 && "why can not peng" );

			// add to ming pai
			auto iterMing = m_vMingCards.begin() ;
			for ( ; iterMing != m_vMingCards.end(); ++iterMing )
			{
				if ( (*iterMing).nCardNumber >= nNumber )
				{
					iterMing = m_vMingCards.insert(iterMing,peer) ;
					iterMing = m_vMingCards.insert(iterMing,peer) ;
					iterMing = m_vMingCards.insert(iterMing,peer) ;
					iterMing = m_vMingCards.insert(iterMing,peer) ;
					return;
				}
			}
			m_vMingCards.push_back(peer) ;
			m_vMingCards.push_back(peer) ;
			m_vMingCards.push_back(peer) ;
			m_vMingCards.push_back(peer) ;
		}
		break;
	case eMJAct_BuGang:
		{
			stSinglePeerCard peer ;
			peer.nCardNumber = nNumber ;
			peer.eState = eSinglePeerCard_BuGang;

			auto iter = m_vAnCards.begin() ;
			for ( ; iter != m_vAnCards.end() ; ++iter )
			{
				if ( (*iter).nCardNumber == nNumber )
				{
					m_vAnCards.erase(iter) ;
					break;
				}
			}

			// add to ming pai
			auto iterMing = m_vMingCards.begin() ;
			for ( ; iterMing != m_vMingCards.end(); ++iterMing )
			{
				if ( (*iterMing).nCardNumber >= nNumber )
				{
					iterMing = m_vMingCards.insert(iterMing,peer) ;
					return;
				}
			}
			assert( 0 && "bu gang shou not come to here");
		}
		break;
	case eMJAct_Chi:
		{
			assert(0 && "how do this" );
		}
		break;
	default:
		break;
	}
}

void CPeerCardSubCollect::getWantedCardList(LIST_WANTED_CARD& vOutList,bool bOmitChi )
{
	assert(bOmitChi == true && "not id don't want the chi" );
	// check an pai  ;
	auto iter = m_vAnCards.begin() ;
	LIST_PEER_CARDS vListTemp ;
	uint8_t nCurListCardNumber = 0 ;
	for ( ; iter != m_vAnCards.end() ; ++iter )
	{
		if ( vListTemp.empty() )
		{
			vListTemp.push_back(*iter) ;
			nCurListCardNumber = (*iter).nCardNumber ;
			continue;
		}

		if ( nCurListCardNumber == (*iter).nCardNumber )
		{
			vListTemp.push_back(*iter) ;
			continue;
		}
		// another number card ;

		// check preType 
		if ( vListTemp.size() == 4 )
		{
			stWantedCard wtc ;
			wtc.eCanInvokeAct = eMJAct_AnGang ;
			wtc.eWanteddCardFrom = ePos_Self ;
			wtc.nNumber = nCurListCardNumber ;
			vOutList.push_back(wtc) ;
		}
		else if ( vListTemp.size() == 3 )
		{
			stWantedCard wtc ;
			wtc.eCanInvokeAct = eMJAct_MingGang ;
			wtc.eWanteddCardFrom = ePos_Other ;
			wtc.nNumber = nCurListCardNumber ;
			vOutList.push_back(wtc) ;

			// peng
			wtc.eCanInvokeAct = eMJAct_Peng ;
			wtc.eWanteddCardFrom = ePos_Other ;
			wtc.nNumber = nCurListCardNumber ;
			vOutList.push_back(wtc) ;
		}
		else if ( vListTemp.size() == 2 )
		{
			stWantedCard wtc ;
			wtc.eCanInvokeAct = eMJAct_Peng ;
			wtc.eWanteddCardFrom = ePos_Other ;
			wtc.nNumber = nCurListCardNumber ;
			vOutList.push_back(wtc) ;
		}
		else
		{
			// clear pre and add new ;
			nCurListCardNumber = 0 ;
			vListTemp.clear() ;
			vListTemp.push_back(*iter) ;
		}
	}

	// check bu gang 
	auto iterBu = m_vMingCards.begin() ;
	uint8_t lastCheck = 0 ;
	for ( ; iterBu != m_vMingCards.begin(); ++iterBu )
	{
		if ( (*iterBu).eState == eSinglePeerCard_Peng && lastCheck != (*iterBu).nCardNumber )
		{
			lastCheck = (*iterBu).nCardNumber ;
			stWantedCard wtc ;
			wtc.eCanInvokeAct = eMJAct_BuGang;
			wtc.eWanteddCardFrom = ePos_Self;
			wtc.nNumber = lastCheck ;
			vOutList.push_back(wtc) ;
		}
	}
}

void CPeerCardSubCollect::clear()
{
	m_vMingCards.clear() ;
	m_vAnCards.clear() ;
}

// peer card impl
bool CMJPeerCard::removeCardNumber( uint8_t nNumber )
{
	auto eType = CMJCard::parseCardType(nNumber) ;
	auto iter = m_vSubCollectionCards.find(eType) ;
	assert(iter != m_vSubCollectionCards.end() && "no this card how to remove" );
	auto b = iter->second.removeCardNumber(nNumber) ;
	if ( b )
	{
		if ( iter->second.getCardCount() == 0 )
		{
			m_vSubCollectionCards.erase(iter) ;
		}
	}
	return b ;
}

void CMJPeerCard::doAction(eMJActType eType, uint8_t nNumber )
{
	auto eCardThisType = CMJCard::parseCardType(nNumber) ;
	auto iter = m_vSubCollectionCards.find(eCardThisType) ;
	assert(iter != m_vSubCollectionCards.end() || eType == eMJAct_Mo && "no this card how to remove" );
	if ( iter == m_vSubCollectionCards.end() && eType == eMJAct_Mo )
	{
		CPeerCardSubCollect v ;
		v.doAction(eType,nNumber) ;
		m_vSubCollectionCards[eCardThisType] = v ;
		return ;
	}
	iter->second.doAction(eType,nNumber);
}

void CMJPeerCard::updateWantedCard(LIST_WANTED_CARD& vWantList)
{
	for ( auto ref : m_vSubCollectionCards )
	{
		if ( ref.first == getMustQueType() )
		{
			continue;
		}
		ref.second.getWantedCardList(vWantList,true);
	}
}

void CMJPeerCard::reset()
{
	m_vSubCollectionCards.clear() ;
	m_eMustQueType = eCT_Max ;
}
