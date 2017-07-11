#pragma once
#include "PeerCard.h"
#include "IPeerCard.h"
class CGoldenPeerCardAdpater
	:public IPeerCard
{
public:
	void reset()override 
	{
		nAddCardIdx = 0 ;
		m_tRealPeerCard.Reset();
		memset(vCards,0,sizeof(vCards));
	}
	void addCompositCardNum( uint8_t nCardCompositNum ) override 
	{
		if ( nAddCardIdx >= GOLDEN_PEER_CARD )
		{
			assert(false && "already add enought idx" );
			return ;
		}
		vCards[nAddCardIdx++] = nCardCompositNum ;
		if ( nAddCardIdx == GOLDEN_PEER_CARD )
		{
			m_tRealPeerCard.SetPeerCardByNumber(vCards[0],vCards[1],vCards[2]) ;
		}
	}

	const char* getNameString()override 
	{
		return "golden not have name";
	}

	uint32_t getWeight() override 
	{
		assert(0 && "should not invoker this function");
		return 0 ;
	}

	IPeerCard* swap(IPeerCard* pTarget)override
	{
		auto pTar = dynamic_cast<CGoldenPeerCardAdpater*>(pTarget);
		assert(pTar && "why target is null , not the goden type ?" );
		assert(nAddCardIdx == GOLDEN_PEER_CARD && "not finish how to pk " );
		assert(pTar->nAddCardIdx == GOLDEN_PEER_CARD && "not finish how to pk " );

		uint8_t vTempCardsForTar[GOLDEN_PEER_CARD] = { 0 } ;
		uint8_t vTempCardsForSelf[GOLDEN_PEER_CARD] = { 0 } ;
		memcpy(vTempCardsForTar,vCards,sizeof(vCards));
		memcpy(vTempCardsForSelf,pTar->vCards,sizeof(vCards));
		reset();
		pTar->reset();
		for ( uint8_t nIdx = 0 ; nIdx < GOLDEN_PEER_CARD ; ++nIdx )
		{
			addCompositCardNum(vTempCardsForSelf[nIdx]) ;
			pTar->addCompositCardNum(vTempCardsForTar[nIdx]);
		}
		return this ;
	}

	PK_RESULT pk( IPeerCard* pTarget )override
	{
		auto pTar = dynamic_cast<CGoldenPeerCardAdpater*>(pTarget);
		assert(pTar && "why target is null , not the goden type ?" );
		assert(nAddCardIdx == GOLDEN_PEER_CARD && "not finish how to pk " );
		assert(pTar->nAddCardIdx == GOLDEN_PEER_CARD && "not finish how to pk " );
		int8_t nRet = m_tRealPeerCard.PKPeerCard(&pTar->m_tRealPeerCard) ;

		if ( nRet == 1 )
		{
			return PK_RESULT_WIN ;
		}

		if ( nRet == 0 )
		{
			return PK_RESULT_EQUAL ;
		}
		return PK_RESULT_FAILED ;
	}

	uint8_t getCardByIdx(uint8_t nidx )override
	{
		if ( nidx < GOLDEN_PEER_CARD )
		{
			return vCards[nidx] ;
		}
		return 0 ;
	}
protected:
	uint8_t nAddCardIdx ; 
	uint8_t vCards[GOLDEN_PEER_CARD];
	CPeerCard m_tRealPeerCard ;
};