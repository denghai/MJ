#include "RobotManager.h"
#include "LogManager.h"
#include "Player.h"
#include "PlayerBaseData.h"
#include "RoomManager.h"
#include "RoomBase.h"
#include "GameServerApp.h"
#include "MessageDefine.h"
#include "CommonDefine.h"
#include "RoomBaseNew.h"
#include "CommonData.h"
#include "RoomBaseNew.h"
#include "RoomBaseData.h"
CRobotManager* CRobotManager::SharedRobotMgr()
{
	static CRobotManager s_gRobtMgr ;
	return &s_gRobtMgr ;
}

void CRobotManager::AddIdleRobotPlayer(CPlayer* pPlayer )
{
#ifndef NDEBUG
	LIST_ROBOT_PLAYERS::iterator iter = m_vIdleRobotPlayers.begin() ;
	for ( ; iter != m_vIdleRobotPlayers.end() ; ++iter )
	{
		if ( *iter == pPlayer )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("this robot already in idle list , name = %s",pPlayer->GetBaseData()->GetPlayerName());
			return ;
		}
	}
#endif
	m_vIdleRobotPlayers.push_back(pPlayer) ;
	OrderRobotToRoom();
}

void CRobotManager::OnPlayerDisconnected(CPlayer* pPlayer )
{
	LIST_ROBOT_PLAYERS::iterator iter = m_vIdleRobotPlayers.begin() ;
	for ( ; iter != m_vIdleRobotPlayers.end() ; ++iter )
	{
		if ( *iter == pPlayer )
		{
			m_vIdleRobotPlayers.erase(iter) ;
			return ;
		}
	}
}

void CRobotManager::RequestRobotToJoin(CRoomBaseNew* pRoom)
{
	stRequestRobotRoom rs ;
	rs.cLevel = pRoom->GetRoomDataOnly()->nRoomLevel ;
	rs.nRoomID = pRoom->GetRoomID() ;
	rs.nRoomType = pRoom->GetRoomType() ;
	// not only texasPoker but also other two type game
	//if ( rs.nRoomType != eRoom_TexasPoker && rs.nRoomType != eRoom_TexasPoker_Diamoned )
	//{
	//	return ;
	//}
	rs.nLackRobotCnt = MIN_PEERS_IN_ROOM_ROBOT - pRoom->GetData()->GetPlayingSeatCnt() ;
	if ( rs.nLackRobotCnt <= 0 )
	{
		CLogMgr::SharedLogMgr()->PrintLog("we think the room need not robot to join") ;
		return ;
	}

	for ( LIST_ROOMS::iterator iter = m_vRoomsRequestRobots.begin() ; iter != m_vRoomsRequestRobots.end(); ++iter )
	{
		if ( (*iter).nRoomID == rs.nRoomID )
		{
			(*iter).nLackRobotCnt = rs.nLackRobotCnt < (*iter).nLackRobotCnt ? rs.nLackRobotCnt : (*iter).nLackRobotCnt;  // get the min value , robot maybe on the way to room , just wait ;
			OrderRobotToRoom() ;
			return ;
		}
	}

	m_vRoomsRequestRobots.push_back(rs) ;
	OrderRobotToRoom() ;
}

void CRobotManager::OrderRobotToRoom()
{
	if ( m_vRoomsRequestRobots.empty() || m_vIdleRobotPlayers.empty() )
	{
		return ;
	}

	LIST_ROOMS::iterator iter = m_vRoomsRequestRobots.begin() ;
	while ( iter != m_vRoomsRequestRobots.end() && m_vIdleRobotPlayers.empty() == false )
	{
		stRequestRobotRoom* stR = &(*iter);
		CRoomBaseNew* pRoom  = CGameServerApp::SharedGameServerApp()->GetRoomMgr()->GetRoom(stR->nRoomType,stR->cLevel,stR->nRoomID) ;
		if ( pRoom == NULL )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("why room is NULL , reclyed ?") ;
		}
		// check if need robot ;
		if ( pRoom == NULL || (MIN_PEERS_IN_ROOM_ROBOT - pRoom->GetData()->GetPlayingSeatCnt()) <= 0 )
		{
			m_vRoomsRequestRobots.erase(iter) ;
			iter = m_vRoomsRequestRobots.begin();
			continue;
		}
		// avoid more than once add robot to room ;

		// real put robot in that room ;
		LIST_ROBOT_PLAYERS::iterator iter_Robot = m_vIdleRobotPlayers.begin() ;
		for ( ; iter_Robot != m_vIdleRobotPlayers.end() ; )
		{
			CPlayer* pRobot = *iter_Robot ;
			if ( pRobot && pRobot->GetState() == CPlayer::ePlayerState_Free && (pRoom->CheckCanJoinThisRoom(pRobot) == 0) && pRobot->GetBaseData()->GetAllCoin() >= pRoom->GetAntesCoin() )
			{
				 // send msg tell to enter this room ;
				stMsgRobotOrderToEnterRoom  msgEnter ;
				msgEnter.cLevel = stR->cLevel;
				msgEnter.nRoomID = stR->nRoomID ;
				msgEnter.nRoomType = stR->nRoomType ;
				pRobot->SendMsgToClient((char*)&msgEnter,sizeof(msgEnter)) ;

				stR->nLackRobotCnt-- ;
				m_vIdleRobotPlayers.erase(iter_Robot) ;
				iter_Robot = m_vIdleRobotPlayers.begin() ;
				if ( stR->nLackRobotCnt <= 0 )
				{
					break;;
				}
				continue;
			}
			 ++iter_Robot ;
		}

		// go to next room ;
		if (  stR->nLackRobotCnt <= 0 )
		{
			m_vRoomsRequestRobots.erase(iter) ;
			iter = m_vRoomsRequestRobots.begin();
			continue;
		}
		++iter ;
	}
}