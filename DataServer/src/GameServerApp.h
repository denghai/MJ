#pragma once
#include "ISeverApp.h"
#include "PlayerManager.h"
#include "ConfigManager.h"
#include "ServerConfig.h"
#include "PokerCircle.h"
class CRobotCenter;
class CEncryptNumber;
class CBrocaster ;
class CGroup ;
class CGameRoomCenter ;
class CQinJiaModule;
class CGameServerApp
	:public IServerApp
{
public:
	enum eInstallModule
	{
		eMod_None = IServerApp::eDefMod_ChildDef,
		eMod_RobotCenter = eMod_None ,
		eMod_EncryptNumber,
		eMod_Group,
		eMod_GameRoomCenter,
		eMod_QinJia,
		eMod_PlayerMgr,
		eMod_Max,
	};

public:
	static CGameServerApp* SharedGameServerApp();
public:
	~CGameServerApp();
	bool init();
	CPlayerManager* GetPlayerMgr();
	CConfigManager* GetConfigMgr(){ return m_pConfigManager ; }
	CRobotCenter* getRobotCenter();
	CEncryptNumber* getEncryptNumber();
	CGroup* getCroupMgr();
	CGameRoomCenter* getGameRoomCenter();
	CQinJiaModule* getQinjiaModule();
	void update(float fdeta );
protected:
	bool onLogicMsg(stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID)override;
	bool onLogicMsg( Json::Value& recvValue , uint16_t nmsgType, eMsgPort eSenderPort , uint32_t nSessionID )override ;
	bool onAsyncRequest(uint16_t nRequestType , const Json::Value& jsReqContent, Json::Value& jsResult )override ;
	uint16_t getLocalSvrMsgPortType(){ return ID_MSG_PORT_DATA ; } ; // et : ID_MSG_PORT_DATA , ID_MSG_PORT_TAXAS
	IGlobalModule* createModule( uint16_t eModuleType );
protected:
	bool ProcessPublicMsg( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID );
	void CheckNewDay();
public:
	static CGameServerApp* s_GameServerApp ;
protected:
	CConfigManager* m_pConfigManager ;
	/*CPokerCircle m_tPokerCircle ;*/
	// check NewDay ;
	unsigned int m_nCurDay ;
};