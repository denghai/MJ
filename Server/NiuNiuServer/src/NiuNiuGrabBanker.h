#pragma once
#include "IRoomState.h"
class CNiuNiuRoom ;
class CNiuNiuRoomGrabBanker
	:public IRoomState
{
public:
	enum  
	{
		eStateID = eRoomState_NN_GrabBanker,
	};
public:
	CNiuNiuRoomGrabBanker(){ m_nState = eRoomState_NN_GrabBanker ; }
	void enterState(IRoom* pRoom);
	bool onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID );
	void onStateDuringTimeUp();
protected:
	CNiuNiuRoom* m_pRoom ;
	std::vector<uint8_t> m_vWaitIdxs ;
	std::vector<uint8_t> m_vGrabedIdx ;
	std::vector<uint8_t> m_vRefusedGrabedIdx ;
};