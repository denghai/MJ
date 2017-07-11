#pragma once
#include "ServerConfig.h"
#include "ISeverApp.h"
#include "MiVerifyManager.h"
#include "TaskPoolModule.h"
class CVerifyApp
	:public IServerApp
{
public:
	enum eMod
	{
		eMod_Pool = eDefMod_ChildDef ,
		eMod_Http,
	};
public:
	bool init();
	uint16_t getLocalSvrMsgPortType()override;
	CTaskPoolModule* getTaskPoolModule();
	IGlobalModule* createModule( uint16_t eModuleType )override ;
};