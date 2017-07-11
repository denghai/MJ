#include "WaitActWithChuedCard.h"
#include "NewMJRoom.h"
#include "log4z.h"
#include <algorithm>
void CWaitActWithChuedCard::enterState(IRoom* pRoom,Json::Value& jsTransferData)
{
	IRoomState::enterState(pRoom,jsTransferData) ;
	m_isWaitingChoseAct = true ;
	m_nCurMaxChosedActPriority = eMJAct_None ;
	m_nInvokeIdx = jsTransferData["invokerIdx"].asUInt() ;
	m_nTargetCard = jsTransferData["card"].asUInt() ;
	m_eCardFrom = (eMJActType)jsTransferData["cardFrom"].asUInt() ;
	auto jsArray = jsTransferData["arrNeedIdxs"];
	if ( jsArray.size() == 0 )
	{
		LOGFMTE("none one need the card , need array is null , why come to this state ?") ;
	}

	setStateDuringTime(eTime_WaitPlayerChoseAct);

	if ( m_vWaitingObject.empty() == false )
	{
		LOGFMTE("why wait object is not empty ,when enter state ") ;
		m_vWaitingObject.clear() ;
	}

	// create waiting object and init them , find each act list of them 
	auto mjRoom = (CNewMJRoom*)pRoom ;
	auto mayEatIdx = mjRoom->getNextActIdx(m_nInvokeIdx) ;
	for ( uint8_t nIdx = 0 ; nIdx < jsArray.size(); ++nIdx )
	{
		auto pPlayerIdx = (uint8_t)jsArray[nIdx].asUInt() ;
		auto peer = getReusePeerInfo();
		m_vWaitingObject.push_back(peer) ;
		peer->nIdx = pPlayerIdx ;
		peer->vCanActList.push_back(eMJAct_Pass) ;
		peer->nExpectedMaxAct = eMJAct_None ;
		if ( eMJAct_BuGang_Declare == m_eCardFrom )
		{
			peer->nExpectedMaxAct = eMJAct_Hu ;
			peer->vCanActList.push_back(eMJAct_Hu) ;
			continue;
		}

		// check peng,
		if ( mjRoom->canPlayerPeng(pPlayerIdx,m_nTargetCard) )
		{
			peer->vCanActList.push_back(eMJAct_Peng) ;
			peer->nExpectedMaxAct = eMJAct_Peng ;
			// check ming gang ;
			if ( mjRoom->canPlayerMingGang(pPlayerIdx,m_nTargetCard) )
			{
				peer->nExpectedMaxAct = eMJAct_MingGang ;
				peer->vCanActList.push_back(eMJAct_MingGang) ;
			}
		}
		
		// check hu ;
		if ( mjRoom->canPlayerHu(pPlayerIdx,m_nTargetCard) )
		{
			peer->nExpectedMaxAct = eMJAct_Hu ;
			peer->vCanActList.push_back(eMJAct_Hu) ;
		}

		// check chi ;
		if ( mayEatIdx == pPlayerIdx && mjRoom->canPlayerEat(pPlayerIdx,m_nTargetCard) )
		{
			if ( peer->nExpectedMaxAct < eMJAct_Chi )
			{
				peer->nExpectedMaxAct = eMJAct_Chi ;
			}
			peer->vCanActList.push_back(eMJAct_Chi) ;
		}
	}

	// send can act list to each player ;
	for ( auto& pref : m_vWaitingObject )
	{
		mjRoom->sendActListToPlayerAboutCard(pref->nIdx,pref->vCanActList,m_nTargetCard,m_nInvokeIdx) ;
	}
}

void CWaitActWithChuedCard::leaveState()
{
	if ( ! m_vWaitingObject.empty() )
	{
		m_vResueObject.insert(m_vResueObject.begin(),m_vWaitingObject.begin(),m_vWaitingObject.end()) ;
	}
	m_vWaitingObject.clear() ;
}

