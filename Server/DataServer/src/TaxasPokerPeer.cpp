#include "TaxasPokerPeer.h"
#include "TaxasPokerMessage.h"
#include "Player.h"
#include "RoomTexasPoker.h"
#include "PlayerEvent.h"
#include "LogManager.h"
#define MAX_NONE_ACT_TIMES 5
CTaxasPokerPeer::CTaxasPokerPeer(CPlayer* pPlayer)
	:CRoomPeer(pPlayer)
{
	m_nContinueNoneAct = 0 ;
}

CTaxasPokerPeer::~CTaxasPokerPeer()
{

}

bool CTaxasPokerPeer::OnMessage(stMsg* pMsg )
{
	if ( m_pRoom == NULL )
	{
		return false ;
	}

	if ( CRoomPeer::OnMessage(pMsg))
	{
		return true ;
	}

	// process msg ;
	return false ;
}

void CTaxasPokerPeer::OnPlayerDisconnect()
{
	if ( m_pRoom == NULL )
	{
		return ;
	}
	{
		CRoomPeer::OnPlayerDisconnect();
	}
}

void CTaxasPokerPeer::Reset()
{
	SetState(eRoomPeer_StandUp);
	m_nAllBetCoin = 0 ;
	m_nBetThisRound = 0 ;
	m_nAllWinCoin = 0 ;
	m_nContinueNoneAct = 0 ;
	m_eMyAction = eRoomPeerAction_None ;
	m_PeerCard.Reset();

	CRoomPeer::Reset();
}

void CTaxasPokerPeer::OnWinCoin(int64_t nWinCoin )
{
	GetPlayerBaseData()->ModifyTakeInMoney(nWinCoin,GetIsInDiamonedRoom());
}

void CTaxasPokerPeer::TryToLeaveRoom()
{
	if ( IsInState(eRoomPeer_WaitCaculate) )
	{
		GetPlayer()->SetState(CPlayer::ePlayerState_WillLeavingRoom) ;
	}
	else
	{
		CRoomPeer::TryToLeaveRoom();
	}
}

void CTaxasPokerPeer::OnGameEnd()
{
	if ( GetCoinOffsetThisGame() > 0 )
	{
		GetPlayerBaseData()->AddWinTimes() ;
	}
	else if ( GetCoinOffsetThisGame() < 0 )
	{
		GetPlayerBaseData()->AddLoseTimes() ;
	}
	m_eMyAction = eRoomPeerAction_None ;
	m_nAllBetCoin = 0 ;
	m_nBetThisRound = 0 ;
	m_nAllWinCoin = 0 ;
	// post player event ;
	stPlayerEventArgRoundEnd eveArg ;
	eveArg.eEventType = ePlayerEvent_RoundEnd ;
	eveArg.cCardType = m_PeerCard.GetCardType();
	if ( GetIsInDiamonedRoom() )
	{
		eveArg.nCoinOffset = 0 ;
		eveArg.nDiamonedOffset = (int)GetCoinOffsetThisGame() ;
		if ( eveArg.nDiamonedOffset > 0 &&  (unsigned int)eveArg.nDiamonedOffset > GetPlayerBaseData()->GetSingleWinMost() )
		{
			//GetPlayerBaseData()->SetSingleWinMost(eveArg.nDiamonedOffset) ;
		}
	}
	else
	{
		eveArg.nCoinOffset = GetCoinOffsetThisGame() ;
		eveArg.nDiamonedOffset = 0 ;
		if ( eveArg.nCoinOffset > 0 && (uint64_t) eveArg.nCoinOffset > GetPlayerBaseData()->GetSingleWinMost() )
		{
			{
				GetPlayerBaseData()->SetSingleWinMost(eveArg.nCoinOffset) ;
			}
		}
	}
	GetPlayer()->PostPlayerEvent(&eveArg) ;
	// update max cards ;must when win 
	if ( GetCoinOffsetThisGame() > 0 )
	{
		unsigned char* pMaxCard = GetPlayerBaseData()->GetMaxCards();
		if ( *pMaxCard == *(pMaxCard + 1 ) && *pMaxCard == 0 )
		{
			m_PeerCard.GetFinalCard(pMaxCard) ;
		}
		else
		{
			CTaxasPokerPeerCard tTemp ;
			for ( int i = 0 ; i < 5 ; ++i )
			{
				tTemp.AddCardByCompsiteNum( *(pMaxCard + i ) ) ;
			}

			if ( m_PeerCard.PK(&tTemp) == 1 )
			{
				m_PeerCard.GetFinalCard(pMaxCard) ;
			}
		}
	}
	SetState(eRoomPeer_WaitNextGame) ;
	CRoomPeer::OnGameEnd();
}

