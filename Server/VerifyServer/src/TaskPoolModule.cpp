#include "TaskPoolModule.h"
#include "WeChatOrderTask.h"
#include "Timer.h"
#include "ServerMessageDefine.h"
#include "log4z.h"
#include "ISeverApp.h"
#include "VerifyRequest.h"
#include "AppleVerifyTask.h"
#include "WeChatVerifyTask.h"
#include "DBVerifyTask.h"
#include "ApnsTask.h"
#include "AnyLoginTask.h"
#include "AsyncRequestQuene.h"
void CTaskPoolModule::init( IServerApp* svrApp )
{
	IGlobalModule::init(svrApp) ;
	m_tTaskPool.init(this,3);

	// test code 
	//static CTimer tTim ;
	//tTim.setInterval(2) ;
	//tTim.setIsAutoRepeat(false);
	//tTim.setCallBack([this](CTimer* p , float f ){ printf("timer invoker\n");testFunc();}) ;
	//tTim.start();
	// test code 
}

void CTaskPoolModule::onExit()
{
	getPool().closeAll() ;
}

bool CTaskPoolModule::onMsg(stMsg* pMsg , eMsgPort eSenderPort , uint32_t nSessionID)
{
	if ( IGlobalModule::onMsg(pMsg,eSenderPort,nSessionID) )
	{
		return true;
	}

	if ( MSG_VERIFY_ITEM_ORDER == pMsg->usMsgType )
	{
		onWechatOrder(pMsg,eSenderPort,nSessionID) ;
		return true ;
	}

	if ( pMsg->usMsgType == MSG_VERIFY_TANSACTION )
	{
		onVerifyMsg(pMsg,eSenderPort,nSessionID) ;
		return true ;
	}

	return  false ;
}

bool CTaskPoolModule::onMsg(Json::Value& prealMsg ,uint16_t nMsgType, eMsgPort eSenderPort , uint32_t nSessionID)
{
	if ( IGlobalModule::onMsg(prealMsg,nMsgType,eSenderPort,nSessionID ) )
	{
		return  true ;
	}

	return false;
}

void CTaskPoolModule::update(float fDeta )
{
	IGlobalModule::update(fDeta) ;
	m_tTaskPool.update();
}

bool CTaskPoolModule::onAsyncRequest(uint16_t nRequestType , const Json::Value& jsReqContent, Json::Value& jsResult )
{
	if ( eAsync_Apns != nRequestType )
	{
		return false ;
	}

	auto p = getPool().getReuseTaskObjByID( eTask_Apns );
	CApnsTask* pTest = (CApnsTask*)p.get();
	pTest->setRequest(jsReqContent);
	getPool().postTask(p);
	return true ;
}

void CTaskPoolModule::testFunc()
{
	printf("task go \n") ;
	uint32_t nCnt = 1 ;
	while (nCnt--)
	{
		auto p = getPool().getReuseTaskObjByID( eTask_Apns );
		CApnsTask* pTest = (CApnsTask*)p.get();
		
		Json::Value jsRequest, target ;
		target[0u] = 12709990 ;
		jsRequest["apnsType"] = 0 ;
		jsRequest["targets"] = target;
		jsRequest["content"] = "test sfhg" ;
		jsRequest["msgID"] = "fs";
		jsRequest["msgdesc"] = "fhsg" ;
		pTest->setRequest(jsRequest);

		getPool().postTask(p);
	}
}

