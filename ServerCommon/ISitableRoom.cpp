#include "ISitableRoom.h"
#include "RoomConfig.h"
#include "ISitableRoomPlayer.h"
#include <cassert>
#include "MessageDefine.h"
#include "AutoBuffer.h"
#include "ServerMessageDefine.h"
#include "log4z.h"
#include <json/json.h>
#include "ServerStringTable.h"
#include "SeverUtility.h"
#include <time.h>
#include "IRoomDelegate.h"
#include <algorithm>
#include "IPeerCard.h"
#include "RobotDispatchStrategy.h"
ISitableRoom::~ISitableRoom()
{
	for ( uint8_t nIdx = 0 ; nIdx < m_nSeatCnt ; ++nIdx )
	{
		if ( m_vSitdownPlayers[nIdx])
		{
			delete m_vSitdownPlayers[nIdx];
			m_vSitdownPlayers[nIdx] = nullptr ;
		}
	}
	delete [] m_vSitdownPlayers;

	for ( ISitableRoomPlayer* pPlayer : m_vReserveSitDownObject )
	{
		delete pPlayer ;
		pPlayer = nullptr ;
	}
	m_vReserveSitDownObject.clear() ;

	delete m_pRobotDispatchStrage ;
	m_pRobotDispatchStrage = nullptr ;
}

bool ISitableRoom::onFirstBeCreated(IRoomManager* pRoomMgr, uint32_t nRoomID, const Json::Value& vJsValue ) 
{
	IRoom::onFirstBeCreated(pRoomMgr,nRoomID,vJsValue) ;
	m_nSeatCnt = 4 ;
	if ( vJsValue["seatCnt"].isNull() == true )
	{
		LOGFMTE("create room seatCnt js key is null") ;
	}
	else
	{
		m_nSeatCnt = vJsValue["seatCnt"].asUInt() ;
	}

	m_vSitdownPlayers = new ISitableRoomPlayer*[m_nSeatCnt] ;
	for ( uint8_t nIdx = 0 ; nIdx < m_nSeatCnt ; ++nIdx )
	{
		m_vSitdownPlayers[nIdx] = nullptr ;
	}

	m_pRobotDispatchStrage = new CRobotDispatchStrategy ;

	m_pRobotDispatchStrage->init(this,0,nRoomID,nRoomID);
	return true ;
}

void ISitableRoom::serializationFromDB(IRoomManager* pRoomMgr,stBaseRoomConfig* pConfig,uint32_t nRoomID , Json::Value& vJsValue )
{
	IRoom::serializationFromDB(pRoomMgr,pConfig,nRoomID,vJsValue);
	stSitableRoomConfig* pC = (stSitableRoomConfig*)pConfig;
	m_nSeatCnt = pC->nMaxSeat ;
	m_vSitdownPlayers = new ISitableRoomPlayer*[m_nSeatCnt] ;
	for ( uint8_t nIdx = 0 ; nIdx < m_nSeatCnt ; ++nIdx )
	{
		m_vSitdownPlayers[nIdx] = nullptr ;
	}

	m_pRobotDispatchStrage = new CRobotDispatchStrategy ;
	m_pRobotDispatchStrage->init(this,pConfig->nNeedRobotLevel,vJsValue["parentRoomID"].asUInt(),nRoomID);
}

void ISitableRoom::willSerializtionToDB(Json::Value& vOutJsValue)
{
	IRoom::willSerializtionToDB(vOutJsValue);
}

void ISitableRoom::roomItemDetailVisitor(Json::Value& vOutJsValue)
{
	IRoom::roomItemDetailVisitor(vOutJsValue);
	vOutJsValue["playerCnt"] = getSitDownPlayerCount();
}

bool ISitableRoom::canStartGame()
{
	if ( IRoom::canStartGame() == false )
	{
		return false ;
	}

	return getPlayerCntWithState(eRoomPeer_WaitNextGame) >= 2 ;
}

void ISitableRoom::update(float fDelta)
{
	IRoom::update(fDelta);
	m_pRobotDispatchStrage->update(fDelta) ;
}

