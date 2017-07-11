#pragma once 
#include "IMJRoomState.h"
#include "IMJRoom.h"
class MJRoomStateGameEnd
	:public IMJRoomState
{
public:
	uint32_t getStateID(){ return eRoomState_GameEnd; }
	void enterState(IMJRoom* pmjRoom, Json::Value& jsTranData)
	{
		IMJRoomState::enterState(pmjRoom, jsTranData);
		getRoom()->onGameEnd();
		setStateDuringTime(eTime_GameOver * 0.1);
	}

	void onStateTimeUp()
	{
		getRoom()->onGameDidEnd();
		getRoom()->goToState(eRoomSate_WaitReady);
	}
};




