#pragma once
#include "ISeverApp.h"
#include "ServerConfig.h"
#include "PushNotificationThread.h"
#include "ServerConfig.h"
#include "Singleton.h"
#include "NoticePlayerManager.h"
class CPushNotificationServer
	:public IServerApp
	,public CSingleton<CPushNotificationServer>
{
public:
	~CPushNotificationServer();
	bool init();
	bool onLogicMsg( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID );
	uint16_t getLocalSvrMsgPortType(){ return ID_MSG_PORT_APNS ;}
	void onExit()override{ m_nPushThread.StopServer();}
protected:
	CPushNotificationThread m_nPushThread ;
	CSeverConfigMgr m_stSvrConfigMgr ;
	CNoticePlayerManager m_tNoticePlayerMgr ;
};