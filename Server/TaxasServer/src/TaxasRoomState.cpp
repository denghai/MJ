#include "TaxasRoomState.h"
#include "TaxasRoom.h"
#include "log4z.h"
#include "TaxasMessageDefine.h"
#include "ServerMessageDefine.h"
#include "TaxasServerApp.h"
#include <time.h>
#include "TaxasPlayer.h"
#include "InsuranceCheck.h"
// start game private card
void CTaxasStateStartGame::enterState(IRoom* pRoom )
{
	LOGFMTD("CTaxasStatePrivateCard start game");
	m_pRoom = (CTaxasRoom*)pRoom ;
	float fTime = ( TIME_TAXAS_DISTRIBUTE_HOLD_CARD_DELAY * (m_pRoom->getPlayerCntWithState(eRoomPeer_CanAct) * 2 - 1) ) + TIME_TAXAS_DISTRIBUTE_ONE_HOLD_CARD;
	if ( fTime < 0 )
	{
		fTime = 0 ;
		LOGFMTE("distribute hold card time < 0 peer = %d",m_pRoom->getPlayerCntWithState(eRoomPeer_CanAct));
	}
	fTime += 1.5 ;
	setStateDuringTime(fTime);
	m_pRoom->startGame();
}

void CTaxasStateStartGame::onStateDuringTimeUp()
{
	if ( m_pRoom->getPlayerCntWithState(eRoomPeer_WaitCaculate) == 0 )
	{
		LOGFMTE("just begin game , why no player want coin room id = %d",m_pRoom->getRoomID()) ;
		m_pRoom->goToState(eRoomState_DidGameOver) ;
		return ;
	}

	if ( m_pRoom->getPlayerCntWithState(eRoomPeer_CanAct) >= 2 )
	{
		m_pRoom->goToState(eRoomState_TP_Beting) ;
	}
	else
	{
		m_pRoom->goToState(eRoomState_TP_OneRoundBetEndResult) ;
	}
}

// player bet state 
void CTaxasStatePlayerBet::enterState(IRoom* pRoom)
{
	LOGFMTD("room id = %d enter CTaxasStatePlayerBet",pRoom->getRoomID() );
	m_pRoom = (CTaxasRoom*)pRoom ;
	m_bIsDoFinished = false ;
	m_pRoom->InformPlayerAct();
	resetStateTime();
}

void CTaxasStatePlayerBet::update(float fDelte )
{
	IRoomState::update(fDelte);
	if ( m_bIsDoFinished == false )
	{
		if ( m_pRoom->IsThisRoundBetOK() )
		{
			m_bIsDoFinished = true ;
			if ( m_pRoom->getMostBetCoinThisRound() > 0 )
			{
				setStateDuringTime(TIME_PLAYER_BET_COIN_ANI + 0.2f);
			}
			else
			{
				setStateDuringTime(0);
			}

			LOGFMTD("some other player act , invoke bet state end room id = %d",m_pRoom->getRoomID()) ;
		}
	}
}

void CTaxasStatePlayerBet::onStateDuringTimeUp()
{
	if ( !m_bIsDoFinished )  // wait player act time out 
	{
		LOGFMTD("wait time out");
		m_pRoom->OnPlayerActTimeOut() ;
		return ;
	}
	
	// bet state end ;
	if ( m_pRoom->getMostBetCoinThisRound() > 0 )
	{
		m_pRoom->goToState(eRoomState_TP_OneRoundBetEndResult);
		return ;
	}

	uint8_t nWaitCaPlyCnt = (uint8_t)m_pRoom->getPlayerCntWithState(eRoomPeer_WaitCaculate);
	if ( nWaitCaPlyCnt == 0 )
	{
		LOGFMTE("why all player leave in one minite room id = %d",m_pRoom->getRoomID()) ;
		m_pRoom->goToState(eRoomState_DidGameOver);
		return ;
	}

	if ( nWaitCaPlyCnt > 1 )  // go on distribute public card 
	{
		if ( m_pRoom->IsPublicDistributeFinish() ) // already distribute all card
		{
			LOGFMTD(" distrubute pubic finsh") ;
			m_pRoom->goToState(eRoomState_TP_GameResult);
		}
		else
		{
			m_pRoom->goToState(eRoomState_TP_PublicCard );
		}
	}
	else  // only one player just go to cacuate coin ;
	{
		LOGFMTD(" wait caculate only one  ") ;
		m_pRoom->goToState(eRoomState_TP_GameResult);
	}
}

