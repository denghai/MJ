#pragma once
#include "IVerifyTask.h"
#include "httpRequest.h"
class CWechatVerifyTask
	:public IVerifyTask
	,public CHttpRequestDelegate
{
public:
	CWechatVerifyTask( uint32_t nTaskID ) ;
	uint8_t performTask()override ;
	void onHttpCallBack(char* pResultData, size_t nDatalen , void* pUserData , size_t nUserTypeArg )override ;
protected:
	CHttpRequest m_tHttpRequest ;
};