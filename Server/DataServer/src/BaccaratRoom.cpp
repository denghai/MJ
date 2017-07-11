#include "BaccaratRoom.h"
#include "BaccaratPeer.h"
#include "LogManager.h"
#include "BaccaratMessageDefine.h"
#include "RobotManager.h"
CBaccaratRoom::CBaccaratRoom()
{
	m_eRoomType = eRoom_Baccarat ;
}

CBaccaratRoom::~CBaccaratRoom()
{

}

void CBaccaratRoom::Init( unsigned int nRoomID , unsigned char nMaxSeat )
{
	CRoomBase::Init(nRoomID,nMaxSeat) ;
	m_fTimeTicket = 0 ;
	memset(m_vBetPort,0,sizeof(m_vBetPort)) ;
	m_eRoomState = eRoomState_BC_WaitBet ;
	m_tPoker.InitBaccarat();

	for ( int i = 0 ; i < MAX_BACCART_CARD_COUNT ; ++i )
	{
		m_vIdleCard[i].SetCard(CCard::eCard_Joker,53) ;
		m_vBankerCard[i].SetCard(CCard::eCard_Joker,53) ;
	}
}

void CBaccaratRoom::AddPeer(CRoomPeer* peer )
{
	CRoomBase::AddPeer(peer) ;
	peer->SetTakeInCoin(peer->GetPlayerBaseData()->GetAllCoin(),false);
}

void CBaccaratRoom::OnPeerLeave( CRoomPeer* peer )
{
	CRoomBase::OnPeerLeave(peer) ;
}

unsigned char CBaccaratRoom::CanJoin(CPlayer* peer )  // 0 means ok , 1 room is full , 2 money is not engough ;
{
	return CRoomBase::CanJoin(peer) ;
}

void CBaccaratRoom::Update(float fTimeElpas, unsigned int nTimerID )
{
	m_fTimeTicket += fTimeElpas ;
	switch ( GetRoomState() )
	{
	case eRoomState_BC_WaitBet:
		{
			if ( m_fTimeTicket >= TIME_BACCARAT_WAIT_BET )
			{
				GoToState(eRoomState_BC_Distribute);
			}
		}
		break;
	case eRoomState_BC_Shuffle:
		{
			if ( m_fTimeTicket >= eRoomState_BC_WaitBet )
			{
				GoToState(eRoomState_BC_WaitBet);
			}
		}
		break;
	case eRoomState_BC_Distribute:
		{
			if ( m_fTimeTicket >= TIME_BACCARAT_DISTRIBUTE )
			{
				unsigned int idx = m_nRound ;
				if ( m_nRound >= MAX_BACCARAT_RECORD )
				{
					for ( int i = 0 ; i < eBC_BetPort_Max ; ++i )
					{
						memcpy(&m_vRecorder[i][0],&m_vRecorder[i][1],sizeof(bool) * (MAX_BACCARAT_RECORD - 1 ));
					}
					idx = MAX_BACCARAT_RECORD - 1 ;
				}
				// check idle pair 
				m_vRecorder[eBC_BetPort_IdlePair][idx] = m_vIdleCard[0].GetCardFaceNum() == m_vIdleCard[1].GetCardFaceNum();
				// check banker pair 
				m_vRecorder[eBC_BetPort_IdlePair][idx] = m_vBankerCard[0].GetCardFaceNum() == m_vBankerCard[1].GetCardFaceNum();
				// if have pair , must tell client ; send message ;

				if ( GetPeerPoint(false) >= 8 ||  GetPeerPoint(true) >= 8 ) // direct stand card ;
				{
					GoToState(eRoomState_BC_Caculate);
				}
				else
				{
					GoToState(eRoomState_BC_AddIdleCard);
				}
			}
		}
		break;
	case eRoomState_BC_AddBankerCard:
		{
			if ( m_fTimeTicket >= TIME_BACCARAT_ADD_CARD )
			{
				GoToState(eRoomState_BC_Caculate);
			}
		}
		break;
	case eRoomState_BC_AddIdleCard:
		{
			if ( m_fTimeTicket >= TIME_BACCARAT_ADD_CARD )
			{
				GoToState(eRoomState_BC_AddBankerCard);
			}
		}
		break;
	case eRoomState_BC_Caculate:
		{
			if ( m_fTimeTicket >= TIME_BACCARAT_CACULATE )
			{
				// processe game end 
				OnGameEnd() ;
				//
				if ( m_tPoker.GetLeftCard() < 6 )
				{
					GoToState(eRoomState_BC_Shuffle);
				}
				else
				{
					GoToState(eRoomState_BC_WaitBet);
				}
			}
		}
		break;
	default:
		CLogMgr::SharedLogMgr()->ErrorLog("unknown baccarat room state = %d",m_eRoomState ) ;
		break;
	}
}

