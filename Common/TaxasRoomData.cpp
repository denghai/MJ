#include "TaxasRoomData.h"
#include "TaxasMessageDefine.h"
#include "LogManager.h"
#include <algorithm>
#include "TaxasPokerPeerCard.h"
void CTaxasRoomData::Init()
{
	tPoker.InitTaxasPoker();

	m_pData = new stTaxasRoomDataSimple  ;
	memset(m_pData,0,sizeof(stTaxasRoomDataSimple));
	m_pData->cGameType = eRoom_TexasPoker ;

	stTaxasRoomDataSimple* pData = (stTaxasRoomDataSimple*)m_pData ;
	pData->cCurWaitPlayerActionIdx = -1 ;
	pData->cBankerIdx = -1 ;
	pData->nBetRound = 0 ;
	pData->cCurRoomState = eRoomState_TP_WaitJoin ;
}

void CTaxasRoomData::InitDataFromMsg(stMsg*pmsg)
{
	if ( pmsg->usMsgType != MSG_TAXAS_ROOM_INFO )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("this msg id = %d ,should process here",pmsg->usMsgType);
		return ;
	}

	stMsgTaxasRoomInfo* pInfo = (stMsgTaxasRoomInfo*)pmsg ;
	memcpy(m_pData,&pInfo->tSimpleData,sizeof(pInfo->tSimpleData));
	
	// add peer ;
	unsigned char* pBuffer = (unsigned char*)pmsg ;
	pBuffer += sizeof(stMsgTaxasRoomInfo);
	stTaxasPeerData* pPeerData = (stTaxasPeerData*)pBuffer;
	for ( int i = 0 ; i < pInfo->nPeerCnt; ++i )
	{
		stTaxasPeerData* ptemp = new stTaxasPeerData ;
		memcpy(ptemp,pPeerData++,sizeof(stTaxasPeerData));
		AddPeer(ptemp) ;
	}

	// add vice pools 
	stVicePoolItem* pItem = (stVicePoolItem*)pPeerData ;
	for ( int i = 0 ; i < pInfo->nVicePoolCnt ; ++i )
	{
		stVicePool* pPool = new stVicePool;
		m_vAllVicePools.push_back(pPool);
		pPool->Idx = pItem->nPoolIdx;
		pPool->nCoinInPool = pItem->nCoinInPool ;
		unsigned char nCnt = pItem->nPeerCnt ;
		++pItem ;

		unsigned char* pIdx = (unsigned char*)pItem;
		while ( nCnt-- )
		{
			pPool->vPeerIdxInPool.push_back(*pIdx);
			++pIdx ;
		}

		pItem = (stVicePoolItem*)pIdx ;
	}

	// init kicked 
	unsigned int* pKickedId = (unsigned int*)pItem ;
	for ( int i = 0 ; i < pInfo->nKickCnt; ++i )
	{
		m_vSessionIDs.push_back(*pKickedId) ;
		++pKickedId ;
	}
}