void CWaitActWithChuedCard::onStateDuringTimeUp()
{
	if ( this->m_isWaitingChoseAct )
	{
		for ( auto iter = m_vWaitingObject.begin() ; iter != m_vWaitingObject.end() ; )
		{
			auto pref = *iter ;
			if ( pref->nChosedAct == eMJAct_Max )
			{
				recyclePeerInfoObject(*iter);
				m_vWaitingObject.erase(iter) ;
				iter = m_vWaitingObject.begin() ;
				continue;
			}
			++iter ;
		}

		doExcutingAct();
	}
	else
	{
		auto pRoom = (CNewMJRoom*)m_pRoom ;

		// none need the card 
		if ( m_vWaitingObject.empty() )
		{
			if ( eMJAct_BuGang_Pre == m_eCardFrom ) // none one robot the bu gang ; go on bang ;
			{
				Json::Value jsTran ;
				jsTran["idx"] = m_nInvokeIdx;
				jsTran["exeAct"] = eMJAct_BuGang_Done ; 
				jsTran["isWaitChoseAct"] = false ;
				jsTran["actCard"] = m_nTargetCard ;
				pRoom->goToState(eRoomState_WaitPlayerAct,&jsTran);
				return  ;
			}

			if ( pRoom->isGameOver() )
			{
				pRoom->goToState(eRoomState_GameEnd);
				return ;;
			}
			// common chu pai , direct go to next player act ;
			auto nNextIdx = pRoom->getNextActIdx(m_nInvokeIdx);
			Json::Value jsTran ;
			jsTran["idx"] = nNextIdx ;
			jsTran["exeAct"] = eMJAct_Mo ; 
			jsTran["isWaitChoseAct"] = false ;
			jsTran["actCard"] = 0 ;
			jsTran["onlyChu"] = 0 ;
			pRoom->goToState(eRoomState_WaitPlayerAct,&jsTran);
			return ;
		}

		// this card do be needed 

		// be roboted by other player ;
		if ( eMJAct_BuGang_Pre == m_eCardFrom )
		{
			pRoom->onPlayerDeclareGangBeRobted(m_nInvokeIdx,m_nTargetCard) ;
		}

		// actor not chose hu , so then must go to chu pai state 
		if ( m_vWaitingObject.size() >= 1 && m_vWaitingObject.front()->nChosedAct != eMJAct_Hu )
		{
			Json::Value jsTran ;
			jsTran["idx"] = m_vWaitingObject.front()->nIdx ;
			jsTran["exeAct"] = eMJAct_Chu ; 
			jsTran["isWaitChoseAct"] = true ;
			jsTran["actCard"] = 0 ;
			jsTran["onlyChu"] = 1 ;
			pRoom->goToState(eRoomState_WaitPlayerAct,&jsTran);
			return  ;
		}

		// target card be chose hu, find the biggest idx to calcualte next act player 
		while( 1 )
		{
			auto nNextIdx = pRoom->getNextActIdx(m_nMaxActedIdx);
			auto iter = std::find_if(m_vWaitingObject.begin(),m_vWaitingObject.end(),[nNextIdx](WAIT_PEER_INFO_PTR& peer ){ return peer->nIdx == nNextIdx ; } );
			if ( iter != m_vWaitingObject.end() )
			{
				m_nMaxActedIdx = nNextIdx ;
				LOGFMTD("already in wait list idx = %u , gon find",nNextIdx) ;
				continue;
			}

			if ( pRoom->isGameOver() )
			{
				pRoom->goToState(eRoomState_GameEnd);
				return ;;
			}

			Json::Value jsTran ;
			jsTran["idx"] = nNextIdx ;
			jsTran["exeAct"] = eMJAct_Mo ; 
			jsTran["isWaitChoseAct"] = false ;
			jsTran["actCard"] = 0 ;
			pRoom->goToState(eRoomState_WaitPlayerAct,&jsTran);
			return ;
		}

		LOGFMTE("state should not come here, you may need restart the server");
	}
}

