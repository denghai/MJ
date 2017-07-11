#include "RoomTexasPoker.h"
#include "TaxasPokerPeer.h"
#include "LogManager.h"
#include "TaxasPokerMessage.h"
#include <assert.h>
#include "Player.h"
#include "RobotManager.h"
#include <vector>
#include <algorithm>
#define MIN_CHOU_SHUI_LIMIT 5000
#define min(a,b)    (((a) < (b)) ? (a) : (b))
CRoomTexasPoker::CRoomTexasPoker()
{
	m_eRoomType = eRoom_TexasPoker ;
	m_fTick = 0 ;
	m_nBankerIdx = -1 ;
	m_nBetRound = 0 ;
	m_nMostBetCoinThisRound = 0 ;
	m_nCurMainBetPool = 0 ;
	m_eRoomState = eRoomState_TP_WaitJoin ;
	m_nCurWaitPlayerActionIdx = 0 ;
	m_nBigBlindIdx = 0 ;
	m_nLittleBlindIdx = 0 ;
	m_eCurPeerAction = eRoomPeerAction_None ;
	m_fCaluateGameResultTime = 0 ;
	m_fCaluateRoundTime = 0 ;
	memset(m_vPublicCardNums,0,sizeof(m_vPublicCardNums));
}

CRoomTexasPoker::~CRoomTexasPoker()
{
	m_vAudiencers.clear();
	LIST_BET_POOLS::iterator iter = m_vAllBetPools.begin() ;
	for ( ; iter != m_vAllBetPools.end(); ++iter )
	{
		delete *iter ;
	}
	m_vAllBetPools.clear() ;
}

//#include "TaxasPokerPeerCard.h"
void CRoomTexasPoker::Init( unsigned int nRoomID , unsigned char nMaxSeat )
{
	CRoomBase::Init(nRoomID,nMaxSeat) ;
	m_vAudiencers.clear();
	m_eRoomState = eRoomState_TP_WaitJoin ;
	m_ptPoker.InitTaxasPoker() ;
	m_nBigBlindBet = 100 ;
	// ----test begin-----
	//int nCount = 10 ;
	//CTaxasPokerPeerCard ptCard ;
	//CLogMgr::SharedLogMgr()->ErrorLog("begin my test \n") ;
	//while ( nCount -- )
	//{
	//	ptCard.Reset();
	//	CLogMgr::SharedLogMgr()->SystemLog("a peer card ...") ;
	//	m_ptPoker.ComfirmKeepCard(7) ;
	//	ptCard.AddCardByCompsiteNum(m_ptPoker.GetCardWithCompositeNum()) ;
	//	ptCard.AddCardByCompsiteNum(m_ptPoker.GetCardWithCompositeNum()) ;
	//	ptCard.AddCardByCompsiteNum(m_ptPoker.GetCardWithCompositeNum()) ;
	//	ptCard.AddCardByCompsiteNum(m_ptPoker.GetCardWithCompositeNum()) ;
	//	ptCard.AddCardByCompsiteNum(m_ptPoker.GetCardWithCompositeNum()) ;
	//	ptCard.AddCardByCompsiteNum(m_ptPoker.GetCardWithCompositeNum()) ;
	//	ptCard.AddCardByCompsiteNum(m_ptPoker.GetCardWithCompositeNum()) ;
	//	//ptCard.AddCardByCompsiteNum(28) ;
	//	//ptCard.AddCardByCompsiteNum(2) ;
	//	//ptCard.AddCardByCompsiteNum(19) ;
	//	//ptCard.AddCardByCompsiteNum(35) ;
	//	//ptCard.AddCardByCompsiteNum(36) ;
	//	//ptCard.AddCardByCompsiteNum(11) ;
	//	//ptCard.AddCardByCompsiteNum(37) ;
	//	ptCard.GetCardType();
	//	ptCard.LogInfo() ;
	//}
	//-----test end----
}

void CRoomTexasPoker::OnPeerLeave( CRoomPeer* peer )
{
	if ( peer->GetPeerIdxInRoom() < GetMaxSeat() )
	{
		//stand up 
		stMsgTaxasPokerPeerAction msg ;
		msg.ePeerAct = eRoomPeerAction_StandUp ;
		peer->OnMessage( &msg ) ;
	}

	LIST_PEERS::iterator iter = m_vAudiencers.find(peer) ;
	if ( iter != m_vAudiencers.end() )
	{
		m_vAudiencers.erase(iter) ;
	}
	else
	{
		CLogMgr::SharedLogMgr()->ErrorLog("not in peer leave room !") ;
	}
#ifndef NDEBUG
	for ( int i = 0 ; i < GetMaxSeat(); ++i )
	{
		if ( m_vRoomPeer[i] == peer )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("Super Error , Error !!! why  !!! ") ;
			m_vRoomPeer[i] = NULL ;
			return ;
		}
	}
#endif
}

unsigned char CRoomTexasPoker::CanJoin(CPlayer* peer )
{
	if ( IsInNotAllowedPeerList(peer->GetUserUID()) )
	{
		return 3 ;
	}
	return 0 ;
}

void CRoomTexasPoker::AddPeer(CRoomPeer* peer )
{
	CTaxasPokerPeer* pTPPeer = dynamic_cast<CTaxasPokerPeer*>(peer);
	peer->SetIsInDiamonedRoom(IsDiamonedRoom()) ;
	if ( pTPPeer == NULL )
	{
		assert(0 && "add not proper peer to taxas poker room " );
		CLogMgr::SharedLogMgr()->ErrorLog( "add not proper peer to taxas poker room " ) ;
		return ;
	}
	m_vAudiencers.insert(peer) ;
	//CLogMgr::SharedLogMgr()->ErrorLog("push audences %s  add peer",peer->GetPlayer()->GetBaseData()->strName);
	peer->SetIdxInRoom(GetMaxSeat()) ;
	peer->SetState(eRoomPeer_StandUp);
	peer->GetPlayer()->SetState(CPlayer::ePlayerState_InRoom); 
}

bool CRoomTexasPoker::AddBeInvitedPlayer(CPlayer* peer,unsigned char nRoomIdx )
{
	// find a proper idx ;
	int nTempIdx = -1 ;
	for ( int i = nRoomIdx ; i < GetMaxSeat() * 2 ; ++i )
	{
		int nIdx = i % GetMaxSeat() ;
		if ( m_vRoomPeer[nIdx] == NULL )
		{
			nTempIdx =  nIdx;
			break; 
		}
	}

	if ( nTempIdx == -1 )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("how , why can not find a proper empty seat for invited player ?") ;
		return false ;
	}

	CTaxasPokerPeer* pTPPeer = (CTaxasPokerPeer*)peer->GetComponent(ePlayerComponent_RoomPeerTaxasPoker);
	// first add room, walk normal player Enter room  process ;
	AddPeer(pTPPeer) ;
	pTPPeer->SetRoom(this) ;
	SendCurRoomInfoToPlayer(pTPPeer) ;

	// auto sit down ;
	stMsgTaxasPokerPeerAction msgAutoSitDown ;
	msgAutoSitDown.ePeerAct = eRoomPeerAction_SitDown ;
	msgAutoSitDown.nActionValue = nTempIdx ;
	pTPPeer->AutoTakein();
	msgAutoSitDown.nTakeInCoin = pTPPeer->GetCoin();
	OnPeerMsg(pTPPeer,&msgAutoSitDown) ;
	return true ;
}

bool CRoomTexasPoker::CanPeerSitDown(CTaxasPokerPeer* peer )
{
	if ( CanJoin(peer->GetPlayer()) != 0 )
	{
		return false ;
	}

	if ( IsDiamonedRoom() && peer->GetPlayerBaseData()->GetAllDiamoned() < GetAntesCoin() )
	{
		return false ;
	}

	if ( IsDiamonedRoom() == false && peer->GetPlayerBaseData()->GetAllCoin() < GetAntesCoin() )
	{
		return false ;
	}

	if ( GetEmptySeatCount() <= 0 )
	{
		return false ;
	}

	return true ;
}

