#include "TaxasRoom.h"
#include "log4z.h"
#include "TaxasRoomState.h"
#include "TaxasServerApp.h"
#include "TaxasPokerPeerCard.h"
#include "ServerMessageDefine.h"
#include <json/json.h>
#include "TaxasServerApp.h"
#include "RoomManager.h"
#include "AutoBuffer.h"
#include "ServerStringTable.h"
#include "TaxasPlayer.h"
#include <algorithm>
#include "InsuranceCheck.h"
#define TIME_SECONDS_PER_DAY (60*60*24)
#define TIME_SAVE_ROOM_INFO 60*30
CTaxasRoom::CTaxasRoom()
{
	
	m_nLittleBlind = 0 ;
	m_nMinTakeIn = 0 ;
	m_nMaxTakeIn = 0;
	// running members ;
	m_nBankerIdx = 0 ;
	m_nLittleBlindIdx = 0 ;
	m_nBigBlindIdx = 0;
	m_nCurWaitPlayerActionIdx = -1 ;
	m_nCurMainBetPool  = 0 ;
	m_nMostBetCoinThisRound = 0 ;
	memset(m_vPublicCardNums,0,sizeof(m_vPublicCardNums)) ;
	m_nPublicCardRound = 0 ;
	for ( auto& pool : m_vAllVicePools )
	{
		pool.Reset();
	}

	getPoker()->InitTaxasPoker() ;
	m_pInsurance = nullptr ;
}

CTaxasRoom::~CTaxasRoom()
{
 
}

uint8_t CTaxasRoom::getRoomType()
{
	return eRoom_TexasPoker ;
}

bool CTaxasRoom::onFirstBeCreated(IRoomManager* pRoomMgr, uint32_t nRoomID, const Json::Value& vJsValue )
{
	m_nLittleBlind = vJsValue["baseBet"].asUInt();
	m_nMinTakeIn = vJsValue["baseTakeIn"].asUInt() ;
	Json::Value jsOpt = vJsValue["opts"];
	assert(jsOpt.isNull() == false && "taxas opts is null");
	m_nMaxTakeIn = jsOpt["maxTakeIn"].asUInt();
	ISitableRoom::onFirstBeCreated(pRoomMgr,nRoomID,vJsValue) ;
	m_isInsured = false ;
	if ( jsOpt["isInsured"].isNull() == false )
	{
		m_isInsured = jsOpt["isInsured"].asUInt() ;
	}

	if ( m_isInsured )
	{
		m_pInsurance = new CInsuranceCheck ;
		m_pInsurance->reset() ;
	}
	return true ;
}

void CTaxasRoom::serializationFromDB(IRoomManager* pRoomMgr,stBaseRoomConfig* pConfig,uint32_t nRoomID , Json::Value& vJsValue )
{
	stTaxasRoomConfig* pRoomConfig = (stTaxasRoomConfig*)pConfig ;
	m_nLittleBlind = (uint32_t)(pRoomConfig->nBigBlind * 0.5f );
	//m_nMinTakeIn = pRoomConfig->nMiniTakeInCoin ;
	//m_nMaxTakeIn = pRoomConfig->nMaxTakeInCoin;
	ISitableRoom::serializationFromDB(pRoomMgr,pConfig,nRoomID,vJsValue) ;
}

void CTaxasRoom::willSerializtionToDB(Json::Value& vOutJsValue)
{
	ISitableRoom::willSerializtionToDB(vOutJsValue) ;
}

void CTaxasRoom::roomItemDetailVisitor(Json::Value& vOutJsValue)
{
	ISitableRoom::roomItemDetailVisitor(vOutJsValue) ;
	//vOutJsValue["bigBlind"] = getLittleBlind() * 2 ;
	//vOutJsValue["minTakeIn"] = getMinTakeIn() ;
	//vOutJsValue["maxTakeIn"] = getMaxTakeIn() ;
}

void CTaxasRoom::prepareState()
{
	ISitableRoom::prepareState();
	// create room state ;
	IRoomState* vState[] = {
		new CTaxasStateStartGame(),new CTaxasStatePlayerBet(),new CTaxasStateOneRoundBetEndResult(),
		new CTaxasStatePublicCard(),new CTaxasStateGameResult(),new CTaxasStateInsurance()
	};
	for ( uint8_t nIdx = 0 ; nIdx < sizeof(vState) / sizeof(IRoomState*); ++nIdx )
	{
		addRoomState(vState[nIdx]) ;
	}
}

bool CTaxasRoom::onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID )
{
	if ( ISitableRoom::onMessage(prealMsg,eSenderPort,nPlayerSessionID) )
	{
		return true ;
	}

	switch (prealMsg->usMsgType )
	{
	//case MSG_TP_REQUEST_ROOM_INFO:
	//	{
	//		LOGFMTI("send room info to player session id = %d",nPlayerSessionID );
	//		sendRoomInfoToPlayer(nPlayerSessionID);
	//	}
	//	break;
	//case MSG_REQUEST_ROOM_DETAIL:
	//	{
	//		stMsgRequestRoomDetailRet msgRet ;
	//		msgRet.detailInfo.nCreatOwnerUID = getOwnerUID();
	//		msgRet.detailInfo.nCurrentCount = GetPlayerCntWithState(eRoomPeer_SitDown);
	//		msgRet.detailInfo.nRoomID = GetRoomID();
	//		msgRet.detailInfo.nSmiallBlind = getLittleBlind();
	//		msgRet.detailInfo.nSeatCnt = getSeatCnt();
	//		sprintf_s(msgRet.detailInfo.vRoomName,sizeof(msgRet.detailInfo.vRoomName),"%s",getRoomName());
	//		SendMsgToPlayer(nPlayerSessionID,&msgRet,sizeof(msgRet)) ;
	//	}
	//	break;
	//case MSG_REQUEST_MY_OWN_ROOM_DETAIL:
	//	{
	//		stMsgRequestMyOwnRoomDetailRet msgRet ;
	//		msgRet.nRet = 0 ;
	//		msgRet.nRoomType = eRoom_TexasPoker ;
	//		msgRet.nCanWithdrawProfit = m_nRoomProfit ;
	//		msgRet.nConfigID = m_stRoomConfig.nConfigID ;
	//		msgRet.nDeadTime = m_nDeadTime ;
	//		msgRet.nFollows = 2 ;
	//		msgRet.nRoomID = GetRoomID() ;
	//		msgRet.nTotalProfit = m_nTotalProfit ;
	//		sprintf_s(msgRet.vRoomName,sizeof(msgRet.vRoomName),"%s",getRoomName());
	//		SendMsgToPlayer(nPlayerSessionID,&msgRet,sizeof(msgRet)) ;
	//	}
	//	break;
	case MSG_TP_CHANGE_ROOM:
		{
			LOGFMTE("change room function canncel");
			return true ;
			stMsgTaxasPlayerLeave leave ;
			onMessage(&leave,ID_MSG_PORT_CLIENT,nPlayerSessionID);
			
			return true ;
		}
		break;
	default:
		return false ;
	}
	return true ;
}

