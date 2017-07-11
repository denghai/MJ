#include "DBVerifyManager.h"
#include "DBRequest.h"
#include "DataBaseThread.h"
#include "ServerConfig.h"
CDBVerifyManager::CDBVerifyManager()
{
	m_pDBThead = NULL ;
}

CDBVerifyManager::~CDBVerifyManager()
{
	if ( m_pDBThead )
	{
		delete m_pDBThead ;
	}
}

void CDBVerifyManager::Init()
{
	m_pDBThead = new CDataBaseThread ;

	CSeverConfigMgr stSvrConfigMgr ;
	stSvrConfigMgr.LoadFile("../configFile/serverConfig.txt");
	// set up data base thread 
	stServerConfig* pDatabase = stSvrConfigMgr.GetServerConfig(eSvrType_DataBase);

	m_pDBThead->InitDataBase(pDatabase->strIPAddress,pDatabase->nPort,pDatabase->strAccount,pDatabase->strPassword,"taxpokerdb");
	m_pDBThead->Start();
}

void CDBVerifyManager::AddRequest(stVerifyRequest* pRequest )
{
	//---
	// save this transaction id  
	stDBRequest* pDBRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest() ;
	pDBRequest->eType = eRequestType_Add ;
	pDBRequest->nRequestUID = 10 ;
	pDBRequest->nSqlBufferLen = sprintf_s(pDBRequest->pSqlBuffer,"INSERT INTO `transnum` (`type`, `transId`) VALUES ('%d', '%s');",pRequest->nChannel,pRequest->pBufferVerifyID) ;
	pDBRequest->pUserData = pRequest ;
	CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pDBRequest) ;
	///-----
	//stDBRequest* pDBRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest() ;
	//pDBRequest->eType = eRequestType_Select ;
	//pDBRequest->nRequestUID = 100 ;
	//pDBRequest->nSqlBufferLen = sprintf_s(pDBRequest->pSqlBuffer,"SELECT * FROM transNum where transId = '%s'&& type = %d", pRequest->nFromSessionID,pRequest->nRequestType ) ;
	//pDBRequest->pUserData = pRequest ;
	//CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pDBRequest) ;
}

stVerifyRequest* CDBVerifyManager::GetResult(LIST_VERIFY_REQUEST& vAllResult)
{
	if ( m_vProcessedRequest.empty() )
	{
		vAllResult.clear();
		m_vProcessedRequest.clear() ;
		return NULL ;
	}
	vAllResult.swap(m_vProcessedRequest) ;
	return vAllResult.front() ;
}

void CDBVerifyManager::ProcessRequest()
{
	// process DB Result ;
	CDBRequestQueue::VEC_DBRESULT vResultOut ;
	CDBRequestQueue::SharedDBRequestQueue()->GetAllResult(vResultOut) ;
	CDBRequestQueue::VEC_DBRESULT::iterator iter = vResultOut.begin() ;
	for ( ; iter != vResultOut.end(); ++iter )
	{
		stDBResult* pRet = *iter ;
		OnDBResult(pRet) ;
		delete pRet ;
	}
	vResultOut.clear();
}

void CDBVerifyManager::OnDBResult(stDBResult*pResult)
{
	stVerifyRequest* pResultQuest = (stVerifyRequest*)pResult->pUserData ;
	if ( pResult->nRequestUID == 10 )
	{
		pResultQuest->eResult = (pResult->nAffectRow == 1 ? eVerify_Success:eVerify_DB_Error);
		m_vProcessedRequest.push_back(pResultQuest) ;
	}
	else
	{
		printf("pResult->nRequestUID = %d Error\n",pResult->nRequestUID) ;
	}
}