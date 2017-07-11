#pragma once
#include "json/json.h"
#include "NativeTypes.h"
#include <functional>
#include "IGlobalModule.h"
#include "ServerMessageDefine.h"
#include <list>
#include "Timer.h"
#define async_request_call_back_bind(obj,func) std::bind(&func,obj,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)
typedef std::function<void ( uint16_t nReqType ,const Json::Value& retContent,Json::Value& jsUserData ) >  async_req_call_back_func ;
class CAsyncRequestQuene
	:public IGlobalModule
{
public:
	struct stAsyncRequest
	{
		uint32_t nReqSerialNum ;
		uint8_t nTargetPortID ;
		uint16_t nReqType ;
		Json::Value jsReqContent ;
		async_req_call_back_func lpCallBack ;
		Json::Value jsUserData ;

		time_t tLastSend ;
		uint16_t nSendTimes ;
	};

	typedef std::map<uint32_t,stAsyncRequest*> MAP_SERIAL_NUM_REQ ;
	typedef std::list<stAsyncRequest*> LIST_ASYNC_REQ ;
public:
	void init( IServerApp* svrApp )override ;
	bool onMsg(stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID)override;
	~CAsyncRequestQuene();
	uint32_t pushAsyncRequest(uint8_t nTargetPortID, uint16_t nReqType,Json::Value& reqContent, async_req_call_back_func lpCallBack,Json::Value& jsUserData );
	uint32_t pushAsyncRequest(uint8_t nTargetPortID, uint16_t nReqType,Json::Value& reqContent, async_req_call_back_func lpCallBack);
	uint32_t pushAsyncRequest(uint8_t nTargetPortID, uint16_t nReqType,Json::Value& reqContent );
	bool canncelAsyncRequest( uint32_t nReqSerialNum );
	void timerCheckReqState(CTimer* pTimer, float fTick );
protected:
	void sendAsyncRequest(stAsyncRequest* pReq);
	stAsyncRequest* getReuseAsyncReqObject();
protected:
	MAP_SERIAL_NUM_REQ m_mapRunningRequest ;
	LIST_ASYNC_REQ m_vReserverReqObject ;
	uint32_t m_nReqSerailNum ;
	CTimer m_tCheckReqStateTimer ;
};