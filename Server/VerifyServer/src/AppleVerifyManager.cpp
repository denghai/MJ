#include "AppleVerifyManager.h"
#include <json/json.h>
size_t OnRecieveData(void *buffer, size_t size, size_t count, void *user_p)
{
	stVerifyRequest* pRequest = (stVerifyRequest*)user_p ;
	CAppleVerifyManager* pAVM = (CAppleVerifyManager*)pRequest->pUserData ;
	assert(pRequest&& "must not null") ;
	Json::Reader reader;
	Json::Value rootValue ;
	static char pTempBuffer[1024] = { 0 } ;
	memset(pTempBuffer,0,sizeof(pTempBuffer)) ;
	memcpy(pTempBuffer,buffer,size*count);
	bool bCheckOk = false ;
	if ( reader.parse(pTempBuffer,rootValue) )
	{
		bCheckOk = rootValue["status"].asInt() == 0 ;
	}
	else
	{
		//printf("Err:Paser Received buffer \n") ;
		//pRequest->eResult = eVerify_Apple_Error ;
		//pAVM->AddProcessedResult(pRequest);
		return size * count ;
	}

	if ( bCheckOk )
	{
		Json::Value receipt = rootValue["receipt"] ;
		memset(pRequest->pBufferVerifyID,0,sizeof(pRequest->pBufferVerifyID)) ;
		sprintf(pRequest->pBufferVerifyID,"%s",receipt["transaction_id"].asCString());
	}
	pRequest->eResult = bCheckOk ? eVerify_Apple_Success : eVerify_Apple_Error;
	pAVM->AddProcessedResult(pRequest);
	return size * count ;
}

CAppleVerifyManager::CAppleVerifyManager()
{
	m_bRunning = false ;
	m_pCurlList = NULL ;
}

CAppleVerifyManager::~CAppleVerifyManager()
{
	// clear finisehd ;
	mFinishedLock.Lock() ;
	m_vFinishedQuest.clear() ;
	mFinishedLock.Unlock() ;
	
	// clear wait to processed request ;
	mRequestLock.Lock() ;
	m_vQuestToVerify.clear();
	mRequestLock.Unlock() ;

	if ( m_pCurlList )
	{
		curl_slist_free_all(m_pCurlList) ;
	}

	if ( m_pCURL )
	{
		curl_easy_cleanup(m_pCURL);
		m_pCURL = NULL ;
	}

	curl_global_cleanup();
}

void CAppleVerifyManager::Init()
{
	curl_global_init(CURL_GLOBAL_DEFAULT);
	m_pCURL = curl_easy_init();
#ifndef NDEBUG
	curl_easy_setopt(m_pCURL,CURLOPT_URL,"https://sandbox.itunes.apple.com/verifyReceipt");
#else
	curl_easy_setopt(m_pCURL,CURLOPT_URL,"https://buy.itunes.apple.com/verifyReceipt");
#endif
	//curl_easy_setopt(m_pCURL,CURLOPT_URL,"https://sandbox.itunes.apple.com/verifyReceipt");
	curl_easy_setopt(m_pCURL,CURLOPT_SSL_VERIFYPEER,0L);
	curl_easy_setopt(m_pCURL,CURLOPT_SSL_VERIFYHOST,0L);
	curl_easy_setopt(m_pCURL,CURLOPT_WRITEFUNCTION,&OnRecieveData );
#ifndef NDEBUG
	curl_easy_setopt(m_pCURL,CURLOPT_VERBOSE,1L) ;
#endif
	curl_easy_setopt(m_pCURL,CURLOPT_POST,1L) ;
	m_pCurlList = curl_slist_append(m_pCurlList,"Content-Type:application/json") ;
	CURLcode urlRet ;
	//urlRet = curl_easy_perform(m_pCURL) ;
	m_bRunning = true ;
	Start();
}

void CAppleVerifyManager::__run()
{
	while ( m_bRunning )
	{
		ProcessQuest() ;
		Sleep(2);
	}
}

void CAppleVerifyManager::AddRequest(stVerifyRequest* pRequest )
{
	mRequestLock.Lock() ;
	m_vQuestToVerify.push_back(pRequest) ;
	mRequestLock.Unlock() ;
}

stVerifyRequest* CAppleVerifyManager::GetProcessedRequest(LIST_VERIFY_REQUEST& vResult )
{
	mFinishedLock.Lock();
	vResult.swap(m_vFinishedQuest) ;
	mFinishedLock.Unlock() ;
	LIST_VERIFY_REQUEST::iterator iter = vResult.begin() ;
	if ( iter != vResult.end() )
	{
		stVerifyRequest* pReq = *iter ;
		return pReq;
	}
	return NULL ;
}

void CAppleVerifyManager::ProcessQuest()
{
	stVerifyRequest* pRequest = NULL ;
	mRequestLock.Lock() ;
	if ( m_vQuestToVerify.empty() == false )
	{
		LIST_VERIFY_REQUEST::iterator iter = m_vQuestToVerify.begin() ;
		pRequest = *iter ;
		m_vQuestToVerify.erase(iter) ;
	}
	mRequestLock.Unlock() ;
	if ( pRequest == NULL )
	{
		return ;
	}
	pRequest->pUserData = this ;
	// processed this requested ;
	Json::FastWriter jWriter ;
	Json::Value jRootValue ;

	jRootValue["receipt-data"]=pRequest->pBufferVerifyID ;
	std::string strFinal = jWriter.write(jRootValue) ;
	static char pBuffer [200] = { 0 } ;
	memset(pBuffer,0 , sizeof(pBuffer));
	sprintf(pBuffer,"Content-Length:%d",strlen(strFinal.c_str()));

	CURLcode urlRet ;
	curl_slist_append(m_pCurlList,pBuffer);
	curl_easy_setopt(m_pCURL,CURLOPT_HEADER,m_pCurlList) ; 
	curl_easy_setopt(m_pCURL,CURLOPT_POSTFIELDS,strFinal.c_str() );
	curl_easy_setopt(m_pCURL,CURLOPT_POSTFIELDSIZE,strlen(strFinal.c_str()));
	curl_easy_setopt(m_pCURL,CURLOPT_WRITEDATA,pRequest);
	urlRet = curl_easy_setopt(m_pCURL, CURLOPT_HEADER, 0L);
	urlRet = curl_easy_perform(m_pCURL) ;
	if ( urlRet != CURLE_OK )
	{
		// Request Error ;
		printf("Curl Connected Error !\n") ;
	}
}

void CAppleVerifyManager::AddProcessedResult(stVerifyRequest* pResult )
{
	mFinishedLock.Lock();
	m_vFinishedQuest.push_back(pResult) ;
	mFinishedLock.Unlock() ;
}