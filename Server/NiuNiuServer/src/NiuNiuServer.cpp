#include "NiuNiuServer.h"
#include "MessageDefine.h"
#include <ctime>
#include "log4z.h"
#include "ServerStringTable.h"
#include "RewardConfig.h"
bool CNiuNiuServerApp::init()
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
	//m_tMgr.LoadFile("../configFile/RoomConfig.txt") ;

	CServerStringTable::getInstance()->LoadFile("../configFile/stringTable.txt");
	CRewardConfig::getInstance()->LoadFile("../configFile/rewardConfig.txt");

	installModule(eMod_RoomMgr);
	return true ;
}

uint16_t CNiuNiuServerApp::getLocalSvrMsgPortType()
{
	return ID_MSG_PORT_NIU_NIU ;
}

IGlobalModule* CNiuNiuServerApp::createModule( uint16_t eModuleType )
{
	auto p = IServerApp::createModule(eModuleType);
	if ( p )
	{
		return p;
	}

	if ( eModuleType == eMod_RoomMgr )
	{
		p = new CNiuNiuRoomManager(getRoomConfigMgr());
	}
	return p ;
}
