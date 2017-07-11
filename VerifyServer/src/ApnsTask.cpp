#include "ApnsTask.h"
#include "log4z.h"
CApnsTask::CApnsTask(uint32_t nTaskID )
	:ITask(nTaskID)
{
	m_tHttpRequest.init("http://msg.umeng.com/api/send?sign=");
	m_tHttpRequest.setDelegate(this);
}

uint8_t CApnsTask::performTask()
{
	//android 
	apns("57bd7b5367e58ea6b300400c","wbiuhy9n0bwmayyyzrcrjdfp397lbglo",false);
	// ios
	apns("571a10e3e0f55a76560001f2","nwdkqjjsmxnvhakcr3ckoqpjyrzzigwj",true);
	return true ;
}

void CApnsTask::onHttpCallBack(char* pResultData, size_t nDatalen , void* pUserData , size_t nUserTypeArg)
{
	Json::Reader reader;
	Json::Value rootValue ;
	static char pTempBuffer[2024] = { 0 } ;
	memset(pTempBuffer,0,sizeof(pTempBuffer)) ;
	memcpy_s(pTempBuffer,sizeof(pTempBuffer),pResultData,nDatalen);
	if ( reader.parse(pTempBuffer,rootValue) )
	{
		auto ret = rootValue["ret"].asString() == "SUCCESS";
		if ( ret )
		{
			LOGFMTD("invoker ok \n") ;
		}
		else
		{
			auto p = rootValue["data"];
			LOGFMTD("error = %s \n", p["error_code"].asCString());
		}
	}
	else
	{
		LOGFMTD("parse json data error , from apple \n");
		return ;
	}

}

std::string CApnsTask::getMySign( const char* cBody,const char* pSvrKey )
{
	std::string strForMd = "POST" ;
	strForMd += "http://msg.umeng.com/api/send";
	strForMd += cBody ;
	strForMd += pSvrKey ;
	m_tMd5.GenerateMD5((unsigned char*)const_cast<char*>(strForMd.c_str()),strForMd.size());
	return m_tMd5.ToString();
}

void CApnsTask::setRequest( const Json::Value& jsmsgContent )
{
	m_jsRequest = jsmsgContent ;
}

void CApnsTask::apns(const char* pAppKey , const char* pSvrkey, bool isIOS )
{
	uint8_t nApnsType = m_jsRequest["apnsType"].asUInt();
	auto jsTargets = m_jsRequest["targets"];
	const char* pContent = m_jsRequest["content"].asCString() ;
	auto pmsgID = m_jsRequest["msgID"].asCString();
	auto pmsgDesc = m_jsRequest["msgdesc"].asCString() ;

	Json::Value jsApns ;
	jsApns["appkey"] = pAppKey;
	jsApns["timestamp"] = std::to_string((uint32_t)time(nullptr)) ;
	if ( 0 == nApnsType ) // tag group ;
	{
		jsApns["type"] = "groupcast" ;

		Json::Value jsFiler;
		jsFiler["tag"] = std::to_string(jsTargets[(uint32_t)0].asUInt());

		Json::Value jsOrArray ;
		jsOrArray[0u] = jsFiler;

		Json::Value jsorn ;
		jsorn["or"] = jsOrArray ;

		Json::Value jsAndArray ;
		jsAndArray[0u] = jsorn ;

		Json::Value jsWhere ;
		jsWhere["and"] = jsAndArray ;

		Json::Value jsFilerf ;
		jsFilerf["where"] = jsWhere ;
		jsApns["filter"] = jsFilerf;
	}
	else if ( 1 == nApnsType ) // alias players ;
	{
		jsApns["type"] = "customizedcast" ;
		jsApns["alias_type"] = "WEIXIN" ;
		std::string strList = std::to_string(jsTargets[(uint32_t)0].asUInt()) ;
		for ( uint32_t nIdx = 1 ; nIdx < jsTargets.size() ; ++nIdx )
		{
			strList.push_back(',');
			strList.append(std::to_string(jsTargets[nIdx].asUInt()));
		}
		jsApns["alias"] = strList ;
	}

	Json::Value jsPayLoad ;
	if ( isIOS )
	{
		getIOSPayload(jsPayLoad,pContent);
	}
	else
	{
		getAndriodPayload(jsPayLoad,pContent) ;
	}

	jsApns["payload"] = jsPayLoad;
	jsApns["description"] = pmsgDesc ;
	jsApns["thirdparty_id"] = pmsgID ;

	jsApns["production_mode"] = "true";

	Json::StyledWriter jsWriter ;
	auto jsString  = jsWriter.write(jsApns);
	auto strSign = getMySign(jsString.c_str(),pSvrkey);
	m_tHttpRequest.performRequest(strSign.c_str(),jsString.c_str(),jsString.size(),nullptr);
}

void CApnsTask::getIOSPayload( Json::Value& jsValueOut , const char* pContent )
{
	Json::Value js ;
	js["alert"] = pContent ;
	js["badge"] = 1 ;
	js["sound"] = "default" ;

	jsValueOut["aps"] = js ;
}

void CApnsTask::getAndriodPayload( Json::Value& jsValueOut , const char* pContent )
{
	jsValueOut["display_type"] = "notification" ;

	Json::Value jsBody ;
	jsBody["ticker"] = "ticker";
	jsBody["title"] = "PaiyouQuan";
	jsBody["text"] = pContent;
	jsValueOut["body"] = jsBody ;
}