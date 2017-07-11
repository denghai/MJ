#include "Timer.h"
#include "log4z.h"
#include <time.h>
#include <cassert>
// timer
uint32_t CTimer::s_TimerCount = 0 ;
CTimer::CTimer()
{
	m_nTimeID = ++s_TimerCount ;
	m_fInterval = 0 ;
	m_eState = eTimerState_None ;
	m_fIntervalKeeper = 0 ;
	m_lpFunc = nullptr ;
	m_isAutoRepeat = false ;
}

CTimer::~CTimer()
{
	if ( eTimerState_Runing == m_eState )
	{
		LOGFMTE("you should canncel the time , befor delete it") ;
		canncel() ;
	}
}

void CTimer::canncel()
{
	if ( eTimerState_Runing == m_eState )
	{
		m_eState = eTimerState_None ;
		CTimerManager::getInstance()->canncelTimer(this) ;
		return ;
	}
	LOGFMTE("timer is not running how to canncel !") ;
}

void CTimer::setIsAutoRepeat( bool isRepeat )
{
	m_isAutoRepeat = isRepeat ;
}

void CTimer::setCallBack( time_func lpfCallBackFunc )
{
	m_lpFunc = lpfCallBackFunc ;
}

void CTimer::setInterval(float fNewInterval )
{
	m_fInterval = fNewInterval ;
}

void CTimer::start()
{
	assert(m_eState != eTimerState_Runing && "timer already running , do not run again" );
	assert( m_lpFunc != nullptr && "please set timer func first");
	if ( m_eState == eTimerState_Runing )
	{
		LOGFMTE("timer already running , don't runed it again") ;
		return ;
	}
	m_eState = eTimerState_Runing ;
	CTimerManager::getInstance()->startTimer(this) ;
}

void CTimer::reset()
{
	if ( eTimerState_Runing == m_eState )
	{
		canncel() ;
	}
	m_eState = eTimerState_None ;
	m_fIntervalKeeper = 0 ;
}

void CTimer::Update(float fTimeElaps)
{
	if ( eTimerState_Runing != m_eState )
		return ;

	// prcess interval ;
	if ( (m_fIntervalKeeper += fTimeElaps) < m_fInterval )
	{
		return ;
	}

	// invoke funcion ;
	assert(m_lpFunc != nullptr && "timer func can not be null" ) ;
	if ( m_lpFunc )
	{
		m_lpFunc(this,m_fInterval <= 0.00001 ? fTimeElaps : m_fInterval ) ;
	}

	if ( m_isAutoRepeat )
	{
		m_fIntervalKeeper -= m_fInterval ;
	}
	else
	{
		canncel() ;
	}
}

uint32_t CTimer::getTimerID()
{
	return m_nTimeID ;
}

// timer manager ;
CTimerManager::CTimerManager()
{
	m_vRunningTimers.clear() ;
	m_fTimerScale = 1.0 ;
	m_vWillCanncelTimer.clear() ;
	m_vWillRunningTimers.clear() ;
	m_isLocked = false ;
}

CTimerManager::~CTimerManager()
{
	m_vRunningTimers.clear() ;
	m_fTimerScale = 1.0 ;
	m_vWillCanncelTimer.clear() ;
	m_vWillRunningTimers.clear() ;
}

void CTimerManager::startTimer(CTimer* pTimer )
{
	if ( pTimer == nullptr )return;

	if ( isLocked() )
	{
		m_vWillRunningTimers.push_back(pTimer) ;
	}
	else
	{
		auto iter = m_vRunningTimers.find(pTimer->getTimerID()) ;
		assert(iter == m_vRunningTimers.end() && "already running , why run it again why");
		if ( iter == m_vRunningTimers.end() )
		{
			m_vRunningTimers[pTimer->getTimerID()] = pTimer ;
		}
	}
}

void CTimerManager::Update( float fDelta )
{
	m_isLocked = true ;
	for ( auto iter : m_vRunningTimers )
	{
		if ( iter.second )
		{
			iter.second->Update(fDelta * m_fTimerScale );
		}
	}
	m_isLocked = false ;

	// do remove timers ;
	for ( auto pCanneclID : m_vWillCanncelTimer )
	{
		auto iter = m_vRunningTimers.find(pCanneclID) ;
		assert(iter != m_vRunningTimers.end() && "not running why , canncel");
		if ( iter != m_vRunningTimers.end() )
		{
			m_vRunningTimers.erase(iter) ;
		}
	}
	m_vWillCanncelTimer.clear() ;

	// do add running 
	for ( auto prun : m_vWillRunningTimers )
	{
		auto iter = m_vRunningTimers.find(prun->getTimerID()) ;
		assert(iter == m_vRunningTimers.end() && "already running , why run it again");
		if ( iter == m_vRunningTimers.end() )
		{
			m_vRunningTimers[prun->getTimerID()] = prun ;
		}
	}

	m_vWillRunningTimers.clear() ;
}

void CTimerManager::canncelTimer( CTimer* pTimer )
{
	if ( pTimer == nullptr )return;
	auto iter = m_vRunningTimers.find(pTimer->getTimerID()) ;
	assert(iter != m_vRunningTimers.end() && "not running why , canncel, why ");
	if ( iter == m_vRunningTimers.end() )
	{
		return ;
	}

	if ( isLocked() )
	{
		iter->second = nullptr ;
		m_vWillCanncelTimer.push_back(pTimer->getTimerID()) ;
	}
	else
	{
		m_vRunningTimers.erase(iter) ;
	}
	
}