void CRoomTexasPoker::Update(float fTimeElpas, unsigned int nTimerID )
{
	m_fTick += fTimeElpas ;
	switch ( GetRoomState() )
	{
	case eRoomState_TP_WaitJoin:
		{
			if ( GetRoomPeerCount() >= 2 )
			{
				GoToState(eRoomState_TP_Player_Distr);
			}
		}
		break;
	case eRoomState_TP_Player_Distr:
		{
			if ( m_fTick >= TIME_TAXAS_DISTRIBUTE_PER_PEER/* * GetCanActPeerCount()*/ )  //modify as yi hua ;
			{
				GoToState(eRoomState_TP_Wait_Bet);
			}
		}
		break;
	case eRoomState_TP_Wait_Bet:
		{
			if ( m_fTick >= GetWaitOperateTime() )
			{
				m_fTick = 0 ;
				// inform peer bet timeout ;
				CTaxasPokerPeer* peer = (CTaxasPokerPeer*)m_vRoomPeer[m_nCurWaitPlayerActionIdx] ;
				if ( peer )
				{
					peer->OnWaitActionTimeOut();
				}
				else
				{
					CLogMgr::SharedLogMgr()->ErrorLog("why wait a empty player ?") ;
				}
			}
		}
		break;
	case eRoomState_TP_Caculate_Round:
		{
			if ( m_fTick >= m_fCaluateRoundTime )
			{
				if ( CheckGameEnd() )
				{
					GoToState(eRoomState_TP_Caculate_GameResult) ;
				}
				else
				{
					GoToState( eRoomState_TP_Distr_Public );
				}
			}
		}
		break;
	case eRoomState_TP_Distr_Public:
		{
			unsigned char nCard = m_nBetRound == 1 ? 3 : 1 ;
			if ( m_fTick >= TIME_TAXAS_ONE_CARD_DISTRIBUTE * nCard )
			{
				GoToState(eRoomState_TP_Wait_Bet);
			}
		}
		break;
	case eRoomState_TP_Caculate_GameResult:
		{
			if ( m_fTick < m_fCaluateGameResultTime )
			{
				break ;
			}
			m_fTick = 0 ;
			OnGameEnd();
			if ( GetRoomPeerCount() < 2 )
			{
				GoToState(eRoomState_TP_WaitJoin);
			}
			else
			{
				GoToState(eRoomState_TP_Player_Distr);
			}
		}
		break;
	default:
		{
			CLogMgr::SharedLogMgr()->ErrorLog("unknown current room state = %d",GetRoomState()) ;
		}
		break;
	}
}

bool CRoomTexasPoker::IsBiggestCardPeer(CTaxasPokerPeer* pTestPlayer)
{
	if ( pTestPlayer->IsInState(eRoomPeer_CanAct) == false )
	{
		return false ;
	}

	for ( int i = 0 ; i < GetMaxSeat() ; ++i )
	{
		if ( m_vRoomPeer[i] == NULL )
		{
			continue;
		}

		CTaxasPokerPeer* pTemp = (CTaxasPokerPeer*)m_vRoomPeer[i] ;
		if ( pTemp->IsInState(eRoomPeer_WaitCaculate) == false )
		{
			continue; 
		}

		if ( pTestPlayer->PK(pTemp) == -1 )
		{
			return false ;
		}
	}
	return true ;
}

bool CRoomTexasPoker::SortBetPeers(CTaxasPokerPeer*pLeft,CTaxasPokerPeer*pRight)
{
	if ( pLeft->GetBetCoinThisRound() < pRight->GetBetCoinThisRound() )
		return true ;
	return false ;
}

bool CRoomTexasPoker::OnPeerMsg(CRoomPeer* pPeer, stMsg* pmsg )
{
	if ( CRoomBase::OnPeerMsg(pPeer,pmsg) )
	{
		return true ;
	}
	CTaxasPokerPeer* pTPPeer = (CTaxasPokerPeer*)pPeer ;
	bool bTurnAction = true ;
	unsigned char nActPeerIdx = pTPPeer->GetPeerIdxInRoom() ;
	switch ( pmsg->usMsgType )
	{
	case MSG_ROBOT_CHECK_BIGGIEST:
		{
			stMsgRobotCheckBiggestRet msgRet ;
			msgRet.nRet = IsBiggestCardPeer(pTPPeer) ? 1 : 0 ;
			pTPPeer->SendMsgToClient((char*)&msgRet,sizeof(msgRet)) ;
			return true ;
		}
		break;
	case MSG_TP_PEER_ACTION:
		{
			stMsgTaxasPokerPeerAction* pMsgReal = (stMsgTaxasPokerPeerAction*)pmsg ;
			stMsgTaxasPokerPeerActionRet msgBack ;
			msgBack.ePeerAct = pMsgReal->ePeerAct ;
			if ( pPeer->GetPeerIdxInRoom() != m_nCurWaitPlayerActionIdx && pMsgReal->ePeerAct != eRoomPeerAction_SitDown && pMsgReal->ePeerAct != eRoomPeerAction_StandUp )
			{
				msgBack.nRet = eTPActRet_NotYourTurn;
				pPeer->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
				return true ;
			}

			if ( pTPPeer->IsInState(eRoomPeer_CanAct) == false && pMsgReal->ePeerAct != eRoomPeerAction_SitDown && pMsgReal->ePeerAct != eRoomPeerAction_StandUp  )
			{
				msgBack.nRet = eTPActRet_StateError ;
				pPeer->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
				return true ;
			}

			if ( GetRoomState() != eRoomState_TP_Wait_Bet && pMsgReal->ePeerAct != eRoomPeerAction_SitDown && pMsgReal->ePeerAct != eRoomPeerAction_StandUp  )
			{
				msgBack.nRet = eTPActRet_RoomStateError ;
				pPeer->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
				return true ;
			}

			switch ( pMsgReal->ePeerAct )
			{
			case eRoomPeerAction_SitDown:
				{
					if ( pMsgReal->nActionValue >= GetMaxSeat() || m_vRoomPeer[pMsgReal->nActionValue] )
					{
						msgBack.nRet = eTPActRet_PosNotEmpty ;
						pPeer->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
						return true;
					}
					else if ( pTPPeer->IsInState(eRoomPeer_SitDown) )
					{
						msgBack.nRet = eTPActRet_Already_SitDown ;
						pPeer->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
						return true;
					}
					else if ( pMsgReal->nTakeInCoin > pTPPeer->GetPlayerBaseData()->GetAllCoin() )
					{
						msgBack.nRet = eTPActRet_NotEnoughCoin ;
						pPeer->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
						return true;
					}
					else if ( (pMsgReal->nTakeInCoin > GetMaxTakeInCoin() || pMsgReal->nTakeInCoin < GetAntesCoin() ) )
					{
						
						msgBack.nRet = eTPActRet_TakeInCoinError ;
						pPeer->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
						return true;
					}
					pTPPeer->SetState(eRoomPeer_WaitNextGame) ;
					m_vRoomPeer[(unsigned char)pMsgReal->nActionValue] = pTPPeer ;
					pTPPeer->SetIdxInRoom((unsigned char)pMsgReal->nActionValue);
					pTPPeer->SetTakeInCoin(pMsgReal->nTakeInCoin,IsDiamonedRoom()) ;
					// remove from audiences 
					LIST_PEERS::iterator iterAud = m_vAudiencers.find(pPeer) ;
					if ( iterAud != m_vAudiencers.end() )
					{
						m_vAudiencers.erase(iterAud) ;
					}
					else
					{
						CLogMgr::SharedLogMgr()->ErrorLog("pre not in audience , how to sit down ?") ;
					}

					stMsgTaxasPokerOtherPeerSitDown msgPlayerSitDown ;
					msgPlayerSitDown.nActionValue = 0 ;
					msgPlayerSitDown.tPlayerInfo.eCurState = pTPPeer->GetStateValue() ;
					msgPlayerSitDown.tPlayerInfo.nCurBetCoin = 0 ;
					pTPPeer->GetPlayerBaseData()->GetPlayerBrifData(&msgPlayerSitDown.tPlayerInfo) ;
					msgPlayerSitDown.tPlayerInfo.nTakeInCoin = pTPPeer->GetCoin();
					msgPlayerSitDown.tPlayerInfo.nIdxInRoom = pTPPeer->GetPeerIdxInRoom() ;
					msgPlayerSitDown.tPlayerInfo.nSessionID = pTPPeer->GetSessionID() ;
					memset(msgPlayerSitDown.tPlayerInfo.vCards,0,sizeof(msgPlayerSitDown.tPlayerInfo.vCards));
					SendMsgRoomPeers(&msgPlayerSitDown,sizeof(msgPlayerSitDown)) ;
					if ( GetRoomState() == eRoomState_TP_WaitJoin && GetRoomPeerCount() >= 2 )
					{
						GoToState(eRoomState_TP_Player_Distr) ;
					}
					return true;
				}
				break;
			case eRoomPeerAction_StandUp:
				{
					if ( pTPPeer->IsInState(eRoomPeer_StandUp) )
					{
						msgBack.nRet = eTPActRet_Alread_StandUp ;
						pPeer->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
						return true;
					}
					OnPeerStandUp(pTPPeer) ;
					bTurnAction = false ;
				}
				break;
			case eRoomPeerAction_GiveUp:
				{
					pTPPeer->SetState(eRoomPeer_GiveUp) ;
					//m_vCurRoundTempBetPool.push_back(pTPPeer->GetBetCoinThisRound());
					m_nCurMainBetPool += pTPPeer->GetBetCoinThisRound() ;
					RemovePeerFromBetPool(pTPPeer);
					pTPPeer->DecreasBetCoinThisRound(pTPPeer->GetBetCoinThisRound());
				}
				break; 
			case eRoomPeerAction_Follow:
				{
					uint64_t nOffset = m_nMostBetCoinThisRound - pTPPeer->GetBetCoinThisRound() ;
					if ( nOffset >= pTPPeer->GetCoin() )
					{
						msgBack.nRet = eTPActRet_NotEnoughCoin ;
						pPeer->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
						return true ;
					}
					pTPPeer->AddBetCoinThisRound(nOffset) ;
					//pTPPeer->SetMyAction(eRoomPeerAction_Follow) ;
					//m_eCurPeerAction = eRoomPeerAction_Follow ;
					pMsgReal->nActionValue = pTPPeer->GetBetCoinThisRound() ;
				}
				break;
			case eRoomPeerAction_Add:
				{
					uint64_t nOffset = m_nMostBetCoinThisRound - pTPPeer->GetBetCoinThisRound() ;
					if ( pMsgReal->nActionValue < m_nBigBlindBet )
					{
						msgBack.nRet = eTPActRet_BetAddNeedBigerThanBlindBet ;
						pPeer->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
						return true ;
					}
					else if ( nOffset > 0  && (pMsgReal->nActionValue < 2*nOffset || (pMsgReal->nActionValue - 2*nOffset) % m_nBigBlindBet != 0 ))
					{
						msgBack.nRet = eTPActRet_BetAddMustTimesOffset ;
						pPeer->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
						return true ;
					}
					
					//pTPPeer->SetMyAction(eRoomPeerAction_Add);
					pTPPeer->AddBetCoinThisRound(pMsgReal->nActionValue) ;
					pMsgReal->nActionValue = pTPPeer->GetBetCoinThisRound() ;
					m_nMostBetCoinThisRound = pTPPeer->GetBetCoinThisRound() ;
					//m_eCurPeerAction = eRoomPeerAction_Add ;
				}
				break;
			case eRoomPeerAction_Pass:
				{

				}
				break;
			case eRoomPeerAction_AllIn:
				{
					pTPPeer->SetState(eRoomPeer_AllIn) ;
					//pTPPeer->SetMyAction(eRoomPeerAction_AllIn);
					//m_eCurPeerAction = eRoomPeerAction_Pass ; 
					uint64_t nAllInCoin = GetAllowMaxAllInCoin();
					nAllInCoin = pTPPeer->GetCoin() < nAllInCoin ? pTPPeer->GetCoin() : nAllInCoin ;
					pTPPeer->AddBetCoinThisRound(nAllInCoin) ;
					CLogMgr::SharedLogMgr()->PrintLog("All In Left = %I64d",pTPPeer->GetCoin()) ;
					m_nMostBetCoinThisRound = m_nMostBetCoinThisRound > pTPPeer->GetBetCoinThisRound() ? m_nMostBetCoinThisRound : pTPPeer->GetBetCoinThisRound() ;
					pMsgReal->nActionValue = pTPPeer->GetBetCoinThisRound() ;
				}
				break;
			default:
				{
					CLogMgr::SharedLogMgr()->ErrorLog( "unknown TaxasPoker peer action ! %d",pMsgReal->ePeerAct ) ;
					return false ;
				}
			}

			pTPPeer->SetMyAction((eRoomPeerAction)pMsgReal->ePeerAct) ;
			m_eCurPeerAction = (eRoomPeerAction)pMsgReal->ePeerAct ; 

			stMsgTaxasPokerOtherPeerAction msgToOther ;
			msgToOther.ePeerAct = pMsgReal->ePeerAct ;
			msgToOther.nActionValue = pMsgReal->nActionValue ;
			msgToOther.nPlayerIdxInRoom = nActPeerIdx ;
			SendMsgRoomPeers(&msgToOther,sizeof(msgToOther)) ;

			// process kick ;
			ProcessKickedPeers();

			if ( GetRoomState() == eRoomState_TP_Wait_Bet && CheckRoundEnd() )
			{
				GoToState(eRoomState_TP_Caculate_Round) ;
			}
			else if ( nActPeerIdx == m_nCurWaitPlayerActionIdx && GetRoomState() == eRoomState_TP_Wait_Bet )
			{
				// next peer action ;
				++m_nCurWaitPlayerActionIdx ;
				WaitActivePeerBet() ;
			}
		}
		return true;
	default: 
		return false ;
	}
	return false ;
}