void CTaxasRoomData::OnStartGame(stMsg*pmsg)
{
	CRoomBaseData::OnStartGame();
	ClearVicePool();

	stTaxasRoomDataSimple* pRoomData = GetSimpleData();
	pRoomData->cCurWaitPlayerActionIdx = -1 ;
	pRoomData->ePrePeerAction = eRoomPeerAction_None;
	pRoomData->fTimeTick = 0 ;
	pRoomData->nBetRound = 0 ;
	pRoomData->nCurMainBetPool = 0 ;
	pRoomData->nMostBetCoinThisRound = 0 ;
	memset(pRoomData->vPublicCardNums,0,sizeof(pRoomData->vPublicCardNums)) ;

	for ( int i = 0 ; i < pRoomData->cMaxPlayingPeers ; ++i )
	{
		stTaxasPeerData* pPeerData = (stTaxasPeerData*)m_vPeerDatas[i] ;
		if ( !pPeerData )
		{
			continue;
		}
		pPeerData->eCurAct = eRoomPeerAction_None;
		pPeerData->nAllBetCoin = 0 ;
		pPeerData->nWinCoinFromPools = 0 ;
		pPeerData->nBetCoinThisRound = 0 ;
		if ( pPeerData->nCurCoin < pRoomData->cMiniCoinNeedToEnter )
		{
			pPeerData->nPeerState = eRoomPeer_WaitNextGame ;
		}
		else
		{
			pPeerData->nPeerState = eRoomPeer_CanAct ;
		}
		memset(pPeerData->vHoldCard,0,sizeof(pPeerData->vHoldCard)) ;
	}
	// decide banker
	pRoomData->cBankerIdx = GetFirstPlayerIdxWithState( ++pRoomData->cBankerIdx,eRoomPeer_StayThisRound );

	// decide little idx 
	pRoomData->cLittleBlindIdx = GetFirstPlayerIdxWithState( 1 + pRoomData->cBankerIdx,eRoomPeer_StayThisRound);
	pRoomData->cBigBlindIdx = GetFirstPlayerIdxWithState( 1 + pRoomData->cLittleBlindIdx,eRoomPeer_StayThisRound);

	// first bet ;
	((stTaxasPeerData*)m_vPeerDatas[pRoomData->cLittleBlindIdx])->nBetCoinThisRound = pRoomData->nBigBlindBet * 0.5 ;
	((stTaxasPeerData*)m_vPeerDatas[pRoomData->cLittleBlindIdx])->nCurCoin -= pRoomData->nBigBlindBet * 0.5 ;

	((stTaxasPeerData*)m_vPeerDatas[pRoomData->cBigBlindIdx])->nBetCoinThisRound = pRoomData->nBigBlindBet ;
	((stTaxasPeerData*)m_vPeerDatas[pRoomData->cBigBlindIdx])->nCurCoin -= pRoomData->nBigBlindBet ;
	pRoomData->nMostBetCoinThisRound = pRoomData->nBigBlindBet;
	
	// prepare card ;
#ifdef GAME_SERVER
	// rest card 
	tPoker.RestAllPoker();
	if ( GetSimpleData()->nBetRound == 1 )
	{
		// peer hold card
		for ( int iHoldeCard = 0 ; iHoldeCard < TAXAS_PEER_CARD ; ++iHoldeCard )
		{
			for ( int iPeerIdx = 0 ; iPeerIdx < m_pData->cMaxPlayingPeers ; ++iPeerIdx )
			{
				stTaxasPeerData* pPeer = (stTaxasPeerData*)m_vPeerDatas[iPeerIdx];
				if ( pPeer == NULL || (pPeer->nPeerState & eRoomPeer_CanAct) != eRoomPeer_CanAct )
				{
					continue;
				}
				pPeer->vHoldCard[iHoldeCard] = tPoker.GetCardWithCompositeNum();
			}
		}

		// public card ;
		for ( int iPublicIdx = 0 ; iPublicIdx < TAXAS_PUBLIC_CARD ; ++iPublicIdx )
		{
			GetSimpleData()->vPublicCardNums[iPublicIdx] = tPoker.GetCardWithCompositeNum() ;
		}
	}
#else
	OnRecievedCardInfos(pmsg);
#endif

}

void CTaxasRoomData::OnEndGame()
{
	CRoomBaseData::OnEndGame();
	
	stTaxasRoomDataSimple* pRoomData = GetSimpleData();
	pRoomData->cCurWaitPlayerActionIdx = -1 ;
	pRoomData->ePrePeerAction = eRoomPeerAction_None;
	pRoomData->fTimeTick = 0 ;
	pRoomData->nBetRound = 0 ;
	pRoomData->nCurMainBetPool = 0 ;
	pRoomData->nMostBetCoinThisRound = 0 ;
	memset(pRoomData->vPublicCardNums,0,sizeof(pRoomData->vPublicCardNums)) ;

	for ( int i = 0 ; i < pRoomData->cMaxPlayingPeers ; ++i )
	{
		stTaxasPeerData* pPeerData = (stTaxasPeerData*)m_vPeerDatas[i] ;
		if ( !pPeerData )
		{
			continue;
		}
		pPeerData->eCurAct = eRoomPeerAction_None;
		pPeerData->nAllBetCoin = 0 ;
		pPeerData->nWinCoinFromPools = 0 ;
		pPeerData->nBetCoinThisRound = 0 ;
		pPeerData->nPeerState = eRoomPeer_WaitNextGame ;
		memset(pPeerData->vHoldCard,0,sizeof(pPeerData->vHoldCard)) ;
	}

	// clear vice pool
	ClearVicePool();
}

float CTaxasRoomData::GetCurStateTime()
{
	switch ( GetSimpleData()->cCurRoomState )
	{
	case eRoomState_TP_WaitJoin:
		{
			return 0 ;
		}
		break;
	case eRoomState_TP_Player_Distr:
		{
			if ( 1 == GetSimpleData()->nBetRound )
			{
				return TIME_TAXAS_DISTRIBUTE_PER_PEER * GetPlayerCntWithState(eRoomPeer_StayThisRound);
			}
			
			if ( 2 == GetSimpleData()->nBetRound )
			{
				return TIME_TAXAS_ONE_CARD_DISTRIBUTE * 3;
			}

			return TIME_TAXAS_ONE_CARD_DISTRIBUTE ;
		}
		break;
	case eRoomState_TP_Wait_Bet:
		{
			return GetSimpleData()->fOperateTime ;
		}
		break;
	case eRoomState_TP_Caculate_Round:
		{
			return TIME_TAXAS_WAIT_COIN_GOTO_MAIN_POOL ;
		}
		break;
	case eRoomState_TP_Caculate_GameResult:
		{
			return TIME_TAXAS_CACULATE_FINAL ;
		}
		break;
	default:
		CLogMgr::SharedLogMgr()->ErrorLog("GetCurStateTime Unknown taxas room state = %d",GetSimpleData()->cCurRoomState );
	}
	return 0 ;
}

