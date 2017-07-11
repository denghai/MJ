#include "GameOverState.h"
#include "ISitableRoom.h"
#include "log4z.h"
void CGameOverState::enterState(IRoom* pRoom )
{
	IRoomState::enterState(pRoom) ;
	LOGFMTI("进入游戏结束状态 ") ;
	pRoom->onGameOver();
	setStateDuringTime(eTime_GameOver);
}

void CGameOverState::onStateDuringTimeUp()
{
	LOGFMTD("game end ,did end ") ;
	this->m_pRoom->onGameDidEnd();
	LOGFMTD("game end to wait ready sate") ;
	this->m_pRoom->goToState(eRoomSate_WaitReady) ;
}

uint16_t CGameOverState::getStateID()
{
	return eRoomState_GameEnd;
}