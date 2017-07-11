#include "DBVerifyTask.h"
#include "VerifyRequest.h"
#include "ServerConfig.h"
#include "DBTask.h"
#include "DBRequest.h"
#include "../ServerCommon/ConfigDefine.h"
CDBVerfiyTask::CDBVerfiyTask(uint32_t nTaskID )
	:IVerifyTask(nTaskID),m_pDBTask(nullptr)
{
	CSeverConfigMgr stSvrConfigMgr ;
	stSvrConfigMgr.LoadFile("../configFile/serverConfig.txt");
	// set up data base thread 
	stServerConfig* pDatabase = stSvrConfigMgr.GetServerConfig(eSvrType_DataBase);
	m_pDBTask = std::shared_ptr<CDBTask>(new CDBTask(nTaskID, pDatabase->strIPAddress, pDatabase->nPort, pDatabase->strAccount, pDatabase->strPassword, Game_DB_Name));
}

uint8_t CDBVerfiyTask::performTask()
{
	return m_pDBTask->performTask();
}

CDBVerfiyTask::VERIFY_REQUEST_ptr CDBVerfiyTask::getVerifyResult()
{
	auto pDBResult = m_pDBTask->getDBResult();
	auto pVerifyResult = IVerifyTask::getVerifyResult() ;
	if ( pDBResult->nRequestUID == 10 )
	{
		pVerifyResult->eResult = (pDBResult->nAffectRow == 1 ? eVerify_Success : eVerify_DB_Error);
	}
	else
	{
		printf("pResult->nRequestUID = %d Error unknown \n",pDBResult->nRequestUID) ;
	}
	return pVerifyResult ;
}

void CDBVerfiyTask::setVerifyRequest( VERIFY_REQUEST_ptr ptr )
{
	if ( !ptr )
	{
		printf("why verify request is null ? ") ;
		return ;
	}

	auto pDBRequest = m_pDBTask->getDBRequest();
	pDBRequest->reset();
	pDBRequest->eType = eRequestType_Add ;
	pDBRequest->nRequestUID = 10 ;
	pDBRequest->nSqlBufferLen = sprintf_s(pDBRequest->pSqlBuffer,"INSERT INTO `transnum` (`type`, `transId`) VALUES ('%d', '%s');",ptr->nChannel,ptr->pBufferVerifyID) ;
	m_pDBTask->setDBRequest(pDBRequest);
	IVerifyTask::setVerifyRequest(ptr);
}