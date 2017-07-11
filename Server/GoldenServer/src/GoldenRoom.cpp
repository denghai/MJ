#include "GoldenRoom.h"
#include "RoomConfig.h"
#include "GoldenRoomPlayer.h"

#include <json/json.h>
#include "GoldenMessageDefine.h"
#include "AutoBuffer.h"
#include "log4z.h"
#include "GoldenBetState.h"
#include "GoldenGameResultState.h"
#include "GoldenPKState.h"
#include "GoldenStartGameState.h"
#include "GoldenWaitReadyState.h"
#include <cassert>
CGoldenRoom::CGoldenRoom()
{
	m_nBankerIdx = 0;
	m_nCurBet = 0;
	m_nBaseBet = 0;
	m_nMailPool = 0;
	m_nCurActIdx = m_nBankerIdx ;
	m_nBetRound = 0 ;
	m_nMaxBetRound = 20;
	getPoker()->InitTaxasPoker() ;
}

bool CGoldenRoom::onFirstBeCreated(IRoomManager* pRoomMgr,uint32_t nRoomID , const Json::Value& vJsValue)
{
	ISitableRoom::onFirstBeCreated(pRoomMgr,nRoomID,vJsValue) ;
	m_nBaseBet = vJsValue["baseBet"].asUInt();
	Json::Value jsOpt = vJsValue["opts"];
	if (jsOpt.isNull() == false && jsOpt["maxRound"].isUInt() )
	{
		m_nMaxBetRound = jsOpt["maxRound"].asUInt();
		LOGFMTD("room id = %u max round = %u",getRoomID(),m_nMaxBetRound );
		if (m_nMaxBetRound == 0)
		{
			m_nMaxBetRound = 20;
		}
	}
	else
	{
		LOGFMTE("do not set max round");
	}
	return true ;
}

void CGoldenRoom::serializationFromDB(IRoomManager* pRoomMgr,stBaseRoomConfig* pConfig,uint32_t nRoomID , Json::Value& vJsValue )
{
	ISitableRoom::serializationFromDB(pRoomMgr,pConfig,nRoomID,vJsValue) ;
	m_nBaseBet = ((stNiuNiuRoomConfig*)pConfig)->nBaseBet;
}

void CGoldenRoom::prepareState()
{
	IRoomState* vState[] = {
		new IRoomStateWaitReadyState(),new IRoomStateClosed(),new IRoomStateDidGameOver()
		,new CGoldenStartGameState(), new CGoldenPKState(), new CGoldenGameResultState(),new CGoldenBetState()
	};

	for ( uint8_t nIdx = 0 ; nIdx < sizeof(vState) / sizeof(IRoomState*); ++nIdx )
	{
		addRoomState(vState[nIdx]) ;
	}
	setInitState(vState[0]);
}

bool CGoldenRoom::onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID )
{
	if ( ISitableRoom::onMessage(prealMsg,eSenderPort,nPlayerSessionID) )
	{
		return true ;
	}
	
	switch ( prealMsg->usMsgType )
	{
	case MSG_GOLDEN_PLAYER_ACT:
		{
			stMsgGoldenPlayerActRet msgBack ;
			stMsgGoldenPlayerAct* ppmsg = (stMsgGoldenPlayerAct*)prealMsg ;
			if ( ppmsg->nPlayerAct == eRoomPeerAction_Ready )
			{
				auto pp = getSitdownPlayerBySessionID(nPlayerSessionID) ;
				if ( pp )
				{
					pp->resetNoneActTimes();
					if ( pp->isHaveState(eRoomPeer_CanAct) == false )
					{
						pp->setState(eRoomPeer_Ready) ;
						stMsgGoldenRoomAct msgR ;
						msgR.nPlayerAct = ppmsg->nPlayerAct ;
						msgR.nValue = 0 ;
						msgR.nPlayerIdx = pp->getIdx() ;
						sendRoomMsg(&msgR,sizeof(msgR)) ;
					}
					else
					{
						LOGFMTE("you are playing game , why set ready sate ?");
						msgBack.nRet = 3 ;
						sendMsgToPlayer(&msgBack,sizeof(msgBack),nPlayerSessionID) ;
					}
				}
				else
				{
					msgBack.nRet = 3 ;
					LOGFMTE("you are not sit down can not ready session id = %u",nPlayerSessionID);
					sendMsgToPlayer(&msgBack,sizeof(msgBack),nPlayerSessionID) ;

				}
				return true ;
			}
			return false ;
		}
		break;
	default:
		return false;
	}
	return true ;
}