bool CWaitActWithChuedCard::onMsg(Json::Value& prealMsg ,uint16_t nMsgType, eMsgPort eSenderPort , uint32_t nSessionID)
{
	if ( MSG_REQ_ACT_LIST == nMsgType )
	{
		Json::Value jsmsgBack ;
		jsmsgBack["ret"] = 1 ;
		if ( !m_isWaitingChoseAct )
		{
			m_pRoom->sendMsgToPlayer(jsmsgBack,MSG_REQ_ACT_LIST,nSessionID);
			return true ; 
		}

		auto idx = m_pRoom->getIdxBySessionID(nSessionID) ;
		auto iter = std::find_if(m_vWaitingObject.begin(),m_vWaitingObject.end(),[idx](WAIT_PEER_INFO_PTR& ptr ){  return (ptr->nIdx == idx);  });
		if ( iter == m_vWaitingObject.end() )
		{
			m_pRoom->sendMsgToPlayer(jsmsgBack,MSG_REQ_ACT_LIST,nSessionID);
			return true ;
		}
		auto mjRoom = (CNewMJRoom*)m_pRoom ;
		mjRoom->sendActListToPlayerAboutCard(idx,(*iter)->vCanActList,m_nTargetCard,m_nInvokeIdx) ;
		return true ;
	}

	if ( nMsgType != MSG_PLAYER_ACT )
	{
		return false ;
	}

	uint8_t nReqActType = prealMsg["actType"].asUInt();
	uint8_t nReqActCard = 0 ;
	if ( prealMsg["card"].isNumeric() )
	{
		nReqActCard = prealMsg["card"].asUInt();
	}

	uint8_t nRet = 0 ;
	do 
	{
		if ( this->m_isWaitingChoseAct == false )
		{
			LOGFMTE("cur state is not wait act, so you can not respone your act = %u, session id = %u",nReqActType,nReqActCard) ;
			nRet = 4 ;
			break;
		}

		auto idx = m_pRoom->getIdxBySessionID(nSessionID) ;
		if ( idx > m_pRoom->getSeatCount() )
		{
			LOGFMTE("you may not in this room , so can not do this act, sessionid = %u , act = %u",nSessionID,nReqActType) ;
			nRet = 1 ;
			break;
		}

		if ( nReqActCard != m_nTargetCard )
		{
			LOGFMTE("you req card is invalid , so can not do this act, sessionid = %u , card = %u, targetCard = %u",nSessionID,nReqActCard,m_nTargetCard);
			nRet = 3 ;
			break;
		}

		auto iter = std::find_if(m_vWaitingObject.begin(),m_vWaitingObject.end(),[idx](WAIT_PEER_INFO_PTR& peer){ return peer->nIdx == idx ;} ) ;
		if ( iter == m_vWaitingObject.end() )
		{
			LOGFMTE("you are not in the wait list") ;
			nRet = 1 ;
			break;
		}

		if ( (*iter)->isCanDoAct((eMJActType)nReqActType) == false )
		{
			LOGFMTE("you can not do this act , act type = %u, idx = %u",nReqActType,idx) ;
			nRet = 2 ;
			break;
		}

		if ( (*iter)->nChosedAct != eMJAct_Max )
		{
			LOGFMTE("you already chosed act = %u , can not chose twice , now = %u",(*iter)->nChosedAct,nReqActType) ;
			nRet = 4 ;
			break;
		}

		if ( eMJAct_Chi == nReqActType )
		{
			auto jsEatWith = prealMsg["eatWith"];
			if ( jsEatWith.isArray() == false || jsEatWith.size() != 2 )
			{
				LOGFMTE("you do eat act ,but do not send 2 eat with card session id =%u",nSessionID) ;
				nRet = 3 ;
				break;
			}

			auto pMJRoom = (CNewMJRoom*)m_pRoom ;
			if ( !pMJRoom->canPlayerEatWith(idx,jsEatWith[0u].asUInt(),jsEatWith[1u].asUInt()) )
			{
				LOGFMTE("you do eat act ,but do not have this two card eat with card session id =%u",nSessionID) ;
				nRet = 3 ;
				break;
			}

			(*iter)->nWithCardA = jsEatWith[0u].asUInt();
			(*iter)->nWithCardB = jsEatWith[1u].asUInt();
		}

		(*iter)->nChosedAct = (eMJActType)nReqActType ;
		if ( m_nCurMaxChosedActPriority < nReqActType )
		{
			m_nCurMaxChosedActPriority = (eMJActType)nReqActType ;
		}

		if ( isWaitingEnd() )
		{
			doExcutingAct();
		}
	} while (0);
	
	Json::Value jsmsgBack ;
	jsmsgBack["ret"] = nRet ;
	m_pRoom->sendMsgToPlayer(jsmsgBack,nMsgType,nSessionID) ;
	// omit the msg which is not player act ;
	// check the player is in waiting list ;
	// check the player is already chosed act ;
	// remove chosed pass act from wait list ;
	// update cur max act priority ;remove lower priority act waiting ;
	// check wait end ;
	// when card from declare gang pre only hu , be perrsion ;
	return true ;
}
	
