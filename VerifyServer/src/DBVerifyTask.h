#pragma once
#include "IVerifyTask.h"
class CDBTask ;
struct stVerifyRequest;
class CDBVerfiyTask
	:public IVerifyTask
{
public:
	CDBVerfiyTask(uint32_t nTaskID ) ;
	uint8_t performTask()override;
	VERIFY_REQUEST_ptr getVerifyResult()override ;
	void setVerifyRequest( VERIFY_REQUEST_ptr ptr )override ;
protected:
	std::shared_ptr<CDBTask> m_pDBTask ;
};