#include "NiuNiuRoomRandBankerState.h"
#include "NiuNiuRoomBetState.h"
#include "NiuNiuRoom.h"
#include "NiuNiuRoomPlayer.h"
#include "NiuNiuMessageDefine.h"
#include "log4z.h"
void CNiuNiuRoomRandBankerState::enterState(IRoom* pRoom)
{
	m_pRoom = (CNiuNiuRoom*)pRoom ;

	//CNiuNiuRoom::LIST_SITDOWN_PLAYERS vMaybeBanker ;
	//uint8_t nSeatCnt = (uint8_t)m_pRoom->getSeatCount() ;
	//int64_t nLeatCoinNeedForBankerWhenNoneTryBanker = 0 ;
	//if ( m_pRoom->getBetBottomTimes() == 0 )
	//{
	//	nLeatCoinNeedForBankerWhenNoneTryBanker = m_pRoom->getLeastCoinNeedForBeBanker(1) ;
	//}

	//CNiuNiuRoomPlayer* pRichestPlayer = nullptr ;
	//for ( uint8_t nIdx = 0 ; nIdx < nSeatCnt ; ++nIdx )
	//{
	//	CNiuNiuRoomPlayer* pPlayer = (CNiuNiuRoomPlayer*)m_pRoom->getPlayerByIdx(nIdx) ;
	//	if ( pPlayer && pPlayer->isHaveState(eRoomPeer_CanAct) && pPlayer->getTryBankerTimes() == m_pRoom->getBetBottomTimes() )
	//	{
	//		
	//		if ( pPlayer->getTryBankerTimes() == 0 && ((int64_t)pPlayer->getCoin() >= nLeatCoinNeedForBankerWhenNoneTryBanker ) )
	//		{
	//			vMaybeBanker.push_back(pPlayer) ;
	//		}

	//		if ( pRichestPlayer == nullptr )
	//		{
	//			pRichestPlayer = pPlayer ;
	//		}
	//		else 
	//		{
	//			if ( pRichestPlayer->getCoin() < pPlayer->getCoin() )
	//			{
	//				pRichestPlayer = pPlayer ;
	//			}
	//		}
	//	}
	//}

	//if ( vMaybeBanker.empty() )
	//{
	//	vMaybeBanker.push_back(pRichestPlayer) ;
	//}

	//uint8_t nBankerCntIdx = rand() % vMaybeBanker.size();
	//CNiuNiuRoom::LIST_SITDOWN_PLAYERS::iterator iter = vMaybeBanker.begin() ;
	//for ( uint8_t nMayBankerIdx = 0 ; iter != vMaybeBanker.end() ; ++iter, ++nMayBankerIdx )
	//{
	//	if ( nBankerCntIdx == nMayBankerIdx )
	//	{
	//		m_pRoom->setBankerIdx((*iter)->getIdx()) ;
	//		m_pRoom->setBankCoinLimitForBet((*iter)->getCoin()) ;
	//		break; 
	//	}
	//}

	//if ( m_pRoom->getBetBottomTimes() == 0 )
	//{
	//	m_pRoom->setBetBottomTimes(1) ;
	//}

	setStateDuringTime( TIME_NIUNIU_RAND_BANKER_PER_WILL_BANKER * m_nRanderCnt );

	stMsgNNRandBanker msgBanker ;
	msgBanker.nBankerIdx = m_pRoom->getBankerIdx() ;
	msgBanker.nBankerBetTimes = m_pRoom->getBetBottomTimes() ;
	m_pRoom->sendRoomMsg(&msgBanker,sizeof(msgBanker)) ;

	LOGFMTD("rand banker idx = %d, betTimes = %d",msgBanker.nBankerIdx,msgBanker.nBankerBetTimes) ;
}

void CNiuNiuRoomRandBankerState::onStateDuringTimeUp()
{
	m_pRoom->goToState(eRoomState_NN_StartBet);
}
