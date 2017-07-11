#include "ITask.h"
#include "httpRequest.h"
#include "json/json.h"
#include "Md5.h"
class CApnsTask
	:public ITask
	,public CHttpRequestDelegate
{
public:
	CApnsTask(uint32_t nTaskID ) ;
	uint8_t performTask()override ;
	void onHttpCallBack(char* pResultData, size_t nDatalen , void* pUserData , size_t nUserTypeArg)override;
	void setRequest( const Json::Value& jsmsgContent );
protected:
	std::string getMySign( const char* cBody,const char* pSvrKey );
	void apns(const char* pAppKey , const char* pSvrkey,bool isIos );
	void getIOSPayload( Json::Value& jsValueOut , const char* pContent );
	void getAndriodPayload( Json::Value& jsValueOut , const char* pContent );
protected:
	Json::Value m_jsRequest ;
	CHttpRequest m_tHttpRequest ;
	CMD5 m_tMd5 ;
};