bool CRoomTexasPoker::SendCurRoomInfoToPlayer(CRoomPeer* pPeer )
{
	stMsgTaxasPokerRoomInfo msg ;
	msg.eCurRoomState = GetRoomState();
	msg.fTick = m_fTick ;
	msg.nCurPlayerCount = GetRoomPeerCount();
	msg.nMainBetPool = m_nCurMainBetPool ;
	msg.nMaxSeat = GetMaxSeat() ;
	msg.nViceBetPoolCnt = m_vAllBetPools.size() ;
	msg.nBankerIdx = m_nBankerIdx ;
	msg.nBigBlindBetCoin = m_nBigBlindBet ;
	msg.nBigBlindIdx = m_nBigBlindIdx ;
	msg.nLittleBlindIdx = m_nLittleBlindIdx ;
	msg.nWaitActionPeerIdx = m_nCurWaitPlayerActionIdx;
	msg.nRoomLevel = GetRoomLevel();
	msg.nRoomID = GetRoomID();
	msg.nMaxTakeIn = GetMaxTakeInCoin();
	msg.nMinTakeIn = GetAntesCoin();
	msg.nCurBetRound = m_nBetRound;
	memcpy(msg.vCardsNum,m_vPublicCardNums,sizeof(msg.vCardsNum)) ;
	static char pBuffer [2048] ; //= new char[sizeof(msg) + msg.nCurPlayerCount * sizeof(stTPRoomPeerBrifInfo) + sizeof(msg.nViceBetPoolCnt) * sizeof(uint64_t)] ;
	// vice pool 
	unsigned short nOffset = 0 ;
	memcpy(pBuffer,&msg, sizeof(msg));
	nOffset += sizeof(msg);
	LIST_BET_POOLS::iterator iter = m_vAllBetPools.begin() ;
	stVicePool stVice ;
	for ( ; iter != m_vAllBetPools.end(); ++iter )
	{ 
		stVice.nIdx = (*iter)->nIdx;
		stVice.nViceCoin = (*iter)->nBetCoinInThisPool ;
		memcpy(pBuffer + nOffset , &stVice,sizeof(stVice));
		nOffset += sizeof(stVice);
	}

	// player count ;
	for ( int i = 0  ; i < GetMaxSeat() ; ++i )
	{
		if ( m_vRoomPeer[i] == NULL )
		{
			continue; 
		}
		stTPRoomPeerBrifInfo stInfo ;
		m_vRoomPeer[i]->GetPlayerBaseData()->GetPlayerBrifData(&stInfo) ;
		stInfo.eCurState = m_vRoomPeer[i]->GetStateValue() ;
		stInfo.nTakeInCoin = m_vRoomPeer[i]->GetCoin();
		stInfo.nCurBetCoin = ((CTaxasPokerPeer*)m_vRoomPeer[i])->GetBetCoinThisRound();
		stInfo.nIdxInRoom = m_vRoomPeer[i]->GetPeerIdxInRoom();
		stInfo.nSessionID = m_vRoomPeer[i]->GetSessionID() ;
		stInfo.vCards[0] = ((CTaxasPokerPeer*)m_vRoomPeer[i])->GetPeerCard().GetPrivateCard(0) ;
		stInfo.vCards[1] = ((CTaxasPokerPeer*)m_vRoomPeer[i])->GetPeerCard().GetPrivateCard(1) ;
		memcpy(pBuffer + nOffset , &stInfo,sizeof(stInfo) );
		nOffset += sizeof(stInfo);
	}
	pPeer->SendMsgToClient(pBuffer,nOffset) ;
	return true ;
}

void CRoomTexasPoker::SendMsgRoomPeers(stMsg*pMsg ,unsigned short nLen ,CRoomPeer* pExcpetPeer)
{
	CRoomBase::SendMsgRoomPeers(pMsg,nLen,pExcpetPeer) ;
	LIST_PEERS::iterator iter = m_vAudiencers.begin() ;
	for ( ; iter != m_vAudiencers.end() ; ++iter)
	{
		if ( *iter && (*iter) != pExcpetPeer )
		{
			(*iter)->SendMsgToClient((char*)pMsg,nLen) ;
		}
	}
}

