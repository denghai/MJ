#pragma once
#include "IPeerCard.h"
#include "CardPoker.h"
#include "CommonDefine.h"
class CNiuNiuPeerCard
	:public IPeerCard
{
public:
	struct CardGroup   
	{
		uint8_t nThreeIdx[3] ;
		uint8_t nTwoIdx[2];
		CardGroup(uint8_t tw0,uint8_t tw1, uint8_t th0 , uint8_t th1 , uint8_t th2 )
		{
			nTwoIdx[0] = tw0 ;
			nTwoIdx[1] = tw1 ;

			nThreeIdx[0] = th0 ;
			nThreeIdx[1] = th1 ;
			nThreeIdx[2] = th2 ;
		}
	};

	enum  NiuNiuType
	{
		Niu_None,
		Niu_Single,
		Niu_Niu,
        Niu_Boom,
        Niu_FiveFlower,
		Niu_FiveSmall,
		Niu_Max,
	};
public:
    CNiuNiuPeerCard();
	void addCompositCardNum( uint8_t nCardCompositNum ) override ;
	const char*  getNameString() override;
	uint32_t getWeight() override;
	void reset() override ;
	NiuNiuType getType();
	uint8_t getPoint();
#ifndef SERVER
    CNiuNiuPeerCard::CardGroup getCardGroup();
#endif
    uint8_t getAddIdx(){return m_nAddIdx;}
	CNiuNiuPeerCard& operator = (CNiuNiuPeerCard& pRight )
	{
		for( uint8_t nIdx = 0 ; nIdx < NIUNIU_HOLD_CARD_COUNT; ++nIdx )
		{
			uint8_t nRight = pRight.m_vHoldCards[nIdx].GetCardCompositeNum() ;
			m_vHoldCards[nIdx].RsetCardByCompositeNum(nRight);
		}

		m_nAddIdx = pRight.m_nAddIdx ;
		m_nBiggestCardIdx = pRight.m_nBiggestCardIdx ;
		m_eType = pRight.m_eType ;
		m_nPoint = pRight.m_nPoint ;
		m_nGroupIdx = pRight.m_nGroupIdx ;
		m_nWeight = pRight.m_nWeight ;
		return *this ;
	}

	uint8_t getCardByIdx(uint8_t nidx ) override
	{
		if ( nidx < NIUNIU_HOLD_CARD_COUNT )
		{
			return m_vHoldCards[nidx].GetCardCompositeNum() ;
		}
		return 0 ;
	}

	IPeerCard* swap(IPeerCard* pTarget)override
	{
		auto pT = dynamic_cast<CNiuNiuPeerCard*>(pTarget);
		assert(pT && "why target niuniu card is null?");
		CNiuNiuPeerCard tTemp ;
		tTemp = *this ;
		*this = *pT ;
		*pT = tTemp ;
		return this ;
	}
protected:
	bool isCaculated();
	void caculateCards();
	bool checkNiu(CardGroup& ref );
	bool checkFiveSmall();
	bool checkFiveFlower();
	bool checkBoom();
protected:
	static CardGroup s_CardGroup[10] ;
protected:
	CCard m_vHoldCards[NIUNIU_HOLD_CARD_COUNT] ;	
	uint8_t m_nAddIdx ;
	uint8_t m_nBiggestCardIdx ;

	NiuNiuType m_eType ;
	uint8_t m_nPoint ;
	uint8_t m_nGroupIdx ;
	uint32_t m_nWeight ;
};