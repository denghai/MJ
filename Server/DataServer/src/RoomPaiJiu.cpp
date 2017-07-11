#include "RoomPaiJiu.h"
#include "RoomPeerPaiJiu.h"
#include "LogManager.h"
#include "PaiJiuMessageDefine.h"
#include "RobotManager.h"
void CRoomPaiJiu::Init( unsigned int nRoomID , unsigned char nMaxSeat )
{
	m_eRoomType = eRoom_PaiJiu ;
	CRoomBase::Init(nRoomID,nMaxSeat) ;
	m_tPoker.InitPaiJiu() ;
	m_tPoker.RestAllPoker() ;
	m_pBanker = NULL ;
	memset(m_nBetCoin, 0,sizeof(m_nBetCoin)) ;
	m_vApplyBankerList.clear() ;
	ClearHistory();

	m_fStateTick = 0 ;
	m_eRoomState = eRoomState_PJ_WaitBanker ;
	m_nBankerCoinRequair = 10 ;
	m_nCurBankerRound = 10 ;

	m_nDiceIdx = 0 ;

	////-----test here ;
	//CPaiJiuPeerCard stCard ;
	//CCard card , cardb;
	//card.SetCard(CCard::eCard_Heart,2);
	//cardb.SetCard(CCard::eCard_Diamond,2);
	//stCard.SetPeerCardCompsiteNumber(cardb.GetCardCompositeNum(),card.GetCardCompositeNum()) ;
	////----end test ;
	m_nPokerShuffleRound = 0 ;
}

unsigned char CRoomPaiJiu::CanJoin(CPlayer* peer )
{
	unsigned char nRet = CRoomBase::CanJoin(peer) ;
	if ( nRet != 0 )
	{
		return nRet ;
	}
	return 0 ;
}

void CRoomPaiJiu::AddPeer(CRoomPeer* peer )
{
	CRoomBase::AddPeer(peer) ;
	// take in all money 
	bool bDiamoned = IsDiamonedRoom();
	uint64_t nCoin = bDiamoned ? peer->GetPlayerBaseData()->GetAllDiamoned() : peer->GetPlayerBaseData()->GetAllCoin() ;
	peer->SetTakeInCoin(nCoin,bDiamoned);
}

void CRoomPaiJiu::OnPeerLeave( CRoomPeer* peer )
{
	CRoomBase::OnPeerLeave(peer) ;
	if ( peer == m_pBanker )
	{
		m_bChangeBanker = true ;
		m_pBanker = NULL ;
	}
	// how to process leave ;
}

