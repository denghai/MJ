#pragma once
#include "IRoomState.h"
#include "ServerCommon.h"
#define TIME_GOLDE_WAIT_PLAYER_READY 15
class CGoldenRoom ;
class IRoomStateWaitReadyState
	: public IRoomState
{
public:
	IRoomStateWaitReadyState(){ m_nState = eRoomState_WaitJoin;}
	void enterState(IRoom* pRoom )override{m_pRoom = (CGoldenRoom*)pRoom ; m_fWaitTicket = TIME_GOLDE_WAIT_PLAYER_READY ; }
	void update(float fDelta )override;
protected:
	CGoldenRoom* m_pRoom ;
	float m_fWaitTicket ;
};