void CRoomTexasPoker::GoToState(eRoomState eTargetSate )
{
	m_fTick = 0 ;
	switch ( eTargetSate )
	{
	case eRoomState_TP_WaitJoin:
		{
			CLogMgr::SharedLogMgr()->PrintLog("Enter wait player join state") ;

			//CRobotManager::SharedRobotMgr()->RequestRobotToJoin(this) ;
		}
		break;
	case eRoomState_TP_Player_Distr:
		{
			OnStartNewGame();
			// decaide banker ;
			if ( m_nBankerIdx < 0 )
			{
				m_nBankerIdx = rand() % GetMaxSeat();
			}
			CRoomPeer* peer = GetFirstActivePeerFromIdx(++m_nBankerIdx);
			m_nBankerIdx = peer->GetPeerIdxInRoom();
			
			// decide blind idx ;
			CRoomPeer* pLittleBlind = GetFirstActivePeerFromIdx(m_nBankerIdx + 1 ) ;
			CRoomPeer* pBigBlind = GetFirstActivePeerFromIdx(pLittleBlind->GetPeerIdxInRoom() + 1 ) ;
			if ( pLittleBlind == NULL || pBigBlind == NULL )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("No pBigBlind and pLittleBlind ") ;
				m_eRoomState = eRoomState_TP_WaitJoin ;
				return ;
			}
			m_nLittleBlindIdx = pLittleBlind->GetPeerIdxInRoom() ;
			m_nBigBlindIdx = pBigBlind->GetPeerIdxInRoom() ;
			// blind bet first ;
			((CTaxasPokerPeer*)pLittleBlind)->AddBetCoinThisRound(m_nBigBlindBet / 2 ) ;
			((CTaxasPokerPeer*)pBigBlind)->AddBetCoinThisRound(m_nBigBlindBet ) ;
			// disribute card ;
			m_ptPoker.ComfirmKeepCard(GetRoomPeerCount() * 2 + 5 ) ;
			unsigned char nCount = 2 ;
			while ( nCount--)
			{
				for ( unsigned char i = 0; i < GetMaxSeat(); ++i )
				{
					if ( m_vRoomPeer[i] != NULL )
					{
						CTaxasPokerPeer* peer = (CTaxasPokerPeer*)m_vRoomPeer[i];
						peer->GetPeerCard().AddCardByCompsiteNum(m_ptPoker.GetCardWithCompositeNum()) ;
					}
				}
			}

#ifndef NDEBUG
			for ( unsigned char i = 0; i < GetMaxSeat(); ++i )
			{
				if ( m_vRoomPeer[i] != NULL )
				{
					CTaxasPokerPeer* peer = (CTaxasPokerPeer*)m_vRoomPeer[i];
					CLogMgr::SharedLogMgr()->SystemLog("si ren pai of %s",peer->GetPlayerBaseData()->GetPlayerName());
					peer->GetPeerCard().LogInfo();
				}
			}
#endif

			// distribute public card 
			for ( int i = 0 ; i < 5 ; ++i )
			{
				m_vPublicCardNums[i] = m_ptPoker.GetCardWithCompositeNum();
			}

			for ( unsigned char i = 0; i < GetMaxSeat(); ++i )
			{
				if ( m_vRoomPeer[i] != NULL )
				{
					CTaxasPokerPeer* peer = (CTaxasPokerPeer*)m_vRoomPeer[i];
					for ( int i = 0 ; i < 5 ; ++i )
					{
						peer->GetPeerCard().AddCardByCompsiteNum(m_vPublicCardNums[i]) ;
					}
				}
			}

#ifdef _DEBUG
			for ( unsigned char i = 0; i < GetMaxSeat(); ++i )
			{
				if ( m_vRoomPeer[i] != NULL )
				{
					CTaxasPokerPeer* peer = (CTaxasPokerPeer*)m_vRoomPeer[i];
					peer->GetPeerCard().LogInfo();
				}
			}
#endif
			// send card msg to players ;
			stMsgTaxasPokerPrivateDistribute msg ;
			msg.nBankerIdx = m_nBankerIdx ;
			msg.nBigBlindIdx = m_nBigBlindIdx ;
			msg.nLittleBlindIdx = m_nLittleBlindIdx ;
			msg.nPeerCount = GetRoomPeerCount() ;
			memcpy(msg.vPublicCard,m_vPublicCardNums,sizeof(m_vPublicCardNums));
			static char pBuffer[1024];
			
			unsigned short nOffset = 0 ;
			memcpy(pBuffer,&msg,sizeof(msg));
			nOffset += sizeof(msg);

			for ( unsigned char i = 0; i < GetMaxSeat(); ++i )
			{
				if ( m_vRoomPeer[i] != NULL )
				{
					CTaxasPokerPeer* peer = (CTaxasPokerPeer*)m_vRoomPeer[i];
					stPTPeerCard peerCard ;
					peerCard.nPlayerIdx = peer->GetPeerIdxInRoom() ;
					peerCard.vCardNum[0] = peer->GetPeerCard().GetPrivateCard(0);
					peerCard.vCardNum[1] = peer->GetPeerCard().GetPrivateCard(1);
					memcpy(pBuffer + nOffset, &peerCard,sizeof(peerCard));
					nOffset += sizeof(peerCard);
				}
			}
			SendMsgRoomPeers((stMsg*)pBuffer,nOffset) ;
			CLogMgr::SharedLogMgr()->PrintLog("Enter eRoomState_TP_Player_Distr") ;
			LogRoomStateInfo();
		}
		break;
	case eRoomState_TP_Wait_Bet:
		{
			if ( m_nBetRound != 0 )
			{
				OnStartNewRound();  // the first round need not reset , it will rewite blind bet ; infact , this time does not need reset ;
			}
			++m_nBetRound ;
			if ( m_nBetRound == 1 )
			{
				m_nCurWaitPlayerActionIdx = m_nBigBlindIdx + 1 ; 
				// send blind bet msg ;
				m_nMostBetCoinThisRound = m_nBigBlindBet ;
			}
			else
			{
				m_nCurWaitPlayerActionIdx = m_nLittleBlindIdx ;
			}

			m_eRoomState = eTargetSate ;
			if ( CheckRoundEnd() )
			{
				GoToState(eRoomState_TP_Caculate_Round);
			}
			else
			{
				WaitActivePeerBet();
			}
			return ;
		}
		break;
	case eRoomState_TP_Caculate_Round:
		{
			m_eRoomState = eTargetSate ; 
			CaculateRoundEnd();
			return ;
		}
		break;
	case eRoomState_TP_Distr_Public:
		{
			//inform send 3 card to players ;
			stMsgTaxasPokerDistribute msg ;
			msg.nBetRound = m_nBetRound ;
			SendMsgRoomPeers(&msg,sizeof(msg)) ;
		}
		break;
	case eRoomState_TP_Caculate_GameResult:
		{
			m_eRoomState = eTargetSate ; 
			CaculateGameResult();
			ProcessKickedPeers();
			return ;
		}
		break;
	//case eRoomState_TP_Caculate_Final:
	//	{
	//		std::list<stTPPeerCoinFinal> vFinalPeers ;
	//		int nCount = GetMaxSeat();
	//		for ( unsigned char i = 0; i < nCount; ++i )
	//		{
	//			CTaxasPokerPeer* peerWait = (CTaxasPokerPeer*)m_vRoomPeer[i];
	//			if ( m_vRoomPeer[i] != NULL && peerWait->IsInState(eRoomPeer_StayThisRound) )
	//			{
	//				 stTPPeerCoinFinal peerFinal ;
	//				 peerFinal.bStandUp = peerWait->GetCoin() < GetAntesCoin() ;
	//				 peerFinal.nIdx = peerWait->GetPeerIdxInRoom() ;
	//				 peerFinal.nOffset = peerWait->GetCoinOffsetThisGame() ;
	//				 peerFinal.nCointLeft = peerWait->GetCoin() ;
	//				 vFinalPeers.push_back(peerFinal) ;
	//				// CLogMgr::SharedLogMgr()->PrintLog("playerid = %d, bstand up = %d , left coin = %I64d, least Coin = %d",) ;
	//			}
	//		}
	//		static char pBuffer[512] ;
	//		stMsgTaxasPokerResultFinal msgFinal ;
	//		msgFinal.nPeerCount = vFinalPeers.size() ;
	//		unsigned short nOffset = 0 ;
	//		memcpy(pBuffer,&msgFinal,sizeof(msgFinal));
	//		nOffset += sizeof(msgFinal);
	//		std::list<stTPPeerCoinFinal>::iterator iter = vFinalPeers.begin() ;
	//		for ( ; iter != vFinalPeers.end(); ++iter )
	//		{
	//			memcpy(pBuffer + nOffset , &(*iter),sizeof(stTPPeerCoinFinal));
	//			nOffset += sizeof(stTPPeerCoinFinal);
	//		}
	//		SendMsgRoomPeers((stMsg*)pBuffer,nOffset) ;

	//		// inform game over this round 
	//		for ( int i = 0 ; i < GetMaxSeat() ; ++i )
	//		{
	//			CRoomPeer* pRoomPeer = m_vRoomPeer[i] ;
	//			if ( pRoomPeer )
	//			{
	//				if ( pRoomPeer->GetCoin() < GetAntesCoin() )
	//				{
	//					pRoomPeer->SetState(eRoomPeer_StandUp) ;
	//					pRoomPeer->SetIdxInRoom(GetMaxSeat()+1);
	//					m_vRoomPeer[i] = NULL ;
	//					m_vAudiencers.insert(pRoomPeer) ;
	//					//CLogMgr::SharedLogMgr()->ErrorLog("push audences %s  Caculate_Final",pRoomPeer->GetPlayer()->GetBaseData()->strName);
	//				}
	//				pRoomPeer->OnGameEnd();
	//			}
	//		}
	//		ProcessKickedPeers();
	//		CRobotManager::SharedRobotMgr()->RequestRobotToJoin(this) ;
	//	}
	//	LogPlayersInfo();
	//	break;
	default:
		{
			CLogMgr::SharedLogMgr()->ErrorLog("Unknown Target State = %d",eTargetSate ) ;
		}
		break;
	}
	m_eRoomState = eTargetSate ; 
}

