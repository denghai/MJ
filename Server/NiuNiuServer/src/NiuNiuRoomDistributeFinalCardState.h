#pragma once
#include "IRoomState.h"
class CNiuNiuRoom;
class CNiuNiuRoomDistributeFinalCardState
	:public IRoomState
{
public:
	enum { eStateID = eRoomState_NN_FinalCard };
public:
	CNiuNiuRoomDistributeFinalCardState(){ m_nState = eRoomState_NN_FinalCard ;}
	void enterState(IRoom* pRoom);
	void onStateDuringTimeUp();
protected:
	CNiuNiuRoom* m_pRoom ;
};