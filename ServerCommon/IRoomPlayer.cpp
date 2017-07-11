#include "IRoomPlayer.h"
IRoomPlayer::IRoomPlayer()
{
	m_nSessionID = 0 ;
	m_nUserUID = 0 ;
	m_nStateFlag = 0 ;
	m_nCoin = 0 ;
}

void IRoomPlayer::init(uint32_t nSessionID , uint32_t nUserUID)
{
	setSessionID(nSessionID) ;
	setUserUID(nUserUID) ;
	m_nStateFlag = 0 ;
	m_nCoin = 0 ;
}

void IRoomPlayer::setState( uint32_t nState )
{
	m_nStateFlag = nState ;
}

bool IRoomPlayer::isHaveState( uint32_t nState )
{
	return ( m_nStateFlag & nState ) == nState ;
}

void IRoomPlayer::addState(uint32_t nState )
{
	m_nStateFlag |= nState ;
}

void IRoomPlayer::removeState(uint32_t nState )
{
	m_nStateFlag &= (~nState);
}

void IRoomPlayer::setSessionID(uint32_t nSessionID )
{
	m_nSessionID = nSessionID ;
}

void IRoomPlayer::setUserUID( uint32_t nUserUID )
{
	m_nUserUID = nUserUID ;
}