void ISitableRoom::onPlayerEnterRoom(stEnterRoomData* pEnterRoomPlayer,int8_t& nSubIdx )
{
	IRoom::onPlayerEnterRoom(pEnterRoomPlayer,nSubIdx);
	auto pp = getSitdownPlayerByUID(pEnterRoomPlayer->nUserUID) ;
	if ( pp )
	{
		auto pStandPlayer = getPlayerByUserUID(pEnterRoomPlayer->nUserUID);
		if ( pStandPlayer->nCoin >= pp->getCoin())
		{
			if (pStandPlayer->nCoin != pp->getCoin())
			{
				LOGFMTD("uid = %u reenter room coin is not the same . stand = %u , sit = %u", pp->getUserUID(), pStandPlayer->nCoin, pp->getCoin());
			}
			pStandPlayer->nCoin = 0;
		}
		else
		{
			LOGFMTE("uid = %u stand coin is few then sit , why ? but stand = %u , sit = %u", pp->getUserUID(), pStandPlayer->nCoin, pp->getCoin());
			pStandPlayer->nCoin = 0;
		}
		pp->reactive(pEnterRoomPlayer->nUserSessionID) ;
	}
}

//bool ISitableRoom::onPlayerSitDown(ISitableRoomPlayer* pPlayer , uint8_t nIdx )
//{
//	if ( isSeatIdxEmpty(nIdx) )
//	{
//		m_vSitdownPlayers[nIdx] = pPlayer ;
//		pPlayer->doSitdown(nIdx) ;
//		pPlayer->setIdx(nIdx);
//
//		// save standup log ;
//		stMsgSaveLog msgLog ;
//		msgLog.nJsonExtnerLen = 0 ;
//		msgLog.nLogType = eLog_PlayerSitDown ;
//		msgLog.nTargetID = pPlayer->getUserUID() ;
//		memset(msgLog.vArg,0,sizeof(msgLog.vArg)) ;
//		msgLog.vArg[0] = getRoomType() ;
//		msgLog.vArg[1] = getRoomID() ;
//		msgLog.vArg[2] = pPlayer->getCoin() ;
//		sendMsgToPlayer(&msgLog,sizeof(msgLog),getRoomID()) ;
//		return true ;
//	}
//	return false ;
//}

void ISitableRoom::playerDoStandUp( ISitableRoomPlayer* pPlayer )
{
	m_pRobotDispatchStrage->onRobotLeave(pPlayer->getSessionID()) ;
	// remove from m_vSortByPeerCardsAsc ;
	auto iterSort = m_vSortByPeerCardsAsc.begin() ;
	for ( ; iterSort != m_vSortByPeerCardsAsc.end(); ++iterSort )
	{
		if ( *iterSort == pPlayer )
		{
			m_vSortByPeerCardsAsc.erase(iterSort) ;
			break;
		}
	}

	// remove other player data ;
	assert(isSeatIdxEmpty(pPlayer->getIdx()) == false && "player not sit down" );
	pPlayer->willStandUp();
	m_vSitdownPlayers[pPlayer->getIdx()] = nullptr ;
	auto standPlayer = getPlayerByUserUID(pPlayer->getUserUID()) ;
	
	if ( pPlayer->isHaveState(eRoomPeer_StayThisRound) )
	{
		LOGFMTD("uid = %d invoke game end , before stand up",pPlayer->getUserUID()) ;
		pPlayer->onGameEnd();
	}

	if ( standPlayer == nullptr )
	{
		if ( getDelegate() )
		{
			getDelegate()->onDelayPlayerWillLeaveRoom(this,pPlayer) ;
		}
	}
	else
	{
		standPlayer->nCoin += pPlayer->getCoin() ;
		standPlayer->nNewPlayerHaloWeight = pPlayer->getHaloWeight() ;
		standPlayer->nPlayerTimes += pPlayer->getPlayTimes();
		standPlayer->nWinTimes += pPlayer->getWinTimes();
		standPlayer->nGameOffset += pPlayer->getTotalGameOffset() ;
		if ( pPlayer->getSingleWinMost() > standPlayer->nSingleWinMost )
		{
			standPlayer->nSingleWinMost = pPlayer->getSingleWinMost() ;
		}
		LOGFMTD("player uid = %d just normal stand up ",pPlayer->getUserUID() ) ;
	}

	stMsgRoomStandUp msgStandUp ;
	msgStandUp.nIdx = pPlayer->getIdx() ;
	sendRoomMsg(&msgStandUp,sizeof(msgStandUp));

	m_vReserveSitDownObject.push_back(pPlayer) ;
}