void CRoomTexasPoker::WaitActivePeerBet()
{
	if ( GetRoomState() != eRoomState_TP_Wait_Bet )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("not wait bet state ,I will not inform bet ") ;
		return ;
	}
	CRoomPeer* pp = GetFirstActivePeerFromIdx(m_nCurWaitPlayerActionIdx) ;
	if ( pp == NULL )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("can not find proper peer to inform waitAction") ;
		return ;
	}
	m_nCurWaitPlayerActionIdx = pp->GetPeerIdxInRoom();
	m_fTick = 0 ;
	stMsgTaxasPokerWaitPeerAction WaitMsg ;
	WaitMsg.ePriePlayerAction = m_eCurPeerAction;
	WaitMsg.nCurMaxBetCoin = m_nMostBetCoinThisRound ;
	WaitMsg.nWaitPlayerIdx = m_nCurWaitPlayerActionIdx ;
	WaitMsg.nWaitPlayerSessionID = pp->GetSessionID() ;
	SendMsgRoomPeers(&WaitMsg,sizeof(WaitMsg)) ;
}

unsigned char CRoomTexasPoker::GetCanActPeerCount()
{
	unsigned char nCount = 0 ;
	for ( unsigned char i = 0; i < GetMaxSeat(); ++i )
	{
		if ( m_vRoomPeer[i] != NULL && m_vRoomPeer[i]->IsInState(eRoomPeer_CanAct) )
		{
			++nCount ;
		}
	}
	return nCount ;
}

CRoomPeer* CRoomTexasPoker::GetFirstActivePeerFromIdx(unsigned char nIdx )
{
	unsigned char nFIdx = 0 ;
	for ( unsigned char i = nIdx; i < GetMaxSeat() * 2 ; ++i )
	{
		nFIdx = i % GetMaxSeat();
		if ( m_vRoomPeer[nFIdx] != NULL && m_vRoomPeer[nFIdx]->IsInState(eRoomPeer_CanAct) )
		{
			return  m_vRoomPeer[nFIdx] ;
		}
	}
	return NULL ;
}

void CRoomTexasPoker::OnStartNewGame()
{
	m_fCaluateGameResultTime = 0 ;
	m_fCaluateRoundTime = 0 ;
	m_nCurMainBetPool = 0 ;
	m_nBetRound = 0 ;
	m_nCurWaitPlayerActionIdx = 0 ;
	m_nMostBetCoinThisRound = 0 ;
	m_eCurPeerAction = eRoomPeerAction_None ;
	//m_vCurRoundTempBetPool.clear();
	memset(m_vPublicCardNums,0,sizeof(m_vPublicCardNums));
	LIST_BET_POOLS::iterator iter = m_vAllBetPools.begin() ;
	for ( ; iter != m_vAllBetPools.end(); ++iter )
	{
		delete *iter ;
	}
	m_vAllBetPools.clear() ;

	for ( unsigned char i = 0; i < GetMaxSeat(); ++i )
	{
		if ( m_vRoomPeer[i] != NULL)
		{
			CTaxasPokerPeer* peerWait = (CTaxasPokerPeer*)m_vRoomPeer[i];
			peerWait->OnStartNewGame();
		}
	}
}

void CRoomTexasPoker::OnStartNewRound()
{
	m_fCaluateRoundTime = 0 ;
	m_nMostBetCoinThisRound = 0 ;
	m_eCurPeerAction = eRoomPeerAction_None ;
	m_nCurWaitPlayerActionIdx = 0 ;
	//m_vCurRoundTempBetPool.clear();
	for ( unsigned char i = 0; i < GetMaxSeat(); ++i )
	{
		if ( m_vRoomPeer[i] != NULL)
		{
			CTaxasPokerPeer* peerWait = (CTaxasPokerPeer*)m_vRoomPeer[i];
			peerWait->OnStartNewRound();
		}
	}
}

void CRoomTexasPoker::OnGameEnd()
{
	// clean game state ;
	LIST_BET_POOLS::iterator iter = m_vAllBetPools.begin() ;
	for ( ; iter != m_vAllBetPools.end(); ++iter )
	{
		delete *iter ;
	}
	m_vAllBetPools.clear() ;
	m_nCurMainBetPool = 0 ;
	//m_vCurRoundTempBetPool.clear();
	// iterater all stay this round peer game end ;
	for ( int i = 0 ;  i < GetMaxSeat() ; ++i )
	{
		if ( m_vRoomPeer[i] == NULL )
		{
			continue;
		}

		CTaxasPokerPeer* pp = (CTaxasPokerPeer*)m_vRoomPeer[i] ;
		if ( pp->IsInState(eRoomPeer_StayThisRound) )
		{
			pp->OnGameEnd();
		}
	}

	// process kick
	ProcessKickedPeers();
	// check which peer must sit up or auto takein coin ;
	for ( int i = 0 ;  i < GetMaxSeat() ; ++i )
	{
		if ( m_vRoomPeer[i] == NULL )
		{
			continue;
		}

		CTaxasPokerPeer* pp = (CTaxasPokerPeer*)m_vRoomPeer[i] ;
		if ( pp->GetCoin() >= GetAntesCoin() )
		{
			continue;
		}
		
		stMsgTaxasPeerAutoAction msgAuto ;
		msgAuto.nSessionID = pp->GetSessionID();
		msgAuto.nIdxInRoom = pp->GetPeerIdxInRoom() ;
		if ( pp->AutoTakein() )
		{
			msgAuto.nAutoActionType = 0 ;
			msgAuto.nTakeInCoin = pp->GetCoin();
			msgAuto.nFinalCoin = pp->GetPlayerBaseData()->GetAllCoin() - pp->GetCoin();
		}
		else
		{
			msgAuto.nAutoActionType = 1 ;
			msgAuto.nTakeInCoin = pp->GetCoin();
			msgAuto.nFinalCoin = pp->GetPlayerBaseData()->GetAllCoin() - pp->GetCoin();
			OnPeerStandUp(pp);
		}
		SendMsgRoomPeers(&msgAuto,sizeof(msgAuto)) ;
	}
}

void CRoomTexasPoker::RemovePeerFromBetPool(CRoomPeer* pTPPeer )
{
	{
		LIST_BET_POOLS::iterator iter = m_vAllBetPools.begin() ;
		for ( ; iter != m_vAllBetPools.end(); ++iter )
		{
			stBetPool* pPool = *iter ;
			LIST_PEERS::iterator iterPP = pPool->vPeersInThisPool.begin() ;
			for ( ; iterPP != pPool->vPeersInThisPool.end(); ++iterPP )
			{
				if ( pTPPeer == *iterPP )
				{
					pPool->vPeersInThisPool.erase(iterPP) ;
					break; 
				}
			}
		}
	}
}

