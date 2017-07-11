#include "WeChatShopItemOrder.h"
#include "tinyxml/tinyxml.h"
#include "Md5.h"
#include<algorithm>
size_t OnRecieveDataOrder(void *buffer, size_t size, size_t count, void *user_p)
{
	stShopItemOrderRequest* pRequest = (stShopItemOrderRequest*)user_p ;
	CWechatShopItemOrder* pAVM = (CWechatShopItemOrder*)pRequest->pUserData ;
	assert(pRequest&& "must not null") ;

	TiXmlDocument t ;
	std::string str((char*)buffer,size * count);
	t.Parse(str.c_str(),0,TIXML_ENCODING_UTF8);
	TiXmlNode* pRoot = t.RootElement();
	if ( pRoot )
	{
		TiXmlElement* pValueParent = (TiXmlElement*)pRoot->FirstChild("return_code");
		if ( pValueParent )
		{
			TiXmlNode* pValue = pValueParent->FirstChild();
			if ( strcmp(pValue->Value(),"SUCCESS") == 0 )
			{
				TiXmlElement* pResultNode = (TiXmlElement*)pRoot->FirstChild("result_code");
				TiXmlNode* pRet = pResultNode->FirstChild();
				if ( strcmp(pRet->Value(),"SUCCESS") == 0 )
				{
					// success ;
					memset(pRequest->cPrepayId,0,sizeof(pRequest->cPrepayId));

					TiXmlElement* pPrePay = (TiXmlElement*)pRoot->FirstChild("prepay_id");
					TiXmlNode* pPayidValue = pPrePay->FirstChild();
					sprintf_s(pRequest->cPrepayId,sizeof(pRequest->cPrepayId),"%s",pPayidValue->Value());
					pRequest->nRet = 0 ;
					pAVM->AddProcessedResult(pRequest);
					return size * count ;
				}
			}
		}
	}
	pRequest->nRet = 1 ;
	pAVM->AddProcessedResult(pRequest);
	printf("we chat order failed \n") ;
	t.SaveFile("reT.xml");
	return size * count ;
}

CWechatShopItemOrder::CWechatShopItemOrder()
{
	m_bRunning = false ;
	m_pCurlList = NULL ;
}

CWechatShopItemOrder::~CWechatShopItemOrder()
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

void CWechatShopItemOrder::Init()
{
	curl_global_init(CURL_GLOBAL_DEFAULT);
	m_pCURL = curl_easy_init();
	curl_easy_setopt(m_pCURL,CURLOPT_URL,"https://api.mch.weixin.qq.com/pay/unifiedorder");
	curl_easy_setopt(m_pCURL,CURLOPT_SSL_VERIFYPEER,0L);
	curl_easy_setopt(m_pCURL,CURLOPT_SSL_VERIFYHOST,0L);
	curl_easy_setopt(m_pCURL,CURLOPT_WRITEFUNCTION,&OnRecieveDataOrder );
#ifndef NDEBUG
	curl_easy_setopt(m_pCURL,CURLOPT_VERBOSE,1L) ;
#endif
	curl_easy_setopt(m_pCURL,CURLOPT_POST,1L) ;
	m_pCurlList = curl_slist_append(m_pCurlList,"Content-Type:text/xml") ;
	//CURLcode urlRet ;
	//urlRet = curl_easy_perform(m_pCURL) ;
	m_bRunning = true ;
	Start();
}

void CWechatShopItemOrder::__run()
{
	while ( m_bRunning )
	{
		ProcessQuest() ;
		Sleep(2);
	}
}

void CWechatShopItemOrder::AddRequest(stShopItemOrderRequest* pRequest )
{
	mRequestLock.Lock() ;
	m_vQuestToVerify.push_back(pRequest) ;
	mRequestLock.Unlock() ;
}

stShopItemOrderRequest* CWechatShopItemOrder::GetProcessedRequest(LIST_ORDER_REQUEST& vResult )
{
	mFinishedLock.Lock();
	vResult.swap(m_vFinishedQuest) ;
	mFinishedLock.Unlock() ;
	LIST_ORDER_REQUEST::iterator iter = vResult.begin() ;
	if ( iter != vResult.end() )
	{
		stShopItemOrderRequest* pReq = *iter ;
		return pReq;
	}
	return NULL ;
}