bool CWaitActWithChuedCard::doExcutingAct()
{
	// prepare final wait list ;
	// remove pass act or smaller than maxact priority act 
	for ( auto iter = m_vWaitingObject.begin(); iter != m_vWaitingObject.end(); )
	{
		auto pRef = *iter ;
		if ( pRef->nChosedAct == eMJAct_Pass || pRef->nExpectedMaxAct < m_nCurMaxChosedActPriority )
		{
			recyclePeerInfoObject(pRef) ;
			m_vWaitingObject.erase(iter) ;
			iter = m_vWaitingObject.begin() ;
			continue;
		}
		++iter ;
	}

	// start to executing act ;
	m_isWaitingChoseAct = false ;
	auto pRoom = (CNewMJRoom*)m_pRoom ;

	if ( m_vWaitingObject.empty() )
	{
		m_nMaxActedIdx = m_nInvokeIdx ;
		setStateDuringTime(0.01f) ;
		LOGFMTD("all body give up about this card operation") ;
		return true ;
	}
	
	m_nMaxActedIdx = 0 ;
	for ( auto& ref : m_vWaitingObject )
	{
		if ( ref->nIdx > m_nMaxActedIdx )
		{
			m_nMaxActedIdx = ref->nIdx ;
		}

		switch ( ref->nChosedAct )
		{
		case eMJAct_Chi:
			{
				auto Ret = pRoom->onPlayerEat(ref->nIdx,m_nInvokeIdx,m_nTargetCard,ref->nWithCardA,ref->nWithCardB);
				if ( Ret == false )
				{
					LOGFMTE("you do eat act ,but can not eat with card = %u , = %u",ref->nWithCardA,ref->nWithCardB) ;
					return false ;
				}
				setStateDuringTime(eTime_DoPlayerAct_Peng);
			}
			break ;
		case eMJAct_Peng:
			{
				pRoom->onPlayerPeng(ref->nIdx,m_nInvokeIdx,m_nTargetCard) ;
				setStateDuringTime(eTime_DoPlayerAct_Peng);
			}
			break ;
		case eMJAct_MingGang:
			{
				pRoom->onPlayerMingGang(ref->nIdx,m_nInvokeIdx,m_nTargetCard) ;
				setStateDuringTime(eTime_DoPlayerAct_Gang);
			}
			break ;
		case eMJAct_Hu:
			{
				pRoom->onPlayerHu(ref->nIdx,m_nInvokeIdx,m_nTargetCard,m_eCardFrom == eMJAct_BuGang_Declare) ;
				setStateDuringTime(eTime_DoPlayerAct_Hu);
			}
			break;
		default:
			LOGFMTE("unknown act type for chued card act = %u",ref->nChosedAct) ;
			break;
		}
	}
	return true ;
}

CWaitActWithChuedCard::WAIT_PEER_INFO_PTR CWaitActWithChuedCard::getReusePeerInfo()
{
	if ( !m_vResueObject.empty() )
	{
		auto iter = m_vResueObject.begin() ;
		auto ref = *iter ;
		m_vResueObject.erase(iter);
		ref->nChosedAct = eMJAct_Max ;
		ref->vCanActList.clear();
		return ref ;
	}

	auto ref = WAIT_PEER_INFO_PTR(new stWaitPeerInfo() );
	return ref ;
}

void CWaitActWithChuedCard::recyclePeerInfoObject(WAIT_PEER_INFO_PTR& peerInfo )
{
	m_vResueObject.push_back(peerInfo) ;
}

bool CWaitActWithChuedCard::isWaitingEnd()
{
	for ( auto pRef : m_vWaitingObject )
	{
		if ( pRef->nExpectedMaxAct >= m_nCurMaxChosedActPriority && pRef->nChosedAct == eMJAct_Max )
		{
			return false ;
		}
	}
	return true ;
}