bool CBaccaratRoom::OnPeerMsg(CRoomPeer* pPeer, stMsg* pmsg )
{
	if ( CRoomBase::OnPeerMsg(pPeer,pmsg) )
	{
		return true ;
	}

	if ( MSG_BC_BET == pmsg->usMsgType )
	{
		stMsgBCPlayerBetRet msgBack ;
		msgBack.nRet = 0 ;
		stMsgBCPlayerBet* pBetRet = (stMsgBCPlayerBet*)pmsg ;
		if ( GetRoomState() != eRoomState_BC_WaitBet )
		{
			msgBack.nRet = 1 ;
			pPeer->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
			return true ;
		}
		if ( pBetRet->nBetCoin <= 0 || pBetRet->nBetCoin > pPeer->GetCoin() )
		{
			msgBack.nRet = 3 ;
			pPeer->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
			return true ;
		}

		if ( pBetRet->cBetPort >= eBC_BetPort_Max )
		{
			msgBack.nRet = 2 ;
			pPeer->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
			return true ;
		}

		CBaccaratPeer* pRp = (CBaccaratPeer*)pPeer;
		pRp->AddBetCoin(pBetRet->nBetCoin,(eBaccaratBetPort)pBetRet->cBetPort) ;
		m_vBetPort[pBetRet->cBetPort] += pBetRet->nBetCoin ;
		stMsgBCOtherPlayerBet msgInfo ;
		msgInfo.nSession = pPeer->GetSessionID() ;
		msgInfo.cBetPort = pBetRet->cBetPort ;
		msgInfo.nBetCoin = pBetRet->nBetCoin ;
		SendMsgRoomPeers(&msgInfo,sizeof(msgInfo)) ;
		return true ;
	}
	return false ;
}

bool CBaccaratRoom::SendCurRoomInfoToPlayer(CRoomPeer* pPeer )
{
	stMsgBaccaratRoomInfo msg ;
	msg.cRoomLevel = GetRoomLevel();
	msg.cRoomState = GetRoomState();
	msg.fTimeTicket = m_fTimeTicket;
	msg.m_nRound = m_nRound ;
	for ( int i = 0 ; i < MAX_BACCART_CARD_COUNT ; ++i )
	{
		msg.nBankerCard[i] = m_vBankerCard[i].GetCardCompositeNum();
		msg.nIdleCard[i] = m_vIdleCard[i].GetCardCompositeNum();
	}
	msg.nCurPeesCount = GetRoomPeerCount();
	msg.nLeftCardCount = m_tPoker.GetLeftCard();
	msg.nMaxPeerCount = GetMaxSeat();
	msg.nMinCoinNeedToEnter = GetAntesCoin();
	msg.nRoomType = GetRoomType();
	memcpy(msg.vBetPort,m_vBetPort,sizeof(m_vBetPort) );
	memcpy(msg.vRecorder,m_vRecorder,sizeof(bool) * eBC_BetPort_Max * MAX_BACCARAT_RECORD );
	pPeer->SendMsgToClient((char*)&msg,sizeof(msg)) ;
	return true ;
}

