#pragma once

#include "NetWorkManager.h"

#include "Timer.h"
#include "ServerConfig.h"
#include "MessageIdentifer.h"
#include "json/json.h"
struct stMsg ;
class CAsyncRequestQuene ;
class IGlobalModule ;
class IServerApp
	:CNetMessageDelegate
{
public:
	enum eDefaultModule
	{
		eDefMod_None,
		eDefMod_AsyncRequestQueu = eDefMod_None,
		eDefMod_ChildDef,
		eDefMod_Max = eDefMod_ChildDef ,
	};
public:
	IServerApp();
	virtual ~IServerApp();
	virtual bool init();
	virtual bool OnMessage( Packet* pMsg ) ;
	virtual bool OnLostSever(Packet* pMsg);
	virtual bool OnConnectStateChanged( eConnectState eSate, Packet* pMsg);
	bool run();
	void shutDown();
	bool sendMsg( const char* pBuffer , int nLen );
	bool sendMsg( uint32_t nSessionID , const char* pBuffer , uint16_t nLen, bool bBroadcast = false );
	bool sendMsg( uint32_t nSessionID , Json::Value& recvValue, uint16_t nMsgID = 0 ,uint8_t nTargetPort = ID_MSG_PORT_CLIENT,bool bBroadcast = false );
	void stop();
	virtual bool onLogicMsg( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID );
	virtual bool onLogicMsg( Json::Value& recvValue , uint16_t nmsgType, eMsgPort eSenderPort , uint32_t nSessionID );
	virtual bool onAsyncRequest(uint16_t nRequestType , const Json::Value& jsReqContent, Json::Value& jsResult );
	virtual void update(float fDeta );
	virtual uint16_t getLocalSvrMsgPortType() = 0 ; // et : ID_MSG_PORT_DATA , ID_MSG_PORT_TAXAS
	virtual uint16_t getTargetSvrPortType();
	bool isConnected();
	CTimerManager* getTimerMgr(){ return m_pTimerMgr ; }
	virtual void onExit();
	virtual void onConnectedToSvr();
	IGlobalModule* getModuleByType( uint16_t nType );
	CAsyncRequestQuene* getAsynReqQueue();
protected:
	bool installModule( uint16_t nModuleType );
	virtual IGlobalModule* createModule( uint16_t eModuleType );
	void setConnectServerConfig(stServerConfig* pConfig );
	void doConnectToTargetSvr();
	uint16_t getVerifyType(); // et:MSG_VERIFY_DATA ,MSG_VERIFY_TAXAS,MSG_VERIFY_LOGIN
	CNetWorkMgr* getNetwork(){ return m_pNetWork ;}
private:
	bool registerModule(IGlobalModule* pModule, uint16_t eModuleType );
private:
	bool m_bRunning;
	CONNECT_ID m_nTargetSvrNetworkID ;
	CNetWorkMgr::eConnectType m_eConnectState ;
	CTimerManager* m_pTimerMgr ;
	CNetWorkMgr* m_pNetWork ;

	stServerConfig m_stConnectConfig ;
	float m_fReconnectTick ;

	char m_pSendBuffer[MAX_MSG_BUFFER_LEN] ;

	std::map<uint16_t,IGlobalModule*> m_vAllModule ;

	uint32_t m_nFrameCnt;
	float m_fFrameTicket;
	float m_fOutputfpsTickt;
};