void CTaxasStatePlayerBet::resetStateTime()
{
	setStateDuringTime(TIME_TAXAS_BET + 2);
}

bool CTaxasStatePlayerBet::onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID )
{
	switch ( prealMsg->usMsgType )
	{
	case MSG_PLAYER_LEAVE_ROOM:
	case MSG_PLAYER_STANDUP:
		{
			LOGFMTD("some player standup");
			//warnning: must return false we can not capture this message ;
			auto pPlayer = (CTaxasPlayer*)m_pRoom->getSitdownPlayerBySessionID(nPlayerSessionID);
			if ( pPlayer == nullptr )
			{
				return false ;
			}

			uint8_t nSeatIdx = pPlayer->getIdx();
			if ( nSeatIdx >= m_pRoom->getSeatCount() )
			{
				LOGFMTE("why have invalid seat idx = %d",nSeatIdx);
				return false ;
			}

			if ( nSeatIdx == m_pRoom->GetCurWaitActPlayerIdx() )
			{
				LOGFMTI("client should send give up msg , before player standup or leave room  player uid = %d",pPlayer->getUserUID()) ;
				// do something ;
				uint32_t nValue = 0 ;
				m_pRoom->OnPlayerAction(pPlayer->getIdx(),eRoomPeerAction_GiveUp,nValue);
				playerDoActOver() ;
				return false ;
			}
			else
			{
				LOGFMTD("some player standup idx = %d",nSeatIdx);
				return false ;
			}
		}
		break;
	case MSG_TP_PLAYER_ACT:
		{
			stMsgTaxasPlayerActRet msgBack ;
			auto pPlayer = (CTaxasPlayer*)m_pRoom->getSitdownPlayerBySessionID(nPlayerSessionID);
			if ( pPlayer == nullptr )
			{
				LOGFMTE("cur act player not sit down room id = %d",m_pRoom->getRoomID()) ;
				msgBack.nRet = 2 ;
				m_pRoom->sendMsgToPlayer(&msgBack,sizeof(msgBack),nPlayerSessionID);
				break; 
			}

			uint8_t nSeatIdx = pPlayer->getIdx();
			if ( nSeatIdx >= m_pRoom->getSeatCount() )
			{
				LOGFMTE("MSG_TP_PLAYER_ACT why have invalid seat idx = %d",nSeatIdx);
				m_pRoom->sendMsgToPlayer(&msgBack,sizeof(msgBack),nPlayerSessionID);
				break; 
			}

			
			stMsgTaxasPlayerAct* pAct = (stMsgTaxasPlayerAct*)prealMsg ;
			msgBack.nRet = m_pRoom->OnPlayerAction( nSeatIdx,(eRoomPeerAction)pAct->nPlayerAct,pAct->nValue) ;
			if ( msgBack.nRet == 0 )
			{
				playerDoActOver();
				LOGFMTD("room id = %d player idx = %d do act = %d, value = %u",m_pRoom->getRoomID(), nSeatIdx,pAct->nPlayerAct,pAct->nValue ) ;
			}
			else
			{
				m_pRoom->sendMsgToPlayer(&msgBack,sizeof(msgBack),nPlayerSessionID);
				LOGFMTE("player idx = %d act error ret = %d , room id = %d",nSeatIdx,msgBack.nRet,m_pRoom->getRoomID()); 
			}

			if ( prealMsg->cSysIdentifer != 0 )
			{
				pPlayer->resetNoneActTimes();
			}
		}
		break;
	default:
		return false;
	}
	return true ;
}

