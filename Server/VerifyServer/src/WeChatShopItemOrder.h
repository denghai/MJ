#pragma once
#include "VerifyRequest.h"
#include "pthread.h"
#include "ThreadMod.h"
#include "mutex.h"
#include "curl/curl.h"
class CWechatShopItemOrder
	:public CThreadT
{
public:
	CWechatShopItemOrder();
	~CWechatShopItemOrder();
	void Init();
	void __run();
	void AddRequest(stShopItemOrderRequest* pRequest );
	stShopItemOrderRequest* GetProcessedRequest(LIST_ORDER_REQUEST& vResult );
	void AddProcessedResult(stShopItemOrderRequest* pResult );
protected:
	void ProcessQuest();
protected:
	Mutex mRequestLock ;
	Mutex mFinishedLock; 
	LIST_ORDER_REQUEST m_vQuestToVerify ;
	LIST_ORDER_REQUEST m_vFinishedQuest ;
	bool m_bRunning ;
	CURL* m_pCURL ;
	curl_slist* m_pCurlList ; 
};