void ISitableRoom::onPlayerWillLeaveRoom(stStandPlayer* pPlayer )
{
	ISitableRoomPlayer* pSitPlayer = getSitdownPlayerByUID(pPlayer->nUserUID) ;
	if ( pSitPlayer == nullptr )
	{
		LOGFMTD("you don't sit down ,so skip this uid = %u",pPlayer->nUserUID) ;
		return ;
	}

	onPlayerWillStandUp(pSitPlayer);
	pSitPlayer = getSitdownPlayerByUID(pPlayer->nUserUID) ;
	if ( pSitPlayer == nullptr )
	{
		LOGFMTD("player direct standup and can leave uid = %u",pPlayer->nUserUID) ;
		return ;
	}

	uint32_t nLeastLeftCoin = getLeastCoinNeedForCurrentGameRound(pSitPlayer) ;
	uint32_t nCoin = pSitPlayer->getCoin() ;
	if ( nCoin > nLeastLeftCoin )
	{
		pSitPlayer->setCoin(nLeastLeftCoin) ;
		pPlayer->nCoin += ( nCoin - nLeastLeftCoin );
		LOGFMTD("uid = %u will leave take away coin = %u, left coin = %u",pPlayer->nUserUID,pPlayer->nCoin,pSitPlayer->getCoin() ) ;
	}
	else
	{
		LOGFMTD("need coin too many ,uid = %u will leave take away coin = %u, left coin = %u",pPlayer->nUserUID,pPlayer->nCoin,pSitPlayer->getCoin() ) ;
	}
}

void ISitableRoom::onPlayerWillStandUp(ISitableRoomPlayer* pPlayer )
{
	if ( pPlayer )
	{
		pPlayer->delayStandUp();
	}
}

uint16_t ISitableRoom::getEmptySeatCount()
{
	if ( m_vSitdownPlayers == nullptr )
	{
		return getSeatCount() ;
	}

	uint16_t nCount = 0 ;
	for ( uint16_t nIdx = 0 ; nIdx < m_nSeatCnt ; ++nIdx )
	{
		if ( m_vSitdownPlayers[nIdx] == nullptr )
		{
			++nCount ;
		}
	}
	return nCount ;
}

ISitableRoomPlayer* ISitableRoom::getPlayerByIdx(uint16_t nIdx )
{
	assert(nIdx < getSeatCount() && "invalid player idx");
	if ( nIdx >= getSeatCount() )
	{
		return nullptr ;
	}
	return m_vSitdownPlayers[nIdx] ;
}

bool ISitableRoom::isSeatIdxEmpty( uint8_t nIdx )
{
	assert(nIdx < getSeatCount() && "invalid player idx");
	if ( nIdx >= getSeatCount() )
	{
		return false ;
	}
	return m_vSitdownPlayers[nIdx] == nullptr ;
}

uint16_t ISitableRoom::getSitDownPlayerCount()
{
	return getSeatCount() - getEmptySeatCount() ;
}

uint16_t ISitableRoom::getSeatCount()
{
	return m_nSeatCnt ;
}

ISitableRoomPlayer* ISitableRoom::getReuseSitableRoomPlayerObject()
{
	LIST_SITDOWN_PLAYERS::iterator iter = m_vReserveSitDownObject.begin() ;
	if ( iter != m_vReserveSitDownObject.end() )
	{
		ISitableRoomPlayer* p = *iter ;
		m_vReserveSitDownObject.erase(iter) ;
		return p ;
	}
	return doCreateSitableRoomPlayer();
}

uint16_t ISitableRoom::getPlayerCntWithState( uint32_t nState )
{
	uint8_t nCount = 0 ;
	for ( uint8_t nIdx = 0 ; nIdx < m_nSeatCnt ; ++nIdx )
	{
		if ( m_vSitdownPlayers[nIdx] && m_vSitdownPlayers[nIdx]->isHaveState(nState) )
		{
			++nCount ;
		}
	}
	return nCount ;
}

ISitableRoomPlayer* ISitableRoom::getSitdownPlayerBySessionID(uint32_t nSessionID)
{
	for ( uint8_t nIdx = 0 ; nIdx < m_nSeatCnt ; ++nIdx )
	{
		if ( m_vSitdownPlayers[nIdx] && m_vSitdownPlayers[nIdx]->getSessionID() == nSessionID )
		{
			return m_vSitdownPlayers[nIdx] ;
		}
	}
	return nullptr ;
}

