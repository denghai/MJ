#include "StartGameState.h"
#include "NewMJRoom.h"
void CStartGameState::enterState(IRoom* pRoom)
{
	IRoomState::enterState(pRoom) ;
	m_pRoom->onGameWillBegin();
	m_pRoom->doStartGame();
	setStateDuringTime(eTime_ExeGameStart);
}

void CStartGameState::onStateDuringTimeUp()
{
	// wait player act state ;
	Json::Value js;
	js["idx"] = ((CNewMJRoom*)m_pRoom)->getBankerIdx() ;
	js["exeAct"] = eMJAct_Chu ;
	js["isWaitChoseAct"] = true ;
	js["actCard"] = 0 ;
	m_pRoom->goToState(eRoomState_WaitPlayerAct,&js) ;
}