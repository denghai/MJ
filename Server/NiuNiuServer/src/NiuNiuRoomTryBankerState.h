#pragma once
#include "IRoomState.h"
class CNiuNiuRoom ;
class CNiuNiuRoomTryBanker
	:public IRoomState
{
public:
	enum  
	{
		eStateID = eRoomState_NN_TryBanker,
	};
public:
	CNiuNiuRoomTryBanker(){ m_nState = eRoomState_NN_TryBanker ; }
	void enterState(IRoom* pRoom);
	bool onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID );
	void onStateDuringTimeUp();
protected:
	CNiuNiuRoom* m_pRoom ;
	uint8_t m_nBiggestTimeTryBanker ;
	uint8_t m_nLeftTryBankerPlayerCnt ;
};