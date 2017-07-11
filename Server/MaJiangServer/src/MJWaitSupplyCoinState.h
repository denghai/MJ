#pragma once
#include "IRoomState.h"
#include <set>
class CMJWaitSupplyCoinState
	:public IRoomState
{
public:
	void onStateDuringTimeUp()override;
	uint16_t getStateID(){ return eRoomState_WaitSupplyCoin ; }
	void enterState(IRoom* pRoom)override ;
	bool onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID )override ;
	bool onMsg(Json::Value& prealMsg ,uint16_t nMsgType, eMsgPort eSenderPort , uint32_t nSessionID)override ;
	void addWaitIdx(uint8_t nIdx );
	void finishWait();
	void setInvokeInfo(uint8_t nInvokeIdx , uint8_t nInvokedByAct ); // 0 gang , 1 shi hu ; 
protected:
	std::set<uint8_t> m_vWaitPlayerIdxs ;
	uint8_t m_nInvokerIdx ;
	uint8_t m_nInvokedByAct ; // 0 gang , 1 shi hu ; 
};