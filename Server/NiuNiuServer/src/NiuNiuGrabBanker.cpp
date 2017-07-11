#include "NiuNiuGrabBanker.h"
#include "NiuNiuRoom.h"
#include "NiuNiuMessageDefine.h"
#include "NiuNiuRoomPlayer.h"
#include "NiuNiuRoomBetState.h"
#include "NiuNiuRoomRandBankerState.h"
#include "log4z.h"
void CNiuNiuRoomGrabBanker::enterState(IRoom* pRoom)
{
	m_vGrabedIdx.clear() ;
	m_vWaitIdxs.clear() ;
	m_vRefusedGrabedIdx.clear() ;
	m_pRoom = (CNiuNiuRoom*)pRoom ;
	if ( m_pRoom->getPlayersHaveGrabBankerPrivilege(m_vWaitIdxs) )
	{
		setStateDuringTime(TIME_NIUNIU_TRY_BANKER) ;
	}
	else
	{
		LOGFMTD("no one can be banker ?") ;
		setStateDuringTime(1);
	}

	if ( m_pRoom->isHaveBanker() )
	{
		LOGFMTE("already have banker why come here ? grab banker ?") ;
	}
	LOGFMTD("room id = %d , try banker ",m_pRoom->getRoomID());
}

bool CNiuNiuRoomGrabBanker::onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID )
{
	if ( IRoomState::onMessage(prealMsg,eSenderPort,nPlayerSessionID) )
	{
		return true ;
	}

	if ( prealMsg->usMsgType != MSG_NN_PLAYER_TRY_BANKER )
	{
		return false ;
	}

	stMsgNNPlayerTryBankerRet msgBack ;
	stMsgNNPlayerTryBanker* pTryBanker = (stMsgNNPlayerTryBanker*)prealMsg ;
	CNiuNiuRoomPlayer* pPlayer = (CNiuNiuRoomPlayer*)m_pRoom->getSitdownPlayerBySessionID(nPlayerSessionID) ;
	//SLOGFMTD("try banker uid = %d ,times = %d" , pPlayer->getUserUID(), pTryBanker->nTryBankerBetTimes) ;
	if ( pPlayer == nullptr )
	{
		msgBack.nRet = 3 ;
		m_pRoom->sendMsgToPlayer(&msgBack,sizeof(msgBack),nPlayerSessionID) ;
		return true ;
	}
	else if ( pPlayer->isHaveState(eRoomPeer_CanAct) == false )
	{
		msgBack.nRet = 1 ;
		LOGFMTD("try banker state error uid = %d ,times = %d" , pPlayer->getUserUID(), pTryBanker->nTryBankerBetTimes) ;
		m_pRoom->sendMsgToPlayer(&msgBack,sizeof(msgBack),nPlayerSessionID) ;
		return true ;
	}
	else if ( pPlayer->getCoin() < m_pRoom->getLeastCoinNeedForBeBanker( 1 ) )
	{
		msgBack.nRet = 2 ;
		LOGFMTD("try banker coin not enough uid = %d ,times = %d" , pPlayer->getUserUID(), pTryBanker->nTryBankerBetTimes) ;
		m_pRoom->sendMsgToPlayer(&msgBack,sizeof(msgBack),nPlayerSessionID) ;
		return true ;
	}

	auto iter = std::find(m_vWaitIdxs.begin(),m_vWaitIdxs.end(),pPlayer->getIdx());
	if ( iter ==  m_vWaitIdxs.end() )
	{
		LOGFMTD("you already grabed banker") ;
		msgBack.nRet = 4 ;
		m_pRoom->sendMsgToPlayer(&msgBack,sizeof(msgBack),nPlayerSessionID) ;
		return true ;
	}
	m_vWaitIdxs.erase(iter) ;
 
	stMsgNNTryBanker msgTry ;
	msgTry.nTryBankerBetTimes = pTryBanker->nTryBankerBetTimes ;
	msgTry.nTryerIdx = pPlayer->getIdx() ;
	m_pRoom->sendRoomMsg(&msgTry,sizeof(msgTry)) ;
	LOGFMTD("try banker ok uid = %d ,times = %d" , pPlayer->getUserUID(), pTryBanker->nTryBankerBetTimes) ;

	// if erveryone have bet , then end this state 
	if ( pTryBanker->nTryBankerBetTimes  > 0 )
	{
		m_vGrabedIdx.push_back(pPlayer->getIdx()) ;
	}
	else
	{
		m_vRefusedGrabedIdx.push_back(pPlayer->getIdx()) ;
	}

	if ( m_vWaitIdxs.empty() )
	{
		onStateDuringTimeUp();
	}
	//LOGFMTD("try banker uid = %d ",pPlayer->getUserUID() ) ;
	return true ;
}

void CNiuNiuRoomGrabBanker::onStateDuringTimeUp()
{
	if ( m_vWaitIdxs.empty() )
	{
		m_vRefusedGrabedIdx.insert(m_vRefusedGrabedIdx.begin(),m_vWaitIdxs.begin(),m_vWaitIdxs.end()) ;
		m_vWaitIdxs.clear() ;
	}
	else
	{
		m_vRefusedGrabedIdx.insert(m_vRefusedGrabedIdx.begin(),m_vWaitIdxs.begin(),m_vWaitIdxs.end()) ;
	}

	uint8_t nBankerIdx = -1 ;
	uint8_t nRandBankerCandidateCnt = 0 ;
	if ( !m_vGrabedIdx.empty() )
	{
		nBankerIdx = m_vGrabedIdx[rand()%m_vGrabedIdx.size()] ;
		nRandBankerCandidateCnt = m_vGrabedIdx.size() ;
	}
	else if ( m_vRefusedGrabedIdx.empty() == false )
	{
		nBankerIdx = m_vRefusedGrabedIdx[rand() % m_vRefusedGrabedIdx.size()] ;
		nRandBankerCandidateCnt = m_vRefusedGrabedIdx.size() ;
	}
	else
	{
		LOGFMTD("no body can be banker , candidates are leave room ") ;
		m_pRoom->goToState(eRoomState_WaitJoin);
		return ;
	}

	m_pRoom->setBetBottomTimes(1);
	m_pRoom->setBankerIdx(nBankerIdx) ;

	if ( nRandBankerCandidateCnt > 1 )
	{
		LOGFMTD("go to rand banker state , candidate cnt = %u",nRandBankerCandidateCnt);
		auto pp = (CNiuNiuRoomRandBankerState*)m_pRoom->getRoomStateByID(eRoomState_NN_RandBanker);
		pp->setRandCnt(nRandBankerCandidateCnt);
		m_pRoom->goToState(eRoomState_NN_RandBanker);
		return ;
	}
	// send msg tell banker idx ;
	stMsgNNProducedBanker msgBanker ;
	msgBanker.nBankerBetTimes = 1 ;
	msgBanker.nBankerIdx = m_pRoom->getBankerIdx();
	m_pRoom->sendRoomMsg(&msgBanker,sizeof(msgBanker)) ;
	m_pRoom->goToState(eRoomState_NN_StartBet);

	LOGFMTD("new banker idx = %u",msgBanker.nBankerIdx) ;
}