stTaxasRoomDataSimple* CTaxasRoomData::GetSimpleData()
{
	return (stTaxasRoomDataSimple*)m_pData ;
}

bool CTaxasRoomData::CheckThisRoundEnd()
{
	if ( GetSimpleData()->cCurRoomState != eRoomState_TP_Wait_Bet )
	{
		return false ;
	}

	if ( GetPlayerCntWithState(eRoomPeer_WaitCaculate) <= 2  )
		return true ;
	bool bOnlyOneAct = GetPlayerCntWithState(eRoomPeer_CanAct) <= 1;

	for ( int i = 0 ; i < GetSimpleData()->cMaxPlayingPeers ; ++i )
	{
		stTaxasPeerData* pData = (stTaxasPeerData*)m_vPeerDatas[i] ;
		if ( pData == NULL || ( (pData->nPeerState & eRoomPeer_CanAct) != eRoomPeer_CanAct ) )
		{
			continue;
		}

		if ( pData->nBetCoinThisRound != GetSimpleData()->nMostBetCoinThisRound )
		{
			return false ;
		}

		if ( bOnlyOneAct == false && pData->eCurAct == eRoomPeerAction_None )
		{
			return false ;
		}
	}

	return true ;
}

unsigned char CTaxasRoomData::OnDistributeCard()   /// return cur Bet Round ;
{
	++GetSimpleData()->nBetRound ;
	return GetSimpleData()->nBetRound ;	
}

bool CTaxasRoomData::CheckCanPlayerBeKickedRightNow(unsigned char nTargetIdx )
{
	stTaxasPeerData* pData = (stTaxasPeerData*)GetPeerDataByIdx(nTargetIdx);
	if ( pData && ( (pData->nPeerState & eRoomPeer_WaitCaculate) == eRoomPeer_WaitCaculate ) )
	{
		return false ;
	}

	return true ;
}

unsigned char CTaxasRoomData::OnEndThisRound()
{
	// end room data ;
	stTaxasRoomDataSimple* pSimpleData = GetSimpleData() ;
	pSimpleData->cCurWaitPlayerActionIdx = -1 ;
	pSimpleData->ePrePeerAction = eRoomPeerAction_None ;
	pSimpleData->nMostBetCoinThisRound = 0 ;
	
	// end player data 
	for ( int i = 0 ; i < pSimpleData->cMaxPlayingPeers ; ++i )
	{
		stTaxasPeerData* pData = (stTaxasPeerData*)m_vPeerDatas[i] ;
		if ( pData == NULL )
		{
			continue; 
		}
		pData->eCurAct = eRoomPeerAction_None ;
		pData->nBetCoinThisRound = 0 ;
	}

	return 0 ;
}

char CTaxasRoomData::OnWaitNextPlayerAct()  // return cur wait player idx , other value means error ;
{
	if ( GetSimpleData()->cCurWaitPlayerActionIdx < 0 )
	{
		GetSimpleData()->cCurWaitPlayerActionIdx = GetFirstPlayerIdxWithState( GetSimpleData()->cBigBlindIdx + 1, eRoomPeer_CanAct);
	}
	else
	{
		GetSimpleData()->cCurWaitPlayerActionIdx = GetFirstPlayerIdxWithState(GetSimpleData()->cCurWaitPlayerActionIdx + 1 , eRoomPeer_CanAct);
	}
	
	return GetSimpleData()->cCurWaitPlayerActionIdx ;
}

char CTaxasRoomData::OnPlayerStandup(unsigned char nIdx )
{
	stTaxasPeerData* pData = (stTaxasPeerData*)GetPeerDataByIdx(nIdx);
	if ( !pData )
	{
		return 1 ;
	}

	if ( (pData->nPeerState & eRoomPeer_WaitCaculate) == eRoomPeer_WaitCaculate )
	{
		GetSimpleData()->nCurMainBetPool += pData->nBetCoinThisRound ;
	}

	RemovePeer(pData->nSessionID) ;
	return 0 ;
}

bool SortTaxasPeerDataByBetCoinThisRound(stTaxasPeerData* pLeft,stTaxasPeerData* pRight)
{
	if ( pLeft->nBetCoinThisRound <= pRight->nBetCoinThisRound )
		return  true ;
	return false ;
}