void CGoldenRoom::roomInfoVisitor(Json::Value& vOutJsValue)
{
	vOutJsValue["bankIdx"] = m_nBankerIdx;
	vOutJsValue["baseBet"] = getBaseBet();
	vOutJsValue["curBet"] = getCurBet();
	vOutJsValue["mainPool"] = m_nMailPool;
	vOutJsValue["curActIdx"] = m_nCurActIdx ;
	vOutJsValue["betRound"] = m_nBetRound ;
	vOutJsValue["maxRound"] = m_nMaxBetRound;
}

void CGoldenRoom::sendRoomPlayersInfo(uint32_t nSessionID)
{
	stMsgGoldenRoomPlayers msgInfo ;
	msgInfo.nPlayerCnt = (uint8_t)getSitDownPlayerCount();
	CAutoBuffer auBuffer(sizeof(msgInfo) + sizeof(stGoldenRoomInfoPayerItem) * msgInfo.nPlayerCnt);
	auBuffer.addContent(&msgInfo,sizeof(msgInfo));

	uint32_t nTargetUID = 0;
	auto p = getPlayerBySessionID(nSessionID);
	if (p)
	{
		nTargetUID = p->nUserUID;
	}
	uint8_t nSeatCount = (uint8_t)getSeatCount();
	stGoldenRoomInfoPayerItem item ;
	uint8_t nDisCardCnt = GOLDEN_PEER_CARD;
	for ( uint8_t nIdx = 0 ; nIdx < nSeatCount ; ++nIdx )
	{
		CGoldenRoomPlayer* psit = (CGoldenRoomPlayer*)getPlayerByIdx(nIdx) ;
		if ( psit )
		{
			memset(item.vHoldChard,0,sizeof(item.vHoldChard)) ;
			item.nBetCoin = psit->getBetCoin() ;
			item.nCoin = psit->getCoin() ;
			item.nIdx = psit->getIdx() ;
			item.nStateFlag = psit->getState() ;
			item.nUserUID = psit->getUserUID() ;
			for ( uint8_t nCardIdx = 0 ; nCardIdx < nDisCardCnt ; ++nCardIdx )
			{
				if ( nTargetUID == item.nUserUID && psit->isHaveState(eRoomPeer_Looked))
				{
					item.vHoldChard[nCardIdx] = psit->getCardByIdx(nCardIdx) ;
				}
				else
				{
					item.vHoldChard[nCardIdx] = 3; //psit->getCardByIdx(nCardIdx) ;
				}
			}
			auBuffer.addContent(&item,sizeof(item)) ;
			LOGFMTD("send players uid = %u, state = %u",item.nUserUID,item.nStateFlag);
		}
	}

	sendMsgToPlayer((stMsg*)auBuffer.getBufferPtr(),auBuffer.getContentSize(),nSessionID) ;
	LOGFMTD("send room info to session id = %d, player cnt = %d ", nSessionID,msgInfo.nPlayerCnt) ;
}

uint32_t CGoldenRoom::getBaseBet() // ji chu di zhu ;
{
	return m_nBaseBet ;
}