ISitableRoomPlayer* ISitableRoom::getSitdownPlayerByUID(uint32_t nUserUID )
{
	for ( uint8_t nIdx = 0 ; nIdx < m_nSeatCnt ; ++nIdx )
	{
		if ( m_vSitdownPlayers[nIdx] && m_vSitdownPlayers[nIdx]->getUserUID() == nUserUID )
		{
			return m_vSitdownPlayers[nIdx] ;
		}
	}
	return nullptr ;
}

bool ISitableRoom::onMessage( Json::Value& prealMsg ,uint16_t nMsgType, eMsgPort eSenderPort , uint32_t nSessionID )
{
	if ( MSG_REQUEST_ROOM_AUDIENTS == nMsgType )
	{
		Json::Value jsmsg ;
		enumAudientsPlayer([this,&jsmsg](IRoom::stStandPlayer* pPlayer){ if (pPlayer == nullptr)return ; auto ps = getSitdownPlayerByUID(pPlayer->nUserUID) ; if (ps) return ; jsmsg[jsmsg.size()] = pPlayer->nUserUID; });
		Json::Value js ;
		js["audients"] = jsmsg ;
		sendMsgToPlayer(nSessionID,js,nMsgType) ;
		return true ;
	}

	return IRoom::onMessage(prealMsg,nMsgType,eSenderPort,nSessionID) ;
}

bool ISitableRoom::onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID )
{
	if ( IRoom::onMessage(prealMsg,eSenderPort,nPlayerSessionID) )
	{
		return true ;
	}

	switch ( prealMsg->usMsgType )
	{
	case MSG_REQ_CUR_GAME_OFFSET:
		{
			auto pPlayer = getSitdownPlayerBySessionID(nPlayerSessionID) ;
			if ( pPlayer )
			{
				stMsgReqRobotCurGameOffsetRet msgback ;
				msgback.nCurGameOffset = pPlayer->getTotalGameOffset();
				sendMsgToPlayer(&msgback,sizeof(msgback),nPlayerSessionID) ;
				LOGFMTD("robot req cur offset = %d , uid = %u",msgback.nCurGameOffset,pPlayer->getUserUID());
			}
		}
		break;
	case MSG_ADD_TEMP_HALO:
		{
			auto pPlayer = getPlayerBySessionID(nPlayerSessionID) ;
			if ( pPlayer == nullptr )
			{
				LOGFMTE("not in room player add temp halo session id = %u",nPlayerSessionID);
				break;
			}

			//if ( pPlayer->nPlayerType == ePlayer_Normal )
			//{
			//	LOGFMTE("normal can not add temp halo");
			//	break;
			//}

			stMsgAddTempHalo* pRet = (stMsgAddTempHalo*)prealMsg ;
			if ( 0 == pRet->nTargetUID )
			{
				pRet->nTargetUID = pPlayer->nUserUID ;
			}

			auto psitpp = getSitdownPlayerByUID(pRet->nTargetUID) ;
			if ( psitpp )
			{
				psitpp->setTempHaloWeight(pRet->nTempHalo);
				LOGFMTD("uid = %u add temp halo = %u",pRet->nTargetUID,pRet->nTempHalo) ;
			}
			else
			{
				LOGFMTE("uid = %u not sit down why add temp halo",pPlayer->nUserUID);
			}
		}
		break;
	case MSG_PLAYER_SITDOWN:
		{
			stMsgPlayerSitDownRet msgBack ;
			msgBack.nRet = 0 ;

			stStandPlayer* pPlayer = getPlayerBySessionID(nPlayerSessionID) ;
			if ( !pPlayer )
			{
				LOGFMTE("palyer session id = %d ,not in this room so , can not sit down",nPlayerSessionID) ;
				msgBack.nRet = 3 ;
				sendMsgToPlayer(&msgBack,sizeof(msgBack),nPlayerSessionID) ;
				break; 
			}

			auto pp = getSitdownPlayerByUID(pPlayer->nUserUID);
			if ( pp )
			{
				LOGFMTE("session id = %d , already sit down , don't sit down again",nPlayerSessionID ) ;
				msgBack.nRet = 4 ;
				sendMsgToPlayer(&msgBack,sizeof(msgBack),nPlayerSessionID) ;
				break;
			}

			stMsgPlayerSitDown* pRet = (stMsgPlayerSitDown*)prealMsg ;
			if ( pRet->nTakeInCoin == 0 || pRet->nTakeInCoin > pPlayer->nCoin)
			{
				pRet->nTakeInCoin = pPlayer->nCoin ;
			}

			if ( pRet->nTakeInCoin < coinNeededToSitDown() )
			{
				msgBack.nRet = 1 ;
				sendMsgToPlayer(&msgBack,sizeof(msgBack),nPlayerSessionID) ;
				break; 
			}

			if ( isSeatIdxEmpty(pRet->nIdx) == false )
			{
				msgBack.nRet = 2 ;
				sendMsgToPlayer(&msgBack,sizeof(msgBack),nPlayerSessionID) ;
				break; 
			}

			auto sitDownPlayer = getReuseSitableRoomPlayerObject() ;
			sitDownPlayer->reset(pPlayer) ;
			pPlayer->nCoin -= pRet->nTakeInCoin ;
			sitDownPlayer->setCoin(pRet->nTakeInCoin) ;
			sitDownPlayer->doSitdown(pRet->nIdx) ;
			sitDownPlayer->setIdx(pRet->nIdx);
			sitDownPlayer->setState(eRoomPeer_WaitNextGame );
			m_vSitdownPlayers[pRet->nIdx] = sitDownPlayer ;

			// tell others ;
			stMsgRoomSitDown msgSitDown ;
			msgSitDown.nIdx = sitDownPlayer->getIdx() ;
			msgSitDown.nSitDownPlayerUserUID = sitDownPlayer->getUserUID() ;
			msgSitDown.nTakeInCoin = sitDownPlayer->getCoin() ;
			sendRoomMsg(&msgSitDown,sizeof(msgSitDown));

			onPlayerSitDown(sitDownPlayer) ;

			if ( pPlayer->nPlayerType == ePlayer_Robot )
			{
				LOGFMTD("robot uid = %d enter room",sitDownPlayer->getUserUID()) ;
				m_pRobotDispatchStrage->onRobotJoin(sitDownPlayer->getSessionID());
			}
		}
		break;
	case MSG_PLAYER_STANDUP:
		{
			stMsgPlayerStandUpRet msgBack ;
			msgBack.nRet = 0 ;
			auto player = getSitdownPlayerBySessionID(nPlayerSessionID) ;
			if ( player == nullptr )
			{
				msgBack.nRet = 1 ;
				sendMsgToPlayer(&msgBack,sizeof(msgBack),nPlayerSessionID) ;
				break; 
			}
			onPlayerWillStandUp(player);
		}
		break;
	default:
		return false;
	}
	return true ;
}

