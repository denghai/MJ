//#include "RoomGolden.h"
//#include "Timer.h"
//#include "RoomPeer.h"
//#include "Player.h"
//#include "LogManager.h"
//#include "GoldenMessageDefine.h"
//#include <list>
//#define TAX_START_COIN 10000
//unsigned int CRoom::s_RoomID = 0 ;
//
//CRoom::CRoom()
//{
//	m_eRoomType = eRoom_Gold ;
//}
//
//CRoom::~CRoom()
//{
//
//}
//
//void CRoom::Init( unsigned int nRoomID , unsigned char nMaxSeat )
//{
//	CRoomBase::Init(nRoomID,nMaxSeat) ;
//	m_nCurMainPeerIdx = 0 ;
//	Rest();
//}
//
//void CRoom::Rest()
//{
//	float m_fRoomSateTick[eRoomState_Max] ;
//	memset(m_fRoomSateTick,0,sizeof(m_fRoomSateTick)) ;
//	m_nCurWaitPeerIdx = m_nCurMainPeerIdx ;
//	m_nSingleBetCoin = 10 ;
//	m_nTotalBetCoin = 0 ;
//	m_nRound = 0;
//	for ( int i = 0 ; i < MAX_ROOM_PEER ; ++i )
//	{
//		if ( m_vRoomPeer[i] )
//		{
//			m_vRoomPeer[i]->m_eState = eRoomPeer_None ;
//		}
//	}
//}
//
//unsigned char CRoom::GetEmptySeatCount()
//{
//	unsigned char n = 0 ;
//	for ( int i =0  ; i < GetMaxSeat(); ++i )
//	{
//		if ( m_vRoomPeer[i] == NULL )
//		{
//			++n ;
//		}
//	}
//	return n ;
//}
//
//bool CRoom::CanJoin(CRoomPeer* peer )
//{
//	if ( CRoomBase::CanJoin(peer) == false )
//	{
//		return false ;
//	}
//
//	// some other condition ;
//	return true ;
//}
//
//void CRoom::AddPeer(CRoomPeer* peer )
//{
//	// find a empty place 
//	for ( int i = 0 ; i < GetMaxSeat() ; ++i )
//	{
//		if ( m_vRoomPeer[i] == NULL )
//		{
//			peer->m_nPeerIdx = i ;
//			break; 
//		}
//	}
//
//	//stMsgRoomPlayerEnter msg ;
//	//peer->GetBrifBaseInfo(msg.nEnterPlayerInfo) ;
//	//SendMsgRoomPeers(&msg,sizeof(msg)) ;
//	//m_vRoomPeer[peer->GetPeerIdxInRoom()] = peer ;
//}
//
//void CRoom::Update(float fTimeElpas, unsigned int nTimerID )
//{
//	eRoomState eState = GetRoomState() ;
//	switch ( eState )
//	{
//	case eRoomState_WaitPeerToJoin:
//		{
//			// just wait 
//			if ( GetEmptySeatCount() == 0 )
//			{
//				SwitchToRoomSate(eRoomState_WaitPeerToJoin, eRoomState_WaitPeerToGetReady ) ;
//			}
//		}
//		break;
//	case eRoomState_WaitPeerToGetReady:
//		{
//			m_fRoomSateTick[eState]-= fTimeElpas ;
//			if ( m_fRoomSateTick[eState] <= 0 || GetReadyPeerCount() == GetMaxSeat() )
//			{
//				if ( GetReadyPeerCount() >= 2 )
//				{
//					SwitchToRoomSate(eState,eRoomState_DistributeCard) ;
//				}
//				else
//				{
//					if ( GetRoomPeerCount() < 2 )
//					{
//						SwitchToRoomSate(GetRoomState(),eRoomState_WaitPeerToJoin ) ;
//					}
//					else
//					{
//						SwitchToRoomSate(GetRoomState(),eRoomState_WaitPeerToGetReady ) ;
//					}
//				}
//			}
//		}
//		break;
//	case eRoomState_DistributeCard:
//		{
//			m_fRoomSateTick[eState]-= fTimeElpas ;
//			if ( m_fRoomSateTick[eState] <= 0 )
//			{
//				SwitchToRoomSate(eState,eRoomState_WaitPeerAction) ;
//			}
//		}
//		break;
//	case eRoomState_WaitPeerAction:
//		{
//			m_fRoomSateTick[eState]-= fTimeElpas ;
//			if ( m_fRoomSateTick[eState] <= 0 )
//			{
//				if ( m_vRoomPeer[m_nCurWaitPeerIdx] )
//				{
//					m_vRoomPeer[m_nCurWaitPeerIdx]->OnWaitTimeOut();
//				}
//				else
//				{
//					CLogMgr::SharedLogMgr()->ErrorLog("waiting action player is null") ;
//				}
//				
//			}
//		}
//		break;
//	case eRoomState_PKing:
//		{
//			m_fRoomSateTick[eState]-= fTimeElpas ;
//			if ( m_fRoomSateTick[eState] <= 0 )
//			{
//				if ( CheckFinish() )
//				{
//					SwitchToRoomSate(GetRoomState(),eRoomState_ShowingResult ) ;
//				}
//				else
//				{
//					NextPlayerAction() ;
//				}
//			}
//		}
//		break;
//	case eRoomState_ShowingResult:
//		{
//			m_fRoomSateTick[eState]-= fTimeElpas ;
//			if ( m_fRoomSateTick[eState] <= 0 )
//			{
//				SwitchToRoomSate(eState,eRoomState_WaitPeerToJoin) ;
//			}
//		}
//		break;
//	default:
//		{
//			CLogMgr::SharedLogMgr()->ErrorLog("unknown room state = %d", eState) ;
//		}
//		break; 
//	}
//}
//
//void CRoom::SendCurRoomToPeer(CRoomPeer* peer )
//{
//	stMsgRoomCurInfo msgRoomInfo ;
//	msgRoomInfo.nRoomID = GetRoomID();
//	msgRoomInfo.nSelfIdx = peer->GetPeerIdxInRoom();
//	msgRoomInfo.eRoomSate = GetRoomState() ;
//	msgRoomInfo.nPlayerCount = GetMaxSeat() - GetEmptySeatCount() -1 ; // -1 self ;
//	msgRoomInfo.nRound = m_nRound ;
//	msgRoomInfo.nSingleBetCoin = m_nSingleBetCoin ;
//	msgRoomInfo.nTotalBetCoin = m_nTotalBetCoin ;
//	int nAllLen = sizeof(msgRoomInfo) + msgRoomInfo.nPlayerCount * sizeof(stRoomPeerBrifInfo) ;
//	stRoomPeerBrifInfo stBrifInfo ;
//	char* pBuffer = new char[nAllLen] ;
//	int nOffset = 0 ;
//	memcpy(pBuffer,&msgRoomInfo,sizeof(msgRoomInfo));
//	nOffset += sizeof(msgRoomInfo);
//	for ( int i = 0 ; i < GetMaxSeat(); ++i )
//	{
//		if ( m_vRoomPeer[i] == NULL || peer->GetSessionID() == m_vRoomPeer[i]->GetSessionID() )
//		{
//			continue;
//		}
//		m_vRoomPeer[i]->GetBrifBaseInfo(stBrifInfo) ;
//		memcpy(pBuffer + nOffset, &stBrifInfo,sizeof(stRoomPeerBrifInfo)) ;
//		nOffset += sizeof(stRoomPeerBrifInfo);
//	}
//	peer->SendMsgToClient(pBuffer,nAllLen) ;
//}
//
//void CRoom::OnPeerLeave( CRoomPeer* peer )
//{
//	CRoomBase::OnPeerLeave(peer) ;
//	if ( eRoomState_WaitPeerToGetReady == GetRoomState() )
//	{
//		if (  GetRoomPeerCount() < 2 )
//		{
//			SwitchToRoomSate(GetRoomState(),eRoomState_WaitPeerToJoin) ;
//			return ;
//		}
//
//		if ( GetReadyPeerCount() < 2 )
//		{
//			SwitchToRoomSate(GetRoomState(),eRoomState_WaitPeerToGetReady) ;
//			return ;
//		}
//
//	}
//}
//
//void CRoom::SwitchToRoomSate( eRoomState eFrom, eRoomState eToDest )
//{
//	switch ( eToDest )
//	{
//	case eRoomState_WaitPeerToJoin:
//		{
//			
//		}
//		break;
//	case eRoomState_WaitPeerToGetReady:
//		{
//			m_fRoomSateTick[eToDest] = TIME_ROOM_WAIT_READY ;
//		}
//		break;
//	case eRoomState_DistributeCard:
//		{
//			// send distribute msg , switch ready player's state to unkook ;
//			m_fRoomSateTick[eToDest] = TIME_ROOM_DISTRIBUTE ;
//			m_Poker.ComfirmKeepCard(GetReadyPeerCount() * 3 ) ;
//			for ( int i = 0 ; i < GetMaxSeat() ; ++i )
//			{
//				if (m_vRoomPeer[i] && eRoomPeer_Ready == m_vRoomPeer[i]->GetState() )
//				{
//					m_vRoomPeer[i]->OnGetCard(m_Poker.GetCardWithCompositeNum(),m_Poker.GetCardWithCompositeNum(),m_Poker.GetCardWithCompositeNum() );
//					m_vRoomPeer[i]->m_eState = eRoomPeer_Unlook ;
//				}
//			}
//			// decised new main idx ;
//			DecideMainPeerIdx();
//			stMsgDistributeCard msg ;
//			msg.nCurMainIdx = m_nCurMainPeerIdx ;
//			SendMsgRoomPeers(&msg,sizeof(msg)) ;
//		}
//		break;
//	case eRoomState_WaitPeerAction:
//		{
//			m_fRoomSateTick[eRoomState_WaitPeerAction] = TIME_ROOM_WAIT_PEER_ACTION ;
//			if ( eRoomState_PKing == eFrom )
//			{
//				NextPlayerAction();
//			}
//			else if ( eRoomState_DistributeCard == eFrom )
//			{
//				m_nCurWaitPeerIdx = m_nCurMainPeerIdx - 1 ;
//				NextPlayerAction() ;
//			}
//			else
//			{
//				CLogMgr::SharedLogMgr()->ErrorLog("Unknown state switch to eRoomState_WaitPeerAction, nFrom = %d",eFrom ) ;
//			}
//		}
//		break;
//	case eRoomState_PKing:
//		{
//			m_fRoomSateTick[eToDest] = TIME_ROOM_PK_DURATION ;
//		}
//		break;
//	case eRoomState_ShowingResult:
//		{
//			m_fRoomSateTick[eToDest] = TIME_ROOM_SHOW_RESULT ;
//			Rest();
//		}
//		break;
//	default:
//		{
//			CLogMgr::SharedLogMgr()->ErrorLog("switch unknown room state = %d", eToDest) ;
//			return ;
//		}
//		break; 
//	}
//	m_eRoomState = eToDest ;
//}
//
//void CRoom::NextPlayerAction()
//{
//	for ( int i = m_nCurWaitPeerIdx + 1 ; 1 ;++i )
//	{
//		int nIdx = i ;
//		if ( nIdx > GetMaxSeat() - 1 )
//		{
//			nIdx -= GetMaxSeat() ;
//		}
//		
//		if ( nIdx == m_nCurMainPeerIdx )
//		{
//			++m_nRound ;
//		}
//
//		if ( m_vRoomPeer[nIdx] && m_vRoomPeer[nIdx]->IsActive() )
//		{
//			m_nCurWaitPeerIdx = nIdx ;
//			break; 
//		}
//	}
//	stMsgRoomWaitPlayerAction msg ;
//	msg.nRound = m_nRound ;
//	msg.nSessionID = m_vRoomPeer[m_nCurWaitPeerIdx]->GetSessionID() ;
//	SendMsgRoomPeers(&msg,sizeof(msg)) ;
//	m_fRoomSateTick[eRoomState_WaitPeerAction] = TIME_ROOM_WAIT_PEER_ACTION ;
//}
//
//void CRoom::DecideMainPeerIdx()
//{
//	for ( m_nCurMainPeerIdx ; 1 ;++m_nCurMainPeerIdx )
//	{
//		if ( m_nCurMainPeerIdx > GetMaxSeat() - 1 )
//		{
//			m_nCurMainPeerIdx -= GetMaxSeat() ;
//		}
//
//		if ( m_vRoomPeer[m_nCurMainPeerIdx] && m_vRoomPeer[m_nCurMainPeerIdx]->IsActive() )
//		{
//			break; 
//		}
//	}
//}
//
//char CRoom::GetReadyPeerCount()
//{
//	char nCount = 0 ;
//	for ( int i = 0 ; i < GetMaxSeat() ; ++i )
//	{
//		if ( m_vRoomPeer[i] && m_vRoomPeer[i]->GetState() == eRoomPeer_Ready )
//		{
//			++nCount ;
//		}
//	}
//	return nCount ;
//}
//
//bool CRoom::CheckFinish()
//{
//	char nCount = 0 ;
//	CRoomPeer* peer = NULL ;
//	for ( int i = 0 ; i < GetMaxSeat() ; ++i )
//	{
//		if ( m_vRoomPeer[i] && m_vRoomPeer[i]->IsActive() )
//		{
//			++nCount ;
//			peer = m_vRoomPeer[i] ;
//			if ( nCount > 1 )
//			{
//				return false ;
//			}
//		}
//	}
//
//	if ( peer == NULL )
//	{
//		CLogMgr::SharedLogMgr()->ErrorLog("No winner , no") ;
//		return true ;
//	}
//
//	std::list<stResultData*> vRData ;
//	for ( int i = 0 ; i < GetMaxSeat() ; ++i )
//	{
//		if ( m_vRoomPeer[i] == NULL )
//		{
//			continue; 
//		}
//		if ( m_vRoomPeer[i]->GetState() == eRoomPeer_GiveUp || eRoomPeer_Failed == m_vRoomPeer[i]->GetState() )
//		{
//			stResultData* pData = new stResultData ;
//			pData->idx = i ;
//			pData->nResultCoin = m_vRoomPeer[i]->m_nBetCoin ;
//			pData->nResultCoin *= -1 ;
//			m_vRoomPeer[i]->m_PeerCard.GetCompositeCardRepresent(pData->vCard) ;
//			vRData.push_back(pData) ;
//		}
//		else  if ( m_vRoomPeer[i]->IsActive() ) // win ;
//		{
//			stResultData* pData = new stResultData ;
//			pData->idx = i ;
//			pData->nResultCoin = m_nTotalBetCoin ;
//			if ( m_nTotalBetCoin > TAX_START_COIN )
//			{
//				float nValue = (float)pData->nResultCoin * 0.95f ;
//				pData->nResultCoin = (int)nValue ;
//			}
//			m_vRoomPeer[i]->OnWinCoin(pData->nResultCoin) ;
//			m_vRoomPeer[i]->m_PeerCard.GetCompositeCardRepresent(pData->vCard) ;
//			vRData.push_back(pData) ;
//		}
//		else
//		{
//			CLogMgr::SharedLogMgr()->ErrorLog("what happen other state ") ;
//		}
//	}
//
//	stMsgRoomResult msgResult ;
//	msgResult.nCount = vRData.size();
//	char * pbuffer = new char[sizeof(msgResult) + vRData.size()*sizeof(stResultData)] ;
//	int nOffset = 0 ;
//	memcpy(pbuffer,&msgResult,sizeof(msgResult));
//	nOffset += sizeof(msgResult) ;
//
//	for (std::list<stResultData*>::iterator iter = vRData.begin() ; iter != vRData.end(); ++iter  )
//	{
//		stResultData* pData = *iter ;
//		memcpy(pbuffer+nOffset,pData,sizeof(stResultData));
//		nOffset += sizeof(stResultData) ;
//		delete pData ; // delete because have copyed to buffer ;
//		pData = NULL ;
//	}
//	SendMsgRoomPeers((stMsg*)pbuffer,nOffset ) ;
//	delete[] pbuffer ;
//
//	vRData.clear() ; // new data have been delete in uplown loop ;
//	return true;
//}
//
//void CRoom::DebugRoomInfo()
//{
//	CLogMgr::SharedLogMgr()->PrintLog("Room id = %d , totalBetCoin = %d , singleBetCoin = %d , roomState = %d , Round = %d , curIdx = %d , MainIdx = %d",m_nRoomID,m_nTotalBetCoin,m_nSingleBetCoin,GetRoomState(),m_nRound,m_nCurWaitPeerIdx,m_nCurMainPeerIdx )  ;
//}
//
//bool CRoom::OnPeerMsg(CRoomPeer* pPeer, stMsg* pmsg )
//{
//	if ( CRoomBase::OnPeerMsg(pPeer,pmsg) )
//	{
//		return true ;
//	}
//
//	switch ( pmsg->usMsgType )
//	{
//	case MSG_ROOM_READY:
//		{
//			if ( eRoomState_WaitPeerAction == GetRoomState() || eRoomState_DistributeCard == GetRoomState() )
//			{
//				stMsgRoomRet msgRet ;
//				msgRet.nRet = 1 ; // room state not fitable ;
//				pPeer->SendMsgToClient((char*)&msgRet, sizeof(msgRet)) ; 
//				return false ;
//			}
//
//			pPeer->m_eState = eRoomPeer_Ready ;
//			stMsgRoomPlayerReady msg ;
//			msg.nReadyPlayerSessionID = pPeer->GetSessionID() ;
//			SendMsgRoomPeers(&msg,sizeof(msg)) ;
//			if ( GetReadyPeerCount() == GetMaxSeat() )
//			{
//				SwitchToRoomSate(GetRoomState(),eRoomState_DistributeCard) ;
//			}
//			else if ( GetReadyPeerCount() >= 2 && GetRoomState() != eRoomState_ShowingResult )
//			{
//				SwitchToRoomSate(GetRoomState(),eRoomState_WaitPeerToGetReady ) ;
//			}
//			DebugRoomInfo();
//		}
//		return true; 
//	case MSG_ROOM_FOLLOW:
//		{
//			if ( pPeer->m_nPeerIdx != m_nCurWaitPeerIdx )
//			{
//				CLogMgr::SharedLogMgr()->PrintLog("it is not your turn , omit the action ") ;
//				return  false ;
//			}
//			if ( eRoomState_WaitPeerAction != GetRoomState() )
//			{
//				stMsgRoomRet msgRet ;
//				msgRet.nRet = 1 ; // room state not fitable ;
//				pPeer->SendMsgToClient((char*)&msgRet, sizeof(msgRet)) ; 
//				return false;
//			}
//			
//			int factor = pPeer->GetState() == eRoomPeer_Look ? 2 : 1 ;
//			unsigned int naddBet = pPeer->AddBetCoin(m_nSingleBetCoin*factor) ; 
//			m_nTotalBetCoin += naddBet ;
//
//			stMsgRoomPlayerFollow msgFollow ;
//			msgFollow.nSessionID = pPeer->GetSessionID() ;
//			SendMsgRoomPeers(&msgFollow,sizeof(msgFollow)) ;
//			DebugRoomInfo();
//			NextPlayerAction();
//		}
//		return true; 
//	case MSG_ROOM_ADD:
//		{
//			if ( pPeer->m_nPeerIdx != m_nCurWaitPeerIdx )
//			{
//				CLogMgr::SharedLogMgr()->PrintLog("it is not your turn , omit the action ") ;
//				return  false ;
//			}
//			if ( eRoomState_WaitPeerAction != GetRoomState() )
//			{
//				stMsgRoomRet msgRet ;
//				msgRet.nRet = 1 ; // room state not fitable ;
//				pPeer->SendMsgToClient((char*)&msgRet, sizeof(msgRet)) ; 
//				return false;
//			}
//
//			// update single 
//			stMsgRoomAdd* msgAdd = (stMsgRoomAdd*)pmsg ;
//			if ( msgAdd->nAddMoney == 0 ) // double ;
//			{
//				m_nSingleBetCoin *= 2 ;
//			}
//			else if ( (unsigned int )msgAdd->nAddMoney < m_nSingleBetCoin )
//			{
//				stMsgRoomRet msgRet ;
//				msgRet.nRet = 2 ; // add money should greate than crrent ;
//				pPeer->SendMsgToClient((char*)&msgRet, sizeof(msgRet)) ; 
//				return false;
//			}
//			else
//			{
//				m_nSingleBetCoin = msgAdd->nAddMoney ;
//			}
//
//			// update player self ;
//			int factor = pPeer->GetState() == eRoomPeer_Look ? 2 : 1 ;
//			unsigned int naddBet = pPeer->AddBetCoin(m_nSingleBetCoin*factor) ; 
//			m_nTotalBetCoin += naddBet ;
//
//			stMsgRoomPlayerAdd msgPlayerAdd ;
//			msgPlayerAdd.nBetCoin = naddBet ;
//			msgPlayerAdd.nNewSingle = m_nSingleBetCoin ;
//			msgPlayerAdd.nSessionID = pPeer->GetSessionID() ;
//			SendMsgRoomPeers(&msgPlayerAdd,sizeof(msgPlayerAdd)) ;
//			NextPlayerAction();
//			DebugRoomInfo() ;
//		}
//		return true;
//	case MSG_ROOM_PK:
//		{
//			if ( pPeer->m_nPeerIdx != m_nCurWaitPeerIdx )
//			{
//				CLogMgr::SharedLogMgr()->PrintLog("it is not your turn , omit the action ") ;
//				return  false ;
//			}
//			if ( eRoomState_WaitPeerAction != GetRoomState() )
//			{
//				stMsgRoomRet msgRet ;
//				msgRet.nRet = 1 ; // room state not fitable ;
//				pPeer->SendMsgToClient((char*)&msgRet, sizeof(msgRet)) ; 
//				return false;
//			}
//			stMsgRoomPK* pkMsg = (stMsgRoomPK*)pmsg ;
//			CRoomPeer* PKpeer = GetRoomPeerBySessionID(pkMsg->nPKWithSessionID );
//			if ( !PKpeer || PKpeer->IsActive() == false )
//			{
//				stMsgRoomRet msgRet ;
//				msgRet.nRet = 3 ; // unlegal target ;
//				pPeer->SendMsgToClient((char*)&msgRet, sizeof(msgRet)) ; 
//				return false;
//			}
//
//			bool bWin = pPeer->m_PeerCard.PKPeerCard(&PKpeer->m_PeerCard) ;
//			if ( bWin )
//			{
//				PKpeer->m_eState = eRoomPeer_Failed ;
//			}
//			else
//			{
//				pPeer->m_eState = eRoomPeer_Failed ;
//			}
//
//			int factor = pPeer->GetState() == eRoomPeer_Look ? 2 : 1 ;
//			unsigned int nConsum = m_nSingleBetCoin * factor * 2 ;
//			unsigned int naddBet = pPeer->AddBetCoin(nConsum) ; 
//			m_nTotalBetCoin += naddBet ;
//
//			stMsgRoomPlayerPK msgplayerpk  ;
//			msgplayerpk.bWin = bWin ;
//			msgplayerpk.nConsumCoin = naddBet ;
//			msgplayerpk.nPKInvokeSessionID = pPeer->GetSessionID() ;
//			msgplayerpk.nPKWithSessionID = PKpeer->GetSessionID() ;
//			SendMsgRoomPeers(&msgplayerpk,sizeof(msgplayerpk)) ;
//			SwitchToRoomSate(GetRoomState(),eRoomState_PKing) ;
//			DebugRoomInfo() ;
//		}
//		return true ;
//	case MSG_ROOM_LOOK:
//		{
//			if ( eRoomPeer_Unlook != pPeer->GetState() )
//			{
//				stMsgRoomRet msgRet ;
//				msgRet.nRet = 1 ; // room state not fitable ;
//				pPeer->SendMsgToClient((char*)&msgRet, sizeof(msgRet)) ; 
//				return false;
//			}
//
//			pPeer->m_eState = eRoomPeer_Look ; 
//			stMsgRoomPlayerLook msgLook ;
//			msgLook.nSessionID = pPeer->GetSessionID() ;
//			pPeer->m_PeerCard.GetCompositeCardRepresent(msgLook.vCard);
//			SendMsgRoomPeers(&msgLook,sizeof(msgLook)) ;
//		}
//		return true;
//	case MSG_ROOM_GIVEUP:
//		{
//			if ( pPeer->IsActive() == false )
//			{
//				stMsgRoomRet msgRet ;
//				msgRet.nRet = 1 ; // room state not fitable ;
//				pPeer->SendMsgToClient((char*)&msgRet, sizeof(msgRet)) ; 
//				return false;
//			}
//			pPeer->m_eState = eRoomPeer_GiveUp ;
//
//			stMsgRoomPlayerGiveUp msgGiveUp ;
//			msgGiveUp.nIdx = pPeer->m_nPeerIdx ;
//			SendMsgRoomPeers(&msgGiveUp,sizeof(msgGiveUp)) ;	
//			if ( CheckFinish() )
//			{
//				SwitchToRoomSate(GetRoomState(),eRoomState_ShowingResult) ;
//			}
//			else
//			{
//				if ( m_nCurWaitPeerIdx == pPeer->m_nPeerIdx )
//				{
//					NextPlayerAction();
//				}
//			}
//		}
//		return true; 
//	}
//	return false ;
//}
