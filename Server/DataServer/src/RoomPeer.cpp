#include "RoomPeer.h"
#include "Player.h"
#include "LogManager.h"
#include "GameServerApp.h"
#include "RoomBase.h"
#include "PlayerOnlineBox.h"
CRoomPeer::CRoomPeer(CPlayer* pPlayer )
	:IPlayerComponent(pPlayer)
{
	m_eType = ePlayerComponent_RoomPeer ;
	m_pRoom = NULL ;
	m_nPeerIdx = 0 ;
	m_bIsInDiamonedRoom = false;
	m_eState = eRoomPeer_StandUp ;
	m_nLastTakeIn = 0 ;
}

CRoomPeer::~CRoomPeer()
{

}

void CRoomPeer::TryToLeaveRoom()
{
	if ( m_pRoom == NULL )
	{
		return ;
	}
	LeaveRoom() ;
}

bool CRoomPeer::OnMessage(stMsg* pMsg )
{
	if ( MSG_ROOM_LEAVE == pMsg->usMsgType && m_pRoom )  // default process leave ;
	{
		LeaveRoom() ;
		stMsgRoomLeaveRet msgBack ;
		msgBack.nRet = 0 ;
		SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
		return true ;
	}

	if ( MSG_ROBOT_APPLY_TO_LEAVE == pMsg->usMsgType && m_pRoom )
	{
		stMsgRobotApplyToLeaveRoomRet msgRet ;
		msgRet.nRet = m_pRoom->GetRoomPeerCount() > MIN_PEERS_IN_ROOM_ROBOT ? 0 : 1 ;
		if ( msgRet.nRet == 0  )
		{
			LeaveRoom() ;
		}
		SendMsgToClient((char*)&msgRet,sizeof(msgRet)) ;
		return true ;
	}

	if ( m_pRoom )
	{
		return m_pRoom->OnPeerMsg(this,pMsg) ;
	}
	return false ;
}


void CRoomPeer::OnPlayerDisconnect()
{
	if ( m_pRoom )
	{
		TryToLeaveRoom();
	}
}

void CRoomPeer::Reset()
{
	m_eState = eRoomPeer_None ;
	m_nLastTakeIn = 0 ;
}

CPlayerBaseData* CRoomPeer::GetPlayerBaseData()
{
	return GetPlayer()->GetBaseData() ;
}

void CRoomPeer::SetState(eRoomPeerState eSate )
{ 
	m_eState = eSate ;
	if ( eSate == eRoomPeer_StandUp )
	{
		CPlayerOnlineBox* pOnlineBox = GetPlayerBaseData()->GetOnlineBox();
		pOnlineBox->PauseTimer();
	}
	else if ( eSate == eRoomPeer_SitDown )
	{
		CPlayerOnlineBox* pOnlineBox = GetPlayerBaseData()->GetOnlineBox();
		pOnlineBox->StartTimer();
	}
}

//void CRoomPeer::OnGetCard( unsigned char nCardA, unsigned char nCardB , unsigned char nCardC )
//{
//	m_PeerCard.SetPeerCardByNumber(nCardA,nCardB,nCardC) ;
//}

void CRoomPeer::LeaveRoom()
{
	if ( m_pRoom == NULL )
	{
		return ;
	}

	if ( m_pRoom )
	{
		m_pRoom->OnPeerLeave(this);
	}
	m_pRoom = NULL ;
	CLogMgr::SharedLogMgr()->PrintLog(" %s Levae room room change to NULL room type = %d",GetPlayerBaseData()->GetPlayerName(),GetComponentType());
	GetPlayer()->GetBaseData()->CaculateTakeInMoney();
	GetPlayer()->SetState(CPlayer::ePlayerState_Free) ;
	Reset();
}

//void CRoomPeer::OnWaitTimeOut()// please make a message fake ;
//{
//	if ( m_pRoom )
//	{
//		stMsgRoomGiveU                                                                                                                                                                                                                                                                                                                              p msg ;
//		m_pRoom->OnPeerMsg(this,&msg) ;
//	}
//}

//bool CRoomPeer::IsActive() // not fail ,not give, not null 
//{
//	return (GetState() == eRoomPeer_Unlook || GetState() == eRoomPeer_Look );
//}

unsigned int CRoomPeer::GetSessionID()
{
	return GetPlayer()->GetSessionID() ;
}

uint64_t CRoomPeer::GetCoin()
{
	return GetPlayerBaseData()->GetTakeInMoney(GetIsInDiamonedRoom());
}

void CRoomPeer::SetTakeInCoin(uint64_t nCoin, bool bDiamoned)
{
	GetPlayerBaseData()->SetTakeInCoin(nCoin,bDiamoned) ;
	m_nLastTakeIn = nCoin ;
}

//unsigned int  CRoomPeer::AddBetCoin( unsigned int naddBetCoin ) // return indeeed added betCoin ;
//{
//	if ( naddBetCoin > GetCoin() )
//	{
//		naddBetCoin  = GetCoin() ;
//	}
//	m_nBetCoin += naddBetCoin ;
//	GetPlayerBaseData()->nCoin -= naddBetCoin ;
//	return naddBetCoin ;
//}

//void CRoomPeer::GetBrifBaseInfo(stRoomPeerBrifInfo& vInfoOut )
//{
//	vInfoOut.ePeerState = GetState() ;
//	vInfoOut.nBetCoin = m_nBetCoin ;
//	vInfoOut.nCoin = GetCoin() ;
//	vInfoOut.nDefaulPhotoID = GetPlayerBaseData()->nDefaulPhotoID ;
//	vInfoOut.nIdx = m_nPeerIdx ;
//	vInfoOut.nSessionID = GetSessionID() ;
//	vInfoOut.nTitle = GetPlayerBaseData()->nTitle ;
//	vInfoOut.nUserDefinePhotoID = GetPlayerBaseData()->nUserDefinePhotoID ;
//	memset(vInfoOut.pName,0	,sizeof(vInfoOut.pName));
//	sprintf_s(vInfoOut.pName,"%s",GetPlayerBaseData()->strName);
//}

void CRoomPeer::OnWinCoin(int64_t nWinCoin )
{
	GetPlayerBaseData()->ModifyTakeInMoney(nWinCoin,GetIsInDiamonedRoom());
	if ( nWinCoin != 0 )
	{
		GetPlayerBaseData()->SaveCoin() ;
	}

	//if ( GetPlayer()->GetState() == CPlayer::ePlayerState_WillLeavingRoom )
	//{
	//	GetPlayer()->SetState(CPlayer::ePlayerState_Free) ;
	//	LeaveRoom() ;
	//}
}

void CRoomPeer::OnGameEnd()
{
	CLogMgr::SharedLogMgr()->PrintLog("%s OnGameEnd type = %d",GetPlayerBaseData()->GetPlayerName(),GetComponentType());

	if ( GetPlayer()->GetState() == CPlayer::ePlayerState_WillLeavingRoom )
	{
		GetPlayer()->SetState(CPlayer::ePlayerState_Free) ;
		LeaveRoom() ;
	}
}