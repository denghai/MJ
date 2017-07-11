#pragma once 
#include "IRoomState.h"
class CNiuNiuRoom ;
class CNiuNiuWaitStartGame
	: public IRoomState
{
public:
	CNiuNiuWaitStartGame(){ m_nState = eRoomState_WaitJoin;}
	void enterState(IRoom* pRoom )override;
	void update(float)override ;
protected:
	CNiuNiuRoom* m_pRoom ;
};