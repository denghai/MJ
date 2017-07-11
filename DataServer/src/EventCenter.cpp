#include "EventCenter.h"
#include "log4z.h"
CEventCenter::CEventCenter()
{

}

CEventCenter::~CEventCenter()
{
	ProcessRemoveAnddAdd();

	for ( int i = 0 ; i < eEvent_Max ; ++i )
	{
		LIST_LISTENNER& vList = m_vAllEventListen[i] ;
		LIST_LISTENNER::iterator iter = vList.begin() ;
		for ( ; iter != vList.end(); ++iter )
		{
			delete *iter ;
			*iter = NULL ;
		}
		vList.clear() ;
	}
}

CEventCenter* CEventCenter::SharedEventCenter()
{
	static CEventCenter s_gCenter ;
	return &s_gCenter ;
}

void CEventCenter::PostEvent(stEventArg* pEventArg )
{
	ProcessRemoveAnddAdd();
	if ( pEventArg == NULL )
	{
		LOGFMTE("event arg is NULL ") ;
		return ;
	}

	if ( pEventArg->cEvent >= eEvent_Max )
	{
		LOGFMTE("event type error !") ;
		return ;
	}

	LIST_LISTENNER& vList =m_vAllEventListen[pEventArg->cEvent] ;
	LIST_LISTENNER::iterator iter = vList.begin();
	for ( ; iter != vList.end(); ++iter )
	{
		stEventListener* pListenner = *iter ;
		if ( pListenner && pListenner->pFunc(pListenner->pUserData,pEventArg) )
		{
			break;
		}
	}
}

void CEventCenter::RegisterEventListenner(eEventType eEvent , void* pUserData, lpEventFunc pFunc,short nOrder )  // small order process first ;
{
	if ( eEvent >= eEvent_Max )
	{
		LOGFMTE("event type error !RegisterEventListenner" ) ;
		return ;
	}

	stEventListener* pListener =new stEventListener ;
	pListener->eEvent = eEvent ;
	pListener->nOrder = nOrder ;
	pListener->pFunc = pFunc ;
	pListener->pUserData = pUserData ;
	m_vWillAdd.push_back(pListener) ;
}

void CEventCenter::RemoveEventListenner(eEventType eEvent , void* pUserData, lpEventFunc pFunc)
{
	if ( eEvent >= eEvent_Max )
	{
		LOGFMTE("event type error !RemoveEventListenner") ;
		return ;
	}

	stEventListener* pListener =new stEventListener ;
	pListener->eEvent = eEvent ;
	pListener->pFunc = pFunc ;
	pListener->pUserData = pUserData ;
	m_vWillRemove.push_back(pListener) ;
}

void CEventCenter::ProcessRemoveAnddAdd()
{
	// process remove ;
	LIST_LISTENNER::iterator iter = m_vWillRemove.begin();
	for ( ; iter != m_vWillRemove.end(); ++iter )
	{
		stEventListener* pListen = *iter ;
		if ( !pListen )
		{
			continue;
		}

		LIST_LISTENNER& vList = m_vAllEventListen[pListen->eEvent] ;
		LIST_LISTENNER::iterator iter_remove = vList.begin();
		for ( ; iter_remove != vList.end(); ++iter_remove )
		{
			stEventListener* pRemove = *iter_remove ;
			if ( pListen->pUserData == pRemove->pUserData && pListen->pFunc == pRemove->pFunc )
			{
				delete *iter_remove ;
				*iter_remove = NULL ;
				vList.erase(iter_remove) ;
				break;
			}
		}
		delete pListen ;
		pListen = NULL ;
	}
	m_vWillRemove.clear();
	
	// process add 
	iter = m_vWillAdd.begin();
	for ( ; iter != m_vWillAdd.end(); ++iter )
	{
		stEventListener* pListen = *iter ;
		if ( !pListen )
		{
			continue; 
		}

		LIST_LISTENNER& vList = m_vAllEventListen[pListen->eEvent] ;

		// check weather can direct push_back ;
		if ( vList.empty() )
		{
			vList.push_back(pListen) ;
			continue;;
		}

		stEventListener* pFront = vList.back();
		if ( pFront->nOrder <= pListen->nOrder )
		{
			vList.push_back(pListen) ;
			continue;;
		}

		// iter to find a pos to inster ;
		LIST_LISTENNER::iterator iter_add = vList.begin();
		for ( ; iter_add != vList.end(); ++iter_add )
		{
			if ( (*iter_add)->nOrder > pListen->nOrder )
			{
				vList.insert(iter_add,pListen) ;
				break;
			}
		}
	}
	m_vWillAdd.clear();
}