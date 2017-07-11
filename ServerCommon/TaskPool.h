#pragma once
#include "ITask.h"
#include <vector>
#include <map>
#include <stack>
#include <queue>
#include <list>
#include "mutex.h"
class CWorkThread ;
class CTaskPool
{
public:
	typedef std::list<ITask::ITaskPrt> LIST_TASK ;
	typedef std::map<uint32_t,LIST_TASK> MAP_CATEGORY_TASK ;
	typedef std::queue<ITask::ITaskPrt> TASK_QUEUE ;
	typedef std::stack<std::shared_ptr<CWorkThread>> STACK_WORK_THREAD ; 
	typedef std::list<std::shared_ptr<CWorkThread>> LIST_WORK_THREAD ;
public:
	CTaskPool();
	bool init( ITaskFactory* pTaskFactory , uint8_t nInitThreadCnt );
	void postTask(ITask::ITaskPrt pTask );
	void update();
	void closeAll();

	ITask::ITaskPrt getReuseTaskObjByID( uint32_t nID );
protected:
	void onThreadFinishTask( CWorkThread* pThread );
	friend class CWorkThread ;
protected:
	bool m_isClosed ;
	ITaskFactory* m_pTaskFactor ;
	MAP_CATEGORY_TASK m_vResultObjs ;

	Mutex m_tFinishTask ;
	LIST_TASK m_vFinishTask ;

	Mutex m_tWaitingTask ;
	TASK_QUEUE m_vWaitingTask ;

	Mutex m_tThread ;
	STACK_WORK_THREAD m_vIdleThread ;
	LIST_WORK_THREAD m_vBusyThread ;
};