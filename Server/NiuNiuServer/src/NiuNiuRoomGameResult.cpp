#include "NiuNiuRoomGameResult.h"
#include "NiuNiuRoom.h"
#include "NiuNiuRoomPlayer.h"
#include "NiuNiuPeerCard.h"
#include "NiuNiuMessageDefine.h"
#include "AutoBuffer.h"
#include "log4z.h"
void CNiuNiuRoomGameResultState::enterState(IRoom* pRoom)
{
	LOGFMTD("room id = %d game result ",pRoom->getRoomID()) ;
	m_pRoom = (CNiuNiuRoom*)pRoom ;
	m_pRoom->caculateGameResult() ;
	setStateDuringTime(TIME_NIUNIU_GAME_RESULT_PER_PLAYER * m_pRoom->getPlayerCntWithState(eRoomPeer_CanAct) + TIME_NIUNIU_GAME_RESULT_EXT ) ;
}

void CNiuNiuRoomGameResultState::onStateDuringTimeUp()
{
	m_pRoom->onGameDidEnd();
	m_pRoom->goToState(eRoomState_WaitJoin) ;
}