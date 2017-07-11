#ifndef _SHARE_SEMAPHORE__H_
#define _SHARE_SEMAPHORE__H_

#include <semaphore.h>
#include <stdio.h>

class CSemaphore
{
public:
	CSemaphore()
	{
		sem_init(&m_count, 0, 0);
	}
	~CSemaphore()
	{
		sem_destroy(&m_count);
	}
	int Wait()
	{
		return sem_wait(&m_count);
	}
	int Post()
	{
		return sem_post(&m_count);
	}
	
private:
	sem_t m_count;
};

#endif

