#pragma once
#include "IRoomState.h"
class CNiuNiuRoom ;
class CNiuNiuRoomGameResultState
	:public IRoomState
{
public:
	enum { eStateID = eRoomState_NN_GameResult };
public:
	CNiuNiuRoomGameResultState(){ m_nState = eRoomState_NN_GameResult ;}
	void enterState(IRoom* pRoom);
	void onStateDuringTimeUp();
protected:
	CNiuNiuRoom* m_pRoom ;
};