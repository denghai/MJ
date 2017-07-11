#pragma once
#include "IRoomState.h"
class CGameOverState
	:public IRoomState
{
public:
	void enterState(IRoom* pRoom )override;
	void onStateDuringTimeUp()override;
	uint16_t getStateID()override ;
};