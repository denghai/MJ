#pragma once
#include "IGameRoom.h"
#include "IGlobalModule.h"
#include <json/json.h>
struct stMsg;
class IGameRoomManager
	:public IGlobalModule
{
public:
	virtual ~IGameRoomManager(){}
	virtual IGameRoom* getRoomByID(uint32_t nRoomID) = 0;
	virtual void sendMsg(stMsg* pmsg, uint32_t nLen, uint32_t nSessionID) = 0;
	virtual void sendMsg(Json::Value& jsContent, unsigned short nMsgType, uint32_t nSessionID, eMsgPort ePort = ID_MSG_PORT_CLIENT) = 0;
};