ITask::ITaskPrt CTaskPoolModule::createTask( uint32_t nTaskID )
{
	switch (nTaskID)
	{
	case eTask_WechatOrder:
		{
			std::shared_ptr<CWeChatOrderTask> pTask ( new CWeChatOrderTask(nTaskID)) ;
			return pTask  ;
		}
		break;
	case eTask_WechatVerify:
		{
			std::shared_ptr<CWechatVerifyTask> pTask ( new CWechatVerifyTask(nTaskID)) ;
			return pTask  ;
		}
		break;
	case eTask_AppleVerify:
		{
			std::shared_ptr<CAppleVerifyTask> pTask ( new CAppleVerifyTask(nTaskID)) ;
			return pTask  ;
		}
		break;
	case eTask_DBVerify:
		{
			std::shared_ptr<CDBVerfiyTask> pTask ( new CDBVerfiyTask(nTaskID)) ;
			return pTask  ;
		}
		break;
	case eTask_Apns:
		{
			std::shared_ptr<CApnsTask> pTask ( new CApnsTask(nTaskID)) ;
			return pTask  ;
		}
		break;
	case eTask_AnyLogin:
		{
			std::shared_ptr<AnyLoginTask> pTask(new AnyLoginTask(nTaskID));
			return pTask;
		}
		break;
	default:
		break;
	}
	return nullptr ;
}

// logic  
void CTaskPoolModule::onWechatOrder( stMsg* pMsg, eMsgPort eSenderPort , uint32_t nSessionID )
{
	stMsgVerifyItemOrder* pOrder = (stMsgVerifyItemOrder*)pMsg ;

	auto pTask = getPool().getReuseTaskObjByID(eTask_WechatOrder);
	CWeChatOrderTask* pTaskObj = (CWeChatOrderTask*)pTask.get();
	// set call back 
	if ( pTask->getCallBack() == nullptr )
	{
		pTask->setCallBack([this](ITask::ITaskPrt ptr )
		{
			CWeChatOrderTask* pTask = (CWeChatOrderTask*)ptr.get();
			auto pOrder = pTask->getCurRequest().get(); 
			stMsgVerifyItemOrderRet msgRet ;
			memset(msgRet.cPrepayId,0,sizeof(msgRet.cPrepayId));
			memset(msgRet.cOutTradeNo,0,sizeof(msgRet.cOutTradeNo));
			memcpy(msgRet.cOutTradeNo,pOrder->cOutTradeNo,sizeof(pOrder->cOutTradeNo));
			memcpy(msgRet.cPrepayId,pOrder->cPrepayId,sizeof(msgRet.cPrepayId));
			msgRet.nChannel = pOrder->nChannel ;
			msgRet.nRet = pOrder->nRet ;
			getSvrApp()->sendMsg(pOrder->nSessionID,(char*)&msgRet,sizeof(msgRet));
			LOGFMTI("finish order for sessionid = %d, ret = %d ",pOrder->nSessionID,pOrder->nRet) ;
		}
		) ;
	}

	// set request info 
	std::shared_ptr<stShopItemOrderRequest> pRe = pTaskObj->getCurRequest() ;
	if ( pRe == nullptr )
	{
		pRe = std::shared_ptr<stShopItemOrderRequest>( new stShopItemOrderRequest );
		pTaskObj->setInfo(pRe);
	}
	memset(pRe.get(),0,sizeof(stShopItemOrderRequest)) ;
	sprintf_s(pRe->cShopDesc,50,pOrder->cShopDesc);
	sprintf_s(pRe->cOutTradeNo,32,pOrder->cOutTradeNo);
	pRe->nPrize = pOrder->nPrize;
	sprintf_s(pRe->cTerminalIp,17,pOrder->cTerminalIp);
	pRe->nChannel = pOrder->nChannel ;
	pRe->nFromPlayerUserUID =  0 ;
	pRe->nSessionID = nSessionID ;

	// do the request 
	getPool().postTask(pTask);
	return  ;
}

static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";  
std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len)  
{  
	std::string ret;  
	int i = 0;  
	int j = 0;  
	unsigned char char_array_3[3];  
	unsigned char char_array_4[4];  

	while (in_len--)  
	{  
		char_array_3[i++] = *(bytes_to_encode++);  
		if (i == 3) {  
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;  
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);  
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);  
			char_array_4[3] = char_array_3[2] & 0x3f;  

			for (i = 0; (i <4) ; i++)  
				ret += base64_chars[char_array_4[i]];  
			i = 0;  
		}  
	}  

	if (i)  
	{  
		for (j = i; j < 3; j++)  
			char_array_3[j] = '/0';  

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;  
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);  
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);  
		char_array_4[3] = char_array_3[2] & 0x3f;  

		for (j = 0; (j < i + 1); j++)  
			ret += base64_chars[char_array_4[j]];  

		while ((i++ < 3))  
			ret += '=';  

	}  

	return ret;  

}  