ISitableRoomPlayer* CTaxasRoom::doCreateSitableRoomPlayer()
{
	auto p = new CTaxasPlayer ;
	return p ;
}

uint32_t CTaxasRoom::coinNeededToSitDown()
{
	return m_nLittleBlind*2*6 ;

	//if ( m_nLittleBlind*2 > m_nMinTakeIn )
	//{
	//	return m_nLittleBlind*20 ;
	//}

	//return m_nMinTakeIn ;
}

void CTaxasRoom::prepareCards()
{
	// parepare cards for all player ;
	for ( uint16_t nPIdx = 0 ; nPIdx < TAXAS_PUBLIC_CARD ; ++nPIdx )
	{
		m_vPublicCardNums[nPIdx] = getPoker()->GetCardWithCompositeNum();
	}

	uint16_t nSeatCnt = getSeatCount() ;
	for ( uint8_t nIdx = 0 ; nIdx < nSeatCnt ; ++nIdx )
	{
		CTaxasPlayer* pRoomPlayer = (CTaxasPlayer*)getPlayerByIdx(nIdx) ;
		if ( pRoomPlayer && pRoomPlayer->isHaveState(eRoomPeer_CanAct))
		{
			uint8_t nCardCount = 2 ;
			uint8_t nCardIdx = 0 ;
			while ( nCardIdx < nCardCount )
			{
				pRoomPlayer->addPeerCard(nCardIdx,getPoker()->GetCardWithCompositeNum());
				++nCardIdx ;
			}

			// add public cards
			pRoomPlayer->addPublicCard(m_vPublicCardNums) ;
		}
	}
}

void CTaxasRoom::onPlayerWillStandUp(ISitableRoomPlayer* pPlayer )
{
	if ( pPlayer->isHaveState(eRoomPeer_WaitCaculate) )
	{
		ISitableRoom::onPlayerWillStandUp(pPlayer) ;
		return ;
	}

	if ( pPlayer->isHaveState(eRoomPeer_StayThisRound) )
	{
		if ( getCurRoomState()->getStateID() != eRoomState_DidGameOver )
		{
			CTaxasPlayer* p = (CTaxasPlayer*)pPlayer ;
			LOGFMTE("will stand up update offset uid = %d",pPlayer->getUserUID());

			if ( getDelegate() )
			{
				getDelegate()->onUpdatePlayerGameResult(this,pPlayer->getUserUID(),p->getGameOffset());
			}
		}
		else
		{
			LOGFMTI("if here update player uid = %d offeset will occure a bug ",pPlayer->getUserUID() ) ;
		}
	}
	playerDoStandUp(pPlayer);
}

uint32_t CTaxasRoom::getLeastCoinNeedForCurrentGameRound(ISitableRoomPlayer* pp)
{
	return 99999999 ;
}

uint8_t CTaxasRoom::OnPlayerAction( uint8_t nSeatIdx ,eRoomPeerAction act , uint32_t& nValue )
{
	if ( nSeatIdx >= getSeatCount() || getPlayerByIdx(nSeatIdx) == nullptr )
	{
		return 2 ;
	}

	if ( nSeatIdx != m_nCurWaitPlayerActionIdx && act != eRoomPeerAction_GiveUp )
	{
		return 1 ;
	}

	if ( getPlayerByIdx(nSeatIdx)->isHaveState(eRoomPeer_CanAct) == false )
	{
		return 3 ;
	}

	CTaxasPlayer* pData = (CTaxasPlayer*)getPlayerByIdx(nSeatIdx) ;
	switch ( act )
	{
	case eRoomPeerAction_GiveUp:
		{
			m_nCurMainBetPool += pData->getBetCoinThisRound() ;
			pData->setCurActType(eRoomPeerAction_GiveUp);
			pData->setState(eRoomPeer_GiveUp);
			// remove from vice pool
			for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM; ++nIdx )
			{
				if ( m_vAllVicePools[nIdx].bUsed == false )
				{
					break;
				}
				m_vAllVicePools[nIdx].RemovePlayer(nSeatIdx) ;
			}
		}
		break;
	case eRoomPeerAction_Follow:
		{
			if ( pData->getCoin() + pData->getBetCoinThisRound() <= m_nMostBetCoinThisRound )
			{
				nValue = pData->getCoin() ; // when all in must tell what value have allIned 
				return OnPlayerAction(nSeatIdx,eRoomPeerAction_AllIn,nValue);
			}
			pData->setCurActType(act);
			pData->betCoin(m_nMostBetCoinThisRound - pData->getBetCoinThisRound()) ;
		}
		break;
	case eRoomPeerAction_Add:
		{
			if ( pData->getCoin() <= nValue )
			{
				nValue = pData->getCoin() ; // when all in must tell what value have allIned
				return OnPlayerAction(nSeatIdx,eRoomPeerAction_AllIn,nValue);
			}

			if ( pData->getBetCoinThisRound() + nValue < m_nMostBetCoinThisRound + m_nLittleBlind * 2  )
			{
				return 6 ;
			}

			if ( ((pData->getBetCoinThisRound() + nValue) - m_nMostBetCoinThisRound ) % (m_nLittleBlind * 2) != 0  )
			{
				return 7 ;
			}

			pData->setCurActType(act);
			pData->betCoin(nValue) ;
			m_nMostBetCoinThisRound = pData->getBetCoinThisRound() ;
		}
		break;
	case eRoomPeerAction_AllIn:
		{
			pData->setCurActType(act);
			pData->setState(eRoomPeer_AllIn);
			nValue = pData->getCoin() ;
			pData->betCoin(nValue) ;
			if ( pData->getBetCoinThisRound() == 0 )
			{
				pData->setBetCoinThisRound(1);// avoid 0 all In bug ;
				LOGFMTE("room id = %d , 0 coin all in player idx = %d, uid = %d",getRoomID(),nSeatIdx,pData->getUserUID()) ;
			}

			if ( pData->getBetCoinThisRound() > m_nMostBetCoinThisRound )
			{
				m_nMostBetCoinThisRound = pData->getBetCoinThisRound() ;
			}
		}
		break;
	case eRoomPeerAction_Pass:
		{
			if ( m_nMostBetCoinThisRound != pData->getBetCoinThisRound() )
			{
				return 5 ;
			}

			pData->setCurActType(eRoomPeerAction_Pass);
		}
		break;
	default:
		return 4 ;
	}

	stMsgTaxasRoomAct msgOtherAct ;
	msgOtherAct.nPlayerAct = act ;
	msgOtherAct.nPlayerIdx = nSeatIdx ;
	msgOtherAct.nValue = nValue ;
	sendRoomMsg(&msgOtherAct,sizeof(msgOtherAct)) ;
	LOGFMTD("player do act") ;

	if ( pData->isDelayStandUp() && act == eRoomPeerAction_GiveUp )
	{
		LOGFMTD("uid = %d have delay standup , give up act , right standup and update offset ",pData->getUserUID());
		if ( getDelegate() )
		{
			getDelegate()->onUpdatePlayerGameResult(this,pData->getUserUID(),pData->getGameOffset());
		}
		playerDoStandUp(pData);	
	}

	return 0 ;
}

