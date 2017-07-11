#pragma once
#include "IRoomState.h"
class ISitableRoom ;
class CNiuNiuRoomDistribute4CardState
	:public IRoomState
{
public:
	enum  
	{
		eStateID = eRoomState_NN_Disribute4Card ,
	};
public:
	CNiuNiuRoomDistribute4CardState(){ m_nState = eRoomState_StartGame ;}
	void enterState(IRoom* pRoom);
	void onStateDuringTimeUp() ;
protected:
	ISitableRoom* m_pRoom ;
};