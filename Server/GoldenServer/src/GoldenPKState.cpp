#include "GoldenPKState.h"
#include "GoldenRoom.h"
#include "log4z.h"
void CGoldenPKState::enterState(IRoom* pRoom)
{
	m_pRoom = (CGoldenRoom*)pRoom ;
	setStateDuringTime(TIME_GOLDEN_ROOM_PK);
}

void CGoldenPKState::onStateDuringTimeUp()
{
	if ( m_pRoom->isReachedMaxRound())
	{
		LOGFMTD("onStateDuringTimeUp reached max round so game over room id = %u", m_pRoom->getRoomID());
		m_pRoom->goToState(eRoomState_Golden_GameResult);
		return;
	}

	if ( m_pRoom->getPlayerCntWithState(eRoomPeer_CanAct) >= 2 )
	{
		m_pRoom->goToState(eRoomState_Golden_Bet) ;
		m_pRoom->informPlayerAct(true) ;
	}
	else
	{
		m_pRoom->goToState(eRoomState_Golden_GameResult ) ;
	}
}
