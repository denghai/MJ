#pragma once
#include "IPlayerComponent.h"
#include "MessageDefine.h"
#include <map>
struct stEventArg ;
struct stPlayerMissionSate
	:public stMissionSate
{
	bool OnEvent(stPlayerEvetArg* pArg );
	void Reset();
};

class CPlayerMission
	:public IPlayerComponent
{
public:
	typedef std::map<unsigned short,stPlayerMissionSate*> MAP_PLAYER_MISSION_STATE ;
public:
	CPlayerMission(CPlayer* pPlayer):IPlayerComponent(pPlayer){ Clear() ;}
	~CPlayerMission(){ Clear() ; }
	virtual bool OnMessage( stMsg* pMessage , eMsgPort eSenderPort ) ;
	virtual void OnPlayerDisconnect();
	virtual void Reset();
	virtual void Init();
	bool OnPlayerEvent(stPlayerEvetArg* pArg);
	virtual void TimerSave();
	static bool EventFunc(void* pUserData,stEventArg* pArg);
protected:
	void Clear();
	void SendStateListToClient();
	stPlayerMissionSate* GetMissionState(unsigned short nMissionID ) ;
	void OnMissionFinish(stPlayerMissionSate* pMission);
	void OnNewDay(stEventArg* pArg);
protected:
	MAP_PLAYER_MISSION_STATE m_vAllMissionStates ;
	bool m_bDirty ;
};
