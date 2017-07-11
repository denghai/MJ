#include "ISitableRoomPlayer.h"
#include "log4z.h"
#include "ServerCommon.h"
#include "IPeerCard.h"
void ISitableRoomPlayer::reset(IRoom::stStandPlayer* pPlayer)
{
	nUserUID = pPlayer->nUserUID ;
	nSessionID = pPlayer->nUserSessionID ;
	nNewPlayerHaloWeight = pPlayer->nNewPlayerHaloWeight ;
	nPlayTimes = nWinTimes = 0 ;
	nSingleWinMost = 0 ;
	m_nHaloState = 0 ;
	nCoin = 0 ;
	m_nIdx = 0 ;
	m_nState = 0 ;
	nTempHaloWeight = 0 ;
	m_isDelayStandUp = false ;
	nTotalGameOffset = 0 ;
	resetNoneActTimes();
}

void ISitableRoomPlayer::onGameEnd()
{
	nTotalGameOffset += getGameOffset();
	m_nHaloState = 0 ;
	if ( nNewPlayerHaloWeight > 0 )
	{
		--nNewPlayerHaloWeight;
	}
	setState(eRoomPeer_WaitNextGame) ;

	if ( getGameOffset() > (int32_t)0 )
	{
		increaseWinTimes() ;
	}
}

bool ISitableRoomPlayer::isHaveHalo()
{
	if ( getTotalHaloWeight() <= 0 )
	{
		return false ;
	}

	if ( m_nHaloState == 2 )
	{
		return false ;
	}

	if ( 1 == m_nHaloState )
	{
		return true ;
	}

	uint32_t nRate = rand() % ( MAX_NEW_PLAYER_HALO + 1 );
	m_nHaloState = nRate <= getTotalHaloWeight() ? 1 : 2 ;
	if ( m_nHaloState == 1 )
	{
		LOGFMTD("uid = %u invoke halo tempHalo = %u",getUserUID(),nTempHaloWeight);
	}
	nTempHaloWeight = 0 ;
	return m_nHaloState == 1 ;
}

void ISitableRoomPlayer::switchPeerCard(ISitableRoomPlayer* pPlayer )
{
	assert(pPlayer && "target player is null , can not swap peer cards" );
	getPeerCard()->swap(pPlayer->getPeerCard()) ;
}


void ISitableRoomPlayer::removeState( uint32_t nStateFlag )
{
	m_nState &=(~nStateFlag);
	LOGFMTD("uid = %u state = %u remove state = %u",getUserUID(),getState(),nStateFlag) ;
}

void ISitableRoomPlayer::addState( uint32_t nStateFlag )
{
	m_nState |=(nStateFlag); 
	LOGFMTD("uid = %u state = %u add state = %u",getUserUID(),getState(),nStateFlag) ;
}

void ISitableRoomPlayer::setState( uint32_t nStateFlag )
{ 
	m_nState = nStateFlag ;
	LOGFMTD("uid = %u state = %u set state",getUserUID(),getState()) ;
}
