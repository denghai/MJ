#pragma once
#include "IRoomState.h"
class CGoldenRoom ;
class CGoldenPKState
	:public IRoomState
{
public:
	enum  
	{
		eStateID = eRoomState_Golden_PK ,
	};
public:
	CGoldenPKState(){ m_nState = eRoomState_Golden_PK ;}
	void enterState(IRoom* pRoom) ;
	void onStateDuringTimeUp();
protected:
	CGoldenRoom* m_pRoom ;
};