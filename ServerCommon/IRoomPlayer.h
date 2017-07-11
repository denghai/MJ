#pragma once
#include "NativeTypes.h"
class IRoomPlayer
{
public:
	IRoomPlayer();
	virtual ~IRoomPlayer(){}
	virtual void init(uint32_t nSessionID , uint32_t nUserUID);
	virtual void willLeave(){}
	virtual void onGameEnd(){}
	virtual void onGameBegin(){}
	void setState( uint32_t nState );
	bool isHaveState( uint32_t nState );
	void addState(uint32_t nState );
	void removeState(uint32_t nState );
	void setSessionID(uint32_t nSessionID );
	void setUserUID( uint32_t nUserUID );
	void setCoin( int32_t nCoin ){ if ( nCoin < 0 ) nCoin = 0 ; m_nCoin = nCoin ;}
	int32_t getCoin(){ return m_nCoin ;}
	uint32_t getUserUID(){ return m_nUserUID ; }
	uint32_t getSessionID(){ return m_nSessionID ; }
private:
	uint32_t m_nUserUID ;
	uint32_t m_nSessionID ;
	uint32_t m_nStateFlag ;
	int32_t m_nCoin ;
};