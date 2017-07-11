#include "AnyLoginTask.h"
#include <json/json.h>
#include "VerifyRequest.h"
#include <cassert>
#include "log4z.h"
#include "ConfigDefine.h"
AnyLoginTask::AnyLoginTask(uint32_t nTaskID)
	:ITask(nTaskID)
{
	m_tHttpRequest.init("http://oauth.anysdk.com/api/User/LoginOauth/");
	//#if defined(GAME_IN_REVIEW)
	//#endif 
	m_tHttpRequest.setDelegate(this);
}

uint8_t AnyLoginTask::performTask()
{
	if (m_strTransferString.empty())
	{
		return 1;
	}
	auto ret = m_tHttpRequest.performRequest(nullptr, m_strTransferString.c_str(), m_strTransferString.size(), nullptr);
	if (ret)
	{
		return 0;
	}
	return 1;
}

void AnyLoginTask::onHttpCallBack(char* pResultData, size_t nDatalen, void* pUserData, size_t nUserTypeArg)
{
	assert(pResultData != nullptr && "must not null");
	Json::Reader reader;
	m_jsResult.clear();	
	if (reader.parse(pResultData, pResultData + nDatalen, m_jsResult))
	{
		 
	}
	else
	{
		printf("parse json data error , from AnySDK Svr \n");
		return;
	}
	return;
}