void CRoomPaiJiu::Update(float fTimeElpas, unsigned int nTimerID )
{
	m_fStateTick += fTimeElpas ;
	switch( GetRoomState() )
	{
	case eRoomState_PJ_WaitBanker:
		{
			if ( m_vApplyBankerList.empty() == false )
			{
				SetNewBanker();
				if ( m_pBanker )
				{
					GoToState(eRoomState_PJWaitNewBankerChoseShuffle) ;
				}
			}
		}
		break;
	case eRoomState_PJWaitNewBankerChoseShuffle:
		{
			if ( m_fStateTick >= TIME_ROOM_PJ_WAIT_NEW_BANKER_CHOSE_SHUFFLE )
			{
				GoToState(eRoomState_PJ_Shuffle) ;
			}
		}
		break;
	case eRoomState_PJ_Shuffle:
		{
			if ( m_fStateTick >= TIME_ROOM_PJ_SHUFFLE )
			{
				m_fStateTick = 0 ;
				GoToState(eRoomState_PJ_WaitForBet ) ;
			}
		}
		break;
	case eRoomState_PJ_WaitForBet:
		{
			if ( m_fStateTick >= TIME_ROOM_PJ_WAIT_BET )
			{
				m_fStateTick = 0 ;
				GoToState(eRoomState_PJ_Dice) ;
			}
		}
		break; 
	case eRoomState_PJ_Dice:
		{
			if ( m_fStateTick >= TIME_ROOM_PJ_DICE )
			{
				m_fStateTick = 0 ;
				GoToState(eRoomState_PJ_Distribute) ;
			}
		}
		break;
	case eRoomState_PJ_Distribute:
		{
			if ( m_fStateTick >= TIME_ROOM_PJ_DISTRIBUTE )
			{
				m_fStateTick = 0 ;
				GoToState(eRoomState_PJ_ShowCard) ;
			}
		}
		break; 
	case eRoomState_PJ_ShowCard:
		{
			if ( m_fStateTick >= TIME_ROOM_PJ_SHOWCARD )
			{
				GoToState(eRoomState_PJ_Settlement);
			}
		}
		break;
	//case eRoomState_PJ_BankViewCard:
	//	{
	//		if ( m_fStateTick >= TIME_ROOM_PJ_BANKER_VIEW_CARD )
	//		{
	//			m_fStateTick = 0 ;
	//			GoToState(eRoomState_PJ_Settlement);
	//		}
	//	}
	//	break ;
	case eRoomState_PJ_Settlement:
		{
			if ( m_fStateTick >= TIME_ROOM_SETTLEMENT )
			{
				m_fStateTick = 0 ;
				if ( !m_bChangeBanker && m_pBanker )
				{
					m_bChangeBanker = m_pBanker->GetCoin() < m_nBankerCoinRequair ;
				}

				if ( !m_bChangeBanker && m_pBanker )
				{
					m_bChangeBanker = m_nCurBankerRound >= 20 && (m_vApplyBankerList.empty() == false ) ;
				}

				if ( m_bChangeBanker || m_pBanker == NULL  )
				{
					SetNewBanker() ;
					eRoomState eTargetStat = m_pBanker == NULL ? eRoomState_PJ_WaitBanker : eRoomState_PJWaitNewBankerChoseShuffle ;
					GoToState(eTargetStat) ;
				}
				else
				{
					GoToState(eRoomState_PJ_BankerSelectGoOn) ;
				}
			}
		}
		break ;
	case eRoomState_PJ_BankerSelectGoOn:
		{
			if ( m_fStateTick >= TIME_ROOM_BANKER_SELECT_GO_ON )
			{
				if ( m_tPoker.GetLeftCard() >= 8 )
				{
					GoToState(eRoomState_PJ_WaitForBet) ;
				}
				else // will go to eRoomState_PJ_Shuffle ? when left card is two little ;
				{
					GoToState(eRoomState_PJ_Shuffle) ;
				}
			}
		}
		break;
	default:
		CLogMgr::SharedLogMgr()->ErrorLog("unKnown room state in PaiJiu Room state = %d", GetRoomState() ) ;
		break;
	}
}

