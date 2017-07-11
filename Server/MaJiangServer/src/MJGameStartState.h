#pragma once
#include "IRoomState.h"
#include "MJRoom.h"
#include "MJRoomPlayer.h"
#include "MessageIdentifer.h"
#include "MJExchangeCardState.h"
class CMJGameStartState
	:public IRoomState
{
public:
	void enterState(IRoom* pRoom)
	{
		IRoomState::enterState(pRoom) ; 
		m_pRoom->onGameWillBegin();
		setStateDuringTime(eTime_ExeGameStart) ;
	} 

	uint16_t getStateID() override { return eRoomState_StartGame ; }
	void onStateDuringTimeUp()override
	{
		auto pTargetState = (CMJWaitDecideQueState*)m_pRoom->getRoomStateByID(eRoomState_WaitDecideQue);

		pTargetState->setWaitTime(((CMJRoom*)m_pRoom)->getWaitPlayerActTime(0,eTime_WaitDecideQue));
		for ( uint8_t nIdx = 0 ; nIdx < m_pRoom->getSeatCount() ; ++nIdx )
		{
			pTargetState->addWaitingTarget(nIdx,eMJAct_Hu);
		}

		m_pRoom->goToState(pTargetState) ;
	}
};