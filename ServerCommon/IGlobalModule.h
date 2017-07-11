#pragma once
#include "MessageDefine.h"
#include "json/json.h"
class IServerApp ;
class IGlobalModule
{
public:
	enum 
	{
		INVALID_MODULE_TYPE = (uint16_t)-1,
	};

public:
	IGlobalModule(){ m_fTicket = 300; m_app = nullptr ; m_nModuleType = INVALID_MODULE_TYPE ; }
	virtual ~IGlobalModule(){}
	IServerApp* getSvrApp(){ return m_app; }
protected:
	void setModuleType( uint8_t nModuleType ){ m_nModuleType = nModuleType ; }
	uint16_t getModuleType(){ return m_nModuleType ; };
	virtual void init( IServerApp* svrApp ) { m_app = svrApp ; m_fTicket = getTimeSave();}
	virtual void onExit(){ onTimeSave() ;}
	virtual bool onMsg(stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID){ return false ;}
	virtual bool onMsg(Json::Value& prealMsg ,uint16_t nMsgType, eMsgPort eSenderPort , uint32_t nSessionID){ return false ;}
	virtual bool onAsyncRequest(uint16_t nRequestType , const Json::Value& jsReqContent, Json::Value& jsResult ){ return false ;};
	virtual void update(float fDeta )
	{
		m_fTicket -= fDeta ;
		if ( m_fTicket < 0 )
		{
			m_fTicket = getTimeSave();
			onTimeSave() ;
		}
	}
	virtual void onTimeSave(){}
	virtual void onConnectedSvr(){}
	virtual float getTimeSave(){ return 650; }
public:
	friend IServerApp ;
private:
	uint16_t m_nModuleType ;
	IServerApp* m_app ;
	float m_fTicket ;
};