bool CRoomPaiJiu::OnPeerMsg(CRoomPeer* pPeer, stMsg* pmsg )
{
	if ( CRoomBase::OnPeerMsg(pPeer,pmsg) )
	{
		return true ;
	}

	CRoomPeerPaiJiu* pPeerPaiJiu = (CRoomPeerPaiJiu*)pPeer ;	
	switch (pmsg->usMsgType)
	{
	case MSG_PJ_BET:
		{
			//stMsgPJBetRet msgRet ;
			//stMsgPJBet* pMsgRet = (stMsgPJBet*)pmsg ;
			//msgRet.nBetCoin = pMsgRet->nBetCoin ;
			//msgRet.nBetPort = pMsgRet->nBetPort ;
			//uint64_t nLeftcoin = GetAllowBetCoin();
			//if ( GetRoomState() != eRoomState_PJ_WaitForBet )
			//{
			//	msgRet.nRet = ePJ_ActRet_State_Not_Fit ;
			//}
			//else if ( pPeer == m_pBanker )
			//{
			//	msgRet.nRet = ePJ_ActRet_Identifer_Wrong ;
			//}
			//else if ( pMsgRet->nBetCoin > nLeftcoin )
			//{
			//	msgRet.nRet = ePJ_ActRet_Banker_Money_Not_Enough ; 
			//}
			//else if ( pPeerPaiJiu->AddBetCoin(pMsgRet->nBetCoin,(ePaiJiuBetPort)pMsgRet->nBetPort) )
			//{
			//	m_nBetCoin[pMsgRet->nBetPort] += pMsgRet->nBetCoin ;

			//	stMsgPJOtherBet msgOtherBet ;
			//	msgOtherBet.nBetCoin =pMsgRet->nBetCoin ;
			//	msgOtherBet.nBetPort = pMsgRet->nBetPort ;
			//	SendMsgRoomPeers(&msgOtherBet,sizeof(msgOtherBet),pPeer) ;
			//	msgRet.nRet = ePJ_ActRet_Success ;
			//}
			//else
			//{
			//	msgRet.nRet = ePJ_ActRet_Banker_Money_Not_Enough ; 
			//}
			//pPeerPaiJiu->SendMsgToClient((char*)&msgRet,sizeof(msgRet)) ;
		}
		break;
	case MSG_PJ_APPLY_BANKER:
		{
			//stMsgPJActionRet msgBack ;
			//msgBack.nAcionType = MSG_PJ_APPLY_BANKER ;
			//if ( pPeerPaiJiu->GetCoin() < m_nBankerCoinRequair )
			//{
			//	msgBack.nRet = ePJ_ActRet_Self_Money_Not_Enough ;
			//}
			//else if ( m_vApplyBankerList.size() >= 10 )
			//{
			//	msgBack.nRet = ePJ_ActRet_Apply_List_Full ;
			//}
			//else if ( pPeer == m_pBanker )
			//{
			//	msgBack.nRet = ePJ_ActRet_Identifer_Wrong ;
			//}
			//else if ( IsInApplyList(pPeer) )
			//{
			//	msgBack.nRet = ePJ_ActRet_Aready_In_ApplyList ;
			//}
			//else 
			//{
			//	msgBack.nRet = ePJ_ActRet_Success ;
			//	m_vApplyBankerList.push_back(pPeer) ;
			//	stMsgPJOtherApplyToBeBanker msgToOther ;
			//	msgToOther.nApplyerCoin = pPeer->GetCoin();
			//	memset(msgToOther.cApplyerName,0,sizeof(msgToOther.cApplyerName));
			//	memcpy(msgToOther.cApplyerName,pPeer->GetPlayerBaseData()->strName,MAX_LEN_CHARACTER_NAME);
			//	SendMsgRoomPeers(&msgToOther,sizeof(msgToOther),pPeer) ;
			//}
			//pPeer->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
		}
		break;
	case MSG_PJ_BANKER_SHOW_CARD:
		{
			//if ( m_pBanker != pPeerPaiJiu )
			//{
			//	CLogMgr::SharedLogMgr()->ErrorLog("only banker can show banker card") ;
			//	return true ;
			//}
			//stMsgPJShowBankerCard msg ;
			//SendMsgRoomPeers(&msg,sizeof(msg),pPeer) ;
		}
		break;
	case MSG_PJ_APPLY_UNBANKER:
		{
			stMsgPJActionRet msgBack ;
			msgBack.nAcionType = MSG_PJ_APPLY_UNBANKER ;
			if ( m_pBanker != pPeer )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("only banker can show banker card") ;
				msgBack.nRet = ePJ_ActRet_SELF_NOT_BANKER ;
			}
			else
			{
				m_bChangeBanker = true ;
				msgBack.nRet = ePJ_ActRet_Success ;
			}
			pPeer->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
		}
		break; 
	case MSG_PJ_BANKER_CHOSE_SHUFFLE:
		{
			stMsgPJActionRet msgBack ;
			msgBack.nAcionType = MSG_PJ_BANKER_CHOSE_SHUFFLE ;
			if ( pPeer != m_pBanker )
			{
				msgBack.nRet = ePJ_ActRet_SELF_NOT_BANKER ; 
			}
			else if ( GetRoomState() == eRoomState_PJWaitNewBankerChoseShuffle )
			{
				msgBack.nRet = ePJ_ActRet_State_Not_Fit ;
			}
			else
			{
				msgBack.nRet = ePJ_ActRet_Success ;
				GoToState(eRoomState_PJ_Shuffle) ;
			}
			pPeer->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
		}
		break;
	case MSG_PJ_BANKER_CHOSE_CONTINUE_CANCEL:
		{
			stMsgPJActionRet msgBack ;
			msgBack.nAcionType = MSG_PJ_BANKER_CHOSE_CONTINUE_CANCEL ;
			stMsgPJBankerChoseGoOnOrCanncel* pRet = (stMsgPJBankerChoseGoOnOrCanncel*)pmsg;
			if ( m_pBanker != pPeer )
			{
				msgBack.nRet = ePJ_ActRet_SELF_NOT_BANKER ; 
			}
			else if ( GetRoomState() != eRoomState_PJ_BankerSelectGoOn )
			{
				msgBack.nRet = ePJ_ActRet_State_Not_Fit ;
			}
			pPeer->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
			if ( pRet->nChoice == 0 ) // go on play 
			{
				if ( m_tPoker.GetLeftCard() >= 8 )
				{
					GoToState(eRoomState_PJ_WaitForBet) ;
				}
				else // will go to eRoomState_PJ_Shuffle ? when left card is two little ;
				{
					GoToState(eRoomState_PJ_Shuffle) ;
				}
			}
			else if ( pRet->nChoice == 2 ) // unbanker 
			{
				SetNewBanker();
				if ( m_pBanker )
				{
					GoToState(eRoomState_PJWaitNewBankerChoseShuffle) ;
				}
				else
				{
					GoToState(eRoomState_PJ_WaitBanker) ;
				}
			}
			else
			{
				CLogMgr::SharedLogMgr()->ErrorLog("unknown banker choice!") ;
			}
		}
		break; 
	default:
		return false ;
	}
	return true ;
}

