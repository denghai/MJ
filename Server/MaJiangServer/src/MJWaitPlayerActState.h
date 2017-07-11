#pragma once ;
#include "IRoomState.h"
#include "MJRoom.h"
#include "MJRoomPlayer.h"

struct stPlayerActTypeActionItem
	:public stActionItem
{
	uint8_t nCardNumber ;
};

class CMJWaitPlayerActState
	:public IWaitingState
{
public:
	void enterState(IRoom* pRoom);
	void onWaitEnd( bool bTimeOut )override ;
	bool onMsg(Json::Value& prealMsg ,uint16_t nMsgType, eMsgPort eSenderPort , uint32_t nSessionID);
	uint16_t getStateID(){ return eRoomState_WaitPlayerAct ; }
	float getExecuteTime();
};

class CMJDoPlayerActState
	:public IExecuingState
{
public:
	void onExecuteOver()override;
	void doExecuteAct( stActionItem* pAct);
	uint16_t getStateID(){ return eRoomState_DoPlayerAct ; }
	void leaveState()override{ m_vecCardPlayerIdxs.clear() ; IExecuingState::leaveState(); }
protected:
	eMJActType m_edoAct ;
	uint8_t m_nCurIdx ;
	uint8_t m_nCardNumber ;
	std::vector<stWaitIdx> m_vecCardPlayerIdxs ;
};

// other player 
struct stWaitCardInfo
{
	uint8_t nCardNumber ;
	uint8_t nCardProvideIdx ;
	bool isBuGang ;
};

class CMJWaitOtherActState
	:public IWaitingState
{
public:
	void enterState(IRoom* pRoom)override;
	void onWaitEnd( bool bTimeOut )override ;
	bool onMsg(Json::Value& prealMsg ,uint16_t nMsgType, eMsgPort eSenderPort , uint32_t nSessionID);
	uint16_t getStateID(){ return eRoomState_WaitOtherPlayerAct ; }
	void setWaitCardInfo( stWaitCardInfo* pInfo )
	{
		memcpy(&m_tInfo,pInfo,sizeof(m_tInfo));
	}
protected:
	stWaitCardInfo m_tInfo ;
};

class CMJDoOtherPlayerActState
	:public IExecuingState
{
public:
	void enterState(IRoom* pRoom){ m_nCurIdx = 0 ; IExecuingState::enterState(pRoom); }
	void onExecuteOver()override;
	void doExecuteAct( stActionItem* pAct);
	uint16_t getStateID(){ return eRoomState_DoOtherPlayerAct ; }
	void setWaitCardInfo( stWaitCardInfo* pInfo )
	{
		memcpy(&m_tInfo,pInfo,sizeof(m_tInfo));
	}
public:
	eMJActType m_edoAct ;
	uint8_t m_nCurIdx ;

	stWaitCardInfo m_tInfo ;
};

