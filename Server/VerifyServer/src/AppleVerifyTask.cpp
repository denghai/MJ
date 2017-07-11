#include "AppleVerifyTask.h"
#include <json/json.h>
#include "VerifyRequest.h"
#include <cassert>
#include "log4z.h"
#include "ConfigDefine.h"
CAppleVerifyTask::CAppleVerifyTask( uint32_t nTaskID ) 
	:IVerifyTask(nTaskID)
{
//#ifdef DEBUG
//	m_tHttpRequest.init("https://sandbox.itunes.apple.com/verifyReceipt");
//#else
	m_tHttpRequest.init("https://buy.itunes.apple.com/verifyReceipt");
//#endif

#if defined(GAME_IN_REVIEW)
	m_tHttpRequest.init("https://sandbox.itunes.apple.com/verifyReceipt");
#endif 
	m_tHttpRequest.setDelegate(this);
}

uint8_t CAppleVerifyTask::performTask()
{
	auto pRequest = getVerifyResult() ;
	if ( pRequest == nullptr )
	{
		return 1 ;
	}
	// processed this requested ;
	Json::FastWriter jWriter ;
	Json::Value jRootValue ;

	jRootValue["receipt-data"] = pRequest->pBufferVerifyID ;
	std::string strFinal = jWriter.write(jRootValue) ;
	auto ret = m_tHttpRequest.performRequest(nullptr,strFinal.c_str(),strFinal.size(),nullptr ) ;
	if ( ret )
	{
		return 0 ;
	}
	return 1 ;
}

void CAppleVerifyTask::onHttpCallBack(char* pResultData, size_t nDatalen , void* pUserData , size_t nUserTypeArg )
{
	auto pResult = getVerifyResult() ;
	assert(pResult != nullptr && "must not null") ;
	Json::Reader reader;
	Json::Value rootValue ;
	//static char pTempBuffer[1024] = { 0 } ;
	//memset(pTempBuffer,0,sizeof(pTempBuffer)) ;
	//memcpy(pTempBuffer,pResultData,nDatalen);
	bool bCheckOk = false ;
	//if ( reader.parse(pTempBuffer,rootValue) )
	if ( reader.parse(pResultData, pResultData + nDatalen, rootValue))
	{
		bCheckOk = rootValue["status"].asInt() == 0 ;
	}
	else
	{
		printf("parse json data error , from apple \n") ;
		pResult->eResult = eVerify_Apple_Error;
		return ;
	}

	if ( bCheckOk )
	{
		Json::Value receipt = rootValue["receipt"] ;
		memset(pResult->pBufferVerifyID,0,sizeof(pResult->pBufferVerifyID)) ;
		sprintf(pResult->pBufferVerifyID,"%s",receipt["transaction_id"].asCString());
		//LOGFMTD("Bundle  id = %s \n",receipt["bundle_id"].asCString());
		LOGFMTD("BUY OK product_id = %s \n",receipt["product_id"].asCString());
		//if ( strcmp(receipt["bundle_id"].asCString(),"com.youhoo.paiyouquan" ) != 0 )
		//{
		//	//bCheckOk = false ;
		//	LOGE("you should not have the bill cation");
		//}
	}

	pResult->eResult = bCheckOk ? eVerify_Apple_Success : eVerify_Apple_Error;
	return   ;
}
