#pragma once
#include <map>
#include <vector>
#include <functional>  
#include "Singleton.h"
#include "NativeTypes.h"
#define timer_bind_obj_func(obj,func ) std::bind(&func,obj,std::placeholders::_1,std::placeholders::_2)
//warning : time merasuse by second ;
class CTimer
{
public:
	enum eTimerState
	{
		eTimerState_None,
		eTimerState_Runing,
		eTimerState_Max,
	};
	typedef std::function<void (CTimer*, float fDelta )> time_func ;

public:
	CTimer();
	~CTimer();
	void reset();
	void canncel();
	void setIsAutoRepeat( bool isAutoRepeat );
	void setCallBack( time_func lpfCallBackFunc );
	void setInterval(float fNewInterval );
	void start();
protected:
	void Update( float fTimeElaps );
	uint32_t getTimerID();
	friend class CTimerManager ;
protected:
	static uint32_t s_TimerCount ;
	uint32_t m_nTimeID ;

	float m_fIntervalKeeper ;
	float m_fInterval ;

	eTimerState m_eState ;

	time_func m_lpFunc ;

	bool m_isAutoRepeat ;
};

class CTimerManager
	:public CSingleton<CTimerManager>
{
public:
	typedef std::map<unsigned int ,CTimer*> MAP_TIMERS ;
	typedef std::vector<CTimer*> VEC_TIME ;
	typedef std::vector<uint32_t> VEC_TIME_ID ;
protected:
	CTimerManager();
public:
	~CTimerManager();
	void Update(float fDelta );
	void SetTimeScale( float fScale ){ m_fTimerScale = fScale ;}
	float GetTimeScale(){ return m_fTimerScale ;} 
protected:
	void startTimer( CTimer* pTimer ); 
	void canncelTimer(CTimer* pTimer );
	bool isLocked(){ return m_isLocked ;}
protected:
	friend class CTimer ;
	friend class CSingleton<CTimerManager> ;
protected:
	MAP_TIMERS m_vRunningTimers ;
	VEC_TIME_ID m_vWillCanncelTimer;
	VEC_TIME m_vWillRunningTimers;
	float m_fTimerScale ;
	bool m_isLocked ;
};
