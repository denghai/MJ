#pragma once
#include <list>
class CPlayer ;
class CRoomBaseNew;
class CRobotManager
{
public:
	struct stRequestRobotRoom
	{
		unsigned char nRoomType ;
		unsigned char cLevel ;
		int nRoomID ;
		short nLackRobotCnt ;
	};
	typedef std::list<CPlayer*> LIST_ROBOT_PLAYERS ;
	typedef std::list<stRequestRobotRoom> LIST_ROOMS ;
public:
	CRobotManager(){  m_vIdleRobotPlayers.clear();}
	~CRobotManager(){  m_vIdleRobotPlayers.clear() ;}
	static CRobotManager* SharedRobotMgr();
	void AddIdleRobotPlayer(CPlayer* pPlayer );   // when robot login ;
	void OnPlayerDisconnected(CPlayer* pPlayer );   // when infom idle robot container logout ;
	void RequestRobotToJoin(CRoomBaseNew* pRoom);
protected:
	void OrderRobotToRoom();
protected:
	LIST_ROBOT_PLAYERS m_vIdleRobotPlayers ;
	LIST_ROOMS m_vRoomsRequestRobots ;
};