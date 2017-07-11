#pragma once 
#include "NativeTypes.h"
#include <vector>
class IMJPlayerCard
{
public:
	typedef std::vector<uint8_t> VEC_CARD;
public:
	virtual ~IMJPlayerCard(){}
	virtual void reset() = 0;
	virtual void addDistributeCard( uint8_t nCardNum ) = 0 ;
	virtual bool onGangCardBeRobot(uint8_t nCard ) = 0;
	virtual bool onCardBeGangPengEat( uint8_t nCard ) = 0 ;

	virtual bool isHaveCard(uint8_t nCard) = 0 ;
	virtual bool canMingGangWithCard(uint8_t nCard) = 0 ;
	virtual bool canAnGangWithCard(uint8_t nCard) = 0;
	virtual bool canBuGangWithCard(uint8_t nCard) = 0;
	virtual bool canPengWithCard(uint8_t nCard) = 0;
	virtual bool canEatCard(uint8_t nCard, uint8_t& nWithA, uint8_t& withB ) = 0 ;
	virtual bool canHuWitCard( uint8_t nCard ) = 0;
	virtual bool isTingPai() = 0 ;
	virtual bool getHoldCardThatCanAnGang( VEC_CARD& vGangCards ) = 0 ;
	virtual bool getHoldCardThatCanBuGang(VEC_CARD& vGangCards) = 0;
	virtual bool isHoldCardCanHu() = 0 ;

	virtual void onMoCard(uint8_t nMoCard) = 0;
	virtual bool onPeng( uint8_t nCard ) = 0 ;
	virtual bool onMingGang(uint8_t nCard, uint8_t nGangGetCard ) = 0;
	virtual bool onAnGang(uint8_t nCard, uint8_t nGangGetCard ) = 0;
	virtual bool onBuGang(uint8_t nCard, uint8_t nGangGetCard ) = 0;
	virtual bool onEat(uint8_t nCard, uint8_t nWithA, uint8_t withB ) = 0;
	virtual bool onChuCard(uint8_t nChuCard) = 0;

	virtual bool getHoldCard(VEC_CARD& vHoldCard ) = 0 ;
	virtual bool getChuedCard(VEC_CARD& vChuedCard ) = 0 ;
	virtual bool getMingGangedCard(VEC_CARD& vGangCard) = 0 ;
	virtual bool getPengedCard(VEC_CARD& vPengedCard ) = 0 ;
	virtual bool getEatedCard(VEC_CARD& vEatedCard ) = 0 ;
	virtual bool getAnGangedCard(VEC_CARD& vAnGanged) = 0;
	virtual uint32_t getNewestFetchedCard() = 0 ;
};