#pragma once
#include "NativeTypes.h"
#include <functional>
#include <memory>
class ITask
{
public:
	typedef std::shared_ptr<ITask> ITaskPrt ;
	typedef std::function<void ( ITaskPrt pTask) >  lpTaskCallBack ;
public:
	ITask( uint32_t nTaskID ){ m_pCallBack = nullptr ; m_nTaskID = nTaskID ;}
	virtual ~ITask(){}
	uint32_t getTaskID(){ return m_nTaskID ;}
	uint8_t getResultCode(){ return m_nResultCode ; };
	void setResultCode(uint8_t nRet ){ m_nResultCode = nRet ;}
	void setCallBack(lpTaskCallBack lpCallBack ){ m_pCallBack = lpCallBack ; }
	lpTaskCallBack getCallBack(){ return m_pCallBack ;}

	virtual uint8_t performTask() = 0;
	virtual void reset(){  }
private:
	lpTaskCallBack m_pCallBack ;
	uint8_t m_nResultCode ;
	uint32_t m_nTaskID ;
};

class ITaskFactory
{
public:
	virtual ~ITaskFactory(){}
	virtual ITask::ITaskPrt createTask( uint32_t nTaskID ) = 0 ;
};