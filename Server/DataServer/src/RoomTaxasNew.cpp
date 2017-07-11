#include "RoomTaxasNew.h"
#include "TaxasRoomData.h"
#include "LogManager.h"
#include "Player.h"
#include "TaxasMessageDefine.h"
#include "PlayerBaseData.h"
#include <cassert>
#include "RoomConfig.h"
bool CRoomTaxasNew::Init(stBaseRoomConfig* pConfig)
{
	m_pRoomData = new CTaxasRoomData ;
	m_pRoomData->Init();
	CRoomBaseNew::Init(pConfig);

	
	stTaxasRoomDataSimple* pTemp = GetRoomData()->GetSimpleData() ;
	stTaxasRoomConfig* pGC = (stTaxasRoomConfig*)pConfig ;
	pTemp->cCurRoomState = eRoomState_TP_WaitJoin ;
	pTemp->nBigBlindBet = pGC->nBigBlind ;
	pTemp->nMaxTakeIn = pGC->nMaxTakeInCoin ;

	if ( pConfig->nMinNeedToEnter % pGC->nBigBlind != 0 || pGC->nMaxTakeInCoin % pGC->nBigBlind != 0 )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("taxas room config error id = %d , take in coin not times bigblind bet",pConfig->nRoomID );
	}
	return true ;
}

void CRoomTaxasNew::Enter(CPlayer* pEnter )
{
	CRoomBaseNew::Enter(pEnter);
	SendRoomInfoToPlayer(pEnter);
}

void CRoomTaxasNew::Leave(CPlayer* pLeaver)
{
	stTaxasPeerData* pPlayer = (stTaxasPeerData*)GetRoomData()->GetPeerDataBySessionID(pLeaver->GetSessionID());
	if ( pPlayer )
	{
		//StandUp(pLeaver);
		stMsgTaxasPlayerStandUp msg;
		OnMessage(pLeaver,&msg);
	}
	CRoomBaseNew::Leave(pLeaver);
}

void CRoomTaxasNew::Update(float fTimeElpas, unsigned int nTimerID )
{
	CRoomBaseNew::Update(fTimeElpas,nTimerID);
	GetData()->m_pData->fTimeTick += fTimeElpas ;
	switch ( GetData()->m_pData->cCurRoomState )
	{
	case eRoomState_TP_WaitJoin:
		{
			if ( GetRoomData()->GetPlayingSeatCnt() > 2 )
			{
				GetRoomData()->OnStartGame();
				// send inform start game  msg ;
				stMsgTaxasRoomGameStart cardMsg ;
				cardMsg.nPeerCnt = ((CTaxasRoomData*)GetData())->GetPlayerCntWithState(eRoomPeer_CanAct) ;
				memcpy(cardMsg.vPublicCard,GetRoomData()->GetSimpleData()->vPublicCardNums,sizeof(cardMsg.vPublicCard));
				unsigned short nMaxSize = sizeof(cardMsg) + cardMsg.nPeerCnt * sizeof(stTaxasHoldCardItems) ;

				char* pBuffer = new char[nMaxSize] ;
				memset(pBuffer,0,nMaxSize) ;

				unsigned short nContentSize = 0 ;
				memcpy(pBuffer,&cardMsg,sizeof(cardMsg));
				nContentSize += sizeof(cardMsg);

				nContentSize += GetRoomData()->GetAllPeersHoldCardToBuffer(pBuffer + nContentSize) ;
				assert(nContentSize == nMaxSize && "why the size not equal?" );
				SendMsgBySessionID((stMsg*)&pBuffer,nContentSize);
				delete[] pBuffer ;

				// change state 
				GotoState(eRoomState_TP_Player_Distr);
			}
		}
		break;
	case eRoomState_TP_Player_Distr:
		{
			if ( GetData()->m_pData->fTimeTick >= GetRoomData()->GetCurStateTime() )
			{	
				if ( GetRoomData()->CheckThisRoundEnd() )
				{
					GotoState(eRoomState_TP_Caculate_Round);
				}
				else
				{
					GotoState(eRoomState_TP_Wait_Bet);
				}
			}
		}
		break;
	case eRoomState_TP_Wait_Bet:
		{
			if ( GetData()->m_pData->fTimeTick >= GetRoomData()->GetCurStateTime() )
			{
				stTaxasPeerData* pPlayer = (stTaxasPeerData*)GetRoomData()->GetPeerDataByIdx(GetRoomData()->GetSimpleData()->cCurWaitPlayerActionIdx);
				if ( !pPlayer )
				{
					CLogMgr::SharedLogMgr()->ErrorLog("why cur act player is NULL");
					NextPlayerAct();
				}
				else
				{
					// if can pass just pass
					uint64_t nMost = GetRoomData()->GetSimpleData()->nMostBetCoinThisRound ;
					if ( nMost == pPlayer->nBetCoinThisRound )
					{
						stMsgTaxasPlayerPass msg ;
						OnMessage(GetPlayerByRoomIdx(pPlayer->cRoomIdx),&msg) ;
					}
					else
					{
						stMsgTaxasPlayerGiveUp msg ;
						OnMessage(GetPlayerByRoomIdx(pPlayer->cRoomIdx),&msg) ;
					}
				}
			}
		}
		break;
	case eRoomState_TP_Caculate_Round:
		{
			if ( GetData()->m_pData->fTimeTick >= GetRoomData()->GetCurStateTime() )
			{
				GetRoomData()->OnEndThisRound();
				if ( GetRoomData()->CheckGameOver() )
				{
					GotoState(eRoomState_TP_Caculate_GameResult);
				}
				else
				{
					GotoState(eRoomState_TP_Player_Distr);
				}
			}
		}
		break;
	case eRoomState_TP_Caculate_GameResult:
		{
			if ( GetData()->m_pData->fTimeTick >= GetRoomData()->GetCurStateTime() )
			{
				GetData()->OnEndGame();
				CheckDelayedKickedPlayer();
				ProcessAutoTakeInAndAutoStandUp();
				GotoState(eRoomState_TP_WaitJoin);
			}
		}
		break;
	default:
		CLogMgr::SharedLogMgr()->ErrorLog("Unknown taxas room state = %d",GetData()->m_pData->cCurRoomState );
	}
}

