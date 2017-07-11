#pragma once
#include "ISeverApp.h"
#include "ServerConfig.h"
class CDBManager ;
class CDataBaseThread ;
class CLogSvrApp
	:public IServerApp
{
public:
	CLogSvrApp();
	~CLogSvrApp();
	bool init();
	void update(float fDeta );
	// net delegate
	virtual bool onLogicMsg( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID );
	void onExit();
	CDataBaseThread* GetDBThread(){ return m_pDBWorkThread ; }
	uint16_t getLocalSvrMsgPortType();
protected:
	CDBManager* m_pDBManager ;
	CDataBaseThread* m_pDBWorkThread ;
};