bool CRoomTexasPoker::CaculateRoundEnd()
{
	m_fCaluateRoundTime = 0 ;
	if ( m_nMostBetCoinThisRound <= 0  )
	{
		return false;
	}
	// find all in peers in this round  
	// and build mail pool
	std::vector<CTaxasPokerPeer*> vThisRoundAllInPeers ;
	for ( unsigned char i = 0; i < GetMaxSeat(); ++i )
	{
		CTaxasPokerPeer* peer = (CTaxasPokerPeer*)m_vRoomPeer[i];
		if ( peer == NULL || peer->GetBetCoinThisRound() <= 0 )
		{
			continue;
		}

		if ( peer->IsInState(eRoomPeer_AllIn) )
		{
			vThisRoundAllInPeers.push_back(peer) ;
		}
	}

	// arrage the vector
	if ( vThisRoundAllInPeers.empty() == false)
	{
		// all peer , gived up peers , act peer ; then arrage by bet coin ;
		sort(vThisRoundAllInPeers.begin(),vThisRoundAllInPeers.end(),&CRoomTexasPoker::SortBetPeers);
	}

	// build vice pool 
	LIST_BET_POOLS vNewBetPools ;
	for (unsigned int iVicPoolIdx = 0 ; iVicPoolIdx < vThisRoundAllInPeers.size(); ++iVicPoolIdx )
	{
		CTaxasPokerPeer* peer = vThisRoundAllInPeers[iVicPoolIdx] ;
		if ( peer->GetBetCoinThisRound() <= 0)
		{
			continue;
		}
		// build pool 
		stBetPool* pbetpool = new stBetPool ;
		uint64_t nLestCoinForThisPool = peer->GetBetCoinThisRound();
		pbetpool->nBetCoinInThisPool = m_nCurMainBetPool;
		m_nCurMainBetPool = 0 ;
		// find peers waitCacluatePeers that belong to this vice pool ;
		for ( int i = 0 ; i < GetMaxSeat(); ++i )
		{
			CTaxasPokerPeer* peerPool = (CTaxasPokerPeer*)m_vRoomPeer[i] ;
			if ( peerPool == NULL || peerPool->GetBetCoinThisRound() <= 0 || (peerPool->IsInState(eRoomPeer_WaitCaculate) == false) )
			{
				continue;
			}

			if ( peerPool->GetBetCoinThisRound() >= nLestCoinForThisPool )
			{
				peerPool->DecreasBetCoinThisRound(nLestCoinForThisPool);
				pbetpool->nBetCoinInThisPool += nLestCoinForThisPool ;
			}
			else
			{
				pbetpool->nBetCoinInThisPool += peerPool->GetBetCoinThisRound() ;
				peerPool->DecreasBetCoinThisRound(peerPool->GetBetCoinThisRound());
				CLogMgr::SharedLogMgr()->ErrorLog("when run to here , means a bug occured ") ;
			}
			
			pbetpool->vPeersInThisPool.insert(peerPool) ;
		}

		// put temp bet pool in this vice pool ;
		//for ( unsigned int i = 0 ; i < m_vCurRoundTempBetPool.size(); ++i )
		//{
		//	if ( m_vCurRoundTempBetPool[i] >= nLestCoinForThisPool )
		//	{
		//		m_vCurRoundTempBetPool[i] -= nLestCoinForThisPool;
		//		pbetpool->nBetCoinInThisPool += nLestCoinForThisPool ;
		//	}
		//	else
		//	{
		//		pbetpool->nBetCoinInThisPool += m_vCurRoundTempBetPool[i] ;
		//		m_vCurRoundTempBetPool[i] = 0 ;
		//	}
		//}
		vNewBetPools.push_back(pbetpool) ;
		pbetpool->nIdx = m_vAllBetPools.size();
		m_vAllBetPools.push_back(pbetpool) ;
	}

	// caculate mail pool ;
	for ( int i = 0 ; i < GetMaxSeat(); ++i )
	{
		CTaxasPokerPeer* peer = (CTaxasPokerPeer*)m_vRoomPeer[i] ;
		if ( peer == NULL || peer->IsInState(eRoomPeer_CanAct) == false )
		{
			continue;
		}
		m_nCurMainBetPool += peer->GetBetCoinThisRound();
	}

	//for ( unsigned int i = 0 ; i < m_vCurRoundTempBetPool.size(); ++i )
	//{
	//	m_nCurMainBetPool += m_vCurRoundTempBetPool[i] ;
	//}

	// send msg this round bet end ;
	stMsgTaxasPokerRoundEnd msgRoundEnd ;
	msgRoundEnd.nMainPool = m_nCurMainBetPool ;
	msgRoundEnd.nNewPoolCount = vNewBetPools.size() ;
	static char pBuffer[1024] ;
	unsigned short nOffset = 0 ;
	memcpy(pBuffer,&msgRoundEnd,sizeof(msgRoundEnd));
	nOffset += sizeof(msgRoundEnd);
	LIST_BET_POOLS::iterator iterNewAddPool = vNewBetPools.begin() ;
	stVicePool stVicePoolSend ;
	for ( ; iterNewAddPool != vNewBetPools.end(); ++iterNewAddPool )
	{
		stVicePoolSend.nViceCoin = (*iterNewAddPool)->nBetCoinInThisPool ;
		stVicePoolSend.nIdx = (*iterNewAddPool)->nIdx;
		memcpy(pBuffer + nOffset , &stVicePoolSend, sizeof(stVicePoolSend) ) ;
		nOffset += sizeof(stVicePoolSend);
	}
	SendMsgRoomPeers((stMsg*)pBuffer,nOffset) ;

	// decide caluate wait time ;
	m_fCaluateRoundTime = TIME_TAXAS_WAIT_COIN_GOTO_MAIN_POOL + TIME_TAXAS_CAL_ROUND_STAY;
	if ( vNewBetPools.size() >= 1 )
	{
		m_fCaluateRoundTime += TIME_TAXAS_MAKE_VICE_POOLS ;
	}
	/////////////--------------------------------
	//CTaxasPokerPeer* pPoolOwner = NULL ;
	//CTaxasPokerPeer* peerWait = NULL ;
	//LIST_BET_POOLS vNewBetPools ;
	//while ( true )
	//{
	//	// find min && all in player 
	//	CTaxasPokerPeer* pPoolOwner = NULL ;
	//	for ( unsigned char i = 0; i < GetMaxSeat(); ++i )
	//	{
	//		peerWait = (CTaxasPokerPeer*)m_vRoomPeer[i];
	//		if ( m_vRoomPeer[i] != NULL && peerWait->GetBetCoinThisRound() > 0 && peerWait->IsInState(eRoomPeer_AllIn)  )
	//		{
	//			if ( pPoolOwner )
	//			{
	//				if ( peerWait->GetBetCoinThisRound() < pPoolOwner->GetBetCoinThisRound() )
	//				{
	//					pPoolOwner = peerWait ;
	//				}	
	//			}
	//			else
	//			{
	//				pPoolOwner = peerWait ;
	//			}
	//		}
	//	}

	//	if ( pPoolOwner == NULL )
	//	{
	//		break; ;
	//	}

	//	//pPoolOwner->SetMyAction(eRoomPeerAction_None) ;
	//	// set pools 
	//	stBetPool* pbetpool = new stBetPool ;
	//	pbetpool->nBetCoinInThisPool = m_nCurMainBetPool ;
	//	m_nCurMainBetPool = 0 ;
	//	uint64_t nMinesOffset = pPoolOwner->GetBetCoinThisRound();
	//	// put peers in this pool ;
	//	CLogMgr::SharedLogMgr()->PrintLog("Build Pool------") ;
	//	for ( unsigned char i = 0; i < GetMaxSeat(); ++i )
	//	{
	//		peerWait = (CTaxasPokerPeer*)m_vRoomPeer[i];
	//		if ( m_vRoomPeer[i] != NULL && peerWait->GetBetCoinThisRound() >= nMinesOffset && m_vRoomPeer[i]->IsInState(eRoomPeer_WaitCaculate) )
	//		{
	//			peerWait->DecreasBetCoinThisRound(nMinesOffset);
	//			pbetpool->nBetCoinInThisPool += nMinesOffset ;
	//			//if ( (  eRoomPeer_AllIn == m_vRoomPeer[i]->GetState() ||  eRoomPeer_Playing == m_vRoomPeer[i]->GetState() ) )
	//			{
	//				pbetpool->vPeersInThisPool.insert(peerWait) ;
	//			}
	//			CLogMgr::SharedLogMgr()->PrintLog("Player in Pool------%s",peerWait->GetPlayerBaseData()->GetPlayerName()) ;
	//		}
	//	}
	//	CLogMgr::SharedLogMgr()->PrintLog("Pool Coin = %I64d",pbetpool->nBetCoinInThisPool) ;
	//	if ( pbetpool->nBetCoinInThisPool == 0 )
	//	{
	//		delete pbetpool ;
	//		pbetpool = NULL ;
	//	}
	//	else
	//	{
	//		vNewBetPools.push_back(pbetpool) ;
	//		m_vAllBetPools.push_back(pbetpool) ;
	//	}
	//}

	//// all bet coin to sum bet coin ; 
	//for ( unsigned char i = 0; i < GetMaxSeat(); ++i )
	//{
	//	if ( m_vRoomPeer[i] != NULL && m_vRoomPeer[i]->IsInState(eRoomPeer_CanAct) )
	//	{
	//		peerWait = (CTaxasPokerPeer*)m_vRoomPeer[i];
	//		m_nCurMainBetPool += peerWait->GetBetCoinThisRound();
	//	}
	//}

	//LogPlayersInfo();

	//m_bWaitCoinGoToAllBetCoin = true ;  // wait client coin go to sum bet ,and make bet pool ;
	//// send msg this round bet end ;
	//stMsgTaxasPokerRoundEnd msgRoundEnd ;
	//msgRoundEnd.nMainPool = m_nCurMainBetPool ;
	//msgRoundEnd.nNewPoolCount = vNewBetPools.size() ;
	//static char pBuffer[512] ;
	//unsigned short nOffset = 0 ;
	//memcpy(pBuffer,&msgRoundEnd,sizeof(msgRoundEnd));
	//nOffset += sizeof(msgRoundEnd);
	//LIST_BET_POOLS::iterator iterNewAddPool = vNewBetPools.begin() ;
	//for ( ; iterNewAddPool != vNewBetPools.end(); ++iterNewAddPool )
	//{
	//	memcpy(pBuffer + nOffset , &(*iterNewAddPool)->nBetCoinInThisPool, sizeof(uint64_t) ) ;
	//	nOffset += sizeof(uint64_t);
	//}
	//SendMsgRoomPeers((stMsg*)pBuffer,nOffset) ;
	return true ;
}