// logic function 
void CTaxasRoom::onGameWillBegin()
{
	m_arrPlayers.clear();
	// parepare all players ;
	getPoker()->RestAllPoker();
	ISitableRoom::onGameWillBegin();
	// prepare running data 
	m_nCurWaitPlayerActionIdx = -1;
	m_nCurMainBetPool = 0;
	m_nMostBetCoinThisRound = 0;
	m_nPublicCardRound = 0 ;
	for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM ; ++nIdx )
	{
		m_vAllVicePools[nIdx].nIdx = nIdx ;
		m_vAllVicePools[nIdx].Reset();
	}

	if ( m_pInsurance )
	{
		m_pInsurance->reset() ;
	}
	m_nBuyInsuraceIdx = -1 ;
}

void CTaxasRoom::onGameDidEnd()
{
	// parepare all players ;
	ISitableRoom::onGameDidEnd();

	// prepare running data 
	m_nCurWaitPlayerActionIdx = -1;
	m_nCurMainBetPool = 0;
	m_nMostBetCoinThisRound = 0;
	memset(m_vPublicCardNums,0,sizeof(m_vPublicCardNums)) ;
	m_nPublicCardRound = 0 ;
	for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM ; ++nIdx )
	{
		m_vAllVicePools[nIdx].nIdx = nIdx ;
		m_vAllVicePools[nIdx].Reset();
	}
}

void CTaxasRoom::startGame()
{
	// init running data 
	m_nBankerIdx = GetFirstInvalidIdxWithState(m_nBankerIdx + 1 , eRoomPeer_CanAct) ;
	m_nLittleBlindIdx = GetFirstInvalidIdxWithState(m_nBankerIdx + 1 , eRoomPeer_CanAct) ;
	m_nBigBlindIdx = GetFirstInvalidIdxWithState(m_nLittleBlindIdx + 1 , eRoomPeer_CanAct) ;

	// bet coin this 
	auto pTaxPlayer = (CTaxasPlayer*)getPlayerByIdx(m_nLittleBlindIdx);
	pTaxPlayer->betCoin( m_nLittleBlind ) ;

	pTaxPlayer = (CTaxasPlayer*)getPlayerByIdx(m_nBigBlindIdx);
	pTaxPlayer->betCoin( m_nLittleBlind * 2 ) ;
	m_nMostBetCoinThisRound = m_nLittleBlind * 2 ;

	stMsgTaxasRoomStartRound msgStart ;
	msgStart.nBankerIdx = m_nBankerIdx ;
	msgStart.nBigBlindIdx = m_nBigBlindIdx ;
	msgStart.nLittleBlindIdx = m_nLittleBlindIdx ;
	sendRoomMsg(&msgStart,sizeof(msgStart));

	// send card msg ;
	stMsgTaxasRoomPrivateCard msgPrivate ;
	msgPrivate.nPlayerCnt = (uint8_t)getPlayerCntWithState(eRoomPeer_CanAct) ;
	uint16_t nBuferLen = sizeof(msgPrivate) + sizeof(stTaxasHoldCardItems) * msgPrivate.nPlayerCnt;
	char* pBuffer = new char[nBuferLen] ;
	uint16_t nOffset = 0 ;
	memcpy(pBuffer,&msgPrivate,sizeof(msgPrivate));
	nOffset += sizeof(msgPrivate);

	for ( uint8_t nIdx = 0 ; nIdx < getSeatCount() ; ++nIdx)
	{
		auto pPlayer = (CTaxasPlayer*)getPlayerByIdx(nIdx);
		if ( pPlayer == nullptr || ( pPlayer->isHaveState(eRoomPeer_CanAct) == false ) )
		{
			continue; 
		}

		stTaxasHoldCardItems privateCards ;
		privateCards.cPlayerIdx = nIdx ;
		privateCards.vCards[0] = pPlayer->getPeerCardByIdx(0);
		privateCards.vCards[1] = pPlayer->getPeerCardByIdx(1);
		memcpy(pBuffer + nOffset , &privateCards,sizeof(privateCards) );
		nOffset += sizeof(privateCards);
	}

	if ( nOffset != nBuferLen )
	{
		LOGFMTE("buffer error for private cards") ;
		return ;
	}
	sendRoomMsg((stMsg*)pBuffer,nBuferLen) ;
	delete[] pBuffer ;
	pBuffer = NULL ;
}

