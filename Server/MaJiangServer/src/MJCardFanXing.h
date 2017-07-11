#pragma once
#include "MJDefine.h"
#include "NativeTypes.h"
#include "MJPeerCard.h"
#include <vector>
class IMJCardFanXing
{
public:
	typedef std::vector<IMJCardFanXing*> LIST_FANXING ;
protected:
	virtual eFanxingType getType() = 0 ;
	virtual uint8_t getFanRate() = 0 ;
	virtual bool checkType(CMJPeerCard& peerCard) = 0 ;
public:
	virtual bool checkFanXingWantedCards(CMJPeerCard& peerCard, LIST_WANTED_CARD& vWaited){ return true ;} ;
protected:
	void addChildFanXing(IMJCardFanXing* pFan )
	{
		m_vChildFanxing.push_back(pFan) ;
	}
public:
	bool checkFinalType(CMJPeerCard& peerCard, eFanxingType& eFanType, uint8_t& nFanRate )
	{
		if ( checkType(peerCard) == false )
		{
			return false ;
		}

		eFanType = getType() ;
		nFanRate = getFanRate();

		eFanxingType eChildType = eFanType ; 
		uint8_t nChildRate = 0 ;
		for ( auto pp : m_vChildFanxing )
		{
			if ( pp->checkFinalType(peerCard,eChildType,nChildRate) )
			{
				if ( nChildRate > nFanRate )
				{
					nFanRate = nChildRate ;
					eFanType = eChildType ;
				}
			}
		}
		return true ;
	}
protected:
	LIST_FANXING m_vChildFanxing ;
};