#pragma once 
#include "MJRoomStateStartGame.h"
class XLRoomStateStartGame
	:public MJRoomStateStartGame
{
public:
	void onStateTimeUp()override 
	{
		Json::Value jsValue;
		getRoom()->goToState(eRoomState_WaitDecideQue, &jsValue);
	}
};