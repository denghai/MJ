#include "RoomBaseNew.h"
#include "Player.h"
#include "MessageDefine.h"
#include "RoomBaseData.h"
#include "LogManager.h"
#include "GameServerApp.h"
#include <assert.h>
#include "RoomConfig.h"
#include "RoomManager.h"
#include "PlayerItem.h"
#include "PlayerBaseData.h"
CRoomBaseNew::CRoomBaseNew()
{
	m_pRoomData = NULL ;
}

CRoomBaseNew::~CRoomBaseNew()
{
	if ( m_pRoomData )
	{
		delete m_pRoomData ;
		m_pRoomData = NULL ;
	}
	m_vAllPeers.clear() ;
}

bool CRoomBaseNew::Init(stBaseRoomConfig* pConfig)
{
	assert(m_pRoomData == NULL&& "error m_pRoomData can not be NULL") ;
	stRoomBaseDataOnly*pBase = GetRoomDataOnly();
	pBase->cGameType = pConfig->nRoomType ;
	pBase->nRoomLevel = pConfig->nRoomLevel ;
	pBase->cMaxPlayingPeers = pConfig->nMaxSeat;
	pBase->nRoomID = ++CRoomManager::s_RoomID ;
	pBase->cMiniCoinNeedToEnter = pConfig->nMinNeedToEnter ;
	pBase->fOperateTime = pConfig->nWaitOperateTime ;

	m_vAllPeers.clear() ;
	SetTimerManager(CGameServerApp::SharedGameServerApp()->GetTimerMgr());
	SetEnableUpdate(true) ;
	
	return true ;
}

void CRoomBaseNew::Enter(CPlayer* pEnter )
{
	MAP_SESSION_PLAYER::iterator iter = m_vAllPeers.find(pEnter->GetSessionID()) ;
	if ( iter != m_vAllPeers.end() )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("player uid = %d ,with session id = %d already in room id = %d , type = %d",pEnter->GetUserUID(),pEnter->GetSessionID(),m_pRoomData->m_pData->nRoomID,m_pRoomData->m_pData->cGameType) ;
		m_vAllPeers.erase(iter) ;
	}
	m_vAllPeers.insert(MAP_SESSION_PLAYER::value_type(pEnter->GetSessionID(),pEnter));
	pEnter->SetCurRoom(this);
}

void CRoomBaseNew::Leave(CPlayer* pLeaver)
{
	RemovePlayerBySessionID(pLeaver->GetSessionID()) ;
}

unsigned int CRoomBaseNew::GetRoomID()
{
	return GetRoomDataOnly()->nRoomID ;
}

unsigned char CRoomBaseNew::GetRoomType()
{
	return GetRoomDataOnly()->cGameType ;
}

unsigned char CRoomBaseNew::GetRoomLevel()
{
	return GetRoomDataOnly()->nRoomLevel ;
}

void CRoomBaseNew::Update(float fTimeElpas, unsigned int nTimerID )
{

}