void CTaxasRoom::PreparePlayersForThisRoundBet()
{
	m_nMostBetCoinThisRound = 0 ;
	if ( m_nCurWaitPlayerActionIdx >= 0 )  // means not first round 
	{
		m_nCurWaitPlayerActionIdx = m_nLittleBlindIdx - 1 ; //  little blid begin act   ps: m_nCurWaitPlayerActionIdx = GetFirstInvalidIdxWithState( m_nCurWaitPlayerActionIdx + 1 ,eRoomPeer_CanAct) ;
	}

	for ( uint8_t nIdx = 0 ; nIdx < getSeatCount() ; ++nIdx)
	{
		auto pPlayer = (CTaxasPlayer*)getPlayerByIdx(nIdx);
		if ( pPlayer == nullptr || ( pPlayer->isHaveState(eRoomPeer_StayThisRound) == false ) )
		{
			continue; 
		}

		pPlayer->setBetCoinThisRound(0);
		pPlayer->setCurActType(eRoomPeerAction_None) ;
	}
}

uint8_t CTaxasRoom::InformPlayerAct()
{
	if ( m_nCurWaitPlayerActionIdx < 0 ) // first round 
	{
		LOGFMTD("fist round");
		m_nCurWaitPlayerActionIdx = GetFirstInvalidIdxWithState(m_nBigBlindIdx + 1 ,eRoomPeer_CanAct) ;
	}
	else
	{
		LOGFMTD("second round");
		m_nCurWaitPlayerActionIdx = GetFirstInvalidIdxWithState( m_nCurWaitPlayerActionIdx + 1 ,eRoomPeer_CanAct) ;
	}

	stMsgTaxasRoomWaitPlayerAct msgWait ;
	msgWait.nActPlayerIdx = m_nCurWaitPlayerActionIdx ;
	sendRoomMsg(&msgWait,sizeof(msgWait));
	LOGFMTD("room id = %d ,wait idx = %d act ",getRoomID(),m_nCurWaitPlayerActionIdx ) ;
	return m_nCurWaitPlayerActionIdx ;
}

void CTaxasRoom::OnPlayerActTimeOut()
{
	stMsgTaxasPlayerAct msg ;
	msg.cSysIdentifer = 0 ;
	msg.nValue = 0 ;
	msg.nRoomID = getRoomID() ;
	CTaxasPlayer* pPlayer = (CTaxasPlayer*)getPlayerByIdx(m_nCurWaitPlayerActionIdx);
	if ( pPlayer == nullptr )
	{
		LOGFMTE("why cur wait player is null ");
		return ;
	}

	if ( m_nMostBetCoinThisRound == pPlayer->getBetCoinThisRound() )
	{
		msg.nPlayerAct = eRoomPeerAction_Pass ;
	}
	else
	{
		msg.nPlayerAct = eRoomPeerAction_GiveUp ;
	}
	LOGFMTD("wait player act time out auto act room id = %d",getRoomID()) ;
	onMessage(&msg,ID_MSG_PORT_CLIENT,pPlayer->getSessionID()) ;
	pPlayer->increaseNoneActTimes();
}

bool CTaxasRoom::IsThisRoundBetOK()
{
	if ( getPlayerCntWithState(eRoomPeer_WaitCaculate) == 1 )
	{
		return true ;
	}

	for ( uint8_t nIdx = 0 ; nIdx < getSeatCount() ; ++nIdx)
	{
		auto pPlayer = (CTaxasPlayer*)getPlayerByIdx(nIdx);
		if ( pPlayer == nullptr || pPlayer->isHaveState(eRoomPeer_CanAct) == false )
		{
			continue; 
		}
		
		if ( (pPlayer->getCurActType() == eRoomPeerAction_None || pPlayer->getBetCoinThisRound() != m_nMostBetCoinThisRound ) )
		{
			return false ;
		}
	}

	return true ;
}

 // return produced vice pool cunt this round ;
uint8_t CTaxasRoom::CaculateOneRoundPool()
{
	// check build vice pool
	uint8_t nBeforeVicePoolIdx = GetFirstCanUseVicePool().nIdx ;
	uint32_t nVicePool = 0 ;
	while ( true )
	{
		// find maybe pool 
		nVicePool = 0 ;
		for ( uint8_t nIdx = 0 ; nIdx < getSeatCount() ; ++nIdx )
		{
			auto pPlayer = (CTaxasPlayer*)getPlayerByIdx(nIdx);
			if ( pPlayer == nullptr || (pPlayer->isHaveState(eRoomPeer_WaitCaculate) == false ) )
			{
				continue;
			}

			if ( pPlayer->getCurActType() == eRoomPeerAction_AllIn && pPlayer->getBetCoinThisRound() > 0 )
			{
				if ( pPlayer->getBetCoinThisRound() < nVicePool || nVicePool == 0 )
				{
					nVicePool = pPlayer->getBetCoinThisRound() ;
				}
			}
		}

		if ( nVicePool == 0 )
		{
			break;
		}

		// real build pool;
		stVicePool& pPool = GetFirstCanUseVicePool();
		pPool.bUsed = true ;
		pPool.nCoin = m_nCurMainBetPool ;
		m_nCurMainBetPool = 0 ;
		
		// put player idx in pool ;
		LOGFMTD("build pool pool idx = %d",pPool.nIdx ) ;
		for ( uint8_t nIdx = 0 ; nIdx < getSeatCount() ; ++nIdx )
		{
			auto pPlayer = (CTaxasPlayer*)getPlayerByIdx(nIdx);
			if ( pPlayer == nullptr || (pPlayer->isHaveState(eRoomPeer_WaitCaculate) == false ) )
			{
				continue;
			}

			if ( pPlayer->getBetCoinThisRound() > 0 )
			{
				if ( pPlayer->getBetCoinThisRound() < nVicePool )
				{
					LOGFMTE("room id = %d , put vice pool coin not enough , why error error",getRoomID()) ;
				}
				pPool.nCoin += nVicePool ;
				pPlayer->setBetCoinThisRound(pPlayer->getBetCoinThisRound() - nVicePool ) ;
				pPool.vInPoolPlayerIdx.push_back(nIdx) ;
				LOGFMTD("put player into pool player Idx = %d, UID = %d",nIdx,pPlayer->getUserUID() ) ;
			}
		}
		LOGFMTI("pool idx = %d : coin = %u",pPool.nIdx,pPool.nCoin) ;
	}

	// build mian pool ;
	LOGFMTD("build main pool: " ) ;
	for ( uint8_t nIdx = 0 ; nIdx < getSeatCount() ; ++nIdx )
	{
		auto pPlayer = (CTaxasPlayer*)getPlayerByIdx(nIdx);
		if ( pPlayer == nullptr || (pPlayer->isHaveState(eRoomPeer_CanAct) == false ) )
		{
			continue;
		}

		if ( pPlayer->getBetCoinThisRound() > 0 )
		{
			m_nCurMainBetPool += pPlayer->getBetCoinThisRound() ;
			pPlayer->setBetCoinThisRound(0);
			LOGFMTD("put player into Main pool player Idx = %d, UID = %d",nIdx,pPlayer->getUserUID()) ;
		}
	}

	uint8_t nProducedVicePoolCnt = GetFirstCanUseVicePool().nIdx - nBeforeVicePoolIdx;
	LOGFMTI("oneRound Caculate over, mainPool = %u, newVicePool = %d",m_nCurMainBetPool,nProducedVicePoolCnt );

	// send msg tell client [ nBeforeVicePoolIdx, GetFirstCanUseVicePoolIdx() ); this set of pool idx are new produced ; not include the last 
	stMsgTaxasRoomOneBetRoundResult msgResult ;
	msgResult.nCurMainPool = m_nCurMainBetPool ;
	msgResult.nNewVicePoolCnt = nProducedVicePoolCnt ;
	memset(msgResult.vNewVicePool,0,sizeof(msgResult.vNewVicePool)) ;
	for ( uint8_t nIdx = nBeforeVicePoolIdx, nNewIdx = 0 ; nIdx < GetFirstCanUseVicePool().nIdx; ++nIdx )
	{
		msgResult.vNewVicePool[nNewIdx++] = m_vAllVicePools[nIdx].nCoin ;
	}
	sendRoomMsg(&msgResult,sizeof(msgResult)) ;
	return nProducedVicePoolCnt ;
}

