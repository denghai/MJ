#include "GoldenServer.h"
#include "MessageDefine.h"
#include <ctime>
#include "log4z.h"
#include "ServerStringTable.h"
#include "RewardConfig.h"
#include "GoldenRoomManager.h"
bool CGoldenServerApp::init()
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

	// install room mgr ;
	installModule(eMod_RoomMgr);
	return true ;
}

uint16_t CGoldenServerApp::getLocalSvrMsgPortType()
{
	return ID_MSG_PORT_GOLDEN ;
}

IGlobalModule* CGoldenServerApp::createModule( uint16_t eModuleType )
{
	auto p = IServerApp::createModule(eModuleType);
	if ( p )
	{
		return p;
	}

	if ( eModuleType == eMod_RoomMgr )
	{
		p = new CGoldenRoomManager(getRoomConfigMgr());
	}
	return p ;
}

CGoldenRoomManager* CGoldenServerApp::getRoomMgr()
{
	auto p = (CGoldenRoomManager*)this->getModuleByType(eMod_RoomMgr);
	return p ;
}

