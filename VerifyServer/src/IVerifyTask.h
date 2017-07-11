#pragma once
#include "ITask.h"
struct stVerifyRequest ;
class IVerifyTask
	:public ITask
{
public:
	typedef std::shared_ptr<stVerifyRequest> VERIFY_REQUEST_ptr ;
public:
	IVerifyTask( uint32_t nTaskID ): ITask(nTaskID) { }
	virtual VERIFY_REQUEST_ptr getVerifyResult(){ return m_pVerifyReq ; }
	virtual void setVerifyRequest( VERIFY_REQUEST_ptr ptr ){ m_pVerifyReq = ptr ;}
private:
	VERIFY_REQUEST_ptr m_pVerifyReq ;
};