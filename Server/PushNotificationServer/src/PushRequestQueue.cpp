#include "PushRequestQueue.h"
void CPushRequestQueue::PushNotice(stNotice* pNotice )
{
	mRequestLock.Lock();
	m_vAllNotices.push_back(pNotice) ;
	mRequestLock.Unlock() ;
}

stNotice* CPushRequestQueue::GetReserverNotice()
{
	stNotice* pNocite = NULL;
	mReserveQuestLock.Lock();
	LIST_NOTICES::iterator iter = m_vReserverNotices.begin() ;
	if ( iter != m_vReserverNotices.end() )
	{
		pNocite = *iter ;
		m_vReserverNotices.erase(iter) ;
		mReserveQuestLock.Unlock();
		memset(pNocite,0,sizeof(stNotice));
		pNocite->nBadge = 1 ;
		return pNocite ;
	}
	mReserveQuestLock.Unlock();
	pNocite = new stNotice ;
	memset(pNocite,0,sizeof(stNotice));
	pNocite->nBadge = 1 ;
	return pNocite ;
}

void CPushRequestQueue::GetAllNoticeToProcess(LIST_NOTICES& vAllNotices )
{
	mRequestLock.Lock();
	vAllNotices.clear();
	m_vAllNotices.swap(vAllNotices) ;
	mRequestLock.Unlock();
}

void CPushRequestQueue::PushProcessedNotice(LIST_NOTICES& vAllProcessedNotice )
{
	mReserveQuestLock.Lock();
	m_vReserverNotices.insert(m_vReserverNotices.end(),vAllProcessedNotice.begin(),vAllProcessedNotice.end()) ;
	mReserveQuestLock.Unlock();
}

CPushRequestQueue* CPushRequestQueue::s_pushQueue = NULL;

CPushRequestQueue* CPushRequestQueue::SharedPushRequestQueue()
{
	if ( s_pushQueue == NULL )
	{
		s_pushQueue = new CPushRequestQueue ;
	}
	return s_pushQueue ;
}