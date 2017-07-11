#include "WorkThread.h"
#include "TaskPool.h"
CWorkThread::CWorkThread( CTaskPool* pool, uint8_t nIdx )
{
	m_isClose = false ;
	m_pTask = nullptr ;
	m_pPool = pool ;
	m_nIdx = nIdx ;
}

void CWorkThread::assignTask(ITask::ITaskPrt pTask )
{
	if ( this->m_isClose )
	{
		return ;
	}

	m_pTask = pTask ;
	if ( m_pTask )
	{
		m_sem.Post() ;
	}
}

ITask::ITaskPrt CWorkThread::getTask()
{
	return m_pTask ;
}

void CWorkThread::__run()
{
	while ( true )
	{
		m_sem.Wait();
		if ( m_isClose )
		{
			printf("do close thread \n") ;
			m_pTask = nullptr ;
			break;
		}

		if ( m_pTask )
		{
			auto ret = m_pTask->performTask();
			m_pTask->setResultCode(ret) ;
			if ( m_pPool )
			{
				printf("thread idx = %u ok \n",m_nIdx) ;
				m_pPool->onThreadFinishTask(this) ;
			}
			else
			{
				printf("why pool is null , how to tell pool result \n ") ;
			}
		}
	}
}

void CWorkThread::close()
{
	printf("send close cmd \n") ;
	m_isClose = true ;
	m_sem.Post() ;
}