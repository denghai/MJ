#pragma once 
#include "ISeverApp.h"
#include "Timer.h"
#include "ServerConfig.h"
class CDBManager ;
class CDataBaseThread ;
class CLoginApp
	:public IServerApp
{
public:
	CLoginApp();
	~CLoginApp();
	bool init();
	void update(float fdeta );
	bool onLogicMsg( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID )override;
	bool onLogicMsg( Json::Value& recvValue , uint16_t nmsgType, eMsgPort eSenderPort , uint32_t nSessionID )override ;
	void onExit();
	uint16_t getLocalSvrMsgPortType(){ return ID_MSG_PORT_LOGIN ;}
protected:
	CDataBaseThread* m_pDBThread;
	CDBManager* m_pDBMgr ;
	CSeverConfigMgr m_stSvrConfigMgr ;
};