uint64_t CRoomPaiJiu::GetCurrentAllBetCoin()
{
	uint64_t nBetCoin = 0 ;
	for ( int i = ePJ_BetPort_None; i < ePJ_BetPort_Max; ++i )
	{
		nBetCoin += m_nBetCoin[i];
	}
	return nBetCoin ;
}

uint64_t CRoomPaiJiu::GetAllowBetCoin()
{
	if ( m_pBanker )
	{
		return m_pBanker->GetCoin() - GetCurrentAllBetCoin() ;
	}
	return 0 ;
}

void CRoomPaiJiu::ClearHistory()
{
	for ( int i = ePJ_BetPort_None ; i < ePJ_BetPort_Normal_Max; ++i )
	{
		memset(m_vWinHistory, 0 , sizeof(unsigned char)* ePJ_BetPort_Normal_Max * MAX_PAIJIU_HISTROY_RECORDER ) ;
	}
}

void CRoomPaiJiu::GoToState(eRoomState eTargetState )
{
	switch( eTargetState )
	{
	case eRoomState_PJ_WaitBanker:
		{
			CLogMgr::SharedLogMgr()->SystemLog("Current do not have banker !") ;
			stMsgPJRoomStateChanged msg ;
			msg.nCurrentState = eTargetState ;
			SendMsgRoomPeers(&msg,sizeof(msg)) ;
			if ( GetRoomPeerCount() < MIN_PEERS_IN_ROOM_ROBOT )
			{
				CRobotManager::SharedRobotMgr()->RequestRobotToJoin(this) ;
			}
		}
		break;
	case eRoomState_PJ_WaitForBet:
		{
			//if ( m_tPoker.GetLeftCard() < 8 )
			//{
			//	GoToState(eRoomState_PJ_Shuffle) ;
			//	return ;
			//}
			//else
			CLogMgr::SharedLogMgr()->SystemLog("eRoomState_PJ_WaitForBet !") ;
			{
				stMsgPJRoomStateChanged msg ;
				msg.nCurrentState = eTargetState ;
				SendMsgRoomPeers(&msg,sizeof(msg)) ;
			}

			if ( GetRoomPeerCount() < MIN_PEERS_IN_ROOM_ROBOT )
			{
				CRobotManager::SharedRobotMgr()->RequestRobotToJoin(this) ;
			}
		}
		break;
	case eRoomState_PJWaitNewBankerChoseShuffle:
	case eRoomState_PJ_BankerSelectGoOn:
	case eRoomState_PJ_ShowCard:
		{
			if ( eTargetState == eRoomState_PJWaitNewBankerChoseShuffle )
			{
				CLogMgr::SharedLogMgr()->SystemLog("eRoomState_PJWaitNewBankerChoseShuffle !") ;
			}
			if ( eTargetState == eRoomState_PJ_BankerSelectGoOn )
			{
				CLogMgr::SharedLogMgr()->SystemLog("eRoomState_PJ_BankerSelectGoOn !") ;
			}
			if ( eTargetState == eRoomState_PJ_ShowCard )
			{
				CLogMgr::SharedLogMgr()->SystemLog("eRoomState_PJ_ShowCard !") ;
			}

			stMsgPJRoomStateChanged msg ;
			msg.nCurrentState = eTargetState ;
			SendMsgRoomPeers(&msg,sizeof(msg)) ;
		}
		break; 
	case eRoomState_PJ_Dice:
		{
			CLogMgr::SharedLogMgr()->SystemLog("eRoomState_PJ_Dice !") ;
			stMsgPJDice msg ;
			msg.nPoint[0] = rand() % 6 + 1  ; 
			msg.nPoint[1] = rand() % 6 + 1  ;
			SendMsgRoomPeers(&msg,sizeof(msg)) ;
			m_nDiceIdx = (msg.nPoint[0] + msg.nPoint[1]) % ePJ_BetPort_Normal_Max ;
		}
		break;
	case eRoomState_PJ_Distribute:
		{
			CLogMgr::SharedLogMgr()->SystemLog("eRoomState_PJ_Distribute !") ;
			unsigned char nCards[ePJ_BetPort_Normal_Max][2]={0} ;
			for ( int i = 0 ; i < 2 ; ++i )
			{
				char idx = m_nDiceIdx ;
				char nCount = 0 ;
				while ( nCount < ePJ_BetPort_Normal_Max )
				{
					idx = m_nDiceIdx + nCount ;
					idx = (idx>=ePJ_BetPort_Normal_Max ? (idx-ePJ_BetPort_Normal_Max):idx);
					nCards[idx][i] = m_tPoker.GetCardWithCompositeNum();
					++nCount ;
				}
			}

			stMsgPJDistribute msg ;
			for ( int i = 0 ; i < 2 ; ++i )
			{
				msg.vCardZhuang[i]  = nCards[ePJ_BetPort_Zhuang][i];
				msg.vCardDao[i] = nCards[ePJ_BetPort_Dao][i];
				msg.vCardShun[i] = nCards[ePJ_BetPort_Shun][i];
				msg.vCardTian[i]  = nCards[ePJ_BetPort_Tian][i];
			}
			SendMsgRoomPeers(&msg,sizeof(msg));

			CLogMgr::SharedLogMgr()->PrintLog("ePJ_BetPort_Zhuang") ;
			m_vPeerCard[ePJ_BetPort_Zhuang].SetPeerCardCompsiteNumber(msg.vCardZhuang[0],msg.vCardZhuang[1]);

			CLogMgr::SharedLogMgr()->PrintLog("ePJ_BetPort_Dao") ;
			m_vPeerCard[ePJ_BetPort_Dao].SetPeerCardCompsiteNumber(msg.vCardDao[0],msg.vCardDao[1]);
			
			CLogMgr::SharedLogMgr()->PrintLog("ePJ_BetPort_Shun") ;
			m_vPeerCard[ePJ_BetPort_Shun].SetPeerCardCompsiteNumber(msg.vCardShun[0],msg.vCardShun[1]);
			
			CLogMgr::SharedLogMgr()->PrintLog("ePJ_BetPort_Tian") ;
			m_vPeerCard[ePJ_BetPort_Tian].SetPeerCardCompsiteNumber(msg.vCardTian[0],msg.vCardTian[1]);
			++m_nCurBankerRound ;
		}
		break; 
	case eRoomState_PJ_Settlement:
		{
			CLogMgr::SharedLogMgr()->SystemLog("eRoomState_PJ_Settlement ! LeftCard = %d",m_tPoker.GetLeftCard() ) ;
			SettlementGame();
		}
		break ;
	case eRoomState_PJ_Shuffle:
		{
			CLogMgr::SharedLogMgr()->SystemLog("eRoomState_PJ_Shuffle !") ;
			stMsgPJRoomStateChanged msg ;
			msg.nCurrentState = eTargetState ;
			SendMsgRoomPeers(&msg,sizeof(msg)) ;
			m_tPoker.RestAllPoker() ;
			++m_nPokerShuffleRound ;
		}
		break;
	//case eRoomState_PJ_BankViewCard:
	//	{

	//	}
	//	break;
	default:
		CLogMgr::SharedLogMgr()->ErrorLog("unKnown room target state in PaiJiu Room state = %d", eTargetState ) ;
		return ;
	}
	m_fStateTick = 0 ;
	m_eRoomState = eTargetState ;
}

