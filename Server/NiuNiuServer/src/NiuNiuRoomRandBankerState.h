#pragma once
#include "IRoomState.h"
class CNiuNiuRoom ;
class CNiuNiuRoomRandBankerState
	:public IRoomState
{
public:
	enum  
	{
		eStateID = eRoomState_NN_RandBanker ,
	};
public:
	CNiuNiuRoomRandBankerState(){ m_nState = eRoomState_NN_RandBanker ;}
	void enterState(IRoom* pRoom);
	void onStateDuringTimeUp();
	uint16_t getStateID(){ return eStateID ;}
	void setRandCnt(uint8_t nRcnt){ m_nRanderCnt = nRcnt ;}
protected:
	CNiuNiuRoom* m_pRoom ;
	uint8_t m_nRanderCnt ; 
};