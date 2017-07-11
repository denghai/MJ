#pragma once 
#include "MJRoomStateStartGame.h"
#include "CommonDefine.h"
class NJRoomStateStartGame
	:public MJRoomStateStartGame
{
public:
	void onStateTimeUp()override
	{
		Json::Value jsValue;
		getRoom()->goToState(eRoomState_NJ_Auto_Buhua, &jsValue);
	}
};