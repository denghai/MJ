#include "RoomBase.h"
#include <assert.h>
#include "RoomPeer.h"
#include "Player.h"
#include "MessageDefine.h"
#include "GameServerApp.h"
#include "Timer.h"
#include "PlayerItem.h"
#include "LogManager.h"
#define KICK_LIMIT_TIME 20*60
CRoomBase::CRoomBase()
{
	m_eRoomType = eRoom_Max ;
	m_nRoomID = 0 ;
	m_vRoomPeer = NULL ;
	m_nMaxSeat = 0 ;
	m_nAntesCoin = 0 ;
	m_bIsDiamonedRoom = false ;
	m_nPassword = 0 ;
	m_strName = "";
	m_nMaxTakeIn = 0 ;
	m_vAllKickedPeers.clear();
	SetWaitOperateTime(13);
}

CRoomBase::~CRoomBase()
{
	delete[] m_vRoomPeer ;
	SetEnableUpdate(false) ;
}

void CRoomBase::Init( unsigned int nRoomID , unsigned char nMaxSeat )
{
	assert(m_eRoomType !=eRoom_Max && "Please assign m_eRoomtype" ) ;
	m_nRoomID = nRoomID ;
	m_nRoomLevel = 0 ;
	m_nMaxSeat = nMaxSeat ;
	m_eRoomState = eRoomState_Golden_WaitPeerToJoin ;
	m_vRoomPeer = new CRoomPeer*[nMaxSeat];
	m_nMaxTakeIn = 10000;
	SetWaitOperateTime(13);
	for ( int i = 0 ; i < nMaxSeat ; ++i )
	{
		m_vRoomPeer[i] = NULL ;
	}
	//CGameServerApp::SharedGameServerApp()->GetTimerMgr()->AddTimer(this,cc_selector_timer(CRoomBase::Update));
	SetTimerManager(CGameServerApp::SharedGameServerApp()->GetTimerMgr());
	SetEnableUpdate(true) ;
}

void CRoomBase::SendMsgRoomPeers(stMsg*pMsg ,unsigned short nLen ,CRoomPeer* pExcpetPeer)
{
	CRoomPeer* pPeer = NULL ;
	for ( int i = 0; i < GetMaxSeat(); ++i )
	{
		pPeer = m_vRoomPeer[i];
		if ( !pPeer || pPeer == pExcpetPeer || CPlayer::ePlayerState_WillLeavingRoom == pPeer->GetPlayer()->GetState() )
		{
			continue; 
		}
		pPeer->SendMsgToClient((char*)pMsg,nLen);
	}
}

CRoomPeer* CRoomBase::GetRoomPeerBySessionID( unsigned int nSessionID )
{
	for ( int i = 0 ; i < GetMaxSeat(); ++i )
	{
		if ( m_vRoomPeer[i] && m_vRoomPeer[i]->GetPlayer()->GetSessionID() == nSessionID )
		{
			return m_vRoomPeer[i] ;
		}
	}
	return NULL ;
}

int CRoomBase::GetRoomPeerCount()
{
	int ncount = 0 ;
	for ( int i = 0  ; i < GetMaxSeat() ; ++i )
	{
		if ( m_vRoomPeer[i] == NULL )
		{
			continue; 
		}
		++ncount ;
	}
	return ncount ;
}

void CRoomBase::OnPeerLeave( CRoomPeer* peer ) 
{
	if ( peer->GetPeerIdxInRoom() >= GetMaxSeat() )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("error OnPeerLeave you idx is too big  ") ;
		return ;
	}
	// send Leave msg ;
	m_vRoomPeer[peer->GetPeerIdxInRoom()] = NULL ;
	stMsgRoomPlayerLeave msg ;
	msg.nSessionID = peer->GetPlayer()->GetSessionID() ;
	SendMsgRoomPeers(&msg,sizeof(msg)) ;
}