bool CRoomBaseNew::OnMessage(CPlayer*pSender, stMsg* pmsg)
{
	// process common msg that don't assocaite with specail card game ;
	// for example : speak work , look other player info ,and make firend , and so on ;
	// when processed , return true , other case return false ;
	switch ( pmsg->usMsgType )
	{
	case MSG_ROOM_SPEAK:
		{
			stMsgRoomPlayerSpeakRet msgBackRet ;
			msgBackRet.nRet = 0 ;
			stMsgRoomPlayerSpeak* pMsgRet = (stMsgRoomPlayerSpeak*)pmsg ;
			if ( 3 == pMsgRet->nContentType ) // interactive speak ;
			{
				CPlayerItemComponent* pItemMgr = (CPlayerItemComponent*)pSender->GetComponent(ePlayerComponent_PlayerItemMgr);	
				if ( !pItemMgr->OnUserItem(ITEM_ID_INTERACTIVE) )
				{
					msgBackRet.nRet = 1 ;
					pSender->SendMsgToClient((char*)&msgBackRet,sizeof(msgBackRet)) ;
					break;
				}

				if ( pMsgRet->nInteraciveWithPeerRoomIdx < 0 || pMsgRet->nInteraciveWithPeerRoomIdx >= GetMaxSeat() || NULL == GetData()->GetPeerDataByIdx(pMsgRet->nInteraciveWithPeerRoomIdx) )
				{
					msgBackRet.nRet = 2 ;
					pSender->SendMsgToClient((char*)&msgBackRet,sizeof(msgBackRet)) ;
					break;
				}
			}

			stMsgRoomOtherPlayerSpeak msg ;
			msg.nContentLen = pMsgRet->nContentLen ;
			msg.nContentType = pMsgRet->nContentType ;
			msg.nSpeakerRoomIdx = GetData()->GetRoomIdxBySessionID(pSender->GetSessionID());
			msg.nSpeakSex = pSender->GetBaseData()->GetSex() ;
			msg.nSystemChatID = pMsgRet->nSystemChatID ;
			msg.nInteraciveWithPeerRoomIdx = pMsgRet->nInteraciveWithPeerRoomIdx;
			memcpy(msg.pSpeakName,pSender->GetBaseData()->GetPlayerName(),sizeof(msg.pSpeakName));
			char* pBuffer = new char[sizeof(msg) + msg.nContentLen] ;
			unsigned short nOffset = 0 ;
			memcpy(pBuffer,&msg,sizeof(msg));
			nOffset += sizeof(msg);
			memcpy(pBuffer + nOffset,((char*)pMsgRet) + sizeof(stMsgRoomPlayerSpeak),msg.nContentLen );
			nOffset += msg.nContentLen ;
			SendMsgBySessionID((stMsg*)pBuffer,nOffset);
			delete[] pBuffer ;
		}
		break;
	case MSG_ROOM_KICK_PEER:
		{
			stMsgKickPeerRet msgBack ;
			stMsgKickPeer* pRealMsg = (stMsgKickPeer*)pmsg ;
			msgBack.nRet = 1 ;

			// check have kick card 
			CPlayerItemComponent* pItemMgr = (CPlayerItemComponent*)pSender->GetComponent(ePlayerComponent_PlayerItemMgr);	
			if ( pItemMgr->GetItemCountByID(ITEM_ID_KICK_CARD) <= 0 )
			{
				pSender->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
				break; 
			}

			// real kick them 
			bool bRigNow = false ;
			CPlayer* pKickPlayer = GetPlayerByRoomIdx(pRealMsg->nIdxToBeKick);
			msgBack.nRet = GetData()->OnPlayerKick(pSender->GetSessionID(),pRealMsg->nIdxToBeKick,&bRigNow) ;
			if ( msgBack.nRet == 0 )
			{
				if ( pItemMgr->OnUserItem(ITEM_ID_KICK_CARD) )
				{
					// tell all other peer ;
					stMsgKickOtherPeerKickPeer msg ;
					msg.nIdxWhoBeKicked = pRealMsg->nIdxToBeKick ;
					msg.nIdxWhoKick = GetData()->GetRoomIdxBySessionID(pSender->GetSessionID()) ;
					SendMsgBySessionID(&msg,sizeof(msg));
				}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          
			}
			pSender->SendMsgToClient((char*)&msgBack,sizeof(msgBack) ) ;
			
			if ( pKickPlayer && bRigNow )
			{
				RemovePlayerBySessionID(pKickPlayer->GetSessionID());
			}
		}
		break;
	case MSG_PlAYER_INVITED_FRIEND_TO_JOIN_ROOM:
		{
			stMsgPlayerInviteFriendToJoinRoomRet msgBack ;
			msgBack.nRet = 0 ;
			stMsgPlayerInviteFriendToJoinRoom* pMsgRet = (stMsgPlayerInviteFriendToJoinRoom*)pmsg ;
			if ( pMsgRet->nRoomIdx >= GetMaxSeat() || GetData()->GetPeerDataByIdx(pMsgRet->nRoomIdx) != NULL )
			{
				msgBack.nRet = 1 ;
				pSender->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
				break; ;
			}

			CPlayer* pPlayer = CGameServerApp::SharedGameServerApp()->GetPlayerMgr()->GetPlayerByUserUID(pMsgRet->nPlayerUID) ;
			if ( !pPlayer )
			{
				msgBack.nRet = 2 ;
				pSender->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
				break; ;
			}

			if ( CheckCanJoinThisRoom(pPlayer) != 0 )
			{
				msgBack.nRet = 3 ;
				pSender->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
				break; ;
			}
			pSender->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;

			// tell the player you invite ;
			stMsgPlayerBeInvitedToJoinRoom msgInfom ;
			memcpy(msgInfom.pNameWhoInviteMe,pSender->GetBaseData()->GetPlayerName(),sizeof(msgInfom.pNameWhoInviteMe));
			msgInfom.nRoomID = GetRoomID();
			msgInfom.nRoomLevel = GetRoomLevel();
			msgInfom.nRoomType = GetRoomType() ;
			msgInfom.nSitIdx = pMsgRet->nRoomIdx ;
			msgInfom.nUserUIDWhoInviteMe = pSender->GetUserUID() ;
			pPlayer->SendMsgToClient((char*)&msgInfom,sizeof(msgInfom)) ;
		}
		break;
	default:
		return false ;
	}
	return true ;
}