char CTaxasRoomData::OnCaculateRound(VEC_VICE_POOL*vNewProducedVicePool)
{
	// put this round bet to all bet coin 
	for ( int i = 0 ; i < GetSimpleData()->cMaxPlayingPeers; ++i )
	{
		stTaxasPeerData* pData = (stTaxasPeerData*)m_vPeerDatas[i] ;
		if ( pData && (pData->nPeerState & eRoomPeer_WaitCaculate) == eRoomPeer_WaitCaculate )
		{
			pData->nAllBetCoin += pData->nBetCoinThisRound ;
		}
	}

	// find all in player this round 
	std::vector<stTaxasPeerData*> vecAllInPlayers ;
	for ( int i = 0 ; i < GetSimpleData()->cMaxPlayingPeers; ++i )
	{
		stTaxasPeerData* pData = (stTaxasPeerData*)m_vPeerDatas[i] ;
		if ( pData && pData->eCurAct == eRoomPeerAction_AllIn )
		{
			vecAllInPlayers.push_back(pData) ;
		}
	}

	std::sort(vecAllInPlayers.begin(),vecAllInPlayers.end(),SortTaxasPeerDataByBetCoinThisRound);

	// construct vice pool
	for ( unsigned int i = 0 ; i < vecAllInPlayers.size() ; ++i )
	{
		stTaxasPeerData* pData = vecAllInPlayers[i];
		if ( pData->nBetCoinThisRound == 0 )   // can not "0 Coin ALL IN ", so skip this condition 
		{
			continue;
		}
		stVicePool* pPool = new stVicePool ;
		pPool->Idx = m_vAllVicePools.size();
		pPool->nCoinInPool = GetSimpleData()->nCurMainBetPool ;
		GetSimpleData()->nCurMainBetPool = 0 ;
		m_vAllVicePools.push_back(pPool);
		if ( vNewProducedVicePool )
		{
			vNewProducedVicePool->push_back(pPool) ;
		}

		uint64_t nNewAddPerPeer = pData->nBetCoinThisRound ;
		for ( int idx = 0 ; idx < GetSimpleData()->cMaxPlayingPeers ; ++idx )
		{
			stTaxasPeerData* pPeer = (stTaxasPeerData*)m_vPeerDatas[idx];
			if ( pPeer && pPeer->nBetCoinThisRound >= nNewAddPerPeer && ((pPeer->nPeerState & eRoomPeer_WaitCaculate) == eRoomPeer_WaitCaculate) )
			{
				pPool->nCoinInPool += nNewAddPerPeer ;
				pPool->vPeerIdxInPool.push_back(idx);
				pPeer->nBetCoinThisRound -= nNewAddPerPeer ;
			}
		}
	}

	// left coin all to main pool 
	for ( int idx = 0 ; idx < GetSimpleData()->cMaxPlayingPeers ; ++idx )
	{
		stTaxasPeerData* pPeer = (stTaxasPeerData*)m_vPeerDatas[idx];
		if ( pPeer && ((pPeer->nPeerState & eRoomPeer_WaitCaculate) == eRoomPeer_WaitCaculate) )
		{
			GetSimpleData()->nCurMainBetPool += pPeer->nBetCoinThisRound ;
		}
	}

	return 0 ;
}

