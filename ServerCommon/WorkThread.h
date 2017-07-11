#pragma once
#include "ThreadMod.h"
#include "ITask.h"
#include "Sem.h"
class CTaskPool ;
class CWorkThread
	:public CThreadT
{
public:
	CWorkThread( CTaskPool* pool,uint8_t nIdx );
	void assignTask(ITask::ITaskPrt pTask );
	ITask::ITaskPrt getTask();
	void __run() ;
	void close();
protected:
	ITask::ITaskPrt m_pTask ;
	CSemaphore m_sem ;
	bool m_isClose; 
	CTaskPool* m_pPool ;
	uint8_t m_nIdx ;
};