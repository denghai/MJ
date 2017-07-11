#pragma once
#include "ServerCommon.h"
#include "json/json.h"
#include "Singleton.h"
#include "AutoBuffer.h"
class CAsyncRequestQuene ; ;
class CSendPushNotification
	:public CSingleton<CSendPushNotification>
{
public:
	CSendPushNotification(){}
	void reset();
	void addTarget(uint32_t nUserUID );
	void setContent(const char* pContent,uint32_t nFlag );
	void postApns( CAsyncRequestQuene* pAsync , bool isGroup,const char* pmsgID = "def" , const char* pmsgDesc = "def" );
protected:
	Json::Value m_arrayTargetIDs ;
	Json::Value m_strApns ;
};