void CRoomTexasPoker::CaculateGameResult()
{
	// default all act peer are belong to  main pool ;
	if ( m_nCurMainBetPool > 0 )
	{
		stBetPool* pMainPool = new stBetPool ;
		pMainPool->nBetCoinInThisPool = m_nCurMainBetPool ;
		pMainPool->nIdx = 100 ;
		for ( int i = 0 ; i < GetMaxSeat() ; ++i )
		{
			CTaxasPokerPeer* peer = (CTaxasPokerPeer*)m_vRoomPeer[i] ;
			if ( peer && peer->IsInState(eRoomPeer_CanAct) )
			{
				pMainPool->vPeersInThisPool.insert(peer) ;
			}
		}
		m_vAllBetPools.push_back(pMainPool) ;
	}

	// make msg to client , and give result coin to peer ;
	char cBuffer[512] = { 0 } ;
	stMsgTaxasPokerGameRessult msgResult ;
	msgResult.nResultPoolCount = m_vAllBetPools.size();
	memcpy(cBuffer,&msgResult,sizeof(msgResult));
	char* pTempBuffer = cBuffer + sizeof(msgResult);

	LIST_BET_POOLS::iterator iter = m_vAllBetPools.begin();
	for ( ; iter != m_vAllBetPools.end(); ++iter )
	{
		stBetPool* pPool = *iter ;
		if ( pPool == NULL )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("why a NULL pool") ;
			continue;
		}

		if ( pPool->vPeersInThisPool.size() == 0 )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("why a pool have no peers ") ;
			continue;
		}

		// find winner ;
		LIST_PEERS vWinners ;
		LIST_PEERS::iterator iterFindWiner = pPool->vPeersInThisPool.begin() ;
		CTaxasPokerPeer* pWiner = NULL ;
		for ( ; iterFindWiner != pPool->vPeersInThisPool.end(); ++iterFindWiner )
		{
			CTaxasPokerPeer* pTemp = (CTaxasPokerPeer*)(*iterFindWiner) ;
			if ( pTemp == NULL )
			{
				continue; 
			}

			if ( !pTemp->IsInState(eRoomPeer_WaitCaculate) )
			{
				CLogMgr::SharedLogMgr()->ErrorLog( "why a not eRoomPeer_WaitCaculate peer add to pool" );
				continue;
			}

			if ( !pWiner )
			{
				pWiner = pTemp;
				vWinners.insert(pWiner) ;
				continue ;
			}

			char nPKRet = pWiner->PK(pTemp) ;
			if ( nPKRet == 0 )
			{
				vWinners.insert(pTemp) ;
			}
			else if ( nPKRet == -1 ) // failed 
			{
				vWinners.clear() ;
				pWiner = pTemp ;
				vWinners.insert(pWiner) ;
			}
		}

		if ( vWinners.empty() )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("why this pool have no proper winner , pool idx = %d , poolCoin = %I64d",pPool->nIdx,pPool->nBetCoinInThisPool);
			continue;
		}

		stPoolResultInfo* pResultInfo = (stPoolResultInfo*)pTempBuffer ;
		pResultInfo->nWinnerCnt = vWinners.size();
		pResultInfo->nPoolIdx = pPool->nIdx ;
		pResultInfo->nPerPeerWinCoin = pPool->nBetCoinInThisPool / vWinners.size();
		// process chou shui
		if ( pResultInfo->nPerPeerWinCoin >= MIN_CHOU_SHUI_LIMIT )
		{
			pResultInfo->nPerPeerWinCoin = pResultInfo->nPerPeerWinCoin - pResultInfo->nPerPeerWinCoin * 0.05f  + 0.5f;
		}
		pTempBuffer =(char*)(pResultInfo + 1);

		LIST_PEERS::iterator iter_winner = vWinners.begin();
		for ( ; iter_winner != vWinners.end(); ++iter_winner )
		{
			CTaxasPokerPeer* pPeer = (CTaxasPokerPeer*)*iter_winner ;
			unsigned char* pIdxValue = (unsigned char*)pTempBuffer ;
			*pIdxValue = pPeer->GetPeerIdxInRoom();
			pTempBuffer =(char*)(++pIdxValue );

			unsigned int* pSessionId = (unsigned int*)pTempBuffer ;
			*pSessionId = pPeer->GetSessionID();
			pTempBuffer = (char*)(++pSessionId) ;

			// give coin to peer ;
			pPeer->OnWinCoinThisPool(pResultInfo->nPerPeerWinCoin);
		}
	}

	SendMsgRoomPeers((stMsg*)cBuffer,pTempBuffer - cBuffer) ;

	// cacluate game result wait time ;
	m_fCaluateGameResultTime = m_vAllBetPools.size() * TIME_TAXAS_CACULATE_PER_BET_POOL + TIME_TAXAS_RESULT_STAY + TIME_TAXAS_FILP_CARD;

	//// caculate all bet pool ;
	//			if ( m_vAllBetPools.empty())
	//			{
	//				// go to final ;
	//				GoToState(eRoomState_TP_Caculate_Final) ;
	//				break;
	//			}
	//#ifndef NDEBUG
	//			CLogMgr::SharedLogMgr()->SystemLog("开始算牌比大小：") ;
	//#endif
	//			stBetPool* pool = NULL ; // must delete pool in the end ;
	//			while ( m_vAllBetPools.empty() == false )   // find a pool that have winners ;
	//			{
	//				LIST_BET_POOLS::reverse_iterator  iter = m_vAllBetPools.rbegin() ;
	//				pool = *iter;  // must delete pool in the end ;
	//				m_vAllBetPools.erase((++iter).base()) ;
	//				if ( pool->vPeersInThisPool.empty() )
	//				{
	//					CLogMgr::SharedLogMgr()->ErrorLog("find a pool that have no winners , pool bet coin = %I64d",pool->nBetCoinInThisPool) ;
	//					delete pool ;
	//					pool = NULL ;
	//				}
	//				else
	//				{
	//					break; 
	//				}
	//			}
	//
	//			if ( pool == NULL || pool->nBetCoinInThisPool == 0 || pool->vPeersInThisPool.empty())  // all pool have no winners ;
	//			{
	//				// go to begin ;
	//				if ( pool )
	//				{
	//					delete pool ;
	//					pool = NULL ;
	//				}
	//
	//				CLogMgr::SharedLogMgr()->ErrorLog("No a pool that have winners ") ;
	//				GoToState(eRoomState_TP_Caculate_Final) ;
	//				return ;
	//			}
	//			// cacaulte ;  find winners ;
	//			LIST_PEERS vWinners ;
	//			LIST_PEERS::iterator iterFindWiner = pool->vPeersInThisPool.begin() ;
	//			CTaxasPokerPeer* pWiner = NULL ;
	//			for ( ; iterFindWiner != pool->vPeersInThisPool.end(); ++iterFindWiner )
	//			{
	//				CTaxasPokerPeer* pTemp = (CTaxasPokerPeer*)(*iterFindWiner) ;
	//				if ( pTemp == NULL || !pTemp->IsInState(eRoomPeer_WaitCaculate) )
	//				{
	//					continue; 
	//				}
	//				if ( !pWiner )
	//				{
	//					pWiner = pTemp;
	//					vWinners.insert(pWiner) ;
	//					continue ;
	//				}
	//#ifndef NDEBUG
	//				CLogMgr::SharedLogMgr()->SystemLog("curwinner is %s,cardtype = %d, cardinfo ",pWiner->GetPlayerBaseData()->GetPlayerName(),pWiner->GetPeerCard().GetCardType()) ;
	//				pWiner->GetPeerCard().LogInfo();
	//
	//				CLogMgr::SharedLogMgr()->SystemLog("Peeker is %s,cardtype = %d, cardinfo ",pTemp->GetPlayerBaseData()->GetPlayerName(),pTemp->GetPeerCard().GetCardType()) ;
	//				pTemp->GetPeerCard().LogInfo();
	//#endif
	//				char nPKRet = pWiner->PK(pTemp) ;
	//				if ( nPKRet == 0 )
	//				{
	//					vWinners.insert(pTemp) ;
	//#ifndef NDEBUG
	//					CLogMgr::SharedLogMgr()->SystemLog("PK Result The same") ;
	//#endif
	//				}
	//				else if ( nPKRet == -1 ) // failed 
	//				{
	//#ifndef NDEBUG
	//					CLogMgr::SharedLogMgr()->SystemLog("PK Result current Winner = %s Failed ",pWiner->GetPlayerBaseData()->GetPlayerName()) ;
	//#endif
	//					vWinners.clear() ;
	//					pWiner = pTemp ;
	//					vWinners.insert(pWiner) ;
	//				}
	//
	//				//CLogMgr::SharedLogMgr()->SystemLog("a winner name = %s , id = %d, cardtype = %d",pTemp->GetPlayerBaseData()->strName,pTemp->GetPlayer()->GetUserUID(),pTemp->GetPeerCard().GetCardType()) ;
	//				//CLogMgr::SharedLogMgr()->SystemLog("card as follow:") ;
	//				//pTemp->GetPeerCard().LogInfo();
	//			}
	//
	//			stMsgTaxasPokerGameRessult msgResult ;
	//			msgResult.nCoinsInPool = pool->nBetCoinInThisPool ;
	//			msgResult.nWinnerCnt = vWinners.size() ;
	//			// give coins to them ;
	//			if ( pool->nBetCoinInThisPool >= 15000 )
	//			{
	//				if ( GetRoomType() == eRoom_TexasPoker && GetRoomLevel() == eRoomLevel_Junior )
	//				{
	//
	//				}
	//				else
	//				{
	//					pool->nBetCoinInThisPool = pool->nBetCoinInThisPool - pool->nBetCoinInThisPool * 0.05  + 0.5; //chou shui zai zhe li zuo 
	//				}
	//			}
	//			pool->nBetCoinInThisPool -= pool->nBetCoinInThisPool % vWinners.size(); // avoid fen pei bu jun ;
	//			uint64_t nCoinPerWiner = pool->nBetCoinInThisPool / vWinners.size() ;
	//			// give coin to winner 
	//			msgResult.nWinCoins = nCoinPerWiner ;
	//			static char pBuffer[512] ;
	//			unsigned short nOffset = 0 ;
	//			memcpy(pBuffer,&msgResult,sizeof(msgResult));
	//			nOffset += sizeof(msgResult);
	//
	//			LIST_PEERS::iterator iterWinners = vWinners.begin() ;
	//			for ( ; iterWinners != vWinners.end(); ++iterWinners )
	//			{
	//				CTaxasPokerPeer* pTemp = (CTaxasPokerPeer*)(*iterWinners) ;
	//				pTemp->OnWinCoinThisPool(nCoinPerWiner) ;
	//				unsigned char nIdx = pTemp->GetPeerIdxInRoom();
	//				memcpy(pBuffer + nOffset,&nIdx,sizeof(nIdx));
	//				nOffset += sizeof(nIdx);
	//#ifndef NDEBUG
	//				CLogMgr::SharedLogMgr()->SystemLog("a winner name = %s , id = %d, cardtype = %d",pTemp->GetPlayerBaseData()->GetPlayerName(),pTemp->GetPlayer()->GetUserUID(),pTemp->GetPeerCard().GetCardType()) ;
	//				CLogMgr::SharedLogMgr()->SystemLog("card as follow:") ;
	//				pTemp->GetPeerCard().LogInfo();
	//#endif
	//			}
	//			SendMsgRoomPeers((stMsg*)pBuffer,nOffset) ;
	//			//LogPlayersInfo();
	//			// send msg to client ;
	//			delete pool ;
}

