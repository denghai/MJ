#pragma once
#include "IRoomState.h"
class CGoldenRoom ;
class CGoldenGameResultState
	:public IRoomState
{
public:
	enum  
	{
		eStateID = eRoomState_Golden_GameResult ,
	};
public:
	CGoldenGameResultState(){ m_nState = eRoomState_Golden_GameResult ;}
	void enterState(IRoom* pRoom) ;
	void onStateDuringTimeUp();
protected:
	CGoldenRoom* m_pRoom ;
};