void CRoomPaiJiu::SettlementGame()
{
	//CLogMgr::SharedLogMgr()->PrintLog("开始结算：") ;
	//// descide which port win ;
	//bool bShunWin = !m_vPeerCard[ePJ_BetPort_Zhuang].PKPeerCard(&m_vPeerCard[ePJ_BetPort_Shun]);
	//bool bDaoWin = !m_vPeerCard[ePJ_BetPort_Zhuang].PKPeerCard(&m_vPeerCard[ePJ_BetPort_Dao]);
	//bool bTianWin = !m_vPeerCard[ePJ_BetPort_Zhuang].PKPeerCard(&m_vPeerCard[ePJ_BetPort_Tian]);

	//// settle banker 
	//double nConOffset = 0 ;
	//nConOffset += ( bShunWin ? -1 : 1 ) * (m_nBetCoin[ePJ_BetPort_Shun] +  (m_nBetCoin[ePJ_BetPort_Qiao] + m_nBetCoin[ePJ_BetPort_ShunJiao]) * 0.5 );
	//nConOffset += ( bDaoWin ? -1 : 1 ) * (m_nBetCoin[ePJ_BetPort_Dao] +  ( m_nBetCoin[ePJ_BetPort_Qiao] + m_nBetCoin[ePJ_BetPort_DaoJiao]) * 0.5 ); 
	//nConOffset += ( bTianWin ? -1 : 1 ) * (m_nBetCoin[ePJ_BetPort_Tian] +  ( m_nBetCoin[ePJ_BetPort_ShunJiao] + m_nBetCoin[ePJ_BetPort_DaoJiao]) * 0.5 );

	//stMsgPJSettlement msgBack ;
	//msgBack.nBankerCoinOffset = (int64_t)nConOffset ;
	//msgBack.nResultCoinOffset = (int64_t)nConOffset ;
	//if ( m_pBanker )
	//{
	//	m_pBanker->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
	//	m_pBanker->OnWinCoin((int64_t)nConOffset) ;
	//	CLogMgr::SharedLogMgr()->PrintLog("庄家本局输赢：%I64d",(int64_t)nConOffset ); 
	//}
	//
	//CRoomPeer* pPeer = NULL ;
	//for ( int i = 0; i < GetMaxSeat(); ++i )
	//{
	//	pPeer = m_vRoomPeer[i];
	//	if ( !pPeer )
	//	{
	//		continue; 
	//	}

	//	if ( pPeer == m_pBanker )
	//	{
	//		continue;
	//	}
	//	CRoomPeerPaiJiu* peer = dynamic_cast<CRoomPeerPaiJiu*>(pPeer);
	//	if ( peer )
	//	{
	//		msgBack.nResultCoinOffset = peer->OnSettlement(bShunWin,bTianWin,bDaoWin);
	//		pPeer->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
	//		CLogMgr::SharedLogMgr()->PrintLog("闲家：%s  本局输赢：%I64d",peer->GetPlayerBaseData()->strName,msgBack.nResultCoinOffset) ; 
	//	}
	//}
	//memset(m_nBetCoin,0,sizeof(m_nBetCoin)) ;

	//// update history 
	//RefreshHistory(bShunWin,bDaoWin,bTianWin) ;
	//// refresh apply list ;
	//RefreshApplyBankList();
}