void CRoomTexasPoker::LogPlayersInfo()
{
	for ( int i = 0 ; i < GetMaxSeat() ; ++i )
	{
		CTaxasPokerPeer* peer = (CTaxasPokerPeer*)m_vRoomPeer[i] ;
		if ( !peer )
		{
			continue;
		}

		if ( IsDiamonedRoom() == false )
		{
			CLogMgr::SharedLogMgr()->PrintLog("%s  Left Coin = %I64d , BetCoinThis Round = %I64d",peer->GetPlayerBaseData()->GetPlayerName(),peer->GetPlayerBaseData()->GetTakeInMoney(),peer->GetBetCoinThisRound()) ;
		}
		else
		{
			CLogMgr::SharedLogMgr()->PrintLog("%s  Left Coin = %d , BetCoinThis Round = %I64d",peer->GetPlayerBaseData()->GetPlayerName(),peer->GetPlayerBaseData()->GetTakeInMoney(true),peer->GetBetCoinThisRound()) ;
		}
	}
}

void CRoomTexasPoker::LogRoomStateInfo()
{
	CLogMgr::SharedLogMgr()->SystemLog("-----------------------------------------------------------");
	CLogMgr::SharedLogMgr()->SystemLog("Room id = %d , playSitDownPlayer:",GetRoomID()) ;
	for ( int i = 0 ; i < GetMaxSeat(); ++i )
	{
		CRoomPeer* p = m_vRoomPeer[i] ;
		if ( p )
		{
			CLogMgr::SharedLogMgr()->SystemLog("Sit Down peer id = %d, peerState = %d, player state = %d",p->GetSessionID(),p->GetStateValue(),p->GetPlayer()->GetState()) ;

			// check if have one peer both sit more than one seat ?
			for ( int ii = i + 1 ; ii < GetMaxSeat() ; ++ii )
			{
				if ( p == m_vRoomPeer[ii] )
				{
					CLogMgr::SharedLogMgr()->ErrorLog("error one peer sit more than one sit") ;
				}
			}
		}
	}
	CLogMgr::SharedLogMgr()->SystemLog("room id = %d audences = %d :",GetRoomID(),m_vAudiencers.size()) ;
	
	LIST_PEERS::iterator iter = m_vAudiencers.begin();
	for ( ; iter != m_vAudiencers.end(); ++iter )
	{
		CRoomPeer* pA = *iter ;
		if ( !pA )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("why adences is NULL room id = %d",GetRoomID()) ;
		}
		else
		{
			CLogMgr::SharedLogMgr()->SystemLog("aduen peer id = %d , peerState = %d, PlayerState = %d",pA->GetSessionID(),pA->GetStateValue(),pA->GetPlayer()->GetState()) ;
		}

		// check duplicate
		for ( int i = 0 ; i < GetMaxSeat(); ++i )
		{
			if ( m_vRoomPeer[i] == pA )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("session id = %d , playerState = %d ,both sit down and up ",pA->GetSessionID(),pA->GetPlayer()->GetState() ) ;
			}
		}
	}

}

void CRoomTexasPoker::OnPeerStandUp(CTaxasPokerPeer* pPeerStandUp)
{
	if ( pPeerStandUp->IsInState(eRoomPeer_SitDown) == false )
	{
		return ;
	}

	if ( pPeerStandUp->IsInState(eRoomPeer_WaitCaculate) )
	{
		// put coin to temp bet pool ;
		//m_vCurRoundTempBetPool.push_back(pPeerStandUp->GetBetCoinThisRound()) ;
		m_nCurMainBetPool += pPeerStandUp->GetBetCoinThisRound();
		pPeerStandUp->DecreasBetCoinThisRound(pPeerStandUp->GetBetCoinThisRound());
		// remove from all vice pool ;
		RemovePeerFromBetPool(pPeerStandUp) ;
	}

	m_vRoomPeer[pPeerStandUp->GetPeerIdxInRoom()] = NULL ;
	m_vAudiencers.insert(pPeerStandUp) ;
	pPeerStandUp->SetState(eRoomPeer_StandUp) ;
	pPeerStandUp->SetIdxInRoom(GetMaxSeat()) ;
	pPeerStandUp->GetPlayerBaseData()->CaculateTakeInMoney();
}

bool CRoomTexasPoker::CheckRoundEnd()
{
	// check peer count 
	if ( GetCanActPeerCount() < 1 )
	{
		return true ;
	}

	bool bOnlyOneAct = GetCanActPeerCount() == 1 ;
	// all act peer have the same bet coin ;
	for ( int i = 0 ; i < GetMaxSeat(); ++i )
	{
		CTaxasPokerPeer* pp = (CTaxasPokerPeer*)m_vRoomPeer[i] ;
		if ( pp == NULL || pp->IsInState(eRoomPeer_CanAct) == false )
		{
			continue;
		}
		
		if ( pp->GetBetCoinThisRound() != m_nMostBetCoinThisRound || (pp->GetMyAction() == eRoomPeerAction_None && bOnlyOneAct == false ) )
		{
			return false ;
		}
	}
	return true ;
}

bool CRoomTexasPoker::CheckGameEnd()
{
	// when bet round end ;
	if ( m_nBetRound == 4 )
	{
		return true;
	}

	// only have one wait cacluate peer ;
	unsigned char nWaitPeer = 0 ;
	for ( int i = 0 ; i < GetMaxSeat(); ++i )
	{
		CTaxasPokerPeer* pp = (CTaxasPokerPeer*)m_vRoomPeer[i] ;
		if ( pp && pp->IsInState(eRoomPeer_WaitCaculate) )
		{
			++nWaitPeer;
		}
	}

	if ( nWaitPeer <= 1 )
	{
		return true ;
	}
	return  false ;
}

uint64_t CRoomTexasPoker::GetAllowMaxAllInCoin()
{
	// find most rich player 
	CTaxasPokerPeer* pRicher = NULL ;
	for ( int i = 0 ; i < GetMaxSeat(); ++i )
	{
		if ( m_vRoomPeer[i] == NULL || ((CTaxasPokerPeer*)m_vRoomPeer[i])->IsInState(eRoomPeer_WaitCaculate) == false )
		{
			continue;
		}

		if ( pRicher == NULL )
		{
			pRicher = (CTaxasPokerPeer*)m_vRoomPeer[i] ;
			continue;
		}

		if ( m_vRoomPeer[i]->GetCoin() + ((CTaxasPokerPeer*)m_vRoomPeer[i])->GetBetCoinThisRound() > pRicher->GetCoin() + pRicher->GetBetCoinThisRound() )
		{
			pRicher = (CTaxasPokerPeer*)m_vRoomPeer[i] ;
		}
	}

	if ( pRicher == NULL )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("most richer is null ? ") ;
		return 0 ;
	}

	// find the second rich ,proper
	CTaxasPokerPeer* pSecondRicher = NULL ;
	for ( int i = 0 ; i < GetMaxSeat(); ++i )
	{
		if ( m_vRoomPeer[i] == pRicher ) // omit first rich ;
		{
			continue; 
		}

		if ( m_vRoomPeer[i] == NULL || ((CTaxasPokerPeer*)m_vRoomPeer[i])->IsInState(eRoomPeer_WaitCaculate) == false )
		{
			continue;
		}

		if ( pSecondRicher == NULL )
		{
			pSecondRicher = (CTaxasPokerPeer*)m_vRoomPeer[i] ;
			continue;
		}

		if ( m_vRoomPeer[i]->GetCoin() + ((CTaxasPokerPeer*)m_vRoomPeer[i])->GetBetCoinThisRound()  > pSecondRicher->GetCoin() + pSecondRicher->GetBetCoinThisRound() )
		{
			pSecondRicher = (CTaxasPokerPeer*)m_vRoomPeer[i] ;
		}
	}

	if ( pSecondRicher == NULL )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("second richer is null ? ") ;
		return 0 ;
	}

	return pSecondRicher->GetCoin() + pSecondRicher->GetBetCoinThisRound();
}