void CTaxasStatePlayerBet::playerDoActOver()
{
	if ( m_pRoom->IsThisRoundBetOK() == false )
	{
		m_pRoom->InformPlayerAct() ;
		resetStateTime() ;
	}
	else
	{
		m_bIsDoFinished = true ;
		if ( m_pRoom->getMostBetCoinThisRound() > 0 )
		{
			setStateDuringTime(TIME_PLAYER_BET_COIN_ANI + 0.2f);
		}
		else
		{
			setStateDuringTime(0);
		}
		LOGFMTD("current player act , invoke bet state end room id = %d",m_pRoom->getRoomID()) ;
	}
}

// caculate vice pool
void CTaxasStateOneRoundBetEndResult::enterState(IRoom* pRoom)
{
	LOGFMTD("room id = %d enter CTaxasStateOneRoundBetEndResult",pRoom->getRoomID() );
	m_pRoom = (CTaxasRoom*)pRoom ;
	uint8_t nCnt = m_pRoom->CaculateOneRoundPool() ;
	float fTime = TIME_TAXAS_WAIT_COIN_GOTO_MAIN_POOL + TIME_TAXAS_MAKE_VICE_POOLS * nCnt + 1.2f ; // if produced vice pool , need more time ;
	setStateDuringTime(fTime) ;
}

void CTaxasStateOneRoundBetEndResult::onStateDuringTimeUp()
{
	if ( m_pRoom->isNeedByInsurance() )
	{
		auto pS = (CTaxasStateInsurance*)m_pRoom->getRoomStateByID(eRoomState_TP_Insurance);
		pS->setNextState(eRoomState_TP_PublicCard);
		m_pRoom->goToState(eRoomState_TP_Insurance);
		return ;
	}

	if ( m_pRoom->getPlayerCntWithState(eRoomPeer_CanAct) <= 1 && m_pRoom->getDistributedPublicCardRound() > 0 )
	{
		Json::Value js ;
		m_pRoom->sendRoomMsg(js,MSG_SKIP_BUY_INSURANCE);
	}

	uint8_t nWaitCaPlyCnt = (uint8_t)m_pRoom->getPlayerCntWithState(eRoomPeer_WaitCaculate);
	if ( nWaitCaPlyCnt == 0 )
	{
		LOGFMTE("why all player leave in one minite room id = %d",m_pRoom->getRoomID()) ;
		m_pRoom->goToState(eRoomState_DidGameOver);
		return ;
	}

	if ( m_pRoom->IsPublicDistributeFinish() )
	{
		m_pRoom->goToState(eRoomState_TP_GameResult) ;
	}
	else
	{
		if ( nWaitCaPlyCnt >= 2 )
		{
			m_pRoom->goToState(eRoomState_TP_PublicCard) ;
		}
		else
		{
			m_pRoom->goToState(eRoomState_TP_GameResult) ;
		}
	}
}

// public card
void CTaxasStatePublicCard::enterState(IRoom* pRoom)
{
	LOGFMTD("room id = %d enter CTaxasStatePublicCard",pRoom->getRoomID() );
	m_pRoom = (CTaxasRoom*)pRoom ;
	float fTime = TIME_DISTRIBUTE_ONE_PUBLIC_CARD * m_pRoom->DistributePublicCard() + 1.0f;
	setStateDuringTime(fTime) ;
	nStateWaiting = 0 ;
}

