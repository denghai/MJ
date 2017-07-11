#pragma once
#include "ISitableRoom.h"
#include <list>
class CRobotDispatchStrategy
{
public:
	struct stJoinRobot
	{
		uint32_t nSessionID ;
		time_t tLeaveTime ;
	};
	typedef std::list<stJoinRobot*> LIST_JOIN_ROBOT ;
	typedef std::map<uint32_t,int> MAP_SESSSION_IDS ;
public:
	CRobotDispatchStrategy();
	~CRobotDispatchStrategy();
	bool init(ISitableRoom* pRoom , uint8_t nReqRobotLevel, uint32_t nRoomID , uint8_t nsubRoomIdx );
	void update(float fTicke);
	void onRobotJoin(uint32_t nSessionID );
	void onRobotLeave(uint32_t nSessioID );
protected:
	void updateRobotDispatch( float fDelta );
protected:
	ISitableRoom* m_pRoom ;
	uint32_t m_nRoomID ;
	uint8_t m_nSubRoomIdx ;
	uint8_t m_nReqRobotLevel ; 
	LIST_JOIN_ROBOT m_vPlayingRobot ;
	float m_fUpdateDispatchTick ;
	MAP_SESSSION_IDS m_vMayDelayLeaveRobot;
};