void CRoomBase::AddPeer(CRoomPeer* peer )
{
		// find a empty place 
	for ( int i = 0 ; i < GetMaxSeat() ; ++i )
	{
		if ( m_vRoomPeer[i] == NULL )
		{
			peer->m_nPeerIdx = i ;
			break; 
		}
	}
	m_vRoomPeer[peer->GetPeerIdxInRoom()] = peer ;
	CLogMgr::SharedLogMgr()->PrintLog("add peer to room %s",peer->GetPlayerBaseData()->GetPlayerName());
}

unsigned char CRoomBase::CanJoin( CPlayer* peer )
{
	if ( GetEmptySeatCount() <= 0 )
		return 1 ;

	if ( IsDiamonedRoom() )
	{
		if ( peer->GetBaseData()->GetAllDiamoned() < GetAntesCoin() )
			return 2 ;
	}
	else
	{
		if ( peer->GetBaseData()->GetAllCoin() < GetAntesCoin() )
			return 2 ;
	}

	if ( IsInNotAllowedPeerList(peer->GetUserUID()) )
	{
		return 3 ;
	}

	return 0 ;
}

void CRoomBase::Update(float fTimeElpas, unsigned int nTimerID )
{
	
}

bool CRoomBase::OnPeerMsg(CRoomPeer* pPeer, stMsg* pmsg )
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
				CPlayerItemComponent* pItemMgr = (CPlayerItemComponent*)pPeer->GetPlayer()->GetComponent(ePlayerComponent_PlayerItemMgr);	
				if ( !pItemMgr->OnUserItem(ITEM_ID_INTERACTIVE) )
				{
					msgBackRet.nRet = 1 ;
					pPeer->SendMsgToClient((char*)&msgBackRet,sizeof(msgBackRet)) ;
					break;
				}

				if ( pMsgRet->nInteraciveWithPeerRoomIdx < 0 || pMsgRet->nInteraciveWithPeerRoomIdx >= GetMaxSeat() || m_vRoomPeer[pMsgRet->nInteraciveWithPeerRoomIdx] == NULL )
				{
					msgBackRet.nRet = 2 ;
					pPeer->SendMsgToClient((char*)&msgBackRet,sizeof(msgBackRet)) ;
					break;
				}
			}

			stMsgRoomOtherPlayerSpeak msg ;
			msg.nContentLen = pMsgRet->nContentLen ;
			msg.nContentType = pMsgRet->nContentType ;
			msg.nSpeakerRoomIdx = pPeer->GetPeerIdxInRoom() ;
			msg.nSpeakSex = pPeer->GetPlayerBaseData()->GetSex() ;
			msg.nSystemChatID = pMsgRet->nSystemChatID ;
			msg.nInteraciveWithPeerRoomIdx = pMsgRet->nInteraciveWithPeerRoomIdx;
			memcpy(msg.pSpeakName,pPeer->GetPlayerBaseData()->GetPlayerName(),sizeof(msg.pSpeakName));
			char* pBuffer = new char[sizeof(msg) + msg.nContentLen] ;
			unsigned short nOffset = 0 ;
			memcpy(pBuffer,&msg,sizeof(msg));
			nOffset += sizeof(msg);
			memcpy(pBuffer + nOffset,((char*)pMsgRet) + sizeof(stMsgRoomPlayerSpeak),msg.nContentLen );
			nOffset += msg.nContentLen ;
			SendMsgRoomPeers((stMsg*)pBuffer,nOffset) ;
			delete[] pBuffer ;
		}
		break;
	case MSG_ROOM_KICK_PEER:
		{
			stMsgKickPeerRet msgBack ;
			stMsgKickPeer* pRealMsg = (stMsgKickPeer*)pmsg ;
			msgBack.nRet = 0 ;
			if ( pRealMsg->nIdxToBeKick == pPeer->GetPeerIdxInRoom() )
			{
				msgBack.nRet = 4 ;
				pPeer->SendMsgToClient((char*)&msgBack,sizeof(msgBack) ) ;
				break;
			}
			
			if (pRealMsg->nIdxToBeKick >= GetMaxSeat() || m_vRoomPeer[pRealMsg->nIdxToBeKick] == NULL )
			{
				msgBack.nRet = 1 ;
				pPeer->SendMsgToClient((char*)&msgBack,sizeof(msgBack) ) ;
				break;				
			}

			if ( m_vRoomPeer[pRealMsg->nIdxToBeKick]->GetPlayerBaseData()->GetVipLevel() > pPeer->GetPlayerBaseData()->GetVipLevel() )
			{
				msgBack.nRet = 3 ;
				pPeer->SendMsgToClient((char*)&msgBack,sizeof(msgBack) ) ;
				break;
			}

			CPlayerItemComponent* pItemMgr = (CPlayerItemComponent*)pPeer->GetPlayer()->GetComponent(ePlayerComponent_PlayerItemMgr);	
			if ( pItemMgr->OnUserItem(ITEM_ID_KICK_CARD) )
			{
				// tell all other peer ;
				stMsgKickOtherPeerKickPeer msg ;
				msg.nIdxWhoBeKicked = pRealMsg->nIdxToBeKick ;
				msg.nIdxWhoKick = pPeer->GetPeerIdxInRoom();
				SendMsgRoomPeers(&msg,sizeof(msg));
				// can kick ;
				stKicPlayer stKickInfo;
				stKickInfo.nIdxInRoom = pRealMsg->nIdxToBeKick ;
				stKickInfo.bDoKicked = false ;
				stKickInfo.nUserUIDLimit = m_vRoomPeer[stKickInfo.nIdxInRoom]->GetPlayer()->GetUserUID();
				//memcpy(stKickInfo.cNameBeKicked,m_vRoomPeer[pRealMsg->nIdxToBeKick]->GetPlayerBaseData()->GetPlayerName(),sizeof(stKickInfo.cNameBeKicked));
				memcpy(stKickInfo.cNameKicker,pPeer->GetPlayerBaseData()->GetPlayerName(),sizeof(stKickInfo.cNameKicker));
				m_vAllKickedPeers.push_back(stKickInfo) ;
				ProcessKickedPeers();
			}
			else
			{
				msgBack.nRet = 2 ;
			}
			pPeer->SendMsgToClient((char*)&msgBack,sizeof(msgBack) ) ;
		}
		break;
	case MSG_ROOM_REQUEST_PEER_DETAIL:
		{
// 			stMsgRoomRequestPeerDetailRet msgBack ;
// 			stMsgRoomRequestPeerDetail* pMsgRet = (stMsgRoomRequestPeerDetail*)pmsg ;
// 			msgBack.nPeerIdxInRoom = pMsgRet->nPeerIdxInRoom ;
// 			CRoomPeer* pFindPeer = NULL ;
// 			if ( pMsgRet->nPeerIdxInRoom >= GetMaxSeat() || m_vRoomPeer[pMsgRet->nPeerIdxInRoom] == NULL )
// 			{
// 				msgBack.nRet = 1 ;
// 			}
// 			else
// 			{
// 				pFindPeer = m_vRoomPeer[pMsgRet->nPeerIdxInRoom];
// 				msgBack.nRet = 0 ;
// 				CPlayerBaseData* pBaseData = pFindPeer->GetPlayerBaseData() ;
// 				pBaseData->GetPlayerDetailData(&msgBack.stDetailInfo) ;
// 			}
// 			pPeer->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
		}
		break;
	case MSG_PlAYER_INVITED_FRIEND_TO_JOIN_ROOM:
		{
			stMsgPlayerInviteFriendToJoinRoomRet msgBack ;
			msgBack.nRet = 0 ;
			stMsgPlayerInviteFriendToJoinRoom* pMsgRet = (stMsgPlayerInviteFriendToJoinRoom*)pmsg ;
			if ( pMsgRet->nRoomIdx >= GetMaxSeat() || m_vRoomPeer[pMsgRet->nRoomIdx] != NULL )
			{
				msgBack.nRet = 1 ;
				pPeer->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
				break; ;
			}

			CPlayer* pPlayer = CGameServerApp::SharedGameServerApp()->GetPlayerMgr()->GetPlayerByUserUID(pMsgRet->nPlayerUID) ;
			if ( !pPlayer )
			{
				msgBack.nRet = 2 ;
				pPeer->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
				break; ;
			}

			//if ( pPlayer->GetState() != CPlayer::ePlayerState_Free )
			//{
			//	msgBack.nRet = 4 ;
			//	pPeer->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
			//	break; ;
			//}

			if ( CanJoin(pPlayer) != 0 )
			{
				msgBack.nRet = 3 ;
				pPeer->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
				break; ;
			}
			pPeer->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;

			// tell the player you invite ;
			stMsgPlayerBeInvitedToJoinRoom msgInfom ;
			memcpy(msgInfom.pNameWhoInviteMe,pPeer->GetPlayerBaseData()->GetPlayerName(),sizeof(msgInfom.pNameWhoInviteMe));
			msgInfom.nRoomID = GetRoomID();
			msgInfom.nRoomLevel = GetRoomLevel();
			msgInfom.nRoomType = GetRoomType() ;
			msgInfom.nSitIdx = pMsgRet->nRoomIdx ;
			msgInfom.nUserUIDWhoInviteMe = pPeer->GetPlayer()->GetUserUID() ;
			pPlayer->SendMsgToClient((char*)&msgInfom,sizeof(msgInfom)) ;
		}
		break;
	default:
		return false ;
	}
	return true ;
}