bool CRoomPaiJiu::SendCurRoomInfoToPlayer(CRoomPeer* pPeer )
{
/*	stMsgPJRoomInfo msgInfo ;
	msgInfo.nRoomID = GetRoomID();
	msgInfo.eRoomState = GetRoomState() ;
	msgInfo.nLeftCard = (unsigned char)m_tPoker.GetLeftCard();
	msgInfo.fStateTick = m_fStateTick ;
	memcpy(msgInfo.nBetCoin,m_nBetCoin,sizeof(m_nBetCoin));
	memset(msgInfo.cBankerName,0,sizeof(msgInfo.cBankerName)) ;
	msgInfo.nBankerCoin = 0 ;
	if ( m_pBanker )
	{
		memcpy(msgInfo.cBankerName,m_pBanker->GetPlayerBaseData()->strName,MAX_LEN_CHARACTER_NAME);
		msgInfo.nBankerCoin = m_pBanker->GetCoin() ;
		msgInfo.nBankerPhotoID = m_pBanker->GetPlayerBaseData()->nDefaulPhotoID ;
	}
	
	for ( int i =  ePJ_BetPort_None; i < ePJ_BetPort_Normal_Max ; ++i )
	{
		memcpy(msgInfo.nHistroy[i],m_vWinHistory[i],sizeof(unsigned char) * MAX_PAIJIU_HISTROY_RECORDER);
		msgInfo.vPortCard[i][0] = m_vPeerCard[i].GetCardByIdx(0).GetCardCompositeNum();
		msgInfo.vPortCard[i][1] = m_vPeerCard[i].GetCardByIdx(1).GetCardCompositeNum();
	}
	msgInfo.nApplyBankerCount = m_vApplyBankerList.size() ;
	char* pBuffer = new char[sizeof(msgInfo) + msgInfo.nApplyBankerCount * sizeof(stApplyBankerItem)] ;
	
	int nOffset = 0 ;
	memcpy(pBuffer,(char*)&msgInfo,sizeof(msgInfo));
	nOffset += sizeof(msgInfo);

	LIST_ROOM_PERR::iterator iter = m_vApplyBankerList.begin() ;
	CRoomPeer* pApplyPeer = NULL ;
	for (  ; iter != m_vApplyBankerList.end() ; ++iter )
	{
		pApplyPeer = *iter ;
		stApplyBankerItem item ;
		item.nSessionID = pApplyPeer->GetSessionID();
		item.nCoin = pApplyPeer->GetCoin() ;
		memcpy(item.nName,pApplyPeer->GetPlayerBaseData()->strName,MAX_LEN_CHARACTER_NAME);
		memcpy(pBuffer+nOffset,&item,sizeof(stApplyBankerItem));
		nOffset += sizeof(stApplyBankerItem) ;
	}
	pPeer->SendMsgToClient(pBuffer,nOffset) */;
	return true ;
}