void CTaxasStatePublicCard::onStateDuringTimeUp()
{
	// check if any one buy insurance 
	if ( 0 == nStateWaiting && m_pRoom->isAnyOneBuyInsurace() )
	{
		// do caculate 
		m_pRoom->doCaculateInsurance();
		nStateWaiting = 1 ;
		setStateDuringTime(TIME_TAXAS_WAIT_CALCULATE_INSURANCE);
		return ;
	}
	
	// add this card , go on check can buy insurance 
	if ( m_pRoom->isNeedByInsurance() )
	{
		// go to buy insurance state ;
		auto pS = (CTaxasStateInsurance*)m_pRoom->getRoomStateByID(eRoomState_TP_Insurance);
		pS->setNextState(eRoomState_TP_PublicCard);
		m_pRoom->goToState(eRoomState_TP_Insurance);
		return ;
	}

	if ( m_pRoom->getPlayerCntWithState(eRoomPeer_CanAct) <= 1 && m_pRoom->getDistributedPublicCardRound() < 3 )
	{
		Json::Value js ;
		m_pRoom->sendRoomMsg(js,MSG_SKIP_BUY_INSURANCE);
	}

	if ( m_pRoom->getPlayerCntWithState(eRoomPeer_CanAct) >= 2 )
	{
		m_pRoom->PreparePlayersForThisRoundBet();
		m_pRoom->goToState(eRoomState_TP_Beting) ;
		return ;
	}

	uint8_t nWaitCaPlyCnt = (uint8_t)m_pRoom->getPlayerCntWithState(eRoomPeer_WaitCaculate);
	if ( nWaitCaPlyCnt == 0 )
	{
		LOGFMTE("why all player leave in one minite room id = %d",m_pRoom->getRoomID()) ;
		m_pRoom->goToState(eRoomState_DidGameOver);
		return ;
	}
 
	if ( nWaitCaPlyCnt >= 2 && (m_pRoom->IsPublicDistributeFinish() == false ) )
	{
		m_pRoom->goToState(eRoomState_TP_PublicCard) ;
	}
	else
	{
		m_pRoom->goToState(eRoomState_TP_GameResult) ;
	}
}

// buy insurance
void CTaxasStateInsurance::enterState(IRoom* pRoom)
{
	m_pRoom = (CTaxasRoom*)pRoom ;
	m_pRoom->setInsuredPlayerIdx(-1) ;
	setStateDuringTime(TIME_TAXAS_WAIT_BUY_INSURANCE) ;

	// send insurance inform ;
	auto pInsur = m_pRoom->getInsuranceCheck() ;
	std::vector<uint8_t> vAllOuts ;
	auto cnt = pInsur->getOuts(m_nNeedBuyInsurancePlayerIdx,vAllOuts);
	assert(cnt > 0 && "why outs is 0 can buy insurance ?" );

	// find all outs 
	Json::Value jsAllOuts ;
	for ( auto& ref : vAllOuts )
	{
		jsAllOuts[jsAllOuts.size()] = ref ;
	}

	// find outs per players ;
	Json::Value jsPlayerOuts ;
	for ( uint8_t nIdx = 0 ; nIdx < m_pRoom->getSeatCount(); ++nIdx )
	{
		if ( nIdx == m_nNeedBuyInsurancePlayerIdx )
		{
			continue;
		}

		auto pp = m_pRoom->getPlayerByIdx(nIdx) ;
		if ( !pp || pp->isHaveState(eRoomPeer_WaitCaculate) == false )
		{
			continue;
		}

		Json::Value js ;
		js["idx"] = nIdx ;
		js["outs"] = pInsur->getOutsForPlayer(nIdx,nullptr) ;
		jsPlayerOuts[jsPlayerOuts.size()] = js ;
	}

	auto pBuyer = (CTaxasPlayer*)m_pRoom->getPlayerByIdx(m_nNeedBuyInsurancePlayerIdx) ;
	assert(pBuyer && "why insurance buyer is null");
	Json::Value jsmsg ;
	jsmsg["buyerIdx"] = m_nNeedBuyInsurancePlayerIdx ;
	jsmsg["lowLimit"] = pInsur->getAmountNeedForPofit(pBuyer->getInsuranceLoseCoin()); 
	jsmsg["outs"] = jsAllOuts ;
	jsmsg["playerouts"] = jsPlayerOuts ;
	m_pRoom->sendRoomMsg(jsmsg,MSG_INFORM_BUY_INSURANCE);
}

