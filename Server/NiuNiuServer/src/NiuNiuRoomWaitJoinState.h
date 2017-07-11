#pragma once
#include "IRoomState.h"
class ISitableRoom ;
class CNiuNiuRoomWaitJoinState
	:public IRoomState
{
public: 
	enum  
	{
		eStateID = eRoomState_NN_WaitJoin,
	};
public:
	void enterState(IRoom* pRoom) override;
	void update(float fDeta) override;
	uint16_t getStateID()override { return eStateID;}
protected:
	ISitableRoom* m_pRoom ;
};