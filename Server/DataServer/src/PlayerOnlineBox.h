#pragma once
#include <time.h>
struct stMsg ;
struct stServerBaseData;
class CPlayerBaseData;
class CPlayerOnlineBox
{
public:
	CPlayerOnlineBox(CPlayerBaseData* pPlayerBaseData );
	~CPlayerOnlineBox();
	void StartTimer();
	void PauseTimer();
	bool OnMessage(stMsg* pMsg ); ;
	void OnPlayerDisconnect();
	void Reset();
	void Init(){Reset();}
	stServerBaseData* GetSeverBaseData();
protected:
	time_t m_nBeginTime ;
	bool m_bRunning ;
	CPlayerBaseData* m_pPlayerBaseData ;
};