void CRoomPaiJiu::SetNewBanker()
{
	//m_bChangeBanker = false ;

	//CRoomPeer* pNew = NULL ;
	//CRoomPeer* pCur = NULL ;
	//LIST_ROOM_PERR::iterator iter = m_vApplyBankerList.begin() ;
	//LIST_ROOM_PERR::iterator Max = m_vApplyBankerList.end();
	//for( ; iter != m_vApplyBankerList.end() ; ++iter )
	//{
	//	pCur = *iter ;
	//	if ( pNew == NULL )
	//	{
	//		pNew = pCur ;
	//		Max = iter ;
	//		continue;
	//	}
	//	else if ( pCur->GetCoin() > pNew->GetCoin() )
	//	{
	//		pNew = pCur ;
	//		Max = iter ;
	//	}
	//}

	//if ( pNew && Max != m_vApplyBankerList.end() )
	//{
	//	m_vApplyBankerList.erase(Max) ;
	//}

	//m_pBanker = (CRoomPeerPaiJiu*)pNew ;
	//stMsgPJBankerChanged msgChange ;
	//memset(msgChange.cNewBankerName,0,sizeof(msgChange.cNewBankerName)) ;
	//msgChange.nNewBankerCoin = 0 ;
	//if ( m_pBanker )
	//{
	//	memcpy(msgChange.cNewBankerName ,m_pBanker->GetPlayerBaseData()->strName,MAX_LEN_CHARACTER_NAME);
	//	msgChange.nNewBankerCoin = m_pBanker->GetCoin();
	//	msgChange.newBankerSessionID = m_pBanker->GetSessionID() ;
	//}
	//SendMsgRoomPeers(&msgChange,sizeof(msgChange));
	//m_nCurBankerRound = 0 ;
}

bool CRoomPaiJiu::IsInApplyList(CRoomPeer* peer )
{
	LIST_ROOM_PERR::iterator iter = m_vApplyBankerList.begin() ; 
	for ( ; iter != m_vApplyBankerList.end(); ++iter )
	{
		if ( *iter == peer )
		{
			return true ;
		}
	}
	return false ;
}

void CRoomPaiJiu::RemoveFromApplyList( CRoomPeer* peer )
{
	LIST_ROOM_PERR::iterator iter = m_vApplyBankerList.begin();
	for ( ; iter != m_vApplyBankerList.end(); ++iter)
	{
		if ( *iter == peer )
		{
			m_vApplyBankerList.erase(iter) ;
			return ;
		}
	}
}

