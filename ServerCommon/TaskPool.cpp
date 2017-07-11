#include "TaskPool.h"
#include "WorkThread.h"
CTaskPool::CTaskPool()
{
	m_pTaskFactor = nullptr ;
	m_isClosed = false ;
}

bool CTaskPool::init( ITaskFactory* pTaskFactory , uint8_t nInitThreadCnt )
{
	assert(nInitThreadCnt > 0 && "init thread cnt must > 0 " );
	if ( nInitThreadCnt == 0 )
	{
		nInitThreadCnt = 2 ;
	}

	m_pTaskFactor = pTaskFactory ;
	for ( uint8_t nidx = 0 ; nidx < nInitThreadCnt ; ++nidx )
	{
		std::shared_ptr<CWorkThread> ptr ( new CWorkThread(this,nidx) );
		m_vIdleThread.push(ptr);
		ptr->Start();
	}
	printf("create %u work thread for task pool\n",nInitThreadCnt) ;
	return true ;
}

void CTaskPool::postTask(ITask::ITaskPrt pTask )
{
	if ( m_isClosed )
	{
		printf("already close task pool \n") ;
		return ;
	}

	m_tThread.Lock() ;
	if ( ! m_vIdleThread.empty() )
	{
		auto ptr = m_vIdleThread.top() ;
		m_vIdleThread.pop();

		m_vBusyThread.push_back(ptr);
		m_tThread.Unlock();

		ptr->assignTask(pTask);
		return ;
	}
	m_tThread.Unlock();

	m_tWaitingTask.Lock() ;
	m_vWaitingTask.push(pTask) ;
	m_tWaitingTask.Unlock() ;
}

void CTaskPool::update()
{
	LIST_TASK vTask ;
	m_tFinishTask.Lock() ;
	if ( m_vFinishTask.empty() == false )
	{
		vTask.swap(m_vFinishTask);
	}
	m_tFinishTask.Unlock() ;

	for ( auto& ref : vTask )
	{
		auto pCallBck = ref->getCallBack();

		if ( pCallBck != nullptr )
		{
			pCallBck(ref) ;
		}
		m_vResultObjs[ref->getTaskID()].push_back(ref);
	}
	vTask.clear();
}

ITask::ITaskPrt CTaskPool::getReuseTaskObjByID( uint32_t nID )
{
	auto iter = m_vResultObjs.find(nID) ;
	if ( iter == m_vResultObjs.end() || iter->second.empty() )
	{
		if ( m_pTaskFactor )
		{
			return m_pTaskFactor->createTask(nID) ;
		}
		return nullptr ;
	}

	auto p = iter->second.back();
	iter->second.pop_back();
	return p ;
}

void CTaskPool::onThreadFinishTask( CWorkThread* pThread )
{
	m_tFinishTask.Lock() ;
	m_vFinishTask.push_back(pThread->getTask());
	m_tFinishTask.Unlock() ;

	pThread->assignTask(nullptr);

	m_tWaitingTask.Lock() ;
	if ( m_vWaitingTask.empty() == false )
	{
		auto pTask = m_vWaitingTask.front();
		m_vWaitingTask.pop();
		m_tWaitingTask.Unlock() ;
		pThread->assignTask(pTask);
		return ;
	}
	m_tWaitingTask.Unlock() ;

	std::shared_ptr<CWorkThread> shardPtr = nullptr ;
	m_tThread.Lock();
	auto iter = std::find_if(m_vBusyThread.begin(),m_vBusyThread.end(),[pThread](std::shared_ptr<CWorkThread>& ref ) { return ref.get() == pThread ; } ) ;
	if ( iter == m_vBusyThread.end() )
	{
		printf("why can not find , big bug !!!! \n") ;
	}
	else
	{
		shardPtr = *iter ;
		m_vBusyThread.erase(iter) ;
	}

	if ( shardPtr == nullptr )
	{
		printf("big error \n") ;
		m_tThread.Unlock();
		return ;
	}

	m_vIdleThread.push(shardPtr) ;
	m_tThread.Unlock();
}

void CTaskPool::closeAll()
{
	m_isClosed = true ;
	m_tThread.Lock() ;
	LIST_WORK_THREAD vThread ;
	while ( m_vIdleThread.empty() == false )
	{
		auto p = m_vIdleThread.top() ;
		p->close();
		vThread.push_back(p);
		m_vIdleThread.pop();
	}

	for ( auto& ref : vThread )
	{
		m_vIdleThread.push(ref);
	}

	for ( auto& ref : m_vBusyThread )
	{
		ref->close();
	}
	m_tThread.Unlock();
}