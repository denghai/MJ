#pragma once
#include "ServerCommon.h"
#include "IRoom.h"
class IPeerCard ;
class ISitableRoomPlayer
{
public:
	ISitableRoomPlayer(){ nNoneActTimes = 0 ;}
	uint8_t getIdx(){return m_nIdx ;}
	void setIdx( uint8_t nIdx){ m_nIdx = nIdx ;}
	virtual void doSitdown(uint8_t nIdx ){ m_nIdx = nIdx ;}
	virtual void willStandUp(){}
	virtual void onGameEnd() ;
	virtual void onGameBegin(){ ++nPlayTimes ;}
	bool isHaveState( uint32_t nStateFlag )
	{
		return ( m_nState & nStateFlag ) == nStateFlag ;
	}

	void removeState( uint32_t nStateFlag );

	void addState( uint32_t nStateFlag ) ;

	void setState( uint32_t nStateFlag );

	uint32_t getState(){ return m_nState ; }
	uint32_t getCoin(){ return nCoin ;}
	void setCoin( int32_t nNewCoin ){ nCoin = nNewCoin ;}
	uint32_t getUserUID(){ return nUserUID ; }
	uint32_t getSessionID(){ return nSessionID ; }
	virtual void reset(IRoom::stStandPlayer* pPlayer);
	bool isHaveHalo();
	void switchPeerCard(ISitableRoomPlayer* pPlayer );
	bool isDelayStandUp(){ return m_isDelayStandUp ;}
	void delayStandUp(){ m_isDelayStandUp = true ; }
	uint32_t getWinTimes(){ return nWinTimes ; }
	uint32_t getPlayTimes(){ return nPlayTimes ;}
	uint32_t getSingleWinMost(){ return nSingleWinMost ;}
	void setTempHaloWeight( uint16_t nTempHalo ){ nTempHaloWeight = nTempHalo ; }
	uint8_t getHaloWeight(){ return nNewPlayerHaloWeight; }
	virtual int32_t getGameOffset() = 0 ;
	virtual IPeerCard* getPeerCard() = 0 ;
	int32_t getTotalGameOffset(){ return nTotalGameOffset ;}
	void increaseNoneActTimes(){ ++nNoneActTimes ;}
	void resetNoneActTimes(){ nNoneActTimes = 0 ;}
	uint32_t getNoneActTimes(){ return nNoneActTimes ; }
	void reactive( uint32_t nNewSessionID ) { nSessionID = nNewSessionID ; m_isDelayStandUp = false ; resetNoneActTimes(); }
private:
	uint16_t getTotalHaloWeight(){ return nNewPlayerHaloWeight + nTempHaloWeight; }
	void increaseWinTimes(){ ++nWinTimes ;}
private:
	bool m_isDelayStandUp;
	uint8_t m_nIdx ;
	uint32_t m_nState ;
	uint8_t m_nHaloState ; // 0 not invoke , 1 in effect , 2 not in effect ;

	uint32_t nCoin ;
	uint32_t nUserUID ;
	uint32_t nSessionID ;
	uint8_t nNewPlayerHaloWeight;
	uint16_t nTempHaloWeight ;
	uint32_t nPlayTimes ;
	uint32_t nWinTimes ;
	uint32_t nSingleWinMost ;
	int32_t nTotalGameOffset ;
	uint32_t nNoneActTimes ; 
};