char CTaxasRoomData::OnCaculateGameResult()
{
	// make a main pool  if have 
	if ( GetSimpleData()->nCurMainBetPool > 0 )
	{
		stVicePool* pPool = new stVicePool ;
		pPool->Idx = m_vAllVicePools.size();
		m_vAllVicePools.push_back(pPool);
		pPool->nCoinInPool = GetSimpleData()->nCurMainBetPool ;
		
		for ( int iPeerIdx = 0 ; iPeerIdx < m_pData->cMaxPlayingPeers; ++iPeerIdx )
		{
			if ( m_vPeerDatas[iPeerIdx] && (m_vPeerDatas[iPeerIdx]->nPeerState & eRoomPeer_CanAct) == eRoomPeer_CanAct )
			{
				pPool->vPeerIdxInPool.push_back(iPeerIdx) ;
			}
		}
	}

	// caculate winners in pool 
	VEC_VICE_POOL::iterator iter = m_vAllVicePools.begin() ;
	for ( ; iter != m_vAllVicePools.end() ; ++iter )
	{
		stVicePool* pPool = *iter ;
		CaculateVicePoolWinnerIdxs(pPool);
		if ( !pPool )
		{
			continue;
		}

		if ( pPool->vWinnerIdxs.empty() )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("this is no winner in this pool idx = %d , coin = %I64d",pPool->Idx,pPool->nCoinInPool );
			continue;
		}

		uint64_t nWinPerWiner = (double)pPool->nCoinInPool / (float)pPool->vWinnerIdxs.size();
		for ( unsigned int iWinIdx = 0 ; iWinIdx < pPool->vWinnerIdxs.size(); ++iWinIdx )
		{
			if ( pPool->vWinnerIdxs[iWinIdx] >= GetSimpleData()->cMaxPlayingPeers )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("why invalid idx = %d",pPool->vWinnerIdxs[iWinIdx]);
				continue;
			}

			stTaxasPeerData* pData = (stTaxasPeerData*)m_vPeerDatas[pPool->vWinnerIdxs[iWinIdx]] ;
			if ( !pData )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("why this winner is NULL , idx = %d",pPool->vWinnerIdxs[iWinIdx] ) ;
			}
			pData->nWinCoinFromPools += nWinPerWiner ;
		}
	}

	// caculate final ;
	for ( int idx = 0 ; idx < m_pData->cMaxPlayingPeers ; ++idx )
	{
		stTaxasPeerData* pData = (stTaxasPeerData*)m_vPeerDatas[idx];
		if ( !pData || (pData->nPeerState & eRoomPeer_WaitCaculate ) != eRoomPeer_WaitCaculate )
		{
			continue;
		}
		pData->nCurCoin += pData->nWinCoinFromPools;
		if ( pData->nWinCoinFromPools  > pData->nAllBetCoin )
		{
			uint64_t nChoushui = 0 ;
			CLogMgr::SharedLogMgr()->PrintLog("chou shui from idx = %d , coin = %I64d",idx,nChoushui );
			pData->nCurCoin -= nChoushui ;
		}
	}
	return 0 ;
}

bool CTaxasRoomData::CheckGameOver()
{
	if ( CheckThisRoundEnd() == false )
		return false ;
	
	if ( GetSimpleData()->nBetRound == 4 )
	{
		return true ;
	}

	if ( GetPlayerCntWithState(eRoomPeer_WaitCaculate) < 2 )
	{
		return true ;
	}

	return false ;
}

unsigned short CTaxasRoomData::GetAllPeerDataToBuffer(const char* pBuffer )
{
	unsigned short nSize = 0 ;
	for ( unsigned char idx = 0 ; idx < m_pData->cMaxPlayingPeers ; ++idx )
	{
		if ( m_vPeerDatas[idx] == NULL  )
		{
			continue; 
		}
		
		memcpy((void*)(pBuffer+nSize),m_vPeerDatas[idx],sizeof(stTaxasPeerData));
		nSize += sizeof(stTaxasPeerData);
	}
	return nSize ;
}

unsigned short CTaxasRoomData::GetAllVicePoolToBuffer(const char* pBuffer)
{
	unsigned short nSize = 0 ;
	VEC_VICE_POOL::iterator iter = m_vAllVicePools.begin() ;
	for ( ; iter != m_vAllVicePools.end(); ++iter )
	{
		stVicePool* pPool = *iter ;
		stVicePoolItem pItem ;
		pItem.nCoinInPool = pPool->nCoinInPool ;
		pItem.nPeerCnt = pPool->vPeerIdxInPool.size();
		pItem.nPoolIdx = pPool->Idx ;
		memcpy((void*)(pBuffer + nSize),&pItem,sizeof(pItem)) ;
		nSize += sizeof(stVicePoolItem);

		for ( unsigned int i = 0 ; i < pPool->vPeerIdxInPool.size(); ++i )
		{
			unsigned char cIdx = pPool->vPeerIdxInPool[i] ;
			memcpy((void*)(pBuffer + nSize),&cIdx,sizeof(unsigned char));
			nSize += sizeof(unsigned char);
		}
	}
	return nSize ;
}

unsigned short CTaxasRoomData::GetAllPeersHoldCardToBuffer(const char* pBuffer )
{
	unsigned short nSize = 0 ;
	for ( unsigned char idx = 0 ; idx < m_pData->cMaxPlayingPeers ; ++idx )
	{
		if ( m_vPeerDatas[idx] == NULL  )
		{
			continue; 
		}

		stTaxasPeerData* peerData = (stTaxasPeerData*)m_vPeerDatas[idx];
		stTaxasHoldCardItems cardItem ;
		cardItem.cPlayerIdx = peerData->cRoomIdx;
		memcpy(cardItem.vCards,peerData->vHoldCard,sizeof(peerData->vHoldCard));
		memcpy((void*)(pBuffer+nSize),&cardItem,sizeof(cardItem));
		nSize += sizeof(cardItem);
	}
	return nSize ;
}

