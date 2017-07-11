#pragma once
#include "IRoomState.h"
class CStartGameState
	:public IRoomState
{
public:
	void enterState(IRoom* pRoom);
	void onStateDuringTimeUp()override;
	uint16_t getStateID(){ return eRoomState_StartGame ;}
};