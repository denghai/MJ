#pragma once
#include "ISeverApp.h"
#include "ServerConfig.h"
class CDBManager ;
class CDataBaseThread ;
class CDBServerApp
	:public IServerApp
{
public:
	CDBServerApp();
	~CDBServerApp();
	bool init();
	uint16_t getLocalSvrMsgPortType(){ return ID_MSG_PORT_DB ;}
	bool onLogicMsg( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID );
	void update(float fdeta );
	bool OnMessage( Packet* pMsg )override ;
	void onExit();
	CDataBaseThread* GetDBThread(){ return m_pDBWorkThread ; }
protected:
	CDBManager* m_pDBManager ;
	CDataBaseThread* m_pDBWorkThread ;

	// server config 
	CSeverConfigMgr m_stSvrConfigMgr ;
};