void CBaccaratRoom::GoToState(eRoomState eState )
{
	m_fTimeTicket = 0 ;
	eRoomState ePreState = m_eRoomState ;
	m_eRoomState = eState ;
	switch( eState )
	{
	case eRoomState_BC_WaitBet:
		{
			OnGameEnd();
			stMsgBCStartBet msg ;
			SendMsgRoomPeers(&msg,sizeof(msg)) ;
			if ( GetRoomPeerCount() < MIN_PEERS_IN_ROOM_ROBOT )
			{
				CRobotManager::SharedRobotMgr()->RequestRobotToJoin(this) ;
			}
		}
		break;
	case eRoomState_BC_Shuffle:
		{
			// send message to tell client ;
			stMsgBCShuffle msg ;
			SendMsgRoomPeers(&msg,sizeof(msg)) ;
			m_tPoker.RestAllPoker();
		}
		break;
	case eRoomState_BC_Distribute:
		{
			stMsgBCDistribute msgDistribute ;
			for ( int i = 0 ; i < 2 ; ++i )
			{
				msgDistribute.vBanker[i] = m_tPoker.GetCardWithCompositeNum() ;
				msgDistribute.vIdle[i] = m_tPoker.GetCardWithCompositeNum() ;
				m_vIdleCard[i].RsetCardByCompositeNum(msgDistribute.vIdle[i]) ;
				m_vBankerCard[i].RsetCardByCompositeNum(msgDistribute.vBanker[i]) ;
			}
			// send message 
			SendMsgRoomPeers(&msgDistribute,sizeof(msgDistribute)) ;
		}
		break;
	case eRoomState_BC_AddIdleCard:
		{
			unsigned char nPoint = GetPeerPoint(false);
			if ( nPoint <= 5 )
			{
				// add a card 
				m_vIdleCard[2].RsetCardByCompositeNum(m_tPoker.GetCardWithCompositeNum()) ;
				// send msg to tell client ;
				stMsgBCAddCard msgAddCard ;
				msgAddCard.bIdle = true ;
				msgAddCard.nCard = m_vIdleCard[2].GetCardFaceNum() ;
				SendMsgRoomPeers(&msgAddCard,sizeof(msgAddCard)) ;
			}
			else
			{
				GoToState(eRoomState_BC_AddBankerCard);
				return ;
			}
		}
		break;
	case eRoomState_BC_AddBankerCard:
		{
			unsigned char nPoint = GetPeerPoint(false);
			if ( nPoint <= 2 ) // must add
			{

			}
			else
			{
				if ( m_vIdleCard[2].GetType() == CCard::eCard_Joker ) // idle player just 2 card ,must not need card 
				{
					GoToState(eRoomState_BC_Caculate);
					return ;
				}

				unsigned char nIdle3Point = m_vIdleCard[2].GetCardFaceNum() ;
				if ( nPoint == 3 && nIdle3Point == 8  )  // no need add card 
				{
					GoToState(eRoomState_BC_Caculate);
					return ;
				}
				
				if ( 4 == nPoint && ( 0 == nIdle3Point || 1 == nIdle3Point || 8 == nIdle3Point || 9 == nIdle3Point ) )
				{
					GoToState(eRoomState_BC_Caculate);
					return ;
				}

				if ( 5 == nPoint && ( nIdle3Point <= 3 ||8 == nIdle3Point || 9 == nIdle3Point  ) )
				{
					GoToState(eRoomState_BC_Caculate);
					return ;
				}
				
				if ( 6 == nPoint && ( nIdle3Point <= 5 ||8 == nIdle3Point || 9 == nIdle3Point  ) )
				{
					GoToState(eRoomState_BC_Caculate);
					return ;
				}

				if ( nPoint >= 7 )
				{
					GoToState(eRoomState_BC_Caculate);
					return ;
				}
			}

			// add card 
			m_vBankerCard[2].RsetCardByCompositeNum(m_tPoker.GetCardWithCompositeNum()) ;
			// send message to client ;
			stMsgBCAddCard msgAddCard ;
			msgAddCard.bIdle = false ;
			msgAddCard.nCard = m_vBankerCard[2].GetCardFaceNum() ;
			SendMsgRoomPeers(&msgAddCard,sizeof(msgAddCard)) ;
			return ;
		}
		break;
	case eRoomState_BC_Caculate:
		{
			unsigned char nIdlePoint = GetPeerPoint(false) ;
			unsigned char nBankerPoint = GetPeerPoint(true) ;
			unsigned char nIdx = m_nRound;
			if ( nIdx >= MAX_BACCARAT_RECORD )
			{
				nIdx = MAX_BACCARAT_RECORD - 1 ;
			}
			m_vRecorder[eBC_BetPort_TheSame][nIdx] = nIdlePoint == nBankerPoint ;
			m_vRecorder[eBC_BetPort_BankerWin][nIdx] = nIdlePoint < nBankerPoint ;
			m_vRecorder[eBC_BetPort_IdleWin][nIdx] = nIdlePoint > nBankerPoint ;

			// make result arrary ;
			bool bResult[eBC_BetPort_Max] ;
			for ( int i = 0 ; i < eBC_BetPort_Max; ++i )
			{
				bResult[i] = m_vRecorder[i][nIdx] ;
			}

			for ( int i = 0 ; i < GetMaxSeat() ; ++i )
			{
				if ( m_vRoomPeer[i] )
				{
					 CBaccaratPeer* peer = (CBaccaratPeer*)m_vRoomPeer[i] ;
					 peer->OnSettlement(bResult) ;
				}
			}

			// send msessage to tell client cal result ;
		}
		break;
	default:
		CLogMgr::SharedLogMgr()->ErrorLog("go to unknown room state Baccarat  = %d",eState) ;
		break;
	}
}

unsigned char CBaccaratRoom::GetPeerPoint(bool bBanker )
{
	CCard* pCard = NULL ;
	pCard = bBanker ? m_vBankerCard : m_vIdleCard;

	unsigned char nPoint = 0 ;
	for ( int i = 0 ; i < MAX_BACCART_CARD_COUNT ; ++i )
	{
		if ( pCard[i].GetType() == CCard::eCard_Joker )
		{
			continue;
		}

		if ( pCard[i].GetCardFaceNum() >= 10 )
		{
			continue;
		}

		nPoint += pCard[i].GetCardFaceNum() ;
	}
	return nPoint ;
}

void CBaccaratRoom::OnGameEnd()
{
	for ( int i = 0 ; i < GetMaxSeat() ; ++i )
	{
		if ( m_vRoomPeer[i] )
		{
			m_vRoomPeer[i]->OnGameEnd() ;
		}
	}

	++m_nRound;
	m_fTimeTicket = 0 ;
	memset(m_vBetPort,0,sizeof(m_vBetPort)) ;
	
	for ( int i = 0 ; i < MAX_BACCART_CARD_COUNT ; ++i )
	{
		m_vIdleCard[i].SetCard(CCard::eCard_Joker,53) ;
		m_vBankerCard[i].SetCard(CCard::eCard_Joker,53) ;
	}
}