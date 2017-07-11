#pragma once
#include "ITask.h"
#include "httpRequest.h"
#include "json/json.h"
class AnyLoginTask
	:public ITask
	, public CHttpRequestDelegate
{
public:
	AnyLoginTask(uint32_t nTaskID);
	uint8_t performTask()override;
	void onHttpCallBack(char* pResultData, size_t nDatalen, void* pUserData, size_t nUserTypeArg)override;
	void setReqString(std::string& strReqContent){ m_strTransferString = strReqContent; }
	Json::Value& getResultJson(){ return m_jsResult; }
protected:
	CHttpRequest m_tHttpRequest;
	std::string m_strTransferString;
	Json::Value m_jsResult;
};
