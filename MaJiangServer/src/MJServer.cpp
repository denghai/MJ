#include "MJServer.h"
#include "MessageDefine.h"
#include <ctime>
#include "log4z.h"
#include "ServerStringTable.h"
#include "RewardConfig.h"
#include "MJRoomManager.h"
#include "MJGameReplayManager.h"
bool CMJServerApp::init()
{
	IServerApp::init();
	srand((unsigned int)time(0));

	CSeverConfigMgr stSvrConfigMgr ;
	stSvrConfigMgr.LoadFile("../configFile/serverConfig.txt");
	stServerConfig* pConfig = stSvrConfigMgr.GetServerConfig(eSvrType_Center) ;
	if ( pConfig == NULL )
	{
		LOGFMTE("center svr config is null , so can not connected to !") ;
		return false;
	}
	setConnectServerConfig(pConfig);
	m_tMgr.LoadFile("../configFile/RoomConfig.txt") ;

	CServerStringTable::getInstance()->LoadFile("../configFile/stringTable.txt");

	installModule(eMod_RoomMgr);
	installModule(eMod_RoomMgrOld);
	installModule(eMod_RecorderMgr);
	installModule(eMod_ReplayMgr);
	return true ;
}

uint16_t CMJServerApp::getLocalSvrMsgPortType()
{
	return ID_MSG_PORT_MJ ;
}

IGlobalModule* CMJServerApp::createModule(uint16_t eModuleType)
{
	auto p = IServerApp::createModule(eModuleType);
	if (p)
	{
		return p;
	}

	 if (eMod_RoomMgr == eModuleType)
	{
		p = new MJRoomManager();
	}
	 else if (eMod_RecorderMgr == eModuleType )
	 {
		 p = new GameRecorderMgr();
	 }
	 else if ( eMod_ReplayMgr == eModuleType )
	 {
		 p = new MJGameReplayManager();
	 }
	return p;
}

GameRecorderMgr* CMJServerApp::getRecorderMgr()
{
	auto p = getModuleByType(eMod_RecorderMgr);
	return (GameRecorderMgr*)p;
}

