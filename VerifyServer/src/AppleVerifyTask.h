#pragma once
#include "httpRequest.h"
#include "IVerifyTask.h"
struct stVerifyRequest ;
class CAppleVerifyTask
	:public IVerifyTask
	,public CHttpRequestDelegate
{
public:
	typedef std::shared_ptr<stVerifyRequest> VERIFY_REQUEST_ptr ;
public:
	CAppleVerifyTask( uint32_t nTaskID ) ;
	uint8_t performTask()override ;
	void onHttpCallBack(char* pResultData, size_t nDatalen , void* pUserData , size_t nUserTypeArg )override ;
protected:
	CHttpRequest m_tHttpRequest ;
};