void ISitableRoom::onGameDidEnd()
{
	for ( uint8_t nIdx = 0 ; nIdx < m_nSeatCnt ; ++nIdx )
	{
		auto pPlayer = m_vSitdownPlayers[nIdx] ;
		if ( pPlayer == nullptr )
		{
			continue;
		}

		if ( pPlayer->isHaveState(eRoomPeer_StayThisRound) && getDelegate() )
		{
			getDelegate()->onUpdatePlayerGameResult(this,pPlayer->getUserUID(),pPlayer->getGameOffset()) ;
			LOGFMTD("update room peer offset uid = %u, offset = %d",pPlayer->getUserUID(),pPlayer->getGameOffset());
		}
		else  if (pPlayer->getGameOffset() != 0)
		{
			getDelegate()->onUpdatePlayerGameResult(this, pPlayer->getUserUID(), pPlayer->getGameOffset());
			LOGFMTE("can not skip update room peer offset uid = %u, offset = %d", pPlayer->getUserUID(), pPlayer->getGameOffset());
		}

		if ( pPlayer->getNoneActTimes() >= getMaxNoneActTimeForStandUp() || (pPlayer->isDelayStandUp() || pPlayer->getCoin() < coinNeededToSitDown() || (getDelegate() && getDelegate()->isPlayerLoseReachMax(this,pPlayer->getUserUID())) ) )
		{
			playerDoStandUp(pPlayer);	
			pPlayer = nullptr ;
			m_vSitdownPlayers[nIdx] = nullptr ;
		}

		if ( pPlayer )
		{
			pPlayer->onGameEnd() ;
		}
	}
	m_vSortByPeerCardsAsc.clear();
	IRoom::onGameDidEnd() ;
}

