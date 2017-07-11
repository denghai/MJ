#pragma once
#include "VerifyRequest.h"
#include "pthread.h"
#include "ThreadMod.h"
#include "mutex.h"
#include "curl/curl.h"
class CMiVerifyManager
	:public CThreadT
{
public:
	CMiVerifyManager();
	~CMiVerifyManager();
	void Init();
	void __run();
	void AddRequest(stVerifyRequest* pRequest );
	stVerifyRequest* GetProcessedRequest(LIST_VERIFY_REQUEST& vResult );
	void AddProcessedResult(stVerifyRequest* pResult );
protected:
	void ProcessQuest();
protected:
	Mutex mRequestLock ;
	Mutex mFinishedLock; 
	LIST_VERIFY_REQUEST m_vQuestToVerify ;
	LIST_VERIFY_REQUEST m_vFinishedQuest ;
	bool m_bRunning ;
	CURL* m_pCURL ;
	curl_slist* m_pCurlList ; 
};