void CTaxasRoomData::OnRecievedCardInfos(stMsg*pmsg)
{
	if (pmsg->usMsgType != MSG_TAXAS_ROOM_GAME_START )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("msg id = %d should not be process here",pmsg->usMsgType);
		return ;
	}

	stMsgTaxasRoomGameStart* pCardInfo = (stMsgTaxasRoomGameStart*)pmsg ;
	memcpy(GetSimpleData()->vPublicCardNums,pCardInfo->vPublicCard,sizeof(pCardInfo->vPublicCard)) ;
	stTaxasHoldCardItems* pItem = (stTaxasHoldCardItems*)(((char*)pmsg)+sizeof(stMsgTaxasRoomGameStart));
	while ( pCardInfo->nPeerCnt-- )
	{
		stTaxasPeerData* pData = (stTaxasPeerData*)GetPeerDataByIdx(pItem->cPlayerIdx) ;
		if ( pData == NULL )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("error , server data is not the same as client");
			++pItem;
			continue;
		}
		memcpy(pData->vHoldCard,pItem->vCards,sizeof(pData->vHoldCard));
		++pItem ;
	}
}

unsigned char CTaxasRoomData::OnPlayerFollow(unsigned char nIdx )
{
	if ( nIdx != GetSimpleData()->cCurWaitPlayerActionIdx )
	{
		return  1 ;
	}

	stTaxasPeerData* pData = (stTaxasPeerData*)GetPeerDataByIdx(nIdx);
	if ( pData == NULL || ((pData->nPeerState & eRoomPeer_CanAct ) != eRoomPeer_CanAct ) )
	{
		return 2 ;
	}

	uint64_t nFCoin = GetSimpleData()->nMostBetCoinThisRound - pData->nBetCoinThisRound ;
	if ( pData->nCurCoin <= nFCoin ) // can not  <= , we can not "0 coin ALL iN"
	{
		return 3 ;   // coin is not engough , can not follow , please all in ;
	}

	if ( GetSimpleData()->nMostBetCoinThisRound > pData->nBetCoinThisRound )
	{
		return 4 ;
	}

	pData->nBetCoinThisRound += nFCoin ;
	pData->nCurCoin -= nFCoin ;
	pData->eCurAct = eRoomPeerAction_Follow;
	GetSimpleData()->ePrePeerAction = eRoomPeerAction_Follow;
	return 0 ;
}

unsigned char CTaxasRoomData::OnPlayerAdd(unsigned char nIdx,uint64_t nAddCoin )
{
	if ( nIdx != GetSimpleData()->cCurWaitPlayerActionIdx )
	{
		return  1 ;
	}

	stTaxasPeerData* pData = (stTaxasPeerData*)GetPeerDataByIdx(nIdx);
	if ( pData == NULL || ((pData->nPeerState & eRoomPeer_CanAct ) != eRoomPeer_CanAct ) )
	{
		return 2 ;
	}

	if ( nAddCoin < GetPlayerMustMinAddCoin(nIdx) )
	{
		return 3 ;
	}

	if ( nAddCoin % GetSimpleData()->nBigBlindBet != 0 )
	{
		return 4 ;
	}

	if ( nAddCoin < pData->nCurCoin )  // can not  <= , we can not "0 coin ALL iN"
	{
		return 5 ;
	}

	if ( nAddCoin > GetPlayerCanMaxAddCoin(nIdx) )
	{
		return 6 ;
	}

	if ( (pData->nBetCoinThisRound + nAddCoin) % GetSimpleData()->nBigBlindBet != 0 )
	{
		return 7 ;              // add coin result must times BigBlindBet ;
	}

	pData->nBetCoinThisRound += nAddCoin ;
	pData->nCurCoin -= nAddCoin ;
	pData->eCurAct = eRoomPeerAction_Add;
	GetSimpleData()->ePrePeerAction = eRoomPeerAction_Add;
	GetSimpleData()->nMostBetCoinThisRound = pData->nBetCoinThisRound ;
	return 0 ;
}

unsigned char CTaxasRoomData::OnPlayerPass(unsigned char nIdx )
{
	if ( nIdx != GetSimpleData()->cCurWaitPlayerActionIdx )
	{
		return  1 ;
	}

	stTaxasPeerData* pData = (stTaxasPeerData*)GetPeerDataByIdx(nIdx);
	if ( pData == NULL || ((pData->nPeerState & eRoomPeer_CanAct ) != eRoomPeer_CanAct ) )
	{
		return 2 ;
	}

	if ( GetSimpleData()->nMostBetCoinThisRound != pData->nBetCoinThisRound )
	{
		return 3 ;
	}

	GetSimpleData()->ePrePeerAction = eRoomPeerAction_Pass;
	pData->eCurAct = eRoomPeerAction_Pass;
	return 0 ;
}