void CGoldenRoom::onGameWillBegin()
{
	IRoom::onGameWillBegin() ;
	m_nCurBet = getBaseBet();
	m_nMailPool = 0;
	m_nBetRound = 0 ;

	uint16_t nSeatCnt = getSeatCount() ;
	for ( uint8_t nIdx = 0; nIdx < nSeatCnt; ++nIdx )
	{
		ISitableRoomPlayer* pp = getPlayerByIdx(nIdx) ;
		if ( pp && pp->isHaveState(eRoomPeer_Ready) )
		{
			pp->setCoin(pp->getCoin() - getDeskFee() ) ;
			addTotoalProfit(getDeskFee());
			pp->onGameBegin();

			// xia di zhu 
			CGoldenRoomPlayer*pRG = (CGoldenRoomPlayer*)pp ;
			pRG->betCoin(getCurBet());
			m_nMailPool += getCurBet() ;
		}
		else if ( pp )
		{
			pp->increaseNoneActTimes();
		}
	}


	getPoker()->RestAllPoker() ;

	m_nBankerIdx = GetFirstInvalidIdxWithState(m_nBankerIdx ,eRoomPeer_Ready );
	m_nCurActIdx = m_nBankerIdx ;
}

void CGoldenRoom::onGameDidEnd()
{
	ISitableRoom::onGameDidEnd();
	m_nCurBet = getBaseBet();
	m_nMailPool = 0;
	m_nBetRound = 0 ;
	if (getDelegate())
	{
		getDelegate()->onOneRoundEnd(this);
	}
}

void CGoldenRoom::onPlayerWillStandUp( ISitableRoomPlayer* pPlayer )
{
	if ( pPlayer->isHaveState(eRoomPeer_CanAct) )
	{
		ISitableRoom::onPlayerWillStandUp(pPlayer) ;
	}
	else
	{
		if ( pPlayer->isHaveState(eRoomPeer_StayThisRound) && getDelegate() && getCurRoomState()->getStateID() != eRoomState_DidGameOver )
		{
			getDelegate()->onUpdatePlayerGameResult(this,pPlayer->getUserUID(),pPlayer->getGameOffset()) ;
		}
		playerDoStandUp(pPlayer);
	}
}

uint32_t CGoldenRoom::getLeastCoinNeedForCurrentGameRound(ISitableRoomPlayer* pp)
{
	return 99999999 ;
}

void CGoldenRoom::prepareCards()
{
	uint8_t nSeatCnt = (uint8_t)getSeatCount() ;
	for ( uint8_t nIdx = 0 ; nIdx < nSeatCnt ; ++nIdx )
	{
		CGoldenRoomPlayer* pRoomPlayer = (CGoldenRoomPlayer*)getPlayerByIdx(nIdx) ;
		if ( pRoomPlayer && pRoomPlayer->isHaveState(eRoomPeer_CanAct))
		{
			uint8_t nCardCount = GOLDEN_PEER_CARD ;
			uint8_t nCardIdx = 0 ;
			while ( nCardIdx < nCardCount )
			{
				pRoomPlayer->onGetCard(nCardIdx,getPoker()->GetCardWithCompositeNum()) ;
				++nCardIdx ;
			}
		}
	}
}

uint32_t CGoldenRoom::coinNeededToSitDown()
{
	return getBaseBet() * 4 ;
}