void CRoomTaxasNew::GotoState(unsigned char TargetState)
{
	unsigned char nPreState = GetData()->m_pData->cCurRoomState ;
	GetData()->SetRoomState(TargetState);
	GetData()->m_pData->fTimeTick = 0 ;
	
	stMsgTaxasRoomNewState msg ;
	msg.cNewState = TargetState ;
	SendMsgBySessionID(&msg,sizeof(msg));

	switch ( GetData()->m_pData->cCurRoomState )
	{
	case eRoomState_TP_WaitJoin:
		{

		}
		break;
	case eRoomState_TP_Player_Distr:
		{
			if ( GetRoomData()->OnDistributeCard() != 1 )
				break;
		}
		break;
	case eRoomState_TP_Wait_Bet:
		{
			NextPlayerAct();
		}
		break;
	case eRoomState_TP_Caculate_Round:
		{
			GetRoomData()->OnCaculateRound();
		}
		break;
	case eRoomState_TP_Caculate_GameResult:
		{
			GetRoomData()->OnCaculateGameResult();
		}
		break;
	default:
		CLogMgr::SharedLogMgr()->ErrorLog("GotoState Unknown taxas room state = %d",GetData()->m_pData->cCurRoomState );
		return ;
	}
}

bool CRoomTaxasNew::OnMessage(CPlayer*pSender, stMsg* pmsg)
{
	if ( CRoomBaseNew::OnMessage(pSender,pmsg) )
	{
		return true ;
	}
	unsigned char idx = SESSION_ID_TO_IDX(pSender->GetSessionID());
	switch ( pmsg->usMsgType )
	{
	case MSG_TAXAS_PLAYER_SITDOWN:
		{
			stMsgTaxasPlayerSitDownRet msgBack ;
			msgBack.nRet = 0 ;

			stMsgTaxasPlayerSitDown* pSitdown = (stMsgTaxasPlayerSitDown*)pmsg ;
			if ( pSitdown->nTakeInCoin > pSender->GetBaseData()->GetAllCoin() )
			{
				msgBack.nRet = 1 ;
				pSender->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
				break; 
			}

			if ( GetRoomData()->GetEmptySeatCnt() <= 0 )
			{
				msgBack.nRet = 2 ;
				pSender->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
				break; 
			}

			if ( GetRoomData()->GetSimpleData()->cMiniCoinNeedToEnter > pSitdown->nTakeInCoin )
			{
				msgBack.nRet = 3 ;
				pSender->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
				break; 
			}

			if ( pSitdown->nTakeInCoin % GetRoomData()->GetSimpleData()->nBigBlindBet != 0 )
			{
				msgBack.nRet = 4 ;
				pSender->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
				break; 
			}

			if ( pSitdown->nTakeInCoin > GetRoomData()->GetSimpleData()->nMaxTakeIn )
			{
				msgBack.nRet = 5 ;
				pSender->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
				break; 
			}

			stTaxasPeerData* pData = new stTaxasPeerData ;
			memcpy(pData->cName,pSender->GetBaseData()->GetData()->cName,sizeof(pData->cName));
			pData->cVipLevel = pSender->GetBaseData()->GetVipLevel();
			pData->eCurAct = eRoomPeerAction_None ;
			pData->nAllBetCoin = 0 ;
			pData->nWinCoinFromPools = 0 ;
			pData->nBetCoinThisRound = 0 ;
			pData->nCurCoin = pSitdown->nTakeInCoin ;
			pSender->GetBaseData()->ModifyMoney(pSitdown->nTakeInCoin * -1) ;
			pData->nDefaultPhotoID = pSender->GetBaseData()->GetData()->nDefaulPhotoID;
			pData->nDiamond = pSender->GetBaseData()->GetAllDiamoned();
			pData->nPeerState = eRoomPeer_WaitNextGame ;
			pData->nSessionID = pSender->GetSessionID();
			pData->nUserDefinePhotoID = pSender->GetBaseData()->GetData()->nUserUID;
			pData->nUserUID = pSender->GetBaseData()->GetData()->nUserUID;
			memset(pData->vHoldCard,0,sizeof(pData->vHoldCard)) ;
			GetRoomData()->AddPeer(pData,pSitdown->nIdx);

			stMsgTaxasRoomSitDown msgAll ;
			memcpy(&msgAll.vPlayerSitDown,pData,sizeof(msgAll.vPlayerSitDown));
			SendMsgBySessionID(&msgAll,sizeof(msgAll));
		}
		break;
	case MSG_TAXAS_PLAYER_STANDUP:
		{
			uint64_t nTakeCoin = 0 ;
			if ( GetData()->GetPeerDataByIdx(idx) )
			{
				nTakeCoin = GetData()->GetPeerDataByIdx(idx)->nCurCoin;
			}

			stMsgTaxasPlayerStandUpRet msgBack;
			msgBack.cRet = GetRoomData()->OnPlayerStandup(idx) ;
			if ( msgBack.cRet == 0 )
			{
				pSender->GetBaseData()->ModifyMoney(nTakeCoin);
				stMsgTaxasRoomStandUp msgAll ;
				msgAll.nPlayerIdx = idx ;
				SendMsgBySessionID(&msgAll,sizeof(msgAll)) ;
			}
			pSender->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
		}
		break;
	case MSG_TAXAS_PLAYER_FOLLOW:
		{
			unsigned char nidx = GetData()->GetRoomIdxBySessionID(pSender->GetSessionID());
			stMsgTaxasPlayerFollowRet msgBack ;
			msgBack.nRet = GetRoomData()->OnPlayerFollow(nidx) ;
			pSender->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;

			if ( msgBack.nRet == 0 )
			{
				stMsgTaxasRoomFollow msgAll ;
				msgAll.nPlayerIdx = nidx ;
				SendMsgBySessionID(&msgAll,sizeof(msgAll));
			}
			pSender->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
		}
		break;
	case MSG_TAXAS_PLAYER_ADD:
		{
			stMsgTaxasPlayerAdd* pRet = (stMsgTaxasPlayerAdd*)pmsg ;
			stMsgTaxasPlayerAddRet msgBack ;
			msgBack.nRet = GetRoomData()->OnPlayerAdd(idx,pRet->nAddCoin);
			if ( msgBack.nRet == 0 )
			{
				stMsgTaxasRoomAdd msgAll ;
				msgAll.nAddCoin = pRet->nAddCoin ;
				msgAll.nPlayerIdx = idx ;
				SendMsgBySessionID(&msgAll,sizeof(msgAll));
			}
			pSender->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
		}
		break;
	case MSG_TAXAS_PLAYER_PASS:
		{
			stMsgTaxasPlayerPassRet msgBack ;
			msgBack.nRet = GetRoomData()->OnPlayerPass(idx);
			if ( msgBack.nRet == 0 )
			{
				stMsgTaxasRoomPass msgAll ;
				msgAll.nPlayerIdx = idx;
				SendMsgBySessionID(&msgAll,sizeof(msgAll));
			}
			pSender->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
		}
		break;
	case MSG_TAXAS_PLAYER_ALLIN:
		{
			stMsgTaxasPlayerAllInRet msgBack ;
			msgBack.nRet = GetRoomData()->OnPlayerAllIn(idx);
			if ( msgBack.nRet == 0 )
			{
				stMsgTaxasRoomAllIn msgAll ;
				msgAll.nPlayerIdx = idx ;
				SendMsgBySessionID(&msgAll,sizeof(msgAll)) ;
			}
			pSender->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
		}
		break;
	case MSG_TAXAS_PLAYER_GIVEUP:
		{
			stMsgTaxasPlayerGiveUpRet msgBack;
			msgBack.nRet = GetRoomData()->OnPlayerGiveUp(idx) ;
			if ( 0 == msgBack.nRet )
			{
				stMsgTaxasRoomGiveUp msgAll ;
				msgAll.nPlayerIdx = idx ;
				SendMsgBySessionID(&msgAll,sizeof(msgAll)) ;

				CheckDelayedKickedPlayer();
			}
			pSender->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
		}
		break;
	default:
		return false;
	}

	if ( GetRoomData()->CheckThisRoundEnd() )
	{
		 GotoState(eRoomState_TP_Caculate_Round) ;
		 return true;
	}

	if ( idx == GetRoomData()->GetSimpleData()->cCurWaitPlayerActionIdx )
	{
		NextPlayerAct();
	}
	return true ;
}

