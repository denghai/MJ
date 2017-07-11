#pragma once
#include "IGlobalModule.h"
#include "TaskPool.h"
#include "httpRequest.h"
class CQinJiaModule
	: public IGlobalModule
	,public ITaskFactory
{
public:
	typedef std::function<void (Json::Value& resultData , Json::Value& jsUserData)> lpQinjiaRequestCallBack ;
public:
	void init( IServerApp* svrApp )override ;
	void onExit()override;
	void update(float fDeta )override ;
	ITask::ITaskPrt createTask( uint32_t nTaskID )override ;
	void sendQinJiaRequest(const char* pApi,Json::Value& jsReqData ,lpQinjiaRequestCallBack lpFunc , Json::Value jsUserData );
protected:
	CTaskPool& getTaskPool(){ return m_tTaskPool ;}
protected:
	CTaskPool m_tTaskPool ;
};

class CQinjiaTask
	:public ITask
	,public CHttpRequestDelegate
{
public:
	CQinjiaTask( uint32_t nTaskID );
	uint8_t performTask()override ;
	void onHttpCallBack(char* pResultData, size_t nDatalen , void* pUserData , size_t nUserTypeArg)override ;
	void reset()override;
	void setRequest( const char* pApi,Json::Value& jsReqData ,CQinJiaModule::lpQinjiaRequestCallBack lpFunc , Json::Value jsUserData );
protected:
	CHttpRequest m_tHttpRequest ;
	std::string m_strApi ;
	Json::Value m_jsReqData , m_jsResultData;
	CQinJiaModule::lpQinjiaRequestCallBack m_lpCallBack ;
	Json::Value m_jsUserData ;
};