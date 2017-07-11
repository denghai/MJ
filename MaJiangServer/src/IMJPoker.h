#pragma once
#include "NativeTypes.h"
class IMJPoker
{
public:
	virtual ~IMJPoker(){}

	virtual void shuffle() = 0;
	virtual void pushCardToFron(uint8_t nCard) = 0;
	virtual uint8_t getLeftCardCount() = 0;
	virtual uint8_t distributeOneCard() = 0;
	virtual void pushCardToBack(uint8_t nCard) = 0;
};