#pragma once 
#include "IMJPlayerCard.h"
#include "IMJPlayerCardCheckPaixingHelper.h"
#include "MJDefine.h"
#include <algorithm>  
#include <set>
class MJPlayerCard
	:public IMJPlayerCard
{
public:
	struct stNotShunCard
	{
		VEC_CARD vCards;
	public:
		stNotShunCard();
		bool operator != (const stNotShunCard& v);
		stNotShunCard& operator = (const stNotShunCard& v);
		bool operator == (const stNotShunCard& v);
		uint8_t getLackCardCntForShun();
		uint8_t getSize()const{ return vCards.size(); }
		bool operator < (const stNotShunCard& v)const;
	};
	typedef std::set<stNotShunCard> SET_NOT_SHUN;
public:
	void reset() override;
	void addDistributeCard(uint8_t nCardNum) final;
	bool onGangCardBeRobot(uint8_t nCard) final;
	bool onCardBeGangPengEat(uint8_t nCard) final;

	bool isHaveCard(uint8_t nCard) final;  // holdCard ;
	bool canMingGangWithCard(uint8_t nCard) override;
	bool canPengWithCard(uint8_t nCard) override;
	bool canEatCard(uint8_t nCard, uint8_t& nWithA, uint8_t& withB) override;
	bool canHuWitCard(uint8_t nCard) override;
	bool canAnGangWithCard(uint8_t nCard)override;
	bool canBuGangWithCard(uint8_t nCard)override;
	bool isTingPai() override;
	bool getHoldCardThatCanAnGang(VEC_CARD& vGangCards)override;
	bool getHoldCardThatCanBuGang(VEC_CARD& vGangCards)override;
	bool isHoldCardCanHu() override;

	void onMoCard(uint8_t nMoCard) final;
	bool onPeng(uint8_t nCard) final;
	bool onMingGang(uint8_t nCard, uint8_t nGangGetCard) override;
	bool onAnGang(uint8_t nCard, uint8_t nGangGetCard) override;
	bool onBuGang(uint8_t nCard, uint8_t nGangGetCard) final;
	bool onEat(uint8_t nCard, uint8_t nWithA, uint8_t withB) final;
	bool onChuCard(uint8_t nChuCard)override;

	bool getHoldCard(VEC_CARD& vHoldCard) final;
	bool getChuedCard(VEC_CARD& vChuedCard) final;
	bool getAnGangedCard(VEC_CARD& vAnGanged)final;
	bool getMingGangedCard(VEC_CARD& vGangCard) final;
	bool getPengedCard(VEC_CARD& vPengedCard) final;
	bool getEatedCard(VEC_CARD& vEatedCard) final;

	uint32_t getNewestFetchedCard()final;
	virtual bool canHoldCard7PairHu();
	virtual bool getCanHuCards(std::set<uint8_t>& vCanHuCards);
protected:
	void addCardToVecAsc(VEC_CARD& vec, uint8_t nCard );
	bool getNotShuns(VEC_CARD vCard, SET_NOT_SHUN& vNotShun, bool bMustKeZiShun );
	bool pickKeZiOut(VEC_CARD vCard, VEC_CARD& vKeZi , VEC_CARD& vLeftCard );
	bool pickNotShunZiOutIgnoreKeZi(VEC_CARD vCardIgnorKeZi, SET_NOT_SHUN& vNotShun);
	virtual bool is7PairTing();
	virtual uint8_t getMiniQueCnt( VEC_CARD vCards[eCT_Max] );
	virtual uint8_t get7PairQueCnt(VEC_CARD vCards[eCT_Max]);
	uint8_t getLestQue(SET_NOT_SHUN& vNotShun, bool bFindJiang, bool bFindDanDiao, uint8_t& nFiandJiang, uint8_t& nFindDanDiao);
	uint8_t tryBestFindLeastNotShun(VEC_CARD& vCard, SET_NOT_SHUN& vNotShun, bool bMustKeZi );
	//uint8_t tryBestFindLeastNotShunMustKeZi(VEC_CARD& vCard, SET_NOT_SHUN& vNotShun );
public:
	void debugCardInfo();
protected:
	VEC_CARD m_vCards[eCT_Max];
	VEC_CARD m_vChuedCard;
	VEC_CARD m_vPenged;
	VEC_CARD m_vGanged;
	VEC_CARD m_vAnGanged;
	VEC_CARD m_vEated;
	uint8_t m_nNesetFetchedCard;
	uint8_t m_nJIang;
	uint8_t m_nDanDiao;
};