void CGoldenRoom::caculateGameResult()
{
	// find winer player . biggest card player 
	int8_t nSortedCnt = getSortedPlayerCnt();
	CGoldenRoomPlayer* pWinner = nullptr;
	for (int8_t nIdx = nSortedCnt - 1; nIdx >= 0; --nIdx)
	{
		auto pp = (CGoldenRoomPlayer*)getSortedPlayerByIdx(nIdx);
		if (pp && pp->isHaveState(eRoomPeer_CanAct))
		{
			pWinner = pp;
			break;
		}
	}

	if (pWinner == nullptr)
	{
		LOGFMTE("can not find winner playe as sorted player vector ? room id = %u",getRoomID() );
	}
	stMsgGoldenResult msgResult ;
	if (pWinner)
	{
		msgResult.cWinnerIdx = pWinner->getIdx();
	}
	else
	{
		msgResult.cWinnerIdx = GetFirstInvalidIdxWithState(1, eRoomPeer_CanAct);
	}

	if (pWinner == nullptr)
	{
		pWinner = (CGoldenRoomPlayer*)getPlayerByIdx(msgResult.cWinnerIdx);
	}

	assert(pWinner && "win player can not be null");
	if (pWinner == nullptr)
	{
		LOGFMTE("why win player is null ? room id = %u",getRoomID()) ;
		return ;
	}
	
	msgResult.nWinCoin = m_nMailPool - pWinner->getBetCoin();
	uint32_t nTax = (uint32_t)(float(msgResult.nWinCoin) * getChouShuiRate() + 0.5f);
	pWinner->setCoin(m_nMailPool + pWinner->getCoin() - nTax);
	pWinner->addWinCoin(m_nMailPool - nTax);
	setBankerIdx(pWinner->getIdx());  // winner is next banker 
	msgResult.nFinalCoin = pWinner->getCoin();
	LOGFMTD("room id = %u uid = %u win game , tax = %u , win = %u , final = %u", getRoomID(), pWinner->getUserUID(), nTax, msgResult.nWinCoin, msgResult.nFinalCoin);

	//sendRoomMsg(&msgResult,sizeof(msgResult)) ;  // new game will not use it ;
	// produce new msg ;
	Json::Value jsMsg;
	jsMsg["winnerIdx"] = pWinner->getIdx();
	Json::Value jsPlayers;
	for (int8_t nIdx = nSortedCnt - 1; nIdx >= 0; --nIdx)
	{
		auto pp = (CGoldenRoomPlayer*)getSortedPlayerByIdx(nIdx);
		if (pp && pp->isHaveState(eRoomPeer_StayThisRound))
		{
			Json::Value jsPlayer;
			jsPlayer["idx"] = pp->getIdx();
			jsPlayer["UID"] = pp->getUserUID();
			jsPlayer["offset"] = pp->getGameOffset();
			jsPlayer["final"] = pp->getCoin();
			
			if (!pp->isHaveState(eRoomPeer_GiveUp))
			{
				Json::Value jsCard;
				jsCard[jsCard.size()] = pp->getCardByIdx(0);
				jsCard[jsCard.size()] = pp->getCardByIdx(1);
				jsCard[jsCard.size()] = pp->getCardByIdx(2);
				jsPlayer["card"] = jsCard;
			}

			jsPlayers[jsPlayers.size()] = jsPlayer;
		}
	}
	jsMsg["players"] = jsPlayers;
	sendRoomMsg(jsMsg, MSG_GOLDEN_ROOM_RESULT_NEW);
	m_jsGameResult = jsMsg;
}

void CGoldenRoom::sendResultToPlayerWhenDuringResultState(uint32_t nSessionID)
{
	if (CGoldenGameResultState::eStateID == getCurRoomState()->getStateID())
	{
		sendMsgToPlayer(nSessionID, m_jsGameResult, MSG_GOLDEN_ROOM_RESULT_NEW);
	}
	
}

uint8_t CGoldenRoom::onPlayerAction(uint32_t nAct, uint32_t& nValue, ISitableRoomPlayer* pPlayer )
{
	if ( pPlayer->getIdx() != getCurActIdx() )
	{
		if ( nAct != eRoomPeerAction_ViewCard && eRoomPeerAction_GiveUp != nAct )
		{
			return 1 ;
		}
	}

	if ( pPlayer->isHaveState(eRoomPeer_CanAct) == false )
	{
		LOGFMTE("can not do act , you are not can act uid = %u",pPlayer->getUserUID()) ;
		return 3 ;
	}

	switch (nAct)
	{
	case eRoomPeerAction_Add:
		{
			uint32_t nNeedCoin = nValue + m_nCurBet ;
			if ( pPlayer->isHaveState(eRoomPeer_Looked) )
			{
				nNeedCoin *= 2 ;
			}

			if ( nNeedCoin > pPlayer->getCoin() )
			{
				LOGFMTD("coin not enough can not add , uid = %u",pPlayer->getUserUID()) ;
				return 6 ; 
			}

			m_nCurBet += nValue ;
			((CGoldenRoomPlayer*)pPlayer)->betCoin(nNeedCoin);
			m_nMailPool += nNeedCoin ;
		}
		break;
	case eRoomPeerAction_Follow:
		{
			uint32_t nNeedCoin = m_nCurBet ;
			if ( pPlayer->isHaveState(eRoomPeer_Looked) )
			{
				nNeedCoin *= 2 ;
			}

			if ( nNeedCoin > pPlayer->getCoin() )
			{
				LOGFMTD("coin not enough can not Follow , uid = %u",pPlayer->getUserUID()) ;
				return 6 ; 
			}

			((CGoldenRoomPlayer*)pPlayer)->betCoin(nNeedCoin);
			m_nMailPool += nNeedCoin ;
		}
		break;
	case eRoomPeerAction_ViewCard:
		{
			pPlayer->setState(eRoomPeer_Looked) ;
		}
		break;
	case eRoomPeerAction_GiveUp:
		{
			if ( pPlayer->isDelayStandUp() )
			{
				if ( getDelegate() )
				{
					CGoldenRoomPlayer* pP = (CGoldenRoomPlayer*)pPlayer;
					getDelegate()->onUpdatePlayerGameResult(this,pPlayer->getUserUID(),(int32_t)pP->getBetCoin() * -1) ;
				}
				playerDoStandUp(pPlayer);
			}
			else
			{
				pPlayer->setState(eRoomPeer_GiveUp) ;
			}
		}
		break;
	default:
		return 4;
	}
	return 0 ;
}

