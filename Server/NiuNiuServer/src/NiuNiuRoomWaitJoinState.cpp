#include "NiuNiuRoomWaitJoinState.h"
#include "ISitableRoom.h"
#include "NiuNiuRoomDistribute4CardState.h"
#include "LogManager.h"
void CNiuNiuRoomWaitJoinState::enterState(IRoom* pRoom)
{
	m_pRoom = (ISitableRoom*)pRoom ;
	CLogMgr::SharedLogMgr()->PrintLog("enter wai join state room id = %d",m_pRoom->getRoomID());
}

void CNiuNiuRoomWaitJoinState::update( float fDeta )
{
	if ( m_pRoom && m_pRoom->isRoomAlive() == false )
	{
		m_pRoom->goToState(IRoomStateDead::eStateID);
		return ;
	}

	if ( m_pRoom && m_pRoom->getPlayerCntWithState(eRoomPeer_WaitNextGame) >= 2 )
	{
		m_pRoom->goToState(CNiuNiuRoomDistribute4CardState::eStateID);
	}
}