void CTaxasPokerPeer::OnStartNewRound()
{
	m_nBetThisRound = 0 ;
	m_eMyAction = eRoomPeerAction_None ;
}

void CTaxasPokerPeer::OnStartNewGame()
{
	SetState(eRoomPeer_CanAct);
	m_nAllBetCoin = 0 ;
	m_nBetThisRound = 0 ;
	m_nAllWinCoin = 0 ;
	m_eMyAction = eRoomPeerAction_None ;
	m_PeerCard.Reset();
}

void CTaxasPokerPeer::AddBetCoinThisRound(int64_t nCoin )
{ 
	m_nBetThisRound += nCoin ; 
	m_nAllBetCoin += nCoin ;
	GetPlayerBaseData()->ModifyTakeInMoney(-1*nCoin,GetIsInDiamonedRoom() );
}

void CTaxasPokerPeer::DecreasBetCoinThisRound( int64_t nCoin ) 
{
	if ( nCoin <= m_nBetThisRound )
	{
		m_nBetThisRound -= nCoin ;
	}
	else
	{
		m_nBetThisRound = 0 ;
		CLogMgr::SharedLogMgr()->ErrorLog("DecreasBetCoinThisRound offset big then already bet coin , offset = %I64, AlreadyBet = %I64d",nCoin,m_nBetThisRound) ;
	}
}

void CTaxasPokerPeer::OnWinCoinThisPool(uint64_t nCoin )
{
	OnWinCoin(nCoin) ;
	m_nAllWinCoin += nCoin ;
}

void CTaxasPokerPeer::OnWaitActionTimeOut()
{
	if ( GetPlayer()->GetState() == CPlayer::ePlayerState_WillLeavingRoom )
	{
		LeaveRoom();
		return ;
	}

	++m_nContinueNoneAct;

	stMsgTaxasPokerPeerAction msg ;
	msg.ePeerAct = eRoomPeerAction_GiveUp ;
	CRoomTexasPoker*pRoom = (CRoomTexasPoker*)GetRoom() ;
	if ( pRoom->GetCurMostBetCoinThisRound() == GetBetCoinThisRound() )
	{
		msg.ePeerAct = eRoomPeerAction_Pass ;
	}
	
	if ( m_nContinueNoneAct > MAX_NONE_ACT_TIMES )
	{
		msg.ePeerAct = eRoomPeerAction_StandUp ;
		m_nContinueNoneAct = 0 ;
	}
	unsigned nNoneActTimes = m_nContinueNoneAct ;
	OnMessage( &msg ) ;
	m_nContinueNoneAct = nNoneActTimes;
}

bool CTaxasPokerPeer::AutoTakein()
{
	// check total coin ;
	if ( GetPlayerBaseData()->GetAllCoin() < m_pRoom->GetAntesCoin() )
	{
		return false ;
	}
	// decide how many to take 
	uint64_t nTakeInCoin = m_nLastTakeIn ;   // the same as last time ;
	if ( nTakeInCoin < m_pRoom->GetAntesCoin() ) 
	{
		nTakeInCoin = m_pRoom->GetAntesCoin() + 0.7f*(m_pRoom->GetMaxTakeInCoin() - m_pRoom->GetAntesCoin() );
	}
	else if ( nTakeInCoin > m_pRoom->GetMaxTakeInCoin() )
	{
		nTakeInCoin = m_pRoom->GetMaxTakeInCoin();
	}

	if ( nTakeInCoin > GetPlayerBaseData()->GetAllCoin() )
	{
		nTakeInCoin = GetPlayerBaseData()->GetAllCoin();
	}
	SetTakeInCoin(nTakeInCoin,GetIsInDiamonedRoom()) ;
	return true ;
}