#pragma once
#include <map>
#include <list>
#include "Events.h"
typedef bool (*lpEventFunc)(void* pUserData,stEventArg* pArg );
struct stEventListener
{
	short nOrder ;
	void* pUserData ;
	lpEventFunc pFunc ;
	eEventType eEvent ;
};

class CEventCenter
{
public:
	typedef std::list<stEventListener*> LIST_LISTENNER ;
public:
	CEventCenter();
	~CEventCenter();
	static CEventCenter* SharedEventCenter();
	void PostEvent(stEventArg* pEventArg );
	void PostEvent(eEventType eEvent , void* pData ){ stEventArg arg ; arg.cEvent = eEvent; arg.pData = pData ; PostEvent(&arg) ;}
	void RegisterEventListenner(eEventType eEvent , void* pUserData, lpEventFunc pFunc,short nOrder = 0 );  // small order process first ;
	void RemoveEventListenner(eEventType eEvent , void* pUserData, lpEventFunc pFunc);
protected:
	void ProcessRemoveAnddAdd();
protected:
	LIST_LISTENNER m_vAllEventListen[eEvent_Max] ;
	LIST_LISTENNER m_vWillAdd;
	LIST_LISTENNER m_vWillRemove ;
};