#pragma once
#include "IRoomState.h"
class CNiuNiuRoom ;
class CNiuNiuRoomBetState
	:public IRoomState
{
public:
	enum { eStateID = eRoomState_NN_StartBet };
public:
	CNiuNiuRoomBetState(){ m_nState = eRoomState_NN_StartBet ;}
	void enterState(IRoom* pRoom);
	void onStateDuringTimeUp();
	bool onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID );
protected:
	CNiuNiuRoom* m_pRoom ;
	std::vector<uint8_t> vWaitBetPlayerIdxs ;
};