void CTaskPoolModule::onVerifyMsg( stMsg* pMsg, eMsgPort eSenderPort , uint32_t nSessionID )
{
	stMsgToVerifyServer* pReal = (stMsgToVerifyServer*)pMsg ;

	IVerifyTask::VERIFY_REQUEST_ptr pRequest ( new stVerifyRequest() );
	pRequest->nFromPlayerUserUID = pReal->nBuyerPlayerUserUID ;
	pRequest->nShopItemID = pReal->nShopItemID;
	pRequest->nBuyedForPlayerUserUID = pReal->nBuyForPlayerUserUID ;
	pRequest->nChannel = pReal->nChannel ;  // now just apple ;
	pRequest->nSessionID = nSessionID ;
	pRequest->nMiUserUID = pReal->nMiUserUID ;
	pRequest->nTotalFee = pReal->nPrice;

	LOGFMTD("received a transfaction need to verify shop id = %u userUID = %u channel = %d\n",pReal->nShopItemID,pReal->nBuyerPlayerUserUID,pReal->nChannel );

	if ( pRequest->nMiUserUID && pRequest->nChannel == ePay_XiaoMi )
	{
		memcpy(pRequest->pBufferVerifyID,((unsigned char*)pMsg) + sizeof(stMsgToVerifyServer),pReal->nTranscationIDLen);
		//m_MiVerifyMgr.AddRequest(pRequest) ;
		LOGFMTE("we don't have xiao mi channel") ;
		return ;
	}
	
	ITask::ITaskPrt pTask = nullptr ;
	if ( pRequest->nChannel == ePay_AppStore )
	{
		std::string str = base64_encode(((unsigned char*)pMsg) + sizeof(stMsgToVerifyServer),pReal->nTranscationIDLen);
		//std::string str = base64_encode(((unsigned char*)pMsg) + sizeof(stMsgToVerifyServer),20);
		memcpy(pRequest->pBufferVerifyID,str.c_str(),strlen(str.c_str()));
		pTask = getPool().getReuseTaskObjByID(eTask_AppleVerify) ;
	}
	else if ( ePay_WeChat == pRequest->nChannel || ePay_WeChat_365Golden == pRequest->nChannel )
	{
		memcpy(pRequest->pBufferVerifyID,((unsigned char*)pMsg) + sizeof(stMsgToVerifyServer),pReal->nTranscationIDLen);
		std::string strTradeNo(pRequest->pBufferVerifyID);
		std::string shopItem = strTradeNo.substr(0,strTradeNo.find_first_of('E')) ;
		if ( atoi(shopItem.c_str()) != pRequest->nShopItemID )
		{
			printf("shop id and verify id not the same \n") ;
			pRequest->eResult = eVerify_Apple_Error ;
			sendVerifyResult(pRequest) ;
			return ;
		}
		else
		{
			pTask = getPool().getReuseTaskObjByID(eTask_WechatVerify) ;
		}
	}
	else
	{
		LOGFMTE("unknown pay channecl = %d, uid = %d",pRequest->nChannel,pReal->nBuyerPlayerUserUID ) ;
		return ;
	}

	if ( !pTask )
	{
		LOGFMTE("why verify task is null ? ") ;
		return ;
	}

	auto* pVerifyTask = (IVerifyTask*)pTask.get();
	pVerifyTask->setVerifyRequest(pRequest) ;
	pVerifyTask->setCallBack([this](ITask::ITaskPrt ptr ) 
	{
		auto* pAready = (IVerifyTask*)ptr.get();
		auto pResult = pAready->getVerifyResult() ;
		if ( eVerify_Apple_Error == pResult->eResult )
		{
			LOGFMTE("apple verify Error  uid = %u, channel = %u,shopItem id = %u",pResult->nFromPlayerUserUID,pResult->nChannel,pResult->nShopItemID) ;
			// send to client ;
			sendVerifyResult(pResult) ;
			return ;
		}

		LOGFMTI("apple verify success  uid = %u, channel = %u,shopItem id = %u,go on DB verify",pResult->nFromPlayerUserUID,pResult->nChannel,pResult->nShopItemID) ;
		doDBVerify(pResult);
	} ) ;
	getPool().postTask(pTask);
}

