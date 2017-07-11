#pragma once
#include "VerifyRequest.h"
class CDataBaseThread ;
struct stDBResult ;
class CDBVerifyManager
{
public:
	CDBVerifyManager();
	~CDBVerifyManager();
	void Init();
	void AddRequest(stVerifyRequest* pRequest );
	stVerifyRequest* GetResult(LIST_VERIFY_REQUEST& vAllResult) ;
	void ProcessRequest();
	void OnDBResult(stDBResult*pResult);
protected:
	LIST_VERIFY_REQUEST m_vProcessedRequest ;
	CDataBaseThread* m_pDBThead ;
};