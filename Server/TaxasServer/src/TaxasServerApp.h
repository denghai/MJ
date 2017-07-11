#pragma once
#include "ISeverApp.h"
#include "RoomConfig.h"
#include "ServerConfig.h"
#include "MessageDefine.h"
class CRoomManager ;
class CTaxasServerApp
	:public IServerApp
{
public:
	enum eModule
	{
		eMod_None = IServerApp::eDefMod_ChildDef,
		eMod_RoomMgr = eMod_None,
		eMod_Max,
	};
public:
	static CTaxasServerApp* SharedGameServerApp();
	CTaxasServerApp();
	~CTaxasServerApp();
	bool init();
	CRoomConfigMgr* GetConfigMgr(){ return m_pRoomConfig ; }
	uint16_t getLocalSvrMsgPortType(){ return ID_MSG_PORT_TAXAS ; }
	IGlobalModule* createModule( uint16_t eModuleType )override ;
public:
	static CTaxasServerApp* s_TaxasServerApp ;
protected:
	CRoomConfigMgr* m_pRoomConfig;
};