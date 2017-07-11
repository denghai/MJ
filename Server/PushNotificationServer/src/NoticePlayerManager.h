#pragma once
#include <string>
#include <map>
#include <list>
#include "ServerMessageDefine.h"
struct stNoticePlayer
	:public stNoticePlayerEntry
{
	struct stWaitNotice
	{
		uint32_t nType ;
		std::string strContent ;
	};


	bool isHaveFlag( uint32_t nFlag )
	{
		return (nNoticeFlag & nFlag) == nFlag ;
	}

	void pushNotice(const char* pNotices , uint32_t nNoticFlag);

	void doReadData();
	stNoticePlayer(){ tStartReadingTime = 0 ; bReadingData = false ; }
public:
	std::list<stWaitNotice> vWaitPushNotices ;
	time_t tStartReadingTime ;
	bool bReadingData ;
};

class CNoticePlayerManager
{
public:
	CNoticePlayerManager();
	~CNoticePlayerManager() ;
	bool onMsg(stMsg* pmsg,uint32_t nSessionID );
	void pushNotice(uint32_t nUserUID , const char* pNoticeContent, uint32_t nNoticFlag );
	stNoticePlayer* getNoticePlayer(uint32_t nUserUID );
protected:
	std::map<uint32_t,stNoticePlayer*> m_vNoticePlayers ;
};