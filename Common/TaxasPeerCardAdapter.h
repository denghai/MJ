#pragma once
#pragma once
#include "TaxasPokerPeerCard.h"
#include "IPeerCard.h"
class CTaxasPeerCardAdpater
	:public IPeerCard
{
public:
	void reset()override 
	{
		m_tRealPeerCard.Reset();
	}
	void addCompositCardNum( uint8_t nCardCompositNum ) override 
	{
		m_tRealPeerCard.AddCardByCompsiteNum(nCardCompositNum);
	}

	const char* getNameString()override 
	{
		return m_tRealPeerCard.GetTypeName();
	}

	uint32_t getWeight() override 
	{
		assert(0 && "Taxas should not invoker this function");
		return 0 ;
	}

	IPeerCard* swap(IPeerCard* pTarget)override
	{
		auto pTar = dynamic_cast<CTaxasPeerCardAdpater*>(pTarget);
		assert(pTar && "why target is null , not the Taxas type ?" );
		CTaxasPokerPeerCard tTemp ;
		tTemp = m_tRealPeerCard ;
		m_tRealPeerCard = pTar->m_tRealPeerCard ;
		pTar->m_tRealPeerCard = tTemp ;
		return this ;
	}

	uint8_t getCardByIdx(uint8_t nidx )
	{
		return m_tRealPeerCard.GetPrivateCard(nidx);
	}

	PK_RESULT pk( IPeerCard* pTarget )override
	{
		auto pTar = dynamic_cast<CTaxasPeerCardAdpater*>(pTarget);
		assert(pTar && "why target is null , not the Taxas type ?" );
		int8_t nRet = m_tRealPeerCard.PK(&pTar->m_tRealPeerCard) ;
		switch (nRet)
		{
		case 0:
			return PK_RESULT_EQUAL ;
		case 1:
			return PK_RESULT_WIN ;
		default:
			break;
		}
		return PK_RESULT_FAILED ;
	}
protected:
	CTaxasPokerPeerCard m_tRealPeerCard ;
};