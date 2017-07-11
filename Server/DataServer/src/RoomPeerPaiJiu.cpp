#include "RoomPeerPaiJiu.h"
#include "NativeTypes.h"
#include "Player.h"
#include "RoomPaiJiu.h"
CRoomPeerPaiJiu::CRoomPeerPaiJiu(CPlayer* pPlayer)
	:CRoomPeer(pPlayer)
{
	m_eType = ePlayerComponent_RoomPeerPaiJiu ;
	memset(nBetCoin, 0 , sizeof(nBetCoin)) ;
}

void CRoomPeerPaiJiu::TryToLeaveRoom()
{
	if ( m_pRoom == NULL )
	{
		return ;
	}

	stMsgRoomLeaveRet msgBack ;
	msgBack.nRet = 0 ;

	CRoomPaiJiu* pPaiJiuRoom = (CRoomPaiJiu*)m_pRoom ;
	//if ( m_pRoom->GetRoomState() != eRoomState_PJ_Settlement  &&  m_pRoom->GetRoomState() != eRoomState_PJ_BankerSelectGoOn && m_pRoom->GetRoomState() != eRoomState_PJ_WaitBanker )
	if ( pPaiJiuRoom->GetBanker() == this || GetBetCoin() != 0 )
	{
		GetPlayer()->SetState(CPlayer::ePlayerState_WillLeavingRoom) ;
	}
	else
	{
		LeaveRoom() ;
		GetPlayer()->SetState(CPlayer::ePlayerState_Free) ;
	}
	SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
}

bool CRoomPeerPaiJiu::OnMessage(stMsg* pMsg )
{
	return CRoomPeer::OnMessage(pMsg) ;
}

void CRoomPeerPaiJiu::Reset()
{
	CRoomPeer::Reset();
	memset(nBetCoin, 0 , sizeof(nBetCoin)) ;
}

bool CRoomPeerPaiJiu::AddBetCoin( uint64_t naddBetCoin, ePaiJiuBetPort ePort )
{
	if ( ePort < ePJ_BetPort_None || ePort >= ePJ_BetPort_Max )
	{
		return false ;
	}
	// have enough coin ?
	if ( GetCoin() - GetBetCoin() >= naddBetCoin )
	{
		nBetCoin[ePort] += naddBetCoin ;
		return true ;
	}
	return false ;
}

uint64_t CRoomPeerPaiJiu::GetBetCoin()
{
	uint64_t nBet = 0 ;
	for ( int i = ePJ_BetPort_None ; i < ePJ_BetPort_Max ; ++i )
	{
		nBet += nBetCoin[i] ;
	}
	return nBet ;
}

int64_t CRoomPeerPaiJiu::OnSettlement(bool bShunWin , bool bTianWin , bool bDaoWin )
{
	double nConOffset = 0 ;
	nConOffset += ( bShunWin ? 1 : -1 ) * (nBetCoin[ePJ_BetPort_Shun] +  (nBetCoin[ePJ_BetPort_Qiao] + nBetCoin[ePJ_BetPort_ShunJiao]) * 0.5 );
	nConOffset += ( bDaoWin ? 1 : -1 ) * (nBetCoin[ePJ_BetPort_Dao] +  (nBetCoin[ePJ_BetPort_Qiao] + nBetCoin[ePJ_BetPort_DaoJiao]) * 0.5 ); 
	nConOffset += ( bTianWin ? 1 : -1 ) * (nBetCoin[ePJ_BetPort_Tian] +  ( nBetCoin[ePJ_BetPort_ShunJiao] + nBetCoin[ePJ_BetPort_DaoJiao] )* 0.5 );
	memset(nBetCoin, 0 , sizeof(nBetCoin)) ;
	OnWinCoin((int64_t)nConOffset) ;
	return (int64_t)nConOffset;
}