unsigned char CTaxasRoomData::OnPlayerGiveUp( unsigned char nIdx )
{
	stTaxasPeerData* pData = (stTaxasPeerData*)GetPeerDataByIdx(nIdx);
	if ( pData == NULL  )
	{
		return 1 ;
	}

	GetSimpleData()->nCurMainBetPool += pData->nBetCoinThisRound ;
	pData->nAllBetCoin += pData->nBetCoinThisRound ;
	pData->nBetCoinThisRound = 0 ;

	GetSimpleData()->ePrePeerAction = eRoomPeerAction_GiveUp;
	pData->eCurAct = eRoomPeerAction_GiveUp;
	pData->nPeerState = eRoomPeer_GiveUp ;
	return 0 ;
}

unsigned char CTaxasRoomData::OnPlayerAllIn(unsigned char nIdx)
{
	if ( nIdx != GetSimpleData()->cCurWaitPlayerActionIdx )
	{
		return  1 ;
	}

	stTaxasPeerData* pData = (stTaxasPeerData*)GetPeerDataByIdx(nIdx);
	if ( pData == NULL || ((pData->nPeerState & eRoomPeer_CanAct ) != eRoomPeer_CanAct ) )
	{
		return 2 ;
	}

	uint64_t nAddCoin = GetPlayerCanMaxAddCoin(nIdx);
	nAddCoin = pData->nCurCoin < nAddCoin ? pData->nCurCoin : nAddCoin ; 
	if ( nAddCoin + pData->nBetCoinThisRound  == 0 )
	{
		return 3 ;   // can not zero All In ;
	}

	pData->nBetCoinThisRound += nAddCoin ;
	pData->nCurCoin -= nAddCoin ;
	if ( pData->nBetCoinThisRound > GetSimpleData()->nMostBetCoinThisRound )
	{
		pData->nBetCoinThisRound = pData->nBetCoinThisRound ;
	}

	GetSimpleData()->ePrePeerAction = eRoomPeerAction_AllIn;
	pData->eCurAct = eRoomPeerAction_AllIn;
	pData->nPeerState = eRoomPeer_AllIn ;
	return 0 ;
}

unsigned char CTaxasRoomData::OnPlayerAutoStand(unsigned char nIdx )
{
	RemovePeer(GetSessionIDByIdx(nIdx));
	return 0 ;
}

unsigned char CTaxasRoomData::OnPlayerAutoTakeIn(unsigned char nIdx ,unsigned int nTakeInCoin )
{
	if ( nTakeInCoin == 0 )
	{
		OnPlayerAutoStand(nIdx);
	}
	else
	{
		stPeerBaseData* pData = GetPeerDataByIdx(nIdx);
		pData->nCurCoin += nTakeInCoin ;
	}
	return 0 ;
}

unsigned char CTaxasRoomData::GetNeedAutoTakeInPeerIdxs(std::vector<unsigned char>& vIdxs)
{
	for ( unsigned char idx = 0 ; idx < m_pData->cMaxPlayingPeers ; ++idx )
	{
		if ( m_vPeerDatas[idx] == NULL  )
		{
			continue; 
		}

		if ( m_vPeerDatas[idx]->nCurCoin < GetSimpleData()->cMiniCoinNeedToEnter )
		{
			vIdxs.push_back(idx) ;
		}
	}
	return vIdxs.size();
}

unsigned char CTaxasRoomData::GetFirstPlayerIdxWithState(unsigned char nIdxBegin, unsigned int nState )
{
	for ( unsigned char idx = nIdxBegin ; idx < m_pData->cMaxPlayingPeers * 2 ; ++idx )
	{
		stPeerBaseData* pData =  m_vPeerDatas[idx%m_pData->cMaxPlayingPeers] ;
		if ( pData == NULL || (pData->nPeerState & nState) != nState )
		{
			continue; 
		}

		return pData->cRoomIdx ;
	}
	return 0 ;
}

unsigned char CTaxasRoomData::GetPlayerCntWithState( unsigned int nState )
{
	unsigned char nCnt = 0 ;
	for ( unsigned char idx = 0 ; idx < m_pData->cMaxPlayingPeers ; ++idx )
	{
		stPeerBaseData* pData =  m_vPeerDatas[idx] ;
		if ( pData == NULL || (pData->nPeerState & nState) != nState )
		{
			continue; 
		}

		++nCnt;
	}
	return nCnt ;
}

uint64_t CTaxasRoomData::GetPlayerMustMinAddCoin(unsigned char nPlayerIdx)
{
	stTaxasPeerData* pPlayer = (stTaxasPeerData*)GetPeerDataByIdx(nPlayerIdx) ;
	if ( pPlayer == NULL )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("why this player is null in must mini add coin");
		return -1 ;
	}

	uint64_t nOffset = GetSimpleData()->nMostBetCoinThisRound - pPlayer->nBetCoinThisRound ;

	uint64_t nAddCoin = nOffset * 2 ;
	uint64_t nleft = ( nAddCoin + pPlayer->nBetCoinThisRound ) % GetSimpleData()->nBigBlindBet ;
	if ( nleft != 0 )
	{
		nAddCoin = nAddCoin + ( GetSimpleData()->nBigBlindBet - nleft );
	}

	return nAddCoin >= GetSimpleData()->nBigBlindBet ? nAddCoin : GetSimpleData()->nBigBlindBet ;
}

