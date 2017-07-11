#include "RobotDispatchStrategy.h"
#include <ctime>
#include "log4z.h"
#include "ServerMessageDefine.h"
#include "IRoomState.h"
#ifdef _DEBUG
	#define TIME_ROBOT_STAY 2*60
	#define  TIME_UPDATE_DISPATCH_TICK 1*30
#else
#define TIME_ROBOT_STAY 30*60
#define  TIME_UPDATE_DISPATCH_TICK 5*60
#endif // DEBUG


CRobotDispatchStrategy::CRobotDispatchStrategy()
{
	m_pRoom = nullptr ;
	m_vPlayingRobot.clear() ;
	m_nReqRobotLevel = 0 ;
	m_fUpdateDispatchTick = 0 ;
	m_nRoomID = 0 ;
	m_nSubRoomIdx = 0 ;
}

CRobotDispatchStrategy::~CRobotDispatchStrategy()
{
	for ( auto ref : m_vPlayingRobot )
	{
		delete ref ;
	}
	m_vPlayingRobot.clear() ;
	m_vMayDelayLeaveRobot.clear() ;
}

bool CRobotDispatchStrategy::init(ISitableRoom* pRoom , uint8_t nReqRobotLevel, uint32_t nRoomID , uint8_t nsubRoomIdx )
{
	m_pRoom = pRoom ;
	m_nReqRobotLevel = nReqRobotLevel ;
	m_nRoomID = nRoomID ;
	m_nSubRoomIdx = nsubRoomIdx ;
	return true ;
}

void CRobotDispatchStrategy::update(float fTicke)
{
	updateRobotDispatch(fTicke) ;
}

void CRobotDispatchStrategy::onRobotJoin(uint32_t nSessionID )
{
	stJoinRobot* pRet = new stJoinRobot ;
	pRet->nSessionID = nSessionID ;
	pRet->tLeaveTime = time(nullptr) + TIME_ROBOT_STAY ;
	m_vPlayingRobot.push_back(pRet) ;
}

void CRobotDispatchStrategy::onRobotLeave(uint32_t nSessioID )
{
	auto doL = m_vMayDelayLeaveRobot.find(nSessioID) ;
	if ( doL != m_vMayDelayLeaveRobot.end() )
	{
		m_vMayDelayLeaveRobot.erase(doL) ;
	}

	auto iter = m_vPlayingRobot.begin() ;
	for ( ; iter != m_vPlayingRobot.end() ; ++iter )
	{
		if ( (*iter)->nSessionID == nSessioID )
		{
			delete (*iter) ;
			(*iter) = nullptr ;
			m_vPlayingRobot.erase(iter) ;
			return ;
		}
	}
}

void CRobotDispatchStrategy::updateRobotDispatch( float fDelta )
{
	return ;
	if ( m_pRoom == nullptr )
	{
		return ;
	}

	m_fUpdateDispatchTick -= fDelta ;
	if ( m_fUpdateDispatchTick > 0 )
	{
		return ;
	}

	m_fUpdateDispatchTick = TIME_UPDATE_DISPATCH_TICK ;

	uint8_t nSitDownCnt = (uint8_t)m_pRoom->getSitDownPlayerCount() ;
	if ( nSitDownCnt > m_vMayDelayLeaveRobot.size() )
	{
		nSitDownCnt -= m_vMayDelayLeaveRobot.size() ;
	}

	bool bClosed = m_pRoom->getCurRoomState()->getStateID() == eRoomState_Close ;
	// check robot whethe time out ;
	if ( m_vPlayingRobot.empty() == false )
	{
		time_t tNow = time(nullptr) ;
		auto pPlayer = m_vPlayingRobot.front() ;
		if ( pPlayer && ( pPlayer->tLeaveTime <= tNow || bClosed )  )
		{
			if ( nSitDownCnt > 4 || bClosed )
			{
				LOGFMTD("robot session id = %u , time up shuld leave room" ) ;
				stMsgTellRobotLeaveRoom msgLeave ;
				m_pRoom->sendMsgToPlayer(&msgLeave,sizeof(msgLeave),pPlayer->nSessionID) ;
				m_vMayDelayLeaveRobot[pPlayer->nSessionID] = 1 ;
				delete pPlayer ;
				pPlayer = nullptr ;
				m_vPlayingRobot.pop_front() ;
			}
			else
			{
				stMsgRequestRobotToEnterRoom msgreq ;
				msgreq.nReqRobotLevel = m_nReqRobotLevel ;
				msgreq.nRoomID = m_nRoomID ;
				msgreq.nRoomType = m_pRoom->getRoomType() ;
				msgreq.nSubRoomIdx = m_nSubRoomIdx ;
				m_pRoom->sendMsgToPlayer(&msgreq,sizeof(msgreq),0) ;
				//LOGFMTD("too few player robot session id = %u delay leave, and req new player to join than leave",pPlayer->nSessionID) ;
			}
		}
	}


	uint8_t nSeatCnt = m_pRoom->getSeatCount() ;
	bool bHavePlayerAddRobt = false ;
	if ( m_pRoom->isHaveRealPlayer() && nSitDownCnt < ( nSeatCnt - 1 ) )
	{
		bHavePlayerAddRobt = ( rand() % 100 ) <= 65 ;
	}

	// situation need add robot ;
	// 1. player cnt < 3 , must req robot 
	// 2. when have player , and all sit down cnt < 4 , have 65% rate , req robot join;
	if ( (/*nSitDownCnt < 4*/ m_pRoom->getEmptySeatCount() > 2 || bHavePlayerAddRobt) && ( bClosed == false ) )
	{
		stMsgRequestRobotToEnterRoom msgreq ;
		msgreq.nReqRobotLevel = m_nReqRobotLevel ;
		msgreq.nRoomID = m_nRoomID ;
		msgreq.nRoomType = m_pRoom->getRoomType() ;
		msgreq.nSubRoomIdx = m_nSubRoomIdx ;
		m_pRoom->sendMsgToPlayer(&msgreq,sizeof(msgreq),0) ;
		//LOGFMTD("too few robot so req more , room id = %u",m_nRoomID) ;
		return ;
	}

	// robot must leave situation 
	// 1. already have 5 player sitdown , so must leave player ;
	// 2. room is full, robot must leave ;
	// 3.  current room do not have real player , but have more than 2 player sit down
	if ( m_pRoom->getEmptySeatCount() < 1 /*|| (nSitDownCnt > 3 && m_pRoom->isHaveRealPlayer() == false)*/ )
	{
		if ( m_vPlayingRobot.empty() == false )
		{
			auto pPlayerLeave = m_vPlayingRobot.front() ;
			if ( pPlayerLeave )
			{
				stMsgTellRobotLeaveRoom msgLeave ;
				m_pRoom->sendMsgToPlayer(&msgLeave,sizeof(msgLeave),pPlayerLeave->nSessionID) ;
				m_vMayDelayLeaveRobot[pPlayerLeave->nSessionID] = 1 ;
				delete pPlayerLeave ;
				pPlayerLeave = nullptr ;
				m_vPlayingRobot.pop_front() ;
				LOGFMTD("no real player so just need 2 robot , other just leave room id = %u",m_nRoomID);
			}
		}
	}
}