bool CTaxasStateInsurance::onMessage( Json::Value& prealMsg ,uint16_t nMsgType, eMsgPort eSenderPort , uint32_t nSessionID )
{
	if ( IRoomState::onMessage(prealMsg,nMsgType,eSenderPort,nSessionID) )
	{
		return true;
	}

	if ( nMsgType == MSG_PLAYER_SELECT_INSURED_AMOUNT )
	{
		auto pp = m_pRoom->getSitdownPlayerBySessionID(nSessionID) ;
		if ( pp == nullptr || pp->getIdx() != m_nNeedBuyInsurancePlayerIdx || pp->isHaveState(eRoomPeer_WaitCaculate) == false  )
		{
			Json::Value jsback ;
			jsback["ret"] = 1 ;
			m_pRoom->sendMsgToPlayer(nSessionID,jsback,nMsgType) ;
			return true ;
		}

		Json::Value jsmsg ;
		jsmsg["buyerIdx"] = m_nNeedBuyInsurancePlayerIdx ;
		jsmsg["amount"] = prealMsg["amount"].asUInt();
		m_pRoom->sendRoomMsg(jsmsg,MSG_ROOM_SELECT_INSURED_AMOUNT) ;
		return true ;
	}

	if ( nMsgType == MSG_CONFIRM_INSURED_AMOUNT )
	{
		uint32_t nAmount = prealMsg["amount"].asUInt() ;
		auto pp = (CTaxasPlayer*)m_pRoom->getSitdownPlayerBySessionID(nSessionID) ;
		if ( pp == nullptr || pp->getIdx() != m_nNeedBuyInsurancePlayerIdx || pp->isHaveState(eRoomPeer_WaitCaculate) == false )
		{
			Json::Value jsback ;
			jsback["ret"] = 1 ;
			m_pRoom->sendMsgToPlayer(nSessionID,jsback,nMsgType) ;
			return true ;
		}

		if ( nAmount > pp->getAllBetCoin() * 0.3 || m_pRoom->getInsuranceCheck()->getInsuredProfit(nAmount) < pp->getInsuranceLoseCoin() )
		{
			Json::Value jsback ;
			jsback["ret"] = 2 ;
			m_pRoom->sendMsgToPlayer(nSessionID,jsback,nMsgType) ;
			return true ;
		}

		Json::Value jsmsg ;
		jsmsg["amount"] = nAmount;
		jsmsg["idx"] = pp->getIdx();
		m_pRoom->sendRoomMsg(jsmsg,MSG_ROOM_FINISHED_BUY_INSURANCE) ;

		pp->setInsuredAmount(nAmount) ;
		m_pRoom->setInsuredPlayerIdx(pp->getIdx()) ;
		if ( nAmount == 0 )  // means give up 
		{
			m_pRoom->setInsuredPlayerIdx(-1) ;
		}

		m_pRoom->goToState(m_nNextState) ;
		return true ;
	}

	return false ;
}

void CTaxasStateInsurance::onStateDuringTimeUp()
{
	Json::Value jsmsg ;
	jsmsg["amount"] = 0;
	jsmsg["idx"] = m_nNeedBuyInsurancePlayerIdx;
	m_pRoom->sendRoomMsg(jsmsg,MSG_ROOM_FINISHED_BUY_INSURANCE) ;
	m_pRoom->setInsuredPlayerIdx(-1) ;
	m_pRoom->goToState(m_nNextState) ;
}

// game result 
void CTaxasStateGameResult::enterState(IRoom* pRoom)
{
	LOGFMTD("room id = %d enter CTaxasStateGameResult",pRoom->getRoomID() );
	m_pRoom = (CTaxasRoom*)pRoom ;
    uint8_t nWaitCal = (uint8_t)m_pRoom->getPlayerCntWithState(eRoomPeer_WaitCaculate);
	float fTime = 0 ;
	if ( nWaitCal > 1 )
	{
		fTime = TIME_TAXAS_CACULATE_PER_BET_POOL * m_pRoom->CaculateGameResult() + TIME_TAXAS_SHOW_BEST_CARD + 1;
	}
	else
	{
		fTime = TIME_TAXAS_CACULATE_PER_BET_POOL * m_pRoom->CaculateGameResult() + 1.3f  + TIME_TAXAS_WAIT_COIN_GOTO_MAIN_POOL ;  // all players give up ;
	}
	setStateDuringTime(fTime) ;
}

void CTaxasStateGameResult::onStateDuringTimeUp()
{
	m_pRoom->goToState(eRoomState_DidGameOver) ;
}