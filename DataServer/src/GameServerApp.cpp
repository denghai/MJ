#include "GameServerApp.h"
#include "PlayerManager.h"
#include <ctime>
#include "PlayerMail.h"
#include "EventCenter.h"
#include "log4z.h"
#include "RewardConfig.h"
#include "ServerStringTable.h"
#include "ExchangeCenter.h"
#include "RobotCenter.h"
#include "encryptNumber.h"
#include "Player.h"
#include "PlayerGameData.h"
#include "Group.h"
#include "GameRoomCenter.h"
#include "QingJiaModule.h"
#ifndef USHORT_MAX
#define USHORT_MAX 65535 
#endif
#include <cassert>
CGameServerApp* CGameServerApp::s_GameServerApp = NULL ;
CGameServerApp* CGameServerApp::SharedGameServerApp()
{
	return s_GameServerApp ;
}

CGameServerApp::~CGameServerApp()
{
	delete m_pConfigManager ;
}

bool CGameServerApp::init()
{
	IServerApp::init();
	if ( s_GameServerApp == NULL )
	{
		s_GameServerApp = this ;
	}
	else
	{
		LOGFMTE("Game Server App can not be init more than once !") ;
		return false;
	}
	srand((unsigned int)time(0));

	CSeverConfigMgr SvrConfigMgr ;
	SvrConfigMgr.LoadFile("../configFile/serverConfig.txt");
	stServerConfig* pConfig = SvrConfigMgr.GetServerConfig(eSvrType_Center) ;
	if ( pConfig == NULL )
	{
		LOGFMTE("center svr config is null , so can not connected to !") ;
		return false;
	}
	setConnectServerConfig(pConfig);

	CServerStringTable::getInstance()->LoadFile("../configFile/stringTable.txt");
	CRewardConfig::getInstance()->LoadFile("../configFile/rewardConfig.txt");

	m_pConfigManager = new CConfigManager ;
	m_pConfigManager->LoadAllConfigFile("../configFile/") ;

	// install module
	for ( uint16_t nModule = eMod_None ; nModule < eMod_Max ; ++nModule )
	{
		auto b = installModule(nModule);
		assert(b && "install this module failed " );
		if ( !b )
		{
			LOGFMTE("install module = %u failed",nModule ) ;
		}
	}

	time_t tNow = time(NULL) ;
	m_nCurDay = localtime(&tNow)->tm_mday ;
	return true ;
}

CPlayerManager* CGameServerApp::GetPlayerMgr()
{
	auto p = (CPlayerManager*)getModuleByType(eMod_PlayerMgr) ;
	return p ;
}

CRobotCenter* CGameServerApp::getRobotCenter()
{
	auto p = (CRobotCenter*)this->getModuleByType(eMod_RobotCenter);
	return p ;
}

CEncryptNumber* CGameServerApp::getEncryptNumber()
{
	auto p = (CEncryptNumber*)this->getModuleByType(eMod_EncryptNumber);
	return p ;
}

CGroup* CGameServerApp::getCroupMgr()
{
	auto p = (CGroup*)this->getModuleByType(eMod_Group);
	return p ;
}

CGameRoomCenter* CGameServerApp::getGameRoomCenter()
{
	auto p = (CGameRoomCenter*)this->getModuleByType(eMod_GameRoomCenter);
	return p ;
}

CQinJiaModule* CGameServerApp::getQinjiaModule()
{
	auto p = (CQinJiaModule*)this->getModuleByType(eMod_QinJia);
	return p ;
}

bool CGameServerApp::onLogicMsg( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID )
{
	if ( IServerApp::onLogicMsg(prealMsg,eSenderPort,nSessionID) )
	{
		return true ;
	}

	//if ( m_tPokerCircle.onMessage(prealMsg,eSenderPort,nSessionID) )
	//{
	//	return true ;
	//}

	if ( ProcessPublicMsg(prealMsg,eSenderPort,nSessionID ) )
	{
		return true ;
	}
	LOGFMTE("unprocess msg = %d , from port = %d , nsssionid = %d",prealMsg->usMsgType,eSenderPort,nSessionID ) ;
	return true ;
}

bool CGameServerApp::onLogicMsg( Json::Value& recvValue , uint16_t nmsgType, eMsgPort eSenderPort , uint32_t nSessionID )
{
	if ( IServerApp::onLogicMsg(recvValue,nmsgType,eSenderPort,nSessionID) )
	{
		return true ;
	}
	return false ;
}

bool CGameServerApp::onAsyncRequest(uint16_t nRequestType , const Json::Value& jsReqContent, Json::Value& jsResult )
{
	if ( IServerApp::onAsyncRequest(nRequestType,jsReqContent,jsResult) )
	{
		return true ;
	}
	switch ( nRequestType )
	{
	case eAsync_PostDlgNotice:
		{
			eNoticeType eType = (eNoticeType)jsReqContent["dlgType"].asUInt() ;
			uint32_t nTargetUID = jsReqContent["targetUID"].asUInt() ;
			auto jsArg = jsReqContent["arg"];
			CPlayerMailComponent::PostDlgNotice(eType,jsArg,nTargetUID) ;
			LOGFMTD("do async post dlg notice etype = %u ,targetUID = %u",eType,nTargetUID) ;
		}
		break ;
	default:
		return false ;
	}
	return true ;
}

bool CGameServerApp::ProcessPublicMsg( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID )
{
	return false ;
}

void CGameServerApp::update(float fdeta )
{
	IServerApp::update(fdeta);
	CheckNewDay();
}

void CGameServerApp::CheckNewDay()
{
	// check new day 
	time_t tNow = time(NULL) ;
	struct tm tmNow = *localtime(&tNow) ;
	if ( tmNow.tm_mday != m_nCurDay )
	{
		m_nCurDay = tmNow.tm_mday ;
		// new day 
		CEventCenter::SharedEventCenter()->PostEvent(eEvent_NewDay,&tmNow) ;
	}
}

IGlobalModule* CGameServerApp::createModule( uint16_t eModuleType )
{
	IGlobalModule* pMod = IServerApp::createModule(eModuleType) ;
	if ( pMod )
	{
		return pMod ;
	}

	switch (eModuleType)
	{
	case eMod_PlayerMgr:
		{
			pMod = new CPlayerManager() ;
		}
		break;
	case eMod_RobotCenter:
		{
			pMod = new CRobotCenter();
		}
		break;
	case eMod_EncryptNumber:
		{
			pMod = new CEncryptNumber();
		}
		break;
	case eMod_Group:
		{
			pMod = new CGroup();
		}
		break;
	case eMod_GameRoomCenter:
		{
			pMod = new CGameRoomCenter();
		}
		break;
	case eMod_QinJia:
		{
			pMod = new CQinJiaModule();
		}
		break ;
	default:
		break;
	}

	return pMod ;
}