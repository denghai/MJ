#pragma once
#include "IGlobalModule.h"
#include "TaskPool.h"
#include "IVerifyTask.h"
struct stVerifyRequest ;
class CTaskPoolModule
	:public IGlobalModule
	,public ITaskFactory
{
public:
	enum eTask
	{
		eTask_WechatOrder,
		eTask_AppleVerify,
		eTask_WechatVerify,
		eTask_DBVerify,
		eTask_Apns,
		eTask_AnyLogin,
		eTask_Max,
	};
public:
	void init( IServerApp* svrApp )override ;
	void onExit()override ;
	bool onMsg(stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID)override ;
	bool onMsg(Json::Value& prealMsg ,uint16_t nMsgType, eMsgPort eSenderPort , uint32_t nSessionID)override ;
	void update(float fDeta )override ;
	bool onAsyncRequest(uint16_t nRequestType , const Json::Value& jsReqContent, Json::Value& jsResult )override;
	void testFunc();
	ITask::ITaskPrt createTask( uint32_t nTaskID )override ;
	CTaskPool& getPool(){ return m_tTaskPool ;}
	void doDBVerify( uint32_t nUserUID , uint16_t nShopID , uint8_t nChannel, std::string& strTransfcationID, uint32_t nFee );
	ITask::ITaskPrt getReuseTask(eTask nTask);
	void postTask(ITask::ITaskPrt pTask );
protected:
	void doDBVerify(IVerifyTask::VERIFY_REQUEST_ptr ptr);
	// logic 
	void onWechatOrder( stMsg* pOrder, eMsgPort eSenderPort , uint32_t nSessionID );
	void onVerifyMsg( stMsg* pOrder, eMsgPort eSenderPort , uint32_t nSessionID );
	void sendVerifyResult(std::shared_ptr<stVerifyRequest> & pResult );
protected:
	CTaskPool m_tTaskPool ;
};