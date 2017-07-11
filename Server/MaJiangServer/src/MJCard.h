#pragma once
#include "MJDefine.h"
#include "NativeTypes.h"
#include <vector>
#include "IMJPoker.h"
#include "CommonDefine.h"
#include "log4z.h"
class CMJCard
	:public IMJPoker
{
public:
	typedef std::vector<uint8_t> VEC_UINT8 ;
public:
	CMJCard(){ m_vAllCards.clear() ; m_nCurCardIdx = 0 ;}
	uint8_t getCard();
	void pushCardToFron(uint8_t nCard);
	void pushCardToBack(uint8_t nCard);
	uint8_t getLeftCardCount()override;
	uint8_t distributeOneCard()override {
		auto nCard = getCard();
		LOGFMTD("++++++++++++++++++++++++++++++++++++ distributeOneCard = %u", nCard);
		return nCard;
	}
	void shuffle()override;
	void debugCardInfo();
	void initAllCard( eMJGameType eType );
	eMJGameType getGameType();
	bool isEmpty();
	void debugPokerInfo();
	static eMJCardType parseCardType(uint8_t nCardNum);
	static uint8_t parseCardValue(uint8_t nNum );
	static uint8_t makeCardNumber(eMJCardType eType,uint8_t nValue );
	static void parseCardTypeValue(uint8_t nCardNum, eMJCardType& eType,uint8_t& nValue );
	static void debugSinglCard(uint8_t nCard );
protected:
	void initTwoBirdCard();
protected:
	eMJGameType m_eMJGameType;
	VEC_UINT8 m_vAllCards ;
	uint8_t m_nCurCardIdx ;
};

#ifndef CARD_TYPE_HELP
#define CARD_TYPE_HELP
#define card_Type(cardNum) CMJCard::parseCardType((cardNum))
#define card_Value(cardNum) CMJCard::parseCardValue((cardNum))
#define make_Card_Num(type,value) CMJCard::makeCardNumber((type),(value))
#endif 