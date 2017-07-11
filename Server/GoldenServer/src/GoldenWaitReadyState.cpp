#include "GoldenWaitReadyState.h"
#include "GoldenRoom.h"
void IRoomStateWaitReadyState::update(float fDelta )
{
	// check close 
	if ( m_pRoom->getDelegate() && m_pRoom->getDelegate()->isRoomShouldClose(m_pRoom) )
	{
		m_pRoom->goToState(eRoomState_Close) ;
		return ;
	}

	uint8_t nSitPlayerCnt = (uint8_t)m_pRoom->getSitDownPlayerCount();
	if ( nSitPlayerCnt < 2 )
	{
		return ;
	}

	//if ( m_pRoom->isHaveRealPlayer() == false )
	//{
	//	return ;
	//}

	bool bStartGame = false ;
	
	if ( m_pRoom->getPlayerCntWithState(eRoomPeer_Ready) >= 2 )
	{
		m_fWaitTicket -= fDelta  ;
	}

	if ( m_pRoom->getPlayerCntWithState(eRoomPeer_Ready) == nSitPlayerCnt )
	{
		bStartGame = true ;
	}
	else
	{
		if ( m_fWaitTicket < 0.00001 )
		{
			if ( m_pRoom->getPlayerCntWithState(eRoomPeer_Ready) >= 2 )
			{
				bStartGame = true ;
			}
		}
	}

	if ( bStartGame )
	{
		m_pRoom->onGameWillBegin();
		m_pRoom->prepareCards();
		m_pRoom->doProcessNewPlayerHalo();
		m_pRoom->goToState(eRoomState_StartGame) ;
		return ;
	}

}