uint64_t CTaxasRoomData::GetPlayerCanMaxAddCoin( unsigned char nPlayerIdx )
{
	stTaxasPeerData* pData = (stTaxasPeerData*)GetPeerDataByIdx(nPlayerIdx);
	if ( pData == NULL )
	{
		return 0 ;
	}
	
	if ( GetCanMaxBetCoinThisRound() <= pData->nBetCoinThisRound )
	{
		return 0 ;
	}

	return GetCanMaxBetCoinThisRound() - pData->nBetCoinThisRound ;
}

uint64_t CTaxasRoomData::GetCanMaxBetCoinThisRound()
{
	uint64_t nSecondbig = 0 , nBigbiggest = 0 ;
	for ( unsigned char idx = 0 ; idx < m_pData->cMaxPlayingPeers ; ++idx )
	{
		stTaxasPeerData* pData = (stTaxasPeerData*)m_vPeerDatas[idx] ;
		if ( pData == NULL || ( (pData->nPeerState & eRoomPeer_CanAct) != eRoomPeer_CanAct ) )
		{
			continue; 
		}

		uint64_t nAll = pData->nBetCoinThisRound + pData->nCurCoin ;
		if ( nSecondbig >= nAll )
		{
			continue;
		}

		if ( nAll > nBigbiggest )
		{
			nSecondbig = nBigbiggest ;
			nBigbiggest = nAll ;
		}
		else
		{
			nSecondbig = nAll ;
		}
	}
	return nSecondbig ;
}

void CTaxasRoomData::ClearVicePool()
{
	VEC_VICE_POOL::iterator iter = m_vAllVicePools.begin() ;
	for ( ; iter != m_vAllVicePools.end() ; ++iter )
	{
		delete *iter ;
		*iter = NULL ;
	}
	m_vAllVicePools.clear();
}

void CTaxasRoomData::CaculateVicePoolWinnerIdxs(stVicePool* pPool )
{
	if ( pPool == NULL || pPool->vPeerIdxInPool.empty() )
	{
		CLogMgr::SharedLogMgr()->ErrorLog( "why pool is null ? or no one in this pool" );
		return ;
	}

	stTaxasPeerData* pDataWinner = NULL;
	CTaxasPokerPeerCard tWinerCard , tTargetCard;
	for ( unsigned int i = 0 ; i < pPool->vPeerIdxInPool.size() ; ++i )
	{
		unsigned char nIdx = pPool->vPeerIdxInPool[i] ;
		stTaxasPeerData* pDataTarget = (stTaxasPeerData*)m_vPeerDatas[nIdx] ;
		if ( pDataTarget == NULL )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("a in pool peer is null , idx = %d",nIdx);
			continue;
		}

		if ( pDataWinner == NULL )
		{
			pDataWinner = pDataTarget ;
			GetTaxasCardForPlayer(tWinerCard,pDataWinner);
			pPool->vWinnerIdxs.push_back(pDataWinner->cRoomIdx) ;
			continue;
		}

		GetTaxasCardForPlayer(tTargetCard,pDataTarget);

		char nRet = tWinerCard.PK(&tTargetCard);
		if ( nRet == 1 )  // win 
		{
			// do nothing 
		}
		else if ( nRet == 0 )  // the same 
		{
			pPool->vWinnerIdxs.push_back(pDataTarget->cRoomIdx) ;
		}
		else  // failed
		{
			pPool->vWinnerIdxs.clear();
			pDataWinner = pDataTarget ;
			pPool->vWinnerIdxs.push_back(pDataWinner->cRoomIdx) ;
			GetTaxasCardForPlayer(tWinerCard,pDataWinner);
		}
	}
}

bool CTaxasRoomData::GetTaxasCardForPlayer(CTaxasPokerPeerCard& pCard,stTaxasPeerData* pPeerData)
{
	pCard.Reset();
	for ( int i = 0 ; i < TAXAS_PEER_CARD ; ++i )
	{
		pCard.AddCardByCompsiteNum(pPeerData->vHoldCard[i]) ;
	}

	for ( int i = 0 ; i < TAXAS_PUBLIC_CARD ; ++i )
	{
		pCard.AddCardByCompsiteNum(GetSimpleData()->vPublicCardNums[i]);
	}
	return true ;
}