void CRoomBaseNew::SendMsgBySessionID(stMsg* pMsg , unsigned short nLen,unsigned int nSessionID,bool bToAll )  // if nsessionid = 0 , means send all peers in this room ;
{
	CPlayer* pPlayer = NULL ;
	if ( bToAll == false )
	{
		MAP_SESSION_PLAYER::iterator iter = m_vAllPeers.find(nSessionID) ;
		if ( iter != m_vAllPeers.end() )
		{
			pPlayer = iter->second ;
			pPlayer->SendMsgToClient((char*)pMsg,nLen);
			return ;
		}
		else
		{
			CLogMgr::SharedLogMgr()->ErrorLog("can not send msg = %d to session id = %d",pMsg->usMsgType ,nSessionID) ;
		}
	}
	else
	{
		MAP_SESSION_PLAYER::iterator iter = m_vAllPeers.begin() ;
		for ( ; iter != m_vAllPeers.end() ; ++iter )
		{
			pPlayer = iter->second ;
			if ( pPlayer->GetSessionID() != nSessionID )
			{
				pPlayer->SendMsgToClient((char*)pMsg,nLen);
			}
		}
	}
}

void CRoomBaseNew::SendMsgByRoomIdx(stMsg* pMsg, unsigned short nLen ,unsigned char nIdx)
{
	stPeerBaseData* pdata = m_pRoomData->GetPeerDataByIdx(nIdx);
	if ( pdata )
	{
		SendMsgBySessionID(pMsg,nLen,pdata->nSessionID) ;
	}
	else
	{
		CLogMgr::SharedLogMgr()->ErrorLog("Can not send msg to player with room idx = %d , msg = %d, idx player is NULL",nIdx,pMsg->usMsgType) ;
	}
}

void CRoomBaseNew::SitDown(CPlayer* pPlayer )
{

}

void CRoomBaseNew::StandUp(CPlayer* pPlayer)
{

}

unsigned char CRoomBaseNew::CheckCanJoinThisRoom(CPlayer* pPlayer) // 0 means ok , other value means failed ;
{
	return 0 ;
}

void CRoomBaseNew::SendRoomInfoToPlayer(CPlayer* pPlayer)
{

}

unsigned int CRoomBaseNew::GetAntesCoin()
{
	return GetRoomDataOnly()->cMiniCoinNeedToEnter ;
}

unsigned short CRoomBaseNew::GetEmptySeatCount()
{
	return GetData()->GetEmptySeatCnt();
}

unsigned short CRoomBaseNew::GetMaxSeat()
{
	return GetData()->GetMaxSeat();
}

void CRoomBaseNew::CheckDelayedKickedPlayer()
{
	if ( m_pRoomData->m_vSessionIDs.empty() )
		return ;
	std::vector<unsigned int> vKickedSessions ;
	GetData()->OnCheckDelayKickPlayers(vKickedSessions);
 
	for ( unsigned int i = 0 ; i < vKickedSessions.size(); ++i )
	{
		RemovePlayerBySessionID(vKickedSessions[i]);
	}
}

stRoomBaseDataOnly* CRoomBaseNew::GetRoomDataOnly()
{
	return m_pRoomData->m_pData ;
}

void CRoomBaseNew::RemovePlayerBySessionID(unsigned int nSessionId)
{
	MAP_SESSION_PLAYER::iterator iter = m_vAllPeers.find(nSessionId) ;
	if ( iter == m_vAllPeers.end() )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("player not in this room , player uid = %d",iter->second->GetUserUID()) ;
		return ;
	}
	iter->second->SetState(CPlayer::ePlayerState_Free);
	iter->second->SetCurRoom(NULL);
	m_vAllPeers.erase(iter) ;
}

CPlayer* CRoomBaseNew::GetPlayerByRoomIdx(unsigned char pIdx )
{
	stPeerBaseData* p = m_pRoomData->GetPeerDataByIdx(pIdx) ;
	if ( p )
	{
		MAP_SESSION_PLAYER::iterator iter = m_vAllPeers.find(p->nSessionID) ;
		if ( iter != m_vAllPeers.end() )
		{
			return iter->second ;
		}
	}
	return NULL ;
}