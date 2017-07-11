#include "DBRequest.h"
#include <assert.h>
stDBResult::~stDBResult()
{
	VEC_MYSQLROW::iterator iter = vResultRows.begin();
	for ( ; iter != vResultRows.end() ; ++iter )
	{
		delete *iter ;
		*iter = NULL ;
	}
	vResultRows.clear() ;
}

CDBRequestQueue* CDBRequestQueue::SharedDBRequestQueue()
{
	static CDBRequestQueue g_sDBRequestQueue ;
	return &g_sDBRequestQueue ;
}

CDBRequestQueue::CDBRequestQueue()
{
	ClearAllRequest();
	ClearAllResult();
	ClearAllReserveRequest();
}

CDBRequestQueue::~CDBRequestQueue()
{
	ClearAllRequest();
	ClearAllResult();
	ClearAllReserveRequest();
}

void CDBRequestQueue::PushRequest(stDBRequest* request )
{
	assert(request->nRequestUID && "this can not be zero ,assign it with msg id" );
	mRequestLock.Lock();
	bool bInsert = false ;
	VEC_DBREQUEST::iterator iter = m_vAllRequest.begin() ;
	for ( ; iter != m_vAllRequest.end(); ++iter )
	{
		 if ( (*iter)->cOrder < request->cOrder )
		 {
			 m_vAllRequest.insert(iter,request) ;
			 bInsert = true ;
			 break; 
		 }
	}

	if ( !bInsert )
	{
		m_vAllRequest.push_back(request) ;
	}
	mRequestLock.Unlock() ;
}

void CDBRequestQueue::PushReserveRequest(VEC_DBREQUEST& vRequest )
{
	mReserveQuestLock.Lock() ;
	VEC_DBREQUEST::iterator iter = vRequest.begin() ;
	for ( ; iter != vRequest.end() ; ++iter )
	{
		m_vReserveRequest.push_back(*iter);
	}
	mReserveQuestLock.Unlock();
}
void CDBRequestQueue::PushResult(VEC_DBRESULT& vResult )
{
	mResultLock.Lock();
	VEC_DBRESULT::iterator iter = vResult.begin() ;
	for ( ;iter != vResult.end(); ++iter )
	{
		m_vAllResult.push_back(*iter);
	}
	mResultLock.Unlock();
}

stDBRequest* CDBRequestQueue::GetReserveRequest()
{
	stDBRequest* pOut = NULL ;
	mReserveQuestLock.Lock();
	VEC_DBREQUEST::iterator iter = m_vReserveRequest.begin() ;
	if ( iter != m_vReserveRequest.end() )
	{
		pOut = *iter ;
		m_vReserveRequest.erase(iter) ;
	}
	mReserveQuestLock.Unlock();
	if ( pOut == NULL )
	{
		pOut = new stDBRequest ;
	}
	memset(pOut,0,sizeof(stDBRequest));
	return pOut ;
}
void CDBRequestQueue::GetAllRequest(VEC_DBREQUEST& vAllRequestOut )
{
	vAllRequestOut.clear();
	mRequestLock.Lock();
	vAllRequestOut.swap(m_vAllRequest);
	mRequestLock.Unlock();
}

void CDBRequestQueue::GetAllResult(VEC_DBRESULT& vAllReslutOut )
{
	vAllReslutOut.clear() ;
	mResultLock.Lock();
	vAllReslutOut.swap(m_vAllResult) ;
	mResultLock.Unlock() ;
}

void CDBRequestQueue::ClearAllRequest()
{
	mRequestLock.Lock();
	VEC_DBREQUEST::iterator iter = m_vAllRequest.begin() ;
	for ( ; iter != m_vAllRequest.end(); ++iter )
	{
		delete *iter ;
		*iter = NULL ;
	}
	m_vAllRequest.clear() ;
	mRequestLock.Unlock();
}

void CDBRequestQueue::ClearAllResult()
{
	mResultLock.Lock() ;
	VEC_DBRESULT::iterator iter = m_vAllResult.begin() ;
	for ( ; iter != m_vAllResult.end(); ++iter)
	{
		delete *iter ;
		*iter = NULL ;
	}
	m_vAllResult.clear();
	mResultLock.Unlock() ;
}

void CDBRequestQueue::ClearAllReserveRequest()
{
	mReserveQuestLock.Lock();
	VEC_DBREQUEST::iterator iter = m_vReserveRequest.begin() ;
	for ( ; iter != m_vReserveRequest.end(); ++iter )
	{
		delete *iter ;
		*iter = NULL ;
	}
	m_vReserveRequest.clear() ;
	mReserveQuestLock.Unlock() ;
}
