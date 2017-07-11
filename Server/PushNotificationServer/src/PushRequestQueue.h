#pragma once
#include <list>
#include "mutex.h"
class CPushRequestQueue;
struct stNotice
{
	unsigned short nBadge ;
	char cSound[50];   // sound file name ; the file must exsit in client ;
	char pDeveiceToken[32] ;  // must proccesed in client ; change to htonl();  // change to network big endain ;
	char pAlert[219];    // a json right value , Warnning: must include [" " ] if not { } value ;
protected:
	stNotice(){ memset(cSound,0,sizeof(cSound)); memset(pAlert,0,sizeof(pAlert)); }
	friend class CPushRequestQueue ;
};

typedef std::list<stNotice*> LIST_NOTICES ;

class CPushRequestQueue
{
public:
	static CPushRequestQueue* SharedPushRequestQueue();
	CPushRequestQueue(){m_vAllNotices.clear(); m_vReserverNotices.clear();}
	~CPushRequestQueue(){Clear(m_vAllNotices);Clear(m_vReserverNotices);}
	void PushNotice(stNotice* pNotice );
	stNotice* GetReserverNotice();
	void GetAllNoticeToProcess(LIST_NOTICES& vAllNotices );
	void PushProcessedNotice(LIST_NOTICES& vAllProcessedNotice );
protected:
	void Clear(LIST_NOTICES& vNotices );
protected:
	LIST_NOTICES m_vAllNotices;
	LIST_NOTICES m_vReserverNotices ;
	Mutex mRequestLock ;
	Mutex mReserveQuestLock ;
protected:
	static CPushRequestQueue* s_pushQueue ;
};