void CRoomPaiJiu::RefreshHistory(bool bShunWin, bool bDaoWin , bool bTianWin )
{
	unsigned char nResult[ePJ_BetPort_Normal_Max]={0} ;
	nResult[ePJ_BetPort_Shun] = bShunWin ? 1 : 2 ;
	nResult[ePJ_BetPort_Dao] = bDaoWin ? 1 : 2 ;
	nResult[ePJ_BetPort_Tian] = bTianWin ? 1 : 2 ;

	for ( int i = ePJ_BetPort_None ; i < ePJ_BetPort_Normal_Max; ++i )
	{
		if ( m_vWinHistory[i][MAX_PAIJIU_HISTROY_RECORDER-1].nResult == 0 ) // not full ;
		{
			// find no null pos ;
			for ( int j = 0 ; j < MAX_PAIJIU_HISTROY_RECORDER ; ++j )
			{
				if (m_vWinHistory[i][j].nResult== 0)
				{
					m_vWinHistory[i][j].nResult = nResult[i] ;
					if ( m_pBanker )
					{
						m_vWinHistory[i][j].nBankerSessionID = m_pBanker->GetSessionID() ;
					}
					m_vWinHistory[i][j].nPokerShuffleRound = m_nPokerShuffleRound ;
					break; 
				}
			}
		}
		else
		{
			memcpy(&(m_vWinHistory[i][0]),&(m_vWinHistory[i][1]),(MAX_PAIJIU_HISTROY_RECORDER-1)*sizeof(stHistroyRecorder)) ;
			m_vWinHistory[i][MAX_PAIJIU_HISTROY_RECORDER-1].nResult = nResult[i] ;
			if ( m_pBanker )
			{
				m_vWinHistory[i][MAX_PAIJIU_HISTROY_RECORDER-1].nBankerSessionID = m_pBanker->GetSessionID() ;
			}
			m_vWinHistory[i][MAX_PAIJIU_HISTROY_RECORDER-1].nPokerShuffleRound = m_nPokerShuffleRound ;
		}
	}

	stMsgPJRoomHistoryRecord msgR ;
	memcpy(msgR.nHistroy,m_vWinHistory,sizeof(m_vWinHistory));
	SendMsgRoomPeers(&msgR,sizeof(msgR)) ;
}

void CRoomPaiJiu::RefreshApplyBankList()
{
	//LIST_ROOM_PERR::iterator iter = m_vApplyBankerList.begin() ;
	//LIST_ROOM_PERR vRemove ;
	//for ( ; iter != m_vApplyBankerList.end(); ++iter )
	//{
	//	if ( (*iter)->GetCoin() <= m_nBankerCoinRequair )
	//	{
	//		vRemove.push_back(*iter) ;
	//	}
	//}
	//
	//iter = vRemove.begin() ;
	//for ( ; iter != vRemove.end(); ++iter )
	//{
	//	LIST_ROOM_PERR::iterator iter2 = m_vApplyBankerList.begin() ;
	//	for ( ; iter2 != m_vApplyBankerList.end(); ++iter2 )
	//	{
	//		if ( *iter2 == *iter )
	//		{
	//			m_vApplyBankerList.erase(iter2) ;
	//			break; 
	//		}
	//	}
	//}
	//vRemove.clear() ;
	//// send new player' message ;
	//unsigned char nCount = m_vApplyBankerList.size() ;
	//stMsgPJRoomApplyList msgList ;
	//msgList.nCount = nCount ;	
	//char* pBuffer = new char[sizeof(msgList) + nCount * sizeof(stApplyBankerItem)] ;
	//unsigned short nOffset = 0;
	//memcpy(pBuffer,&msgList,sizeof(msgList));
	//nOffset += sizeof(msgList) ;
	//
	//CRoomPeer* pPeer = NULL ;
	//iter = m_vApplyBankerList.begin();
	//for ( ;iter != m_vApplyBankerList.end(); ++iter )
	//{
	//	pPeer = *iter ;
	//	if ( pPeer == NULL )
	//	{
	//		continue;
	//	}
	//	stApplyBankerItem item ;
	//	item.nCoin = pPeer->GetCoin() ;
	//	item.nSessionID = pPeer->GetSessionID() ;
	//	memset(item.nName,0 , sizeof(item.nName)) ;
	//	memcpy(item.nName,pPeer->GetPlayerBaseData()->strName,sizeof(item.nName));
	//	memcpy(pBuffer + nOffset , &item,sizeof(item));
	//	nOffset += sizeof(item);
	//}
	//SendMsgRoomPeers((stMsg*)pBuffer,nOffset) ;
	//delete [] pBuffer ;
}


