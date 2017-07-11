#pragma once ;
#include "IRoomState.h"
#include "MJRoom.h"
#include "MJRoomPlayer.h"

struct stQueTypeActionItem
	:public stActionItem
{
	uint8_t nType;
};

class CMJWaitDecideQueState
	:public IWaitingState
{
public:
	void enterState(IRoom* pRoom)override;
	void onWaitEnd( bool bTimeOut )override ;
	bool onMsg(Json::Value& prealMsg ,uint16_t nMsgType, eMsgPort eSenderPort , uint32_t nSessionID);
	uint16_t getStateID(){ return eRoomState_WaitDecideQue ; }
};

class CMJDoDecideQueState
	:public IExecuingState
{
public:
	void enterState(IRoom* pRoom)override ;
	void onExecuteOver()override;
	void doExecuteAct( stActionItem* pAct);
	uint16_t getStateID(){ return eRoomState_DoDecideQue ; }
};