void CRoomTaxasNew::SitDown(CPlayer* pPlayer )
{

}

void CRoomTaxasNew::StandUp(CPlayer* pPlayer)
{
// 	stTaxasPeerData* peerdata = (stTaxasPeerData*)GetRoomData()->GetPeerDataBySessionID(pPlayer->GetSessionID()) ;
// 	if ( !peerdata )
// 	{
// 		CLogMgr::SharedLogMgr()->ErrorLog("you are not sit down ,why stand up ?");
// 		return ;
// 	}
// 
// 	pPlayer->GetBaseData()->ModifyMoney(peerdata->nCurCoin);
// 	peerdata->nCurCoin = 0 ;
// 
// 	bool bCurAct = peerdata->cRoomIdx == GetRoomData()->GetSimpleData()->cCurWaitPlayerActionIdx ;
// 	GetRoomData()->OnPlayerStandup(peerdata->cRoomIdx) ;
// 	if ( bCurAct )
// 	{
// 		NextPlayerAct();
// 	}
// 	else
// 	{
// 		if ( GetRoomData()->CheckThisRoundEnd() )
// 		{
// 			GotoState(eRoomState_TP_Caculate_Round);
// 		}
// 	}
}

void CRoomTaxasNew::SendRoomInfoToPlayer(CPlayer* pPlayer)
{
	stMsgTaxasRoomInfo msg;
	memcpy(&msg.tSimpleData,GetRoomData()->GetSimpleData(),sizeof(msg.tSimpleData));
	msg.nVicePoolCnt = GetRoomData()->m_vAllVicePools.size();
	msg.nPeerCnt = GetRoomData()->GetPlayingSeatCnt() ;
	msg.nKickCnt = GetData()->GetKickPlayerCnt();
	
	unsigned short nMaxBufferSize = sizeof(msg) + sizeof(stTaxasPeerData)* msg.nPeerCnt + ( sizeof(stVicePoolItem) + sizeof(unsigned char) * msg.nPeerCnt ) * msg.nVicePoolCnt + sizeof(unsigned int) * msg.nKickCnt;
	char* pBuffer = new char[nMaxBufferSize] ;
	memset(pBuffer,0,nMaxBufferSize) ;
	
	unsigned short nContentSize = 0 ;
	memcpy(pBuffer + nContentSize,&msg,sizeof(msg));
	nContentSize += sizeof(msg);

	nContentSize += GetRoomData()->GetAllPeerDataToBuffer(pBuffer + nContentSize );
	nContentSize += GetRoomData()->GetAllVicePoolToBuffer(pBuffer + nContentSize) ;
	nContentSize += GetRoomData()->GetKickSessionIDsToBuffer(pBuffer + nContentSize) ;
	assert(nContentSize <= nMaxBufferSize && "buffer caculate error");
	pPlayer->SendMsgToClient(pBuffer,nContentSize) ;
	delete[] pBuffer;
}