// return dis card cnt ;
uint8_t CTaxasRoom::DistributePublicCard()
{
	LOGFMTD("pulick card = %d",m_nPublicCardRound);
	stMsgTaxasRoomPublicCard msgPublicCard ;
	msgPublicCard.nCardSeri = m_nPublicCardRound++ ;
	// distr 3 
	if ( m_nPublicCardRound == 1 )
	{
		for ( uint8_t nIdx = 0 ; nIdx < 3 ; ++nIdx )
		{
			msgPublicCard.vCard[nIdx] = m_vPublicCardNums[nIdx] ;
		}
		// send msg to tell client ;
		sendRoomMsg(&msgPublicCard,sizeof(msgPublicCard)) ;
		return 3 ;
	}

	if ( m_nPublicCardRound == 2 )
	{
		msgPublicCard.vCard[0] = m_vPublicCardNums[3] ;
		sendRoomMsg(&msgPublicCard,sizeof(msgPublicCard)) ;
		return 1 ;
	}

	if ( m_nPublicCardRound == 3 )
	{
		msgPublicCard.vCard[0] = m_vPublicCardNums[4] ;
		sendRoomMsg(&msgPublicCard,sizeof(msgPublicCard)) ;
		return 1 ;
	}

	LOGFMTE("already finish public card why one more time ") ;
	return 0 ;
}

//return pool cnt ;
uint8_t CTaxasRoom::CaculateGameResult()
{
	// build a main pool;
	if ( m_nCurMainBetPool > 0 )
	{
		stVicePool& pPool = GetFirstCanUseVicePool();
		pPool.nCoin = m_nCurMainBetPool ;
		pPool.bUsed = true ;
		for ( uint8_t nIdx = 0 ; nIdx < getSeatCount() ; ++nIdx )
		{
			auto pPlayer = getPlayerByIdx(nIdx);
			if ( nullptr == pPlayer )
			{
				continue;
			}

			if ( pPlayer->isHaveState(eRoomPeer_CanAct) )
			{
				pPool.vInPoolPlayerIdx.push_back(nIdx) ;
			}
		}
	}

	// cacluate a main pool ;
	for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM; ++nIdx )
	{
		if ( m_vAllVicePools[nIdx].bUsed )
		{
			CaculateVicePool(m_vAllVicePools[nIdx]) ;
		}
	}

	// send msg tell client ;
	if ( GetFirstCanUseVicePool().nIdx == 0 )
	{
		LOGFMTE("why this game have no pool ? at least should have one room id = %d",getRoomID() ) ;
		didCaculateGameResult();
		return 0 ;
	}

	uint8_t nLastPoolIdx = GetFirstCanUseVicePool().nIdx ;
	bool bSendEndFlag = false ;
	for ( uint8_t nIdx = 0 ; nIdx < nLastPoolIdx; ++nIdx )
	{
		stVicePool& pool = m_vAllVicePools[nIdx] ;
		if ( pool.vWinnerIdxs.empty() )
		{
			LOGFMTE("this pool have no winners , coin = %u, room = %d ",pool.nCoin,getRoomID() ) ;
			continue;
		}

		stMsgTaxasRoomGameResult msgResult ;
		msgResult.nCoinPerWinner = pool.nCoin / pool.vWinnerIdxs.size() ;
		msgResult.nPoolIdx = nIdx ;
		msgResult.nWinnerCnt = 0;
		msgResult.bIsLastOne = (nIdx + 1) >= nLastPoolIdx ;
		LOGFMTD("game reuslt pool idx = %d  isLast one = %d",nIdx,msgResult.bIsLastOne ) ;
		VEC_INT8::iterator iter = pool.vWinnerIdxs.begin() ;
		for ( ; iter != pool.vWinnerIdxs.end() ; ++iter )
		{
			msgResult.vWinnerIdx[msgResult.nWinnerCnt++] = (*iter); 
		}
		sendRoomMsg(&msgResult,sizeof(msgResult)) ;
		if ( msgResult.bIsLastOne )
		{
			bSendEndFlag = true ;
		}
	}

	if ( bSendEndFlag == false )  // must have a end flag msg ;
	{
		stMsgTaxasRoomGameResult msgResult ;
		msgResult.nCoinPerWinner = 0;
		msgResult.nPoolIdx = nLastPoolIdx ;
		msgResult.nWinnerCnt = 0;
		msgResult.bIsLastOne = true ;
		sendRoomMsg(&msgResult,sizeof(msgResult)) ;
	}
	didCaculateGameResult();
	return GetFirstCanUseVicePool().nIdx ;
}

bool CTaxasRoom::IsPublicDistributeFinish()
{
	return (m_nPublicCardRound >= 3 );
}