uint8_t CGoldenRoom::informPlayerAct( bool bStepNext )
{
	stMsgGoldenRoomWaitPlayerAct msgAct ;
	if ( bStepNext )
	{
		int8_t nPreIdx = m_nCurActIdx ;
		m_nCurActIdx = GetFirstInvalidIdxWithState(m_nCurActIdx + 1 ,eRoomPeer_CanAct) ;
		int8_t nNowIdx = m_nCurActIdx ;
		if ( nPreIdx < nNowIdx )
		{
			if ( nPreIdx < m_nBankerIdx && m_nBankerIdx <= nNowIdx )
			{
				++m_nBetRound ;
			}
		}
		else 
		{
			if ( m_nBankerIdx <= nNowIdx || m_nBankerIdx > nPreIdx )
			{
				++m_nBetRound ;
			}
		}
	}
	msgAct.nActPlayerIdx = m_nCurActIdx ;
	sendRoomMsg(&msgAct,sizeof(msgAct)) ;
	return m_nCurActIdx ;
}

bool CGoldenRoom::isReachedMaxRound()
{
	int8_t nPreIdx = m_nCurActIdx;
	auto nNextIdx = GetFirstInvalidIdxWithState(m_nCurActIdx + 1, eRoomPeer_CanAct);
	int8_t nNowIdx = nNextIdx;
	uint8_t nNextRound = m_nBetRound;
	if (nPreIdx < nNowIdx)
	{
		if (nPreIdx < m_nBankerIdx && m_nBankerIdx <= nNowIdx)
		{
			++nNextRound;
		}
	}
	else
	{
		if (m_nBankerIdx <= nNowIdx || m_nBankerIdx > nPreIdx)
		{
			++nNextRound;
		}
	}

	return  ((nNextRound + 1) >= m_nMaxBetRound);
}

bool CGoldenRoom::onPlayerPK(ISitableRoomPlayer* pActPlayer , ISitableRoomPlayer* pTargetPlayer )
{
	CGoldenRoomPlayer* pP = (CGoldenRoomPlayer*)pActPlayer;
	CGoldenRoomPlayer* pT = (CGoldenRoomPlayer*)pTargetPlayer ;
	uint32_t nNeedCoin = m_nCurBet * 2 ;
	if ( pP->isHaveState(eRoomPeer_Looked) )
	{
		nNeedCoin *= 2 ;
	}

	if ( nNeedCoin > pP->getCoin() )
	{
		nNeedCoin = pP->getCoin();
	}

	pP->betCoin(nNeedCoin);
	m_nMailPool += nNeedCoin ;

	bool bWin = pP->getPeerCard()->pk(pT->getPeerCard()) == IPeerCard::PK_RESULT_WIN;
	if ( bWin )
	{
		pT->setState(eRoomPeer_PK_Failed);
	}
	else
	{
		pP->setState(eRoomPeer_PK_Failed) ;
	}
	return bWin ;
}

ISitableRoomPlayer* CGoldenRoom::doCreateSitableRoomPlayer()
{
	return new CGoldenRoomPlayer();
}