void CTaskPoolModule::sendVerifyResult(std::shared_ptr<stVerifyRequest> & pResult )
{
	stMsgFromVerifyServer msg ;
	msg.nShopItemID = pResult->nShopItemID ;
	msg.nRet = pResult->eResult ;
	msg.nBuyerPlayerUserUID = pResult->nFromPlayerUserUID ;
	msg.nBuyForPlayerUserUID = pResult->nBuyedForPlayerUserUID ;
	getSvrApp()->sendMsg(pResult->nSessionID,(char*)&msg,sizeof(msg));
	LOGFMTI( "finish verify transfaction shopid = %u ,uid = %d ret = %d",msg.nShopItemID,msg.nBuyerPlayerUserUID,msg.nRet ) ;
	if (msg.nRet == 4) // purchase success
	{
		Json::Value jssql;
		char pBuffer[512] = { 0 };
		sprintf(pBuffer, "insert into wxrecharge ( userUID,fee,time,tradeOrder ) values ('%u','%u',now(),'%s');", msg.nBuyerPlayerUserUID, pResult->nTotalFee,pResult->pBufferVerifyID );
		jssql["sql"] = pBuffer;
		getSvrApp()->getAsynReqQueue()->pushAsyncRequest(ID_MSG_PORT_DB, eAsync_DB_Add, jssql);
	}
}

void CTaskPoolModule::doDBVerify(uint32_t nUserUID, uint16_t nShopID, uint8_t nChannel,std::string& strTransfcationID, uint32_t nFee )
{
	IVerifyTask::VERIFY_REQUEST_ptr pRequest(new stVerifyRequest());
	pRequest->nFromPlayerUserUID = nUserUID;
	pRequest->nShopItemID = nShopID;
	pRequest->nBuyedForPlayerUserUID = nUserUID;
	pRequest->nChannel = nChannel; 
	pRequest->nSessionID = 0;
	pRequest->nMiUserUID = 0;
	pRequest->nTotalFee = nFee;

	memset(pRequest->pBufferVerifyID,0,sizeof(pRequest->pBufferVerifyID));
	memcpy_s(pRequest->pBufferVerifyID, sizeof(pRequest->pBufferVerifyID),strTransfcationID.data(),strTransfcationID.size());

	doDBVerify(pRequest);
}

void CTaskPoolModule::doDBVerify(IVerifyTask::VERIFY_REQUEST_ptr ptr)
{
	auto pDBTask = getPool().getReuseTaskObjByID(eTask_DBVerify);
	auto pDBVerifyTask = (IVerifyTask*)pDBTask.get();

	pDBVerifyTask->setVerifyRequest(ptr);
	pDBVerifyTask->setCallBack([this](ITask::ITaskPrt ptr){ auto pAready = (IVerifyTask*)ptr.get(); sendVerifyResult(pAready->getVerifyResult()); });
	getPool().postTask(pDBTask);
}

ITask::ITaskPrt CTaskPoolModule::getReuseTask(eTask nTask)
{
	return getPool().getReuseTaskObjByID(nTask);
}

void CTaskPoolModule::postTask(ITask::ITaskPrt pTask)
{
	getPool().postTask(pTask);
}