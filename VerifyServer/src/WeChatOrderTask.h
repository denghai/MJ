#pragma once
#include "httpRequest.h"
#include "ITask.h"
struct stShopItemOrderRequest ;
class CWeChatOrderTask
	:public ITask
	,public CHttpRequestDelegate
{
public:
	CWeChatOrderTask( uint32_t nTaskID );
	uint8_t performTask()override ;
	void onHttpCallBack(char* pResultData, size_t nDatalen , void* pUserData , size_t nUserTypeArg)override ;
	void setInfo( std::shared_ptr<stShopItemOrderRequest> prequst );
	std::shared_ptr<stShopItemOrderRequest> getCurRequest(){ return m_ptrCurRequest ;}
protected:
	CHttpRequest m_tHttpRequest ;
	std::shared_ptr<stShopItemOrderRequest> m_ptrCurRequest ;
};