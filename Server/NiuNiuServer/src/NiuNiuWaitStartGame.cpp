#include "NiuNiuWaitStartGame.h"
#include "NiuNiuRoom.h"
void CNiuNiuWaitStartGame::enterState(IRoom* pRoom )
{
	m_pRoom = (CNiuNiuRoom*)pRoom ;
	if ( m_pRoom->getSitDownPlayerCount() <= 1 )
	{
		m_pRoom->clearBanker();
	}
}

void CNiuNiuWaitStartGame::update(float fd)
{
	// check close 
	if ( m_pRoom->getDelegate() && m_pRoom->getDelegate()->isRoomShouldClose(m_pRoom) )
	{
		m_pRoom->goToState(eRoomState_Close) ;
		return ;
	}

	if ( m_pRoom->canStartGame() )
	{
		m_pRoom->onGameWillBegin();
		m_pRoom->prepareCards();
		m_pRoom->doProcessNewPlayerHalo();
		if ( m_pRoom->isHaveBanker() )
		{
			m_pRoom->goToState(eRoomState_NN_StartBet) ;
		}
		else
		{
			m_pRoom->goToState(eRoomState_NN_GrabBanker);
		}
		return ;
	}

}