CTaxasRoomData* CRoomTaxasNew::GetRoomData()
{
	return (CTaxasRoomData*)GetData();
}

void CRoomTaxasNew::NextPlayerAct()
{
	if ( GetRoomData()->GetSimpleData()->cCurRoomState != eRoomState_TP_Wait_Bet )
	{
		return  ;
	}

	if ( GetRoomData()->CheckThisRoundEnd() )
	{
		GotoState(eRoomState_TP_Caculate_Round) ;
	}
	else
	{
		stMsgTaxasRoomWaitPlayerAct msg ;
		msg.nActPlayerIdx = GetRoomData()->OnWaitNextPlayerAct();
		SendMsgBySessionID(&msg,sizeof(msg));
	}
}

void CRoomTaxasNew::ProcessAutoTakeInAndAutoStandUp()
{
	std::vector<unsigned char> vIdxs ;
	GetRoomData()->GetNeedAutoTakeInPeerIdxs(vIdxs);
	if ( vIdxs.empty() )
	{
		return ;
	}

	stMsgTaxasRoomAutoTakeIn msg ;
	msg.nAutoTakeInPeerCnt = vIdxs.size();
	char* pBuffer = new char[sizeof(msg) + sizeof(stAutoTakeInItem) * msg.nAutoTakeInPeerCnt ] ;
	memcpy(pBuffer,&msg,sizeof(msg));
	stAutoTakeInItem* pItem = (stAutoTakeInItem*)(pBuffer + sizeof(msg));

	for ( unsigned int i = 0 ; i < vIdxs.size(); ++i )
	{
		unsigned char nPeerIdx = vIdxs[i] ;
		pItem->nIdx = i ;
		CPlayer* pPlayer = GetPlayerByRoomIdx(nPeerIdx) ;
		if ( pPlayer == NULL )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("why this is NULL player autotakein") ;
			pItem->nAutoTakeInCoin = 0 ;
			++pItem ;
			continue;
		}

		uint64_t nTakeIn = GetRoomData()->GetSimpleData()->nMaxTakeIn <= pPlayer->GetBaseData()->GetAllCoin() ? GetRoomData()->GetSimpleData()->nMaxTakeIn : pPlayer->GetBaseData()->GetAllCoin() ;
		nTakeIn = nTakeIn - nTakeIn % GetRoomData()->GetSimpleData()->nBigBlindBet;   // must times big blindbet ;
		pItem->nAutoTakeInCoin = nTakeIn >= GetRoomData()->GetSimpleData()->cMiniCoinNeedToEnter ? nTakeIn : 0 ;
		GetRoomData()->OnPlayerAutoTakeIn(pItem->nIdx,pItem->nAutoTakeInCoin);
		pPlayer->GetBaseData()->ModifyMoney(nTakeIn*-1);
		
		++pItem ;
	}
	SendMsgBySessionID((stMsg*)pBuffer,((char*)pItem) - pBuffer );
	delete[] pBuffer ;
	pBuffer = NULL ;
}
