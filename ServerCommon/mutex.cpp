
#include "mutex.h"

#define recursive_mutex_flag PTHREAD_MUTEX_RECURSIVE_NP

/* Linux mutex implementation */
bool Mutex::attr_initalized = false;
pthread_mutexattr_t Mutex::attr;

Mutex::Mutex()
{
	if(!attr_initalized)
	{
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, recursive_mutex_flag);
		attr_initalized = true;
	}

	pthread_mutex_init(&mutex, &attr);
}

Mutex::~Mutex() { pthread_mutex_destroy(&mutex); }
