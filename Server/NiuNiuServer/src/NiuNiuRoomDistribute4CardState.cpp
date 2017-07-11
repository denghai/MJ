#include "NiuNiuRoomDistribute4CardState.h"
#include "NiuNiuRoomTryBankerState.h"
#include "ISitableRoom.h"
#include "NiuNiuRoomPlayer.h"
#include "NiuNiuMessageDefine.h"
#include "AutoBuffer.h"
#include "log4z.h"
void CNiuNiuRoomDistribute4CardState::enterState(IRoom* pRoom)
{
	m_pRoom = (ISitableRoom*)pRoom ;

	// distribute card ;
	uint8_t nPlayerCnt = (uint8_t)m_pRoom->getPlayerCntWithState(eRoomPeer_CanAct) ;
	// send msg ;
	stMsgNNDistriute4Card msgForCard ;
	msgForCard.nPlayerCnt = nPlayerCnt;
	CAutoBuffer buffer(sizeof(msgForCard) + msgForCard.nPlayerCnt * sizeof(stDistriuet4CardItem)) ;
	buffer.addContent(&msgForCard,sizeof(msgForCard)) ;
	for ( uint8_t nIdx = 0 ; nIdx < m_pRoom->getSeatCount() ; ++nIdx )
	{
		CNiuNiuRoomPlayer* pRoomPlayer = (CNiuNiuRoomPlayer*)m_pRoom->getPlayerByIdx(nIdx) ;
		if ( pRoomPlayer && pRoomPlayer->isHaveState(eRoomPeer_CanAct))
		{
			 stDistriuet4CardItem item ;
			 item.nSeatIdx = pRoomPlayer->getIdx();
			 for ( uint8_t nIdx = 0 ; nIdx < 5 ; ++nIdx )
			 {
				 item.vCardCompsitNum[nIdx] = pRoomPlayer->getCardByIdx(nIdx);
			 }
			 buffer.addContent(&item,sizeof(item)) ;
		}
	}

	m_pRoom->sendRoomMsg((stMsg*)buffer.getBufferPtr(),buffer.getContentSize()) ;

	setStateDuringTime( m_pRoom->getPlayerCntWithState(eRoomPeer_CanAct) * TIME_NIUNIU_DISTRIBUTE_4_CARD_PER_PLAYER );
	LOGFMTD("player cnt = %d room id = %d , distribute 4 card",nPlayerCnt,m_pRoom->getRoomID());
}

void CNiuNiuRoomDistribute4CardState::onStateDuringTimeUp()
{
	m_pRoom->goToState(eRoomState_NN_FinalCard) ;
}