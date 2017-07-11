#pragma once
#include "MJDefine.h"
#include "NativeTypes.h"
#include <vector>
class CMJCard
{
public:
	typedef std::vector<uint8_t> VEC_UINT8 ;
public:
	CMJCard(){ m_vAllCards.clear() ; m_nCurCardIdx = 0 ;}
	uint8_t getCard();
	uint8_t getLeftCardCount();
	void shuffle();
	void debugCardInfo();
	void initAllCard( eMJGameType eType );
	eMJGameType getGameType();
	bool isEmpty();
	static eMJCardType parseCardType(uint8_t nCardNum);
	static uint8_t parseCardValue(uint8_t nNum );
	static uint8_t makeCardNumber(eMJCardType eType,uint8_t nValue );
	static void parseCardTypeValue(uint8_t nCardNum, eMJCardType& eType,uint8_t& nValue );
protected:
	eMJGameType m_eMJGameType;
	VEC_UINT8 m_vAllCards ;
	uint8_t m_nCurCardIdx ;
};