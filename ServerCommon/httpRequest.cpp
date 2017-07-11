#include "httpRequest.h"
#include "log4z.h"
CHttpRequest::~CHttpRequest()
{
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

bool CHttpRequest::init(const char* pBaseUrl,const char* pContentType )
{
	m_pCurlList = nullptr;
	m_pCURL = nullptr;
	m_strBaseURL = pBaseUrl ;
	curl_global_init(CURL_GLOBAL_DEFAULT);
	m_pCURL = curl_easy_init();
	curl_easy_setopt(m_pCURL,CURLOPT_SSL_VERIFYPEER,0L);
	curl_easy_setopt(m_pCURL,CURLOPT_SSL_VERIFYHOST,0L);
	curl_easy_setopt(m_pCURL,CURLOPT_WRITEFUNCTION,&CHttpRequest::onRecieveData );
	curl_easy_setopt(m_pCURL,CURLOPT_WRITEDATA,this);
//#ifndef NDEBUG
//	curl_easy_setopt(m_pCURL,CURLOPT_VERBOSE,1L) ;   
//#endif
	curl_easy_setopt(m_pCURL,CURLOPT_POST,1L) ;

	std::string strCT = "Content-Type:" ;
	std::string strAcpt = "Accept:";
	strCT += pContentType ;
	strAcpt += pContentType ;
	m_pCurlList = curl_slist_append(m_pCurlList,strCT.c_str()) ;
	curl_slist_append(m_pCurlList,strAcpt.c_str()) ;
	return true ;
}

void CHttpRequest::setHead(const char* pHeadStr)
{
	m_pCurlList = curl_slist_append(m_pCurlList,pHeadStr) ;
}

bool CHttpRequest::performRequest(const char* pAddtionUrl , const char* pData , size_t nLen, void* pUserData, size_t nUserTypeArg )
{
	m_pUserData = pUserData ;
	m_nUserTypeArg = nUserTypeArg;
	static char pBuffer [512] = { 0 } ;
	memset(pBuffer,0 , sizeof(pBuffer));
	sprintf_s(pBuffer,sizeof(pBuffer),"Content-Length:%d",nLen);
	CURLcode urlRet ;
	curl_slist_append(m_pCurlList,pBuffer);
	curl_easy_setopt(m_pCURL,CURLOPT_HEADER,m_pCurlList) ; 
	curl_easy_setopt(m_pCURL,CURLOPT_POSTFIELDS,pData );
	curl_easy_setopt(m_pCURL,CURLOPT_POSTFIELDSIZE,nLen);
	urlRet = curl_easy_setopt(m_pCURL, CURLOPT_HEADER, 0L);

	std::string strUrl = m_strBaseURL ;
	if ( pAddtionUrl )
	{
		strUrl += pAddtionUrl ;
	}

	curl_easy_setopt(m_pCURL,CURLOPT_URL,strUrl.c_str()); 
	urlRet = curl_easy_perform(m_pCURL) ;
	if ( urlRet != CURLE_OK )
	{
		// Request Error ;
		printf("Curl Connected Error !\n") ;
	}
	return urlRet == CURLE_OK ;
}

void CHttpRequest::setDelegate(CHttpRequestDelegate* pDelegate)
{
	m_pDelegate = pDelegate ;
}

size_t CHttpRequest::onRecieveData(void *buffer, size_t size, size_t count, void *user_p)
{
	CHttpRequest* pRequest = (CHttpRequest*)user_p ;
	if ( pRequest->m_pDelegate )
	{
		pRequest->m_pDelegate->onHttpCallBack((char*)buffer,size*count,pRequest->m_pUserData ,pRequest->m_nUserTypeArg) ;
	}
	else
	{
		LOGFMTE("why htttp request call back delegate is null ") ;	
	}
	return size * count ;
}