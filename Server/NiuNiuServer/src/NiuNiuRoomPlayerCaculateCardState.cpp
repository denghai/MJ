#include "NiuNiuRoomPlayerCaculateCardState.h"
#include "ISitableRoom.h"
#include "NiuNiuMessageDefine.h"
#include "NiuNiuRoomGameResult.h"
#include "ISitableRoomPlayer.h"
#include "log4z.h"
void CNiuNiuRoomStatePlayerCaculateCardState::enterState(IRoom* pRoom)
{
	m_pRoom = (ISitableRoom*)pRoom;
	m_nWaitOperPlayerCnt = (uint8_t)m_pRoom->getPlayerCntWithState(eRoomPeer_CanAct) ;
	setStateDuringTime(TIME_NIUNIU_PLAYER_CACULATE_CARD) ;
	LOGFMTD("enter cacualte card state");
	m_vOperoateRecord.clear();
}

void CNiuNiuRoomStatePlayerCaculateCardState::onStateDuringTimeUp()
{
	m_pRoom->goToState(CNiuNiuRoomGameResultState::eStateID);
}

bool CNiuNiuRoomStatePlayerCaculateCardState::onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID )
{
	if ( IRoomState::onMessage(prealMsg,eSenderPort,nPlayerSessionID) )
	{
		return true ;
	}

	switch ( prealMsg->usMsgType )
	{
	case MSG_NN_PLAYER_CACULATE_CARD_OK:
		{
			ISitableRoomPlayer* pPlayer = m_pRoom->getSitdownPlayerBySessionID(nPlayerSessionID) ;
			if ( pPlayer == nullptr )
			{
				LOGFMTE("you are not sit down how to caculate card session id = %d",nPlayerSessionID) ;
				return true ;
			}

			if ( pPlayer->isHaveState(eRoomPeer_CanAct) == false )
			{
				LOGFMTE("you are not player in this game , how to cacuate card session id = %d", nPlayerSessionID ) ;
				return true ;
			}

			if ( m_vOperoateRecord.find(nPlayerSessionID) != m_vOperoateRecord.end() )
			{
				LOGFMTE("don't do twice cacualte card session id %d",nPlayerSessionID ) ;
				return true ;
			}
			m_vOperoateRecord[nPlayerSessionID] = 1 ;

			--m_nWaitOperPlayerCnt;
			stMsgNNCaculateCardOk msg ;
			msg.nPlayerIdx = pPlayer->getIdx() ;
			m_pRoom->sendRoomMsg(&msg,sizeof(msg));
			if ( m_nWaitOperPlayerCnt <= 0 )
			{
				onStateDuringTimeUp();
			}
		}
		break;
	default:
		return false;
	}
	return true ;
}