void CTaxasRoom::didCaculateGameResult()
{
	// save serve log 
	writeGameResultLog();
	
	// update coin Tax  and insurance ;
	Json::Value arrayJs ;
	for ( uint8_t nIdx = 0 ; nIdx < getSeatCount(); ++nIdx )
	{
		CTaxasPlayer* pPlayer = (CTaxasPlayer*)getPlayerByIdx(nIdx);
		if ( pPlayer == nullptr || pPlayer->isHaveState(eRoomPeer_StayThisRound) == false || pPlayer->getGameOffset() < (int)0 ) 
		{
			continue;
		}
		LOGFMTE("game end update offset");
		int32_t nTaxFee = (int32_t)(pPlayer->getGameOffset() * getChouShuiRate()) + pPlayer->getInsuranceLoseCoin();
		pPlayer->setCoin(pPlayer->getCoin() - nTaxFee );
		addTotoalProfit((uint32_t)nTaxFee);
		LOGFMTD("room id = %u uid = %u , give tax = %d coin = %u",getRoomID(),nTaxFee,pPlayer->getCoin()) ;

		Json::Value jsPeer ;
		jsPeer["idx"] = nIdx ;
		jsPeer["coin"] = pPlayer->getCoin() ;
		arrayJs[arrayJs.size()] = jsPeer ;
	}

	Json::Value jsmsg ;
	jsmsg["players"] = arrayJs ;
	sendRoomMsg(jsmsg,MSG_SYNC_TAXAS_ROOM_PLAYER_COIN) ;
}

void CTaxasRoom::writeGameResultLog()
{
	LOGFMTD("write game result dlg");
	stMsgSaveLog saveMsg ;
	saveMsg.nLogType = eLog_TaxasGameResult ;
	saveMsg.nTargetID = getRoomID();
	saveMsg.vArg[0] = 0;
	for ( uint8_t nIdx = 0 ; nIdx < TAXAS_PUBLIC_CARD; ++nIdx )
	{
		saveMsg.vArg[nIdx+1] = m_vPublicCardNums[nIdx];
	}

	for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM ; ++nIdx )
	{
		writePlayerResultLogToJson((CTaxasPlayer*)getPlayerByIdx(nIdx)) ;
	}

	Json::StyledWriter write ;
	std::string str = write.write(m_arrPlayers);
	CAutoBuffer auBuffer (sizeof(saveMsg) + str.size());
	saveMsg.nJsonExtnerLen = str.size() ;
	auBuffer.addContent((char*)&saveMsg,sizeof(saveMsg)) ;
	auBuffer.addContent(str.c_str(),str.size());
	CTaxasServerApp::SharedGameServerApp()->sendMsg(getRoomID(),auBuffer.getBufferPtr(),auBuffer.getContentSize()) ;
	LOGFMTD("all player info json str = %s" , str.c_str());
}

void CTaxasRoom::writePlayerResultLogToJson(CTaxasPlayer* pWritePlayer)
{
	if ( !pWritePlayer )
	{
		return ;
	}

	if ( pWritePlayer->isHaveState(eRoomPeer_StayThisRound) == false )
	{
		return ;
	}

	Json::Value refPlayer ;
	refPlayer["uid"] = pWritePlayer->getUserUID() ;
	refPlayer["idx"] = pWritePlayer->getIdx();
	refPlayer["card0"] = pWritePlayer->getPeerCardByIdx(0);
	refPlayer["card1"] = pWritePlayer->getPeerCardByIdx(1);
	refPlayer["betCoin"] = (uint32_t)pWritePlayer->getAllBetCoin();
	refPlayer["offset"] = int32_t(pWritePlayer->getGameOffset()) ;
	refPlayer["coin"] = (int32_t)pWritePlayer->getCoin() ;
	refPlayer["state"] = pWritePlayer->getState();
	m_arrPlayers[pWritePlayer->getIdx()] = refPlayer ;
	LOGFMTD("write player uid = %d result log to json",pWritePlayer->getUserUID());
}

stVicePool& CTaxasRoom::GetFirstCanUseVicePool()
{
	for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM; ++nIdx )
	{
		if ( !m_vAllVicePools[nIdx].bUsed )
		{
			 return m_vAllVicePools[nIdx] ;
		}
	}
	LOGFMTE("why all vice pool was used ? error ") ;
	return m_vAllVicePools[MAX_PEERS_IN_TAXAS_ROOM-1] ;
}

void CTaxasRoom::CaculateVicePool(stVicePool& pPool )
{
	if ( pPool.nCoin == 0 )
	{
		LOGFMTE("why this pool coin is 0 ? players = %d room id = %d ",pPool.vInPoolPlayerIdx.size(),getRoomID()) ;
		return ;
	}

	if ( pPool.vInPoolPlayerIdx.empty() )
	{
		LOGFMTE("why pool coin = %u , peers is 0 room id = %d  ",pPool.nCoin,getRoomID() ) ;
	}

	// find winner ;
	if ( pPool.vInPoolPlayerIdx.size() == 1 )
	{
		uint8_t nPeerIdx = pPool.vInPoolPlayerIdx[0] ;
		auto pPlayer = (CTaxasPlayer*)getPlayerByIdx(nPeerIdx);
		if ( pPlayer == nullptr )
		{
			LOGFMTE("why this winner idx is invalid = %d, system got coin = %u",nPeerIdx,pPool.nCoin ) ;
			return ;
		}

		if ( pPlayer->isHaveState(eRoomPeer_WaitCaculate) == false )
		{
			LOGFMTE("why this winner idx state is invalid = %d, system got coin = %u",nPeerIdx,pPool.nCoin ) ;
			return ;
		}
		pPool.vWinnerIdxs.push_back( nPeerIdx ) ;
		pPlayer->addWinCoinThisGame(pPool.nCoin);
		return ;
	}

	// pk card
	if ( IsPublicDistributeFinish() == false )
	{
		LOGFMTE("public is not finish how to pk card ? error room id = %d",getRoomID());
		return ;
	}

	CTaxasPlayer* pWiner = nullptr ;
	for ( int8_t nIdx = getSortedPlayerCnt() - 1 ; nIdx >= 0; --nIdx )
	{
		CTaxasPlayer* pData = (CTaxasPlayer*)getSortedPlayerByIdx(nIdx);
		if ( pData == nullptr || pData->isHaveState( eRoomPeer_WaitCaculate ) == false )
		{
			continue; ;
		}

		if ( pPool.isPlayerInThisPool(pData->getIdx()) == false )
		{
			continue;
		}

		if ( pPool.vWinnerIdxs.empty() )
		{
			pPool.vWinnerIdxs.push_back(pData->getIdx()) ;
			pWiner = pData ;
			continue;
		}

		if ( pWiner->getPeerCard()->pk(pData->getPeerCard()) != IPeerCard::PK_RESULT_EQUAL ) // not the same , means small 
		{
			break ;
		}

		pPool.vWinnerIdxs.push_back(pData->getIdx()) ;
	}

	// give coin 
	if ( pPool.vWinnerIdxs.empty() )
	{
		LOGFMTE("why room id = %d pool idx = %d winner is empty , system got coin = %u ",getRoomID(),pPool.nIdx,pPool.nCoin ) ;
		return ;
	}

	uint8_t nElasCoin = uint8_t(pPool.nCoin % pPool.vWinnerIdxs.size()) ;
	pPool.nCoin -= nElasCoin ;
	if ( nElasCoin > 0 )
	{
		addTotoalProfit(nElasCoin);
		LOGFMTI("system got the elaps coin = %d, room id = %d , pool idx = %d ",nElasCoin,getRoomID(),pPool.nIdx ) ;
	}
	uint32_t nCoinPerWinner = pPool.nCoin / pPool.vWinnerIdxs.size() ;
	for ( uint8_t nIdx = 0 ; nIdx < pPool.vWinnerIdxs.size(); ++nIdx )
	{
		uint8_t nPeerIdx = pPool.vWinnerIdxs[nIdx];
		auto pPlayer = (CTaxasPlayer*)getPlayerByIdx(nPeerIdx);
		pPlayer->addWinCoinThisGame(nCoinPerWinner);
		LOGFMTI("player use uid = %d win coin = %u , from pool idx = %d, room id = %d",pPlayer->getUserUID(),nCoinPerWinner,pPool.nIdx,getRoomID()) ;
	}
}

