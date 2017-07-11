#pragma once
#include "IRoomManager.h"
class ISitableRoomManager
	:public IRoomManager
{
public:
	IRoom* getRoomByConfigID(uint32_t nRoomID )override ;
};