char CRoomBase::GetRoomPeerIdx(CRoomPeer* pPeer )
{
	for ( int i = 0  ; i < GetMaxSeat() ; ++i )
	{
		if ( m_vRoomPeer[i] == pPeer )
		{
			return  i ;
		}
	}
	return -1 ;
}

void CRoomBase::ProcessKickedPeers()
{
	// remove reached time peer  ;
	LIST_KICK_PEERS::iterator iter = m_vAllKickedPeers.begin() ;
	unsigned int nCurTime = (unsigned int)time(NULL) ;
	while ( iter != m_vAllKickedPeers.end() )
	{
		if ( (*iter).nLimitEndTime != 0 && nCurTime >= (*iter).nLimitEndTime )
		{
			m_vAllKickedPeers.erase(iter) ;
			iter = m_vAllKickedPeers.begin() ;
			continue; 
		}
		++iter ;
	}
	
	// kicked the peer neeed to be click
	iter = m_vAllKickedPeers.begin();
	for ( ; iter != m_vAllKickedPeers.end(); ++iter )
	{
		stKicPlayer& refKick = *iter ;
		if ( refKick.bDoKicked )
		{
			continue;
		}

		if ( m_vRoomPeer[refKick.nIdxInRoom] == NULL || m_vRoomPeer[refKick.nIdxInRoom]->GetPlayer()->GetUserUID() != refKick.nUserUIDLimit )  // peer is null or pos be other person
		{
			refKick.bDoKicked = true ;
			refKick.nLimitEndTime = nCurTime + KICK_LIMIT_TIME ;
			continue;
		}

		if ( m_vRoomPeer[refKick.nIdxInRoom]->IsInState(eRoomPeer_WaitCaculate) )  // game end will proccess 
		{
			continue;
		}

		// tell client ;
		stMsgExeBeKicked msgDo ;
		memcpy(msgDo.cNameKicer,refKick.cNameKicker,sizeof(msgDo.cNameKicer) );
		msgDo.nRoomIdxBeKicked = refKick.nIdxInRoom ;
		SendMsgRoomPeers(&msgDo,sizeof(msgDo)) ;

		// server do kick ;
		refKick.bDoKicked = true ;
		refKick.nLimitEndTime = nCurTime + KICK_LIMIT_TIME ;
		m_vRoomPeer[refKick.nIdxInRoom]->LeaveRoom();
	}
}

bool CRoomBase::IsInNotAllowedPeerList(unsigned int nUserUID )
{
	LIST_KICK_PEERS::iterator iter = m_vAllKickedPeers.begin();
	for ( ; iter != m_vAllKickedPeers.end(); ++iter )
	{
		if ( (*iter).nUserUIDLimit == nUserUID )
		{
			// check limit time 
			if ( time(NULL) >= (*iter).nLimitEndTime && (*iter).nLimitEndTime != 0 )
			{
				m_vAllKickedPeers.erase(iter) ;
				return false ;
			}
			return true ;
		}
	}
	return false ;
}
