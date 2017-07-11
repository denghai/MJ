#include "BaccaratPeer.h"
#include "LogManager.h"
#include "Player.h"
#include "BaccaratMessageDefine.h"
CBaccaratPeer::CBaccaratPeer(CPlayer* pPlayer )
	:CRoomPeer(pPlayer)
{
	memset(m_vBetPort,0,sizeof(m_vBetPort)) ;
	m_eType = ePlayerComponent_BaccaratPeer ;
}

bool CBaccaratPeer::OnMessage(stMsg* pMsg )
{
	return CRoomPeer::OnMessage(pMsg) ;
}

void CBaccaratPeer::Reset()
{
	CRoomPeer::Reset();
	memset(m_vBetPort,0,sizeof(m_vBetPort)) ;
}

void CBaccaratPeer::TryToLeaveRoom()
{
	if ( m_pRoom == NULL )
	{
		return ;
	}

	stMsgRoomLeaveRet msgBack ;
	msgBack.nRet = 0 ;

	if ( GetBetCoin() > 0 )
	{
		GetPlayer()->SetState(CPlayer::ePlayerState_WillLeavingRoom) ;
		msgBack.nRet = 0 ;
	}
	else
	{
		LeaveRoom() ;
		GetPlayer()->SetState(CPlayer::ePlayerState_Free) ;
	}
	SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
}

bool  CBaccaratPeer::AddBetCoin( uint64_t naddBetCoin, eBaccaratBetPort ePort )
{
	//if ( GetCoin() < naddBetCoin )
	//{
	//	CLogMgr::SharedLogMgr()->ErrorLog("BC add Coin money not enough!") ;
	//	return false ;
	//}
	//if ( naddBetCoin <= 0 || ePort >= eBC_BetPort_Max )
	//{
	//	CLogMgr::SharedLogMgr()->ErrorLog("BC add coin error !") ;
	//	return false ;
	//}
	//m_vBetPort[ePort] += naddBetCoin ;
	//GetPlayerBaseData()->nTakeInRoomCoin -= naddBetCoin ;
	return true ;
}

uint64_t CBaccaratPeer::GetBetCoin()
{
	uint64_t nBet = 0 ;
	for ( int i = eBC_BetPort_One ; i < eBC_BetPort_Max ; ++i )
	{
		nBet += m_vBetPort[i] ;
	}
	return nBet ;
}

int64_t CBaccaratPeer::OnSettlement(bool bResult[eBC_BetPort_Max] )
{
	if ( GetBetCoin() <= 0 )
	{
		return 0 ;
	}

 	stMsgBCCaculate msgResult ;
	int64_t nWin = 0 ;
	for ( int i = eBC_BetPort_One; i < eBC_BetPort_Max ; ++i )
	{
		msgResult.vWinCoin[i] = 0 ;
		if ( bResult[i] )
		{
			msgResult.vWinCoin[i] = (m_vBetPort[i] * g_vfBaccaratRate[i] + 0.5);
			nWin += msgResult.vWinCoin[i];
		}
	}
	OnWinCoin(nWin) ;
	// send message to tell client ;
	msgResult.nFinalCoin = GetCoin();
	SendMsgToClient((char*)&msgResult,sizeof(msgResult)) ;
	return nWin ;
}

void CBaccaratPeer::OnGameEnd()
{
	memset(m_vBetPort,0,sizeof(m_vBetPort)) ;
	CRoomPeer::OnGameEnd();
}