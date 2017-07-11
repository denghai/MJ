#include "WeChatOrderTask.h"
#include "tinyxml/tinyxml.h"
#include "Md5.h"
#include<algorithm>
#include "VerifyRequest.h"
#include "log4z.h"
#include "ConfigDefine.h"
#include "CommonDefine.h"
CWeChatOrderTask::CWeChatOrderTask( uint32_t nTaskID)
	:ITask(nTaskID)
{
	m_ptrCurRequest = nullptr ;
	m_tHttpRequest.init("https://api.mch.weixin.qq.com/pay/unifiedorder","text/xml");
	m_tHttpRequest.setDelegate(this);
}

uint8_t CWeChatOrderTask::performTask()
{
	if ( m_ptrCurRequest == nullptr )
	{
		printf("cur request is null  how to do work \n");
		return 0 ;
	}

	auto pRequest = m_ptrCurRequest;
#ifdef GAME_IN_REVIEW
	pRequest->nPrize = 1 ;
	LOGFMTE("temp set prize = 1 ") ;
#endif
	auto appID = Wechat_appID;
	auto mchID = Wechat_MchID;
	auto mchKey = Wechat_MchKey;
	if ( ePay_WeChat_365Golden == pRequest->nChannel)
	{
		appID = "wxae3a38cb9960bc84";
		mchID = "1385365702";
		mchKey = "NUN5DKS5MJW4UBVJIL1G2XUQ66LU2ENA";
	}
	TiXmlElement *xmlRoot = new TiXmlElement("xml"); 

	TiXmlElement *pNode = new TiXmlElement("appid"); 
	TiXmlText *pValue = new TiXmlText(appID);
	xmlRoot->LinkEndChild(pNode);
	pNode->LinkEndChild(pValue);
	std::string strForMd5 = "appid=";
	strForMd5 += appID;
	// ok 
	pNode = new TiXmlElement("body"); 
	pValue = new TiXmlText( pRequest->cShopDesc );
	xmlRoot->LinkEndChild(pNode);
	pNode->LinkEndChild(pValue);
	strForMd5 += "&body=" ;
	strForMd5.append(pRequest->cShopDesc);

	// ok 
	pNode = new TiXmlElement("mch_id"); 
	pValue = new TiXmlText(mchID);
	xmlRoot->LinkEndChild(pNode);
	pNode->LinkEndChild(pValue);
	strForMd5 += "&mch_id=" ;
	strForMd5 += mchID;

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
#ifdef _DEBUG
	printf("rand str: %s\n",strRandString.c_str()) ;
#endif 
	// ok 
	pNode = new TiXmlElement("notify_url"); 
	pValue = new TiXmlText(Wechat_notifyUrl);
	xmlRoot->LinkEndChild(pNode);
	pNode->LinkEndChild(pValue);
	strForMd5 += "&notify_url=" ;
	strForMd5 += Wechat_notifyUrl;

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

	// st to 
	//std::string strAppKey = "5fc4164aee7ba3360452c4e0e5d02d9d";
	//transform(strAppKey.begin(), strAppKey.end(), strAppKey.begin(),  toupper);
	// and key value 
	strForMd5 += "&key=" ;
	strForMd5 += mchKey;
	//strForMd5 += strAppKey ;
#ifdef _DEBUG
	printf("formd5Str: %s\n",strForMd5.c_str());
#endif 
	CMD5 md5 ;
	md5.GenerateMD5((unsigned char*)strForMd5.c_str(),strlen(strForMd5.c_str())) ;
	std::string strSign = md5.ToString() ;
	transform(strSign.begin(), strSign.end(), strSign.begin(),  toupper);
	// not ok 
#ifdef _DEBUG
	printf("sing: %s\n",strSign.c_str());
#endif 
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
#ifdef _DEBUG
	printf("finsStr : %s\n",stringBuffer.c_str());
#endif

	if ( !m_tHttpRequest.performRequest(nullptr,stringBuffer.c_str(),strlen(stringBuffer.c_str()),nullptr) )
	{
		// Request Error ;
		printf("Curl Connected Error !\n") ;
	}

	delete xmlRoot ;
	xmlRoot = nullptr ;
	return 0 ;
}

void CWeChatOrderTask::onHttpCallBack(char* pResultData, size_t nDatalen , void* pUserData , size_t nUserTypeArg)
{
	stShopItemOrderRequest* pRequest = m_ptrCurRequest.get() ;

	assert(pRequest&& "must not null") ;

	TiXmlDocument t ;
	std::string str(pResultData,nDatalen);
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
					return  ;
				}
			}
		}
	}
	pRequest->nRet = 1 ;
	LOGFMTE("we chat order failed : %s \n",str.c_str()) ;
	t.SaveFile("reT.xml");
}

void CWeChatOrderTask::setInfo( std::shared_ptr<stShopItemOrderRequest> prequst )
{
	m_ptrCurRequest = prequst ;
}