void CWechatShopItemOrder::ProcessQuest()
{
	stShopItemOrderRequest* pRequest = NULL ;
	mRequestLock.Lock() ;
	if ( m_vQuestToVerify.empty() == false )
	{
		LIST_ORDER_REQUEST::iterator iter = m_vQuestToVerify.begin() ;
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

	TiXmlElement *xmlRoot = new TiXmlElement("xml"); 

	TiXmlElement *pNode = new TiXmlElement("appid"); 
	TiXmlText *pValue = new TiXmlText( "wx66f2837c43330a7b" );
	xmlRoot->LinkEndChild(pNode);
	pNode->LinkEndChild(pValue);
	std::string strForMd5 = "appid=wx66f2837c43330a7b";
	// ok 
	pNode = new TiXmlElement("body"); 
	pValue = new TiXmlText( pRequest->cShopDesc );
	xmlRoot->LinkEndChild(pNode);
	pNode->LinkEndChild(pValue);
	strForMd5 += "&body=" ;
	strForMd5.append(pRequest->cShopDesc);

	// ok 
	pNode = new TiXmlElement("mch_id"); 
	pValue = new TiXmlText( "1308480601" );
	xmlRoot->LinkEndChild(pNode);
	pNode->LinkEndChild(pValue);
	strForMd5 += "&mch_id=1308480601" ;

	// ok 
	// ok 
	std::string strRandString = "";
	uint8_t nCnt = 32; 
	while ( nCnt-- )
	{
		uint8_t n = rand() % 2 ;
		char tCh = '0' + rand() % 10 ;
		if ( n == 0 )
		{
			tCh = 'A' + rand() % 26 ;
		}
		strRandString.append(1,tCh) ;
	}

	pNode = new TiXmlElement("nonce_str"); 
	pValue = new TiXmlText( strRandString.c_str() );
	xmlRoot->LinkEndChild(pNode);
	pNode->LinkEndChild(pValue);
	strForMd5 += "&nonce_str=" ;
	strForMd5 += strRandString;
	printf("rand str: %s\n",strRandString.c_str()) ;
	// ok 
	pNode = new TiXmlElement("notify_url"); 
	pValue = new TiXmlText( "http://www.paiyouquan/pay.php");
	xmlRoot->LinkEndChild(pNode);
	pNode->LinkEndChild(pValue);
	strForMd5 += "&notify_url=http://www.paiyouquan/pay.php" ;

	// ok 
	pNode = new TiXmlElement("out_trade_no"); 
	pValue = new TiXmlText( pRequest->cOutTradeNo );
	xmlRoot->LinkEndChild(pNode);
	pNode->LinkEndChild(pValue);
	strForMd5 += "&out_trade_no=" ;
	strForMd5.append(pRequest->cOutTradeNo);

	// ok 
	pNode = new TiXmlElement("spbill_create_ip"); 
	pValue = new TiXmlText( pRequest->cTerminalIp );
	xmlRoot->LinkEndChild(pNode);
	pNode->LinkEndChild(pValue);
	strForMd5 += "&spbill_create_ip=" ;
	strForMd5.append(pRequest->cTerminalIp);

	static char pBuffer [200] = { 0 } ;
	// ok 
	memset(pBuffer,0,sizeof(pBuffer)) ;
	sprintf_s(pBuffer,sizeof(pBuffer),"%d",pRequest->nPrize) ;
	pNode = new TiXmlElement("total_fee"); 
	pValue = new TiXmlText( pBuffer );
	xmlRoot->LinkEndChild(pNode);
	pNode->LinkEndChild(pValue);
	strForMd5 += "&total_fee=" ;
	strForMd5.append(pBuffer);

	// ok 
	pNode = new TiXmlElement("trade_type"); 
	pValue = new TiXmlText( "APP" );
	xmlRoot->LinkEndChild(pNode);
	pNode->LinkEndChild(pValue);
	strForMd5 += "&trade_type=APP" ;

	// and key value 
	strForMd5 += "&key=E97ED2537D229C0E967042D2E7F1C936" ;
	printf("formd5Str: %s\n",strForMd5.c_str());
	CMD5 md5 ;
	md5.GenerateMD5((unsigned char*)strForMd5.c_str(),strlen(strForMd5.c_str())) ;
	std::string strSign = md5.ToString() ;
	transform(strSign.begin(), strSign.end(), strSign.begin(),  toupper);
	// not ok 
	printf("sing: %s\n",strSign.c_str());
	pNode = new TiXmlElement("sign"); 
	pValue = new TiXmlText( strSign.c_str() );
	xmlRoot->LinkEndChild(pNode);
	pNode->LinkEndChild(pValue);

	TiXmlPrinter printer; 
	xmlRoot->Accept( &printer ); 
	std::string stringBuffer = printer.CStr(); 
	//<xml>
	//	<appid>wx2421b1c4370ec43b</appid>
	//	<body>shopItem desc</body>
	//	<mch_id>10000100</mch_id>
	//	<nonce_str>1add1a30ac87aa2db72f57a2375d8fec</nonce_str>
	//	<sign>10000100</sign>
	//	<out_trade_no>1415659990</out_trade_no>
	//	<total_fee>1</total_fee>
	//	<spbill_create_ip>14.23.150.211</spbill_create_ip>
	//	<notify_url>http://wxpay.weixin.qq.com/pub_v2/pay/notify.v2.php</notify_url>
	//	<trade_type>APP</trade_type>
	//</xml>

	printf("finsStr : %s\n",stringBuffer.c_str());
	memset(pBuffer,0 , sizeof(pBuffer));
	sprintf(pBuffer,"Content-Length:%d",strlen(stringBuffer.c_str()));

	CURLcode urlRet ;
	curl_slist_append(m_pCurlList,pBuffer);
	curl_easy_setopt(m_pCURL,CURLOPT_HEADER,m_pCurlList) ; 
	curl_easy_setopt(m_pCURL,CURLOPT_POSTFIELDS,stringBuffer.c_str() );
	curl_easy_setopt(m_pCURL,CURLOPT_POSTFIELDSIZE,strlen(stringBuffer.c_str()));
	
	curl_easy_setopt(m_pCURL,CURLOPT_WRITEDATA,pRequest);
	urlRet = curl_easy_setopt(m_pCURL, CURLOPT_HEADER, 0L);
	urlRet = curl_easy_perform(m_pCURL) ;
	if ( urlRet != CURLE_OK )
	{
		// Request Error ;
		printf("Curl Connected Error !\n") ;
	}

	delete xmlRoot ;
	xmlRoot = nullptr ;
}

void CWechatShopItemOrder::AddProcessedResult(stShopItemOrderRequest* pResult )
{
	mFinishedLock.Lock();
	m_vFinishedQuest.push_back(pResult) ;
	mFinishedLock.Unlock() ;
}