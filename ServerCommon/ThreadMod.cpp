#include "ThreadMod.h"


CThreadT::CThreadT()
{
}


CThreadT::~CThreadT()
{
}


bool CThreadT::Start()
{
	return pthread_create(&m_threadId, NULL, __threadfunc, (void *)this) == 0;
}

void CThreadT::Stop()
{
}

