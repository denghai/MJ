#pragma once
#include "IRoomState.h"
class ISitableRoom ;
class CGoldenStartGameState
	:public IRoomState
{
public:
	enum  
	{
		eStateID = eRoomState_StartGame ,
	};
public:
	CGoldenStartGameState(){ m_nState = eRoomState_StartGame ;}
	void enterState(IRoom* pRoom);
	void onStateDuringTimeUp() ;
protected:
	ISitableRoom* m_pRoom ;
};