void CTaxasRoom::roomInfoVisitor(Json::Value& vOutJsValue)
{
	vOutJsValue["litBlind"] = m_nLittleBlind;
	vOutJsValue["maxTakIn"] = m_nMaxTakeIn;
	vOutJsValue["bankIdx"] = m_nBankerIdx;
	vOutJsValue["litBlindIdx"] = m_nLittleBlindIdx;
	vOutJsValue["bigBlindIdx"] = m_nBigBlindIdx;
	vOutJsValue["curActIdx"] = m_nCurWaitPlayerActionIdx;
	vOutJsValue["curPool"] = m_nCurMainBetPool;
	vOutJsValue["mostBet"] = m_nMostBetCoinThisRound;
	vOutJsValue["isInsurd"] = (uint8_t)m_isInsured ;
	
	// send base info 
	Json::Value pubCards;
	if ( m_nPublicCardRound == 1 )
	{
		for ( uint8_t nidx = 0 ; nidx < 3 ; ++nidx )
		{
			pubCards[nidx] = m_vPublicCardNums[nidx];
		}
	}
	else if ( 2 == m_nPublicCardRound )
	{
		for ( uint8_t nidx = 0 ; nidx < 4 ; ++nidx )
		{
			pubCards[nidx] = m_vPublicCardNums[nidx];
		}
	}
	else if ( 3 == m_nPublicCardRound )
	{
		for ( uint8_t nidx = 0 ; nidx < 5 ; ++nidx )
		{
			pubCards[nidx] = m_vPublicCardNums[nidx];
		}
	}

	vOutJsValue["pubCards"] = pubCards;
}

void CTaxasRoom::sendRoomPlayersInfo(uint32_t nSessionID)
{
	// send vice pool 
	if ( GetFirstCanUseVicePool().nIdx )
	{
		stMsgTaxasRoomInfoVicePool msgVicePool ;
		memset(msgVicePool.vVicePool,0,sizeof(msgVicePool.vVicePool)) ;
		for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM ; ++nIdx )
		{
			msgVicePool.vVicePool[nIdx] = m_vAllVicePools[nIdx].nCoin;
		}
		sendMsgToPlayer(&msgVicePool,sizeof(msgVicePool),nSessionID);
	}

	// send Player data 
	uint8_t nCnt = (uint8_t)getPlayerCntWithState(eRoomPeer_SitDown) ;
	stMsgTaxasRoomInfoPlayerData msgPlayerData ;
	for ( uint8_t nIdx = 0 ; nIdx < getSeatCount() ; ++nIdx )
	{
		CTaxasPlayer* pPlayer = (CTaxasPlayer*)getPlayerByIdx(nIdx);
		if ( pPlayer == nullptr )
		{
			continue;
		}
		msgPlayerData.bIsLast = (--nCnt <= 0 );
		msgPlayerData.eCurAct = pPlayer->getCurActType() ;
		msgPlayerData.nBetCoinThisRound = pPlayer->getBetCoinThisRound() ;
		msgPlayerData.nSeatIdx = pPlayer->getIdx() ;
		msgPlayerData.nStateFlag = pPlayer->getState() ;
		msgPlayerData.nTakeInMoney = pPlayer->getCoin() ;
		msgPlayerData.nUserUID = pPlayer->getUserUID() ;
		msgPlayerData.vHoldCard[0] = pPlayer->getPeerCardByIdx(0) ;
		msgPlayerData.vHoldCard[1] = pPlayer->getPeerCardByIdx(1) ;
		sendMsgToPlayer(&msgPlayerData,sizeof(msgPlayerData),nSessionID) ;
	}
	LOGFMTD("send room data to player ");
}

