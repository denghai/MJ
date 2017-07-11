#pragma once
#include "ISeverApp.h"
#include "Singleton.h"
#include "RoomConfig.h"
class CGoldenRoomManager;
class CGoldenServerApp
	:public IServerApp
	,public CSingleton<CGoldenServerApp>
{
public:
	enum eModule
	{
		eMod_None = IServerApp::eDefMod_ChildDef,
		eMod_RoomMgr = eMod_None,
		eMod_Max,
	};
public:
	CGoldenServerApp(){}
	bool init()override;
	uint16_t getLocalSvrMsgPortType() override ;
	CRoomConfigMgr* getRoomConfigMgr(){ return &m_tMgr ;}
	CGoldenRoomManager* getRoomMgr();
	IGlobalModule* createModule( uint16_t eModuleType )override ;
protected:
	CRoomConfigMgr m_tMgr ;
};