#include "GoldenGameResultState.h"
#include "GoldenRoom.h"
void CGoldenGameResultState::enterState(IRoom* pRoom)
{
	m_pRoom = (CGoldenRoom*)pRoom ;
	setStateDuringTime(TIME_GOLDEN_ROOM_RESULT);
	m_pRoom->caculateGameResult();
}

void CGoldenGameResultState::onStateDuringTimeUp()
{
	m_pRoom->goToState(eRoomState_DidGameOver);
}