#include "MJWaitSupplyCoinState.h"
#include "log4z.h"
#include "MJRoomPlayer.h"
#include "MJRoom.h"
#include "MJWaitPlayerActState.h"
#include "MessageDefine.h"
#include "ServerMessageDefine.h"
void CMJWaitSupplyCoinState::enterState(IRoom* pRoom)
{
	IRoomState::enterState(pRoom) ;
	Json::Value js ;
	Json::Value jsArray ;
	for ( auto& ref : m_vWaitPlayerIdxs )
	{
		jsArray[jsArray.size()] = ref ;
	}
	js["players"] = jsArray ;
	pRoom->sendRoomMsg(js,MSG_ROOM_INFORM_SUPPLY_COIN);
	setStateDuringTime(eTime_WaitSupplyCoin);
}

bool CMJWaitSupplyCoinState::onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID )
{
	if ( IRoomState::onMessage(prealMsg,eSenderPort,nPlayerSessionID) )
	{
		return true ;
	}

	if ( MSG_SYNC_IN_GAME_ADD_COIN == prealMsg->usMsgType )
	{
		stMsgSyncInGameCoin* pRet = (stMsgSyncInGameCoin*)prealMsg ;
		auto pSitPlayer = m_pRoom->getSitdownPlayerBySessionID(nPlayerSessionID) ;
		
		// is sitdown layer 
		if ( !pSitPlayer )
		{
			LOGFMTD("you are not sit down player") ;
			return false ;
		}

		// is in waiting list ;
		auto pW = std::find(m_vWaitPlayerIdxs.begin(),m_vWaitPlayerIdxs.end(),pSitPlayer->getIdx()) ;
		if ( pW == m_vWaitPlayerIdxs.end() )
		{
			LOGFMTD("you are not in the wait list ") ;
			return false ;
		}

		auto nFinalCoin = pSitPlayer->getCoin() + pRet->nAddCoin ;
		// inform room players ;
		if ( nFinalCoin >= ((CMJRoom*)m_pRoom)->coinNeededToSitDown() )
		{
			m_vWaitPlayerIdxs.erase(pW) ;

			Json::Value jsmsg ;
			jsmsg["playerIdx"] = pSitPlayer->getIdx() ;
			jsmsg["result"] = 1 ;
			m_pRoom->sendRoomMsg(jsmsg,MSG_ROOM_PLAYER_SUPPLY_COIN_RESULT);
			LOGFMTD("uid = %u supply coin ok", pSitPlayer->getUserUID() ) ;
		}
		else
		{
			LOGFMTD("uid = %u supply coin is not enough , go on waiting to supply coin",pSitPlayer->getUserUID()) ;
		}

		if ( m_vWaitPlayerIdxs.empty())
		{
			finishWait();
		}

		return false ;
	}
	return false ;
}

bool CMJWaitSupplyCoinState::onMsg(Json::Value& prealMsg ,uint16_t nMsgType, eMsgPort eSenderPort , uint32_t nSessionID)
{
	if ( IRoomState::onMsg(prealMsg,nMsgType,eSenderPort,nSessionID) )
	{
		return true ;
	}

	if ( MSG_PLAYER_DECIDE_LOSE == nMsgType )
	{
		auto pp = m_pRoom->getSitdownPlayerBySessionID(nSessionID) ;
		pp->setState(eRoomPeer_DecideLose);

		Json::Value jsmsg ;
		jsmsg["playerIdx"] = pp->getIdx() ;
		jsmsg["result"] = 1 ;
		m_pRoom->sendRoomMsg(jsmsg,MSG_ROOM_PLAYER_SUPPLY_COIN_RESULT);

		auto removeIter = std::find(m_vWaitPlayerIdxs.begin(),m_vWaitPlayerIdxs.end(),pp->getIdx()) ;
		if ( removeIter != m_vWaitPlayerIdxs.end() )
		{
			m_vWaitPlayerIdxs.erase(removeIter) ;
		}

		if (m_vWaitPlayerIdxs.empty())
		{
			finishWait();
		}
		return true ;
	}
	return false ;
}

void CMJWaitSupplyCoinState::addWaitIdx(uint8_t nIdx )
{
	m_vWaitPlayerIdxs.insert(nIdx) ;
}

void CMJWaitSupplyCoinState::onStateDuringTimeUp()
{ 
	// check wait player idx , if not empty , all set to lose ;
	for ( auto& ref : m_vWaitPlayerIdxs )
	{
		auto ppPlayer = m_pRoom->getPlayerByIdx(ref) ; 
		ppPlayer->setState(eRoomPeer_DecideLose);

		Json::Value jsmsg ;
		jsmsg["playerIdx"] = ref ;
		jsmsg["result"] = 1 ;
		m_pRoom->sendRoomMsg(jsmsg,MSG_ROOM_PLAYER_SUPPLY_COIN_RESULT);
	}
	m_vWaitPlayerIdxs.clear();
	finishWait() ;
}

void CMJWaitSupplyCoinState::finishWait()
{
	if (m_vWaitPlayerIdxs.empty() == false)
	{
		LOGFMTE("wait supply coin is not empty , why finish wait ? ");
		return;
	}

	auto pRoom = (CMJRoom*)m_pRoom ;
	if ( 0 == m_nInvokedByAct )
	{
		if ( pRoom->isGameOver() )
		{
			pRoom->goToState(eRoomState_GameEnd) ;
			return ;
		}

		auto ppPlayer = (CMJRoomPlayer*)pRoom->getPlayerByIdx(m_nInvokerIdx) ;
		auto pTargeState = (IWaitingState*)m_pRoom->getRoomStateByID(eRoomState_WaitPlayerAct) ;

		float fWaitTime = eTime_WaitPlayerAct;
		if ( ppPlayer->isHaveState(eRoomPeer_AlreadyHu) )
		{
			fWaitTime *= 0.5 ;
			LOGFMTD("idx = %u already hu, so act time will be half as before",m_nInvokerIdx) ;
		}
		pTargeState->setWaitTime(pRoom->getWaitPlayerActTime(m_nInvokerIdx,fWaitTime)) ;

		pTargeState->addWaitingTarget(m_nInvokerIdx,eMJAct_Hu) ;
		m_pRoom->goToState(pTargeState);
	}
	else if ( 1 == m_nInvokedByAct )
	{
		if ( pRoom->isGameOver() )
		{
			pRoom->goToState(eRoomState_GameEnd) ;
			return ;
		}

		auto pTargeState = (IExecuingState*)m_pRoom->getRoomStateByID(eRoomState_DoPlayerAct) ;
		pTargeState->setExecuteTime(eTime_DoPlayerMoPai) ;

		stPlayerActTypeActionItem* pActTypeItem = new stPlayerActTypeActionItem ;
		pActTypeItem->nActType = eMJAct_Mo ;
		pActTypeItem->nActIdx = pRoom->getNextActPlayerIdx(m_nInvokerIdx) ;
		pActTypeItem->nCardNumber = 0 ;
		VEC_ACTITEM v ;
		v.push_back(pActTypeItem) ;
		pTargeState->setExecuteActs(v) ;

		m_pRoom->goToState(pTargeState) ;
	}
	else
	{
		assert("what type of invoker type " && 0);
	}
}

void CMJWaitSupplyCoinState::setInvokeInfo(uint8_t nInvokeIdx , uint8_t nInvokedByAct )
{
	m_nInvokedByAct = nInvokedByAct ;
	m_nInvokerIdx = nInvokeIdx ;
}