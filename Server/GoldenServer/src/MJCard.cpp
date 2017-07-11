#include "MJCard.h"
#include <cassert>
uint8_t CMJCard::getCard()
{
	if ( isEmpty() )
	{
		return 0 ;
	}

	return m_vAllCards[m_nCurCardIdx++] ;
}

uint8_t CMJCard::getLeftCardCount()
{
	if ( m_vAllCards.size() <= m_nCurCardIdx )
	{
		return 0 ;
	}

	return m_vAllCards.size() - m_nCurCardIdx ;
}

void CMJCard::shuffle()
{
	uint16_t n = 0 ;
	for ( uint16_t i = 0 ; i < m_vAllCards.size() - 2 ; ++i )
	{
		n = rand() % ( m_vAllCards.size() - i - 1 ) + i + 1    ;
		m_vAllCards[i] = m_vAllCards[n] + m_vAllCards[i] ;
		m_vAllCards[n] = m_vAllCards[i] - m_vAllCards[n] ;
		m_vAllCards[i] = m_vAllCards[i] - m_vAllCards[n] ;
	}
	m_nCurCardIdx = 0 ;
}

void CMJCard::debugCardInfo()
{
	printf("card Info: \n");
	for ( uint8_t nCard : m_vAllCards )
	{
		printf("cardNumber : %u\n",nCard) ;
	}
	printf("card info end \n\n") ;
}

void CMJCard::initAllCard( eMJGameType eType )
{
	m_vAllCards.clear() ;
	m_nCurCardIdx = 0 ;

	assert(eType < eMJ_Max && eType >= eMJ_None && "invalid card type" );
	m_eMJGameType = eType ;
	// add base 
	uint8_t vType[3] = { eCT_Wan,eCT_Tiao,eCT_Tong } ;
	for ( uint8_t nType : vType )
	{
		for ( uint8_t nValue = 1 ; nValue <= 9 ; ++nValue )
		{
			m_vAllCards.push_back(makeCardNumber((eMJCardType)nType,nValue)) ;
		}
	}

	if ( eMJ_COMMON == m_eMJGameType )
	{
		// add feng , add ke
		for ( uint8_t nValue = 1 ; nValue <= 4 ; ++nValue )
		{
			m_vAllCards.push_back(makeCardNumber(eCT_Feng,nValue)) ;
		}

		for ( uint8_t nValue = 1 ; nValue <= 3 ; ++nValue )
		{
			m_vAllCards.push_back(makeCardNumber(eCT_ZFB,nValue)) ;
		}
	}
}

eMJGameType CMJCard::getGameType()
{
	return m_eMJGameType ;
}

bool CMJCard::isEmpty()
{
	return getLeftCardCount() <= 0 ;
}

eMJCardType CMJCard::parseCardType(uint8_t nCardNum)
{
	uint8_t nType = nCardNum & 0xF0 ;
	nType = nType >> 4 ;
	assert(nType < eCT_Max && nType >= eCT_None && "invalid card type" );
	return (eMJCardType)nType ;
}

uint8_t CMJCard::parseCardValue(uint8_t nCardNum )
{
	return  (nCardNum & 0xF) ;
}

uint8_t CMJCard::makeCardNumber(eMJCardType eType,uint8_t nValue )
{
	assert(eType < eCT_Max && eType >= eCT_None && "invalid card type" );
	assert(nValue <= 9 && nValue >= 1 && "invalid card value" );
	uint8_t nType = eType ;
	nType = nType << 4 ;
	uint8_t nNum = nType | nValue ;
	return nNum ;
}

void CMJCard::parseCardTypeValue(uint8_t nCardNum, eMJCardType& eType,uint8_t& nValue )
{
	eType = parseCardType(nCardNum) ;
	nValue = parseCardValue(nCardNum) ;
	assert(eType < eCT_Max && eType >= eCT_None && "invalid card type" );
	assert(nValue <= 9 && nValue >= 1 && "invalid card value" );
}