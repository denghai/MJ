#ifndef _MUTEX__H_
#define _MUTEX__H_

#include <pthread.h>

class Mutex
{
public:
	friend class Condition;
	Mutex();
	~Mutex();
	void Acquire()
	{
		pthread_mutex_lock(&mutex);
	}
	 void Release()
	{
		pthread_mutex_unlock(&mutex);
	}
	void Lock()
	{
		return Acquire();
	}
	void Unlock()
	{
		return Release();
	}
	bool AttemptAcquire()
	{
		return (pthread_mutex_trylock(&mutex) == 0);
	}
protected:
	static bool attr_initalized;
	static pthread_mutexattr_t attr;

	pthread_mutex_t mutex;
};

#endif

