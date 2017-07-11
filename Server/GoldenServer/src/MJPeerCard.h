#pragma once
#include "MJDefine.h"
#include "NativeTypes.h"
#include <list>
#include <map>
enum eSinglePeerCardState
{
	eSinglePeerCard_An,
	eSinglePeerCard_Peng,
	eSinglePeerCard_MingGang,
	eSinglePeerCard_AnGang,
	eSinglePeerCard_BuGang,
	eSinglePeerCard_Chi,
	eSinglePeerCard_Max,
};

struct stSinglePeerCard
{
	uint8_t nCardNumber ;
	eSinglePeerCardState eState ;
	stSinglePeerCard(){ eState = eSinglePeerCard_An; nCardNumber = 0 ;}
};

struct stWantedCard
{
	uint8_t nNumber ;
	ePosType eWanteddCardFrom;
	eMJActType eCanInvokeAct ;
	uint8_t nFanRate ; // used when act type == hu ;
	eFanxingType eFanxing ; //  used when act type == hu ;
	stWantedCard(){ nFanRate = 0 ; eFanxing = eFanxing_PingHu ; }
};

typedef std::list<stWantedCard> LIST_WANTED_CARD;


class CPeerCardSubCollect
{
public:
	typedef std::list<stSinglePeerCard> LIST_PEER_CARDS;
public:
	bool removeCardNumber( uint8_t nNumber );
	void doAction(eMJActType eType, uint8_t nNumber );
	void getWantedCardList(LIST_WANTED_CARD& vList,bool bOmitChi );
	void clear();
	uint8_t getCardCount();
public:
	LIST_PEER_CARDS m_vAnCards ;
	LIST_PEER_CARDS m_vMingCards ;
};

class CMJPeerCard
{
public:
	typedef std::map<eMJCardType,CPeerCardSubCollect> MAP_CT_COLLECT;
public:
	void setMustQueType(eMJCardType eQueType ){ m_eMustQueType = eQueType ;}
	bool removeCardNumber( uint8_t nNumber );
	void doAction(eMJActType eType, uint8_t nNumber );
	virtual void reset();
	eMJCardType getMustQueType(){ return m_eMustQueType ;}
	void updateWantedCard( LIST_WANTED_CARD& vWantList );
	bool isContainMustQue();
protected:
	friend class CBloodQingYiSe ;
	friend class CBloodFanxingPingHu ;
protected:
	MAP_CT_COLLECT m_vSubCollectionCards ;
	eMJCardType m_eMustQueType ;
};