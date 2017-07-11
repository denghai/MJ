#include "WeChatVerifyTask.h"
#include "VerifyRequest.h"
#include <cassert>
#include "tinyxml/tinyxml.h"
#include "Md5.h"
#include<algorithm>
#include "log4z.h"
#include "ConfigDefine.h"
CWechatVerifyTask::CWechatVerifyTask( uint32_t nTaskID ) 
	:IVerifyTask(nTaskID)
{
	m_tHttpRequest.init("https://api.mch.weixin.qq.com/pay/orderquery","text/xml");
	m_tHttpRequest.setDelegate(this);
}

uint8_t CWechatVerifyTask::performTask()
{
	auto pRequest = getVerifyResult() ; 
	TiXmlElement *xmlRoot = new TiXmlElement("xml"); 

	TiXmlElement *pNode = new TiXmlElement("appid"); 
	TiXmlText *pValue = new TiXmlText(Wechat_appID);
	xmlRoot->LinkEndChild(pNode);
	pNode->LinkEndChild(pValue);
	std::string strForMd5 = "appid=";
	strForMd5 += Wechat_appID;

	// ok 
	pNode = new TiXmlElement("mch_id");
	pValue = new TiXmlText(Wechat_MchID);
	xmlRoot->LinkEndChild(pNode);
	pNode->LinkEndChild(pValue);
	strForMd5 += "&mch_id=";
	strForMd5 += Wechat_MchID;

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
	pNode = new TiXmlElement("out_trade_no"); 
	pValue = new TiXmlText( pRequest->pBufferVerifyID );
	xmlRoot->LinkEndChild(pNode);
	pNode->LinkEndChild(pValue);
	strForMd5 += "&out_trade_no=" ;
	strForMd5.append(pRequest->pBufferVerifyID);


	static char pBuffer [200] = { 0 } ;
	// and key value 
	strForMd5 += "&key=" ;
	strForMd5 += Wechat_MchKey;
#ifdef _DEBUG
	printf("formd5Str: %s\n", strForMd5.c_str());
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
	//printf("finsStr : %s\n",stringBuffer.c_str());
	auto ret = m_tHttpRequest.performRequest(nullptr,stringBuffer.c_str(),stringBuffer.size(),nullptr ) ;
	
	delete xmlRoot ;
	xmlRoot = nullptr ;

	if ( ret )
	{
		return 0 ;
	}
	return 1 ;
}

void CWechatVerifyTask::onHttpCallBack(char* pResultData, size_t nDatalen , void* pUserData , size_t nUserTypeArg )
{
	auto pRequest = getVerifyResult() ;
	assert(pRequest != nullptr && "must not null") ;
	TiXmlDocument t ;
	std::string str( pResultData,nDatalen );
	t.Parse(str.c_str(),0,TIXML_ENCODING_UTF8);
	//LOGFMTD("weChatPayRet : %s",str.c_str()) ;
	LOGD("client request verify back ");
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
					TiXmlElement* pState = (TiXmlElement*)pRoot->FirstChild("trade_state");
					TiXmlNode* pStateRet = pState->FirstChild();
					if ( strcmp(pStateRet->Value(),"SUCCESS") == 0 )
					{
						LOGFMTD("weChatVerfiy success ") ;
						// success ;
						pRequest->eResult = eVerify_Apple_Success ;
						return  ;
					}
				}
			}
		}
	}
	pRequest->eResult = eVerify_Apple_Error ;
	printf("we chat verify failed \n") ;
	t.SaveFile("reTCheck.xml");
	return ;
}
