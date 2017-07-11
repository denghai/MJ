#include "NiuNiuRoomTryBankerState.h"
#include "NiuNiuRoom.h"
#include "NiuNiuMessageDefine.h"
#include "NiuNiuRoomPlayer.h"
#include "NiuNiuRoomBetState.h"
#include "NiuNiuRoomRandBankerState.h"
#include "log4z.h"
void CNiuNiuRoomTryBanker::enterState(IRoom* pRoom)
{
	m_pRoom = (CNiuNiuRoom*)pRoom ;
	m_nBiggestTimeTryBanker = 0 ;
	setStateDuringTime(TIME_NIUNIU_TRY_BANKER) ;
	m_nLeftTryBankerPlayerCnt = (uint8_t)m_pRoom->getPlayerCntWithState(eRoomPeer_CanAct) ;
	LOGFMTD("room id = %d , try banker ",m_pRoom->getRoomID());
}

bool CNiuNiuRoomTryBanker::onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID )
{
	if ( IRoomState::onMessage(prealMsg,eSenderPort,nPlayerSessionID) )
	{
		return true ;
	}

	if ( prealMsg->usMsgType == MSG_NN_PLAYER_TRY_BANKER )
	{
		stMsgNNPlayerTryBankerRet msgBack ;
		stMsgNNPlayerTryBanker* pTryBanker = (stMsgNNPlayerTryBanker*)prealMsg ;
		CNiuNiuRoomPlayer* pPlayer = (CNiuNiuRoomPlayer*)m_pRoom->getSitdownPlayerBySessionID(nPlayerSessionID) ;
		//SLOGFMTD("try banker uid = %d ,times = %d" , pPlayer->getUserUID(), pTryBanker->nTryBankerBetTimes) ;
		if ( pPlayer == nullptr )
		{
			msgBack.nRet = 3 ;
		}
		else if ( pPlayer->isHaveState(eRoomPeer_CanAct) == false )
		{
			 msgBack.nRet = 1 ;
			 LOGFMTD("try banker state error uid = %d ,times = %d" , pPlayer->getUserUID(), pTryBanker->nTryBankerBetTimes) ;
		}
		else if ( pPlayer->getTryBankerTimes() != 0  )
		{
			msgBack.nRet = 4 ;
			 LOGFMTD("try banker one more times  error uid = %d ,times = %d" , pPlayer->getUserUID(), pTryBanker->nTryBankerBetTimes) ;
		}
		else if ( (uint64_t)pPlayer->getCoin() < m_pRoom->getLeastCoinNeedForBeBanker( pTryBanker->nTryBankerBetTimes ) )
		{
			msgBack.nRet = 2 ;
			LOGFMTD("try banker coin not engough uid = %d ,times = %d" , pPlayer->getUserUID(), pTryBanker->nTryBankerBetTimes) ;
		}
		else
		{
			--m_nLeftTryBankerPlayerCnt ;
			msgBack.nRet = 0 ;
			if ( m_nBiggestTimeTryBanker < pTryBanker->nTryBankerBetTimes )
			{
				m_nBiggestTimeTryBanker = pTryBanker->nTryBankerBetTimes ;
			}
			pPlayer->setTryBankerTimes(pTryBanker->nTryBankerBetTimes) ;
		}

		m_pRoom->sendMsgToPlayer(&msgBack,sizeof(msgBack),nPlayerSessionID) ;
		if ( msgBack.nRet == 0 )
		{
			stMsgNNTryBanker msgTry ;
			msgTry.nTryBankerBetTimes = pTryBanker->nTryBankerBetTimes ;
			msgTry.nTryerIdx = pPlayer->getIdx() ;
			m_pRoom->sendRoomMsg(&msgTry,sizeof(msgTry)) ;
			m_pRoom->setBankCoinLimitForBet(pPlayer->getCoin()) ;
			LOGFMTD("try banker ok uid = %d ,times = %d" , pPlayer->getUserUID(), pTryBanker->nTryBankerBetTimes) ;
		}

		// if erveryone have bet , then end this state 
		if ( m_nLeftTryBankerPlayerCnt <= 0 )
		{
			onStateDuringTimeUp();
		}
		//LOGFMTD("try banker uid = %d ",pPlayer->getUserUID() ) ;
		return true ;
	}
	return false ;
}

void CNiuNiuRoomTryBanker::onStateDuringTimeUp()
{
	CNiuNiuRoom::LIST_SITDOWN_PLAYERS vMaybeBanker ;
	uint8_t nSeatCnt = (uint8_t)m_pRoom->getSeatCount() ;
	for ( uint8_t nIdx = 0 ; nIdx < nSeatCnt ; ++nIdx )
	{
		CNiuNiuRoomPlayer* pPlayer = (CNiuNiuRoomPlayer*)m_pRoom->getPlayerByIdx(nIdx) ;
		if ( pPlayer && pPlayer->isHaveState(eRoomPeer_CanAct) && pPlayer->getTryBankerTimes() == m_nBiggestTimeTryBanker )
		{
			vMaybeBanker.push_back(pPlayer) ;
		}
	}

	m_pRoom->setBetBottomTimes(m_nBiggestTimeTryBanker);
	if ( vMaybeBanker.size() == 1 )
	{
		m_pRoom->setBankerIdx(vMaybeBanker.front()->getIdx()) ;
		// send msg tell banker idx ;
		stMsgNNProducedBanker msgBanker ;
		msgBanker.nBankerBetTimes = m_nBiggestTimeTryBanker ;
		msgBanker.nBankerIdx = m_pRoom->getBankerIdx();
		m_pRoom->sendRoomMsg(&msgBanker,sizeof(msgBanker)) ;

		m_pRoom->goToState(eRoomState_NN_StartBet);
	}
	else if ( vMaybeBanker.size() > 1 )
	{
		m_pRoom->goToState(CNiuNiuRoomRandBankerState::eStateID) ;
	}
}