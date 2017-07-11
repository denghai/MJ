#include "MiVerifyManager.h"
#include <json/json.h>
#include <HMAC_SHA1.h>
size_t OnRecieveDataMi(void *buffer, size_t size, size_t count, void *user_p)
{
	stVerifyRequest* pRequest = (stVerifyRequest*)user_p ;
	CMiVerifyManager* pAVM = (CMiVerifyManager*)pRequest->pUserData ;
	assert(pRequest&& "must not null") ;
	Json::Reader reader;
	Json::Value rootValue ;
	static char pTempBuffer[1024] = { 0 } ;
	memset(pTempBuffer,0,sizeof(pTempBuffer)) ;
	memcpy(pTempBuffer,buffer,size*count);
	bool bCheckOk = false ;
	if ( reader.parse(pTempBuffer,rootValue) )
	{
		if ( rootValue["orderStatus"].isNull() )
		{
			printf("xiao result errorcode = %d\n",rootValue["errcode"].asInt());
			return size * count ;
		}
		else
		{
			bCheckOk = strcmp(rootValue["orderStatus"].asCString(),"TRADE_SUCCESS") == 0 ;
		}
		
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
		memset(pRequest->pBufferVerifyID,0,sizeof(pRequest->pBufferVerifyID)) ;
		sprintf(pRequest->pBufferVerifyID,"%s",rootValue["orderId"].asCString());
	}
	pRequest->eResult = bCheckOk ? eVerify_Apple_Success : eVerify_Apple_Error;
	pAVM->AddProcessedResult(pRequest);
	return size * count ;
}

CMiVerifyManager::CMiVerifyManager()
{
	m_bRunning = false ;
	m_pCurlList = NULL ;
}

CMiVerifyManager::~CMiVerifyManager()
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

void CMiVerifyManager::Init()
{
	curl_global_init(CURL_GLOBAL_DEFAULT);
	m_pCURL = curl_easy_init();
//#ifndef NDEBUG
	curl_easy_setopt(m_pCURL,CURLOPT_URL,"https://sandbox.itunes.apple.com/verifyReceipt");
//#else
//	curl_easy_setopt(m_pCURL,CURLOPT_URL,"https://buy.itunes.apple.com/verifyReceipt");
//#endif
	curl_easy_setopt(m_pCURL,CURLOPT_SSL_VERIFYPEER,0L);
	curl_easy_setopt(m_pCURL,CURLOPT_SSL_VERIFYHOST,0L);
	curl_easy_setopt(m_pCURL,CURLOPT_WRITEFUNCTION,&OnRecieveDataMi );
#ifndef NDEBUG
	curl_easy_setopt(m_pCURL,CURLOPT_VERBOSE,1L) ;
#endif
	//curl_easy_setopt(m_pCURL,CURLOPT_POST,1L) ;
	//m_pCurlList = curl_slist_append(m_pCurlList,"Content-Type:application/json") ;
	//CURLcode urlRet ;
	//urlRet = curl_easy_perform(m_pCURL) ;
	m_bRunning = true ;
	Start();
}

void CMiVerifyManager::__run()
{
	while ( m_bRunning )
	{
		ProcessQuest() ;
		Sleep(2);
	}
}

void CMiVerifyManager::AddRequest(stVerifyRequest* pRequest )
{
	mRequestLock.Lock() ;
	m_vQuestToVerify.push_back(pRequest) ;
	mRequestLock.Unlock() ;
}

stVerifyRequest* CMiVerifyManager::GetProcessedRequest(LIST_VERIFY_REQUEST& vResult )
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

void CMiVerifyManager::ProcessQuest()
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
	// http://mis.migc.xiaomi.com/api/biz/service/queryOrder.do?appId=2882303761517239138&cpOrderId=9786bffc-996d-4553-aa33-f7e92c0b29d5&uid=100010&signature=e91d68b864b272b6ec03f35ee5a640423d01a0a1
	
	char pCrypeBuffer[1024] = {0} ;
	sprintf_s(pCrypeBuffer,"appId=2882303761517415923&cpOrderId=%s&uid=%u",pRequest->pBufferVerifyID,pRequest->nMiUserUID) ;
	CHMAC_SHA1 crype ;
	char* key = "bKgKJPGrgjQlWFQBJpTOhw==";
	std::string strSign = crype.HMAC_SHA1((BYTE*)pCrypeBuffer,strlen(pCrypeBuffer),(BYTE*)key,strlen(key)); 
	std::string strURL = "http://mis.migc.xiaomi.com/api/biz/service/queryOrder.do?" ;
	strURL.append(pCrypeBuffer);
	strURL.append("&signature=");
	strURL += strSign ;
	curl_easy_setopt(m_pCURL,CURLOPT_URL,strURL.c_str());


 	CURLcode urlRet ;
// 	curl_slist_append(m_pCurlList,pBuffer);
// 	curl_easy_setopt(m_pCURL,CURLOPT_HEADER,m_pCurlList) ; 
// 	curl_easy_setopt(m_pCURL,CURLOPT_POSTFIELDS,strFinal.c_str() );
// 	curl_easy_setopt(m_pCURL,CURLOPT_POSTFIELDSIZE,strlen(strFinal.c_str()));
	curl_easy_setopt(m_pCURL,CURLOPT_WRITEDATA,pRequest);
	urlRet = curl_easy_setopt(m_pCURL, CURLOPT_HEADER, 0L);
	urlRet = curl_easy_perform(m_pCURL) ;
	if ( urlRet != CURLE_OK )
	{
		// Request Error ;
		printf("Curl Connected Error !\n") ;
	}
}

void CMiVerifyManager::AddProcessedResult(stVerifyRequest* pResult )
{
	mFinishedLock.Lock();
	m_vFinishedQuest.push_back(pResult) ;
	mFinishedLock.Unlock() ;
}