void ISitableRoom::onGameWillBegin()
{
	IRoom::onGameWillBegin() ;
	uint16_t nSeatCnt = getSeatCount() ;
	for ( uint8_t nIdx = 0; nIdx < nSeatCnt; ++nIdx )
	{
		ISitableRoomPlayer* pp = getPlayerByIdx(nIdx) ;
		if ( pp )
		{
			pp->setCoin(pp->getCoin() - getDeskFee() ) ;
			addTotoalProfit(getDeskFee());
			pp->onGameBegin();
		}
	}
}

bool sortPlayerByCard(ISitableRoomPlayer* pLeft , ISitableRoomPlayer* pRight )
{
	if ( pLeft->getPeerCard()->pk(pRight->getPeerCard()) == IPeerCard::PK_RESULT_FAILED )
	{
		return true ;
	}
	return false ;
}

void ISitableRoom::doProcessNewPlayerHalo()
{
	// add peer 
	for ( uint8_t nIdx = 0 ; nIdx < m_nSeatCnt ; ++nIdx )
	{
		auto pPlayer = m_vSitdownPlayers[nIdx] ;
		if ( pPlayer && (pPlayer->isHaveState(eRoomPeer_CanAct)) )
		{
			m_vSortByPeerCardsAsc.push_back(pPlayer) ;
		}
	}

	if ( m_vSortByPeerCardsAsc.size() < 2 )
	{
		LOGFMTE("why can act player count not bigger than 2 room id = %u",getRoomID()) ;
		return ;
	}

	// sort by peer card 
	std::sort(m_vSortByPeerCardsAsc.begin(),m_vSortByPeerCardsAsc.end(),sortPlayerByCard);
	//if ( isOmitNewPlayerHalo() )
	//{
	//	return ;
	//}

	// process halo 
	uint8_t nHalfCnt = m_vSortByPeerCardsAsc.size() ;
	uint8_t nSwitchTargetIdx = m_vSortByPeerCardsAsc.size() - 1 ;
	for ( uint8_t nIdx = 0 ; nIdx < nHalfCnt; ++nIdx)
	{
		if ( m_vSortByPeerCardsAsc[nIdx]->isHaveHalo() == false )
		{
			continue;
		}

		for ( ; nSwitchTargetIdx > nIdx ; --nSwitchTargetIdx )
		{
			if ( m_vSortByPeerCardsAsc[nSwitchTargetIdx]->isHaveHalo() )
			{
				continue;
			}

			m_vSortByPeerCardsAsc[nIdx]->switchPeerCard(m_vSortByPeerCardsAsc[nSwitchTargetIdx]);
			auto player = m_vSortByPeerCardsAsc[nIdx] ;
			m_vSortByPeerCardsAsc[nIdx] = m_vSortByPeerCardsAsc[nSwitchTargetIdx] ;
			m_vSortByPeerCardsAsc[nSwitchTargetIdx] = player ;

			if ( nSwitchTargetIdx == 0 )
			{
				return ;
			}
			--nSwitchTargetIdx;
			break;
		}
	}
#ifndef NDEBUG
	LOGFMTD("room id = %u do halo result:",getRoomID());
	for ( uint8_t nIdx = 0 ; nIdx < m_vSortByPeerCardsAsc.size() ; ++nIdx )
	{
		LOGFMTD("idx = %u uid = %u",nIdx,m_vSortByPeerCardsAsc[nIdx]->getUserUID());
	}
	LOGFMTD("room id = %u halo end",getRoomID());
#endif // !NDEBUG

}

uint8_t ISitableRoom::GetFirstInvalidIdxWithState( uint8_t nIdxFromInclude , eRoomPeerState estate )
{
	auto seatCnt = getSeatCount() ;
	for ( uint8_t nIdx = nIdxFromInclude ; nIdx < seatCnt * 2 ; ++nIdx )
	{
		uint8_t nRealIdx = nIdx % seatCnt ;
		if ( getPlayerByIdx(nRealIdx) == nullptr )
		{
			continue;
		}

		if ( getPlayerByIdx(nRealIdx)->isHaveState(estate) )
		{
			return nRealIdx ;
		}
	}
	LOGFMTE("why don't have peer with state = %d",estate ) ;
	return 0 ;
}
