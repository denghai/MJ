#pragma once
#include "IRoomState.h"
class CGoldenRoom;
class CGoldenBetState
	:public IRoomState
{
public:
	enum  
	{
		eStateID = eRoomState_Golden_Bet ,
	};
public:
	CGoldenBetState(){ m_nState = eRoomState_Golden_Bet ;}
	void enterState(IRoom* pRoom)override;
	void onStateDuringTimeUp() override;
	bool onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID )override ;
protected:
	CGoldenRoom* m_pRoom ;
};