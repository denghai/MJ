#include "NiuNiuRoomBetState.h"
#include "NiuNiuRoomDistributeFinalCardState.h"
#include "NiuNiuRoom.h"
#include "NiuNiuMessageDefine.h"
#include "NiuNiuRoomPlayer.h"
#include "log4z.h"
void CNiuNiuRoomBetState::enterState(IRoom* pRoom)
{
	m_pRoom = (CNiuNiuRoom*)pRoom ;	
	setStateDuringTime(TIME_NIUNIU_PLAYER_BET) ;
	vWaitBetPlayerIdxs.clear() ;
	m_pRoom->getPlayersWillBetPlayer(vWaitBetPlayerIdxs) ;
	LOGFMTD("room id = %d start bet ",m_pRoom->getRoomID()) ;
}

void CNiuNiuRoomBetState::onStateDuringTimeUp()
{
	for ( auto idx : vWaitBetPlayerIdxs )
	{
		CNiuNiuRoomPlayer* pPlayer = (CNiuNiuRoomPlayer*)m_pRoom->getPlayerByIdx(idx);
		if ( pPlayer )
		{
			//pPlayer->increaseNoneActTimes();
			LOGFMTD("niuniu will not auto stand up ; roomid = %u , player id = %u",m_pRoom->getRoomID(),pPlayer->getUserUID());
			pPlayer->setBetTimes(1) ;
		}
	}
	vWaitBetPlayerIdxs.clear() ;
	m_pRoom->goToState(eRoomState_NN_Disribute4Card);
}

bool CNiuNiuRoomBetState::onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID )
{
	if ( IRoomState::onMessage(prealMsg,eSenderPort,nPlayerSessionID) )
	{
		return true ;
	}

	if ( prealMsg->usMsgType != MSG_NN_PLAYER_BET )
	{
		return false ;
	}

	stMsgNNPlayerBet* pBet = (stMsgNNPlayerBet*)prealMsg ;
	stMsgNNPlayerBetRet msgBack ;
	int32_t nBetCoin = m_pRoom->getBaseBet() * m_pRoom->getBetBottomTimes() * pBet->nBetTimes ;
	CNiuNiuRoomPlayer* pPlayer = (CNiuNiuRoomPlayer*)m_pRoom->getSitdownPlayerBySessionID(nPlayerSessionID);
	if ( pPlayer == nullptr || pPlayer->isHaveState(eRoomPeer_CanAct) == false )
	{
		msgBack.nRet = 3 ;
	}
	else if ( pPlayer->getCoin() < nBetCoin * m_pRoom->getMaxRate() )  // you must can offer the lose 
	{
		msgBack.nRet = 2 ;
	}
	else if ( m_pRoom->getBankCoinLimitForBet() < nBetCoin * m_pRoom->getMaxRate() )
	{
		msgBack.nRet = 1 ;
	}
	else 
	{
		auto iterF = std::find(vWaitBetPlayerIdxs.begin(),vWaitBetPlayerIdxs.end(),pPlayer->getIdx()) ;
		if ( iterF == vWaitBetPlayerIdxs.end() )
		{
			msgBack.nRet = 4 ;
			LOGFMTD("session id = %u , you already bet , don't bet twice",pPlayer->getSessionID()) ;
		}
		else
		{
			pPlayer->resetNoneActTimes();
			vWaitBetPlayerIdxs.erase(iterF) ;
			msgBack.nRet = 0 ;
			pPlayer->setBetTimes(pBet->nBetTimes) ;
			stMsgNNBet msgRoomBet ;
			msgRoomBet.nBetTimes = pBet->nBetTimes ;
			msgRoomBet.nPlayerIdx = pPlayer->getIdx() ;
			m_pRoom->sendRoomMsg(&msgRoomBet,sizeof(msgRoomBet)) ;
			//m_pRoom->setBankCoinLimitForBet(m_pRoom->getBankCoinLimitForBet() - nBetCoin ) ;
			LOGFMTD("uid = %d bet times = %d, ret = %d",pPlayer->getUserUID(),pBet->nBetTimes,msgBack.nRet) ;
		}
	}

	m_pRoom->sendMsgToPlayer(&msgBack,sizeof(msgBack),nPlayerSessionID) ;
	
	if ( vWaitBetPlayerIdxs.empty() )
	{
		onStateDuringTimeUp();
	}
	return true ;
}