// insurance 
bool CTaxasRoom::isNeedByInsurance()
{
	if ( !m_isInsured )
	{
		return false ;
	}

	// is still have card not distribute ? , and already distribute 3 public card 
	if ( m_nPublicCardRound != 1 && 2 != m_nPublicCardRound )
	{
		return false ;
	}
	
	// is all player all in and player cnt >= 2 ?
	uint8_t nWaitCalPlayer = getPlayerCntWithState(eRoomPeer_WaitCaculate) ;
	uint8_t nAllInPlayer = getPlayerCntWithState(eRoomPeer_AllIn) ;
	if ( nWaitCalPlayer < 2 || nWaitCalPlayer > (nAllInPlayer + 1) )
	{
		return false ;
	}
	
	// reset insurance module , and add cards to it 
	m_pInsurance->reset();
	// find the biggest pool , and  chose a insurance buyer 
	uint8_t nBigIdx = 0 ;
	for ( uint8_t nIdx = nBigIdx + 1  ; nIdx < MAX_PEERS_IN_TAXAS_ROOM ; ++nIdx )
	{
		if ( m_vAllVicePools[nIdx].nCoin > m_vAllVicePools[nBigIdx].nCoin )
		{
			nBigIdx = nIdx ;
		}
	}

	// put player into check;
	for ( uint8_t nIdx = 0 ; nIdx < getSeatCount(); ++nIdx )
	{
		auto pp = getPlayerByIdx(nIdx) ;
		if ( !pp || pp->isHaveState(eRoomPeer_WaitCaculate) == false )
		{
			continue;
		}

		auto tpp = (CTaxasPlayer*)pp ;
		if ( m_vAllVicePools[nBigIdx].isPlayerInThisPool(nIdx) )
		{
			m_pInsurance->addCheckPeer(nIdx,tpp->getPeerCardByIdx(0),tpp->getPeerCardByIdx(1));
		}
		else
		{
			m_pInsurance->addExincludeCard(tpp->getPeerCardByIdx(1));
			m_pInsurance->addExincludeCard(tpp->getPeerCardByIdx(0));
		}
	}

	// add public cards 
	if ( m_nPublicCardRound >= 1 )
	{
		m_pInsurance->addPublicCard(m_vPublicCardNums[0]);
		m_pInsurance->addPublicCard(m_vPublicCardNums[1]);
		m_pInsurance->addPublicCard(m_vPublicCardNums[2]);
	}
	
	if ( 2 == m_nPublicCardRound )
	{
		m_pInsurance->addPublicCard(m_vPublicCardNums[3]);
	}

	uint16_t nBuyers = -1 ;
	std::vector<uint8_t> vOuts ;
	auto nOuts = m_pInsurance->getOuts(nBuyers,vOuts);
	if ( nOuts == 0 || nOuts > 16 )
	{
		return false ;
	}
	// is outs < 16 ? 
	// is have insurance buyer ? // must in final pool 
	return true ;
}

bool CTaxasRoom::isAnyOneBuyInsurace()
{
	if ( !m_isInsured )
	{
		return false ;
	}

	return m_nBuyInsuraceIdx != (uint8_t)-1 ;
}

void CTaxasRoom::doCaculateInsurance()
{
	if ( !m_isInsured )
	{
		return ;
	}

	if ( !isAnyOneBuyInsurace() )
	{
		return ;
	}

	uint16_t nIdx = 0 ;
	std::vector<uint8_t> vOuts ;
	m_pInsurance->getOuts(nIdx,vOuts);
	uint8_t nCurCard = 0 ;
	if ( m_nPublicCardRound == 2 )
	{
		nCurCard = m_vPublicCardNums[3] ;
	}
	else if ( 3 == m_nPublicCardRound )
	{
		nCurCard = m_vPublicCardNums[4] ;
	}
	else
	{
		assert("invalid round , this time can not do calculate" && 0 );
	}
	
	auto pPlayer = (CTaxasPlayer*)getPlayerByIdx(nIdx) ;
	if ( pPlayer->getInsuredAmount() == 0 )
	{
		LOGFMTE("why buy insurance is 0 ") ;
		return ;
	}
	bool isHit = std::find(vOuts.begin(),vOuts.end(),nCurCard) != vOuts.end();
	assert(pPlayer && "why buyer is null ?" );
	uint32_t nProfit = 0 ;
	uint32_t nAmout = pPlayer->getInsuredAmount();
	if ( isHit )  // hit the point , player win coin
	{
		nProfit = m_pInsurance->getInsuredProfit(nAmout);
		pPlayer->addWinInsuredCoin(nProfit);
	}
	else  // not hit point , player lose coin
	{
		pPlayer->setInsuranceLoseCoin(pPlayer->getInsuranceLoseCoin() + nAmout) ;
	}

	pPlayer->setInsuredAmount(0) ;
	// send msg tell the result 
	Json::Value jsresult ;
	jsresult["idx"] = nIdx ;
	int32_t nOffset = isHit ? nProfit : (((int32_t)nAmout) * -1 );
	jsresult["offset"] = nOffset;
	LOGFMTD("insurance offset = %d",nOffset) ;
	sendRoomMsg(jsresult,MSG_INSURANCE_CALCULATE_RESULT);
}

CInsuranceCheck* CTaxasRoom::getInsuranceCheck()
{
	return m_pInsurance ;
}

void CTaxasRoom::setInsuredPlayerIdx(uint8_t nIdx )
{
	m_nBuyInsuraceIdx = nIdx ;
}

//void CTaxasRoom::syncPlayerDataToDataSvr( stTaxasPeerData& pPlayerData )
//{
//	// if player requesting coin , do not sync data ;
//	stMsgCrossServerRequest msgReq ;
//	msgReq.cSysIdentifer = ID_MSG_PORT_DATA ;
//	msgReq.nReqOrigID = GetRoomID();
//	msgReq.nTargetID = pPlayerData.nUserUID ;
//	msgReq.nRequestType = eCrossSvrReq_AddMoney ;
//	msgReq.nRequestSubType = eCrossSvrReqSub_TaxasStandUp ;
//	msgReq.vArg[0] = true ;
//	msgReq.vArg[1] = pPlayerData.nTakeInMoney;
//	CTaxasServerApp::SharedGameServerApp()->sendMsg(pPlayerData.nSessionID,(char*)&msgReq,sizeof(msgReq)) ;
//	/// and just after game result ;
//	if ( pPlayerData.nPlayTimes > 0 )
//	{
//		stMsgSyncTaxasPlayerData msg ;
//		msg.nUserUID = pPlayerData.nUserUID ;
//		msg.nPlayTimes = pPlayerData.nPlayTimes ;
//		msg.nWinTimes = pPlayerData.nWinTimes ;
//		msg.nSingleWinMost = pPlayerData.nSingleWinMost ;
//		memset(msg.vBestCard,0,sizeof(msg.vBestCard));
//		if ( pPlayerData.vBestCards[0] ) // vBestCard not empty
//		{
//			memcpy(msg.vBestCard,pPlayerData.vBestCards,sizeof(msg.vBestCard));
//		}
//		CTaxasServerApp::SharedGameServerApp()->sendMsg(GetRoomID(),(char*)&msg,sizeof(msg)) ;
//	}
//}