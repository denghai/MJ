#include "GoldenRoomData.h"
#include "CommonData.h"
#include "LogManager.h"
#include <iostream>
#define  IS_STATE( SRC,CEK) (((SRC)&(CEK)) == CEK )
void CGoldenRoomData::Init()
{
	tPoker.InitGolden();

	m_pData = new stRoomGoldenDataOnly  ;
	memset(m_pData,0,sizeof(stRoomGoldenDataOnly));
	m_pData->cGameType = eRoom_Gold ;
	
	stRoomGoldenDataOnly* pData = (stRoomGoldenDataOnly*)m_pData ;
	pData->cCurActIdx = -1 ;
}

stRoomGoldenDataOnly* CGoldenRoomData::GetDataOnly()
{
	stRoomGoldenDataOnly* pData = (stRoomGoldenDataOnly*)m_pData ;
	return pData ;
}

unsigned char CGoldenRoomData::GetReadyPeerCnt()
{
	unsigned char nMax = GetDataOnly()->cMaxPlayingPeers ;
	unsigned char nCnt = 0 ;
	for ( unsigned char i = 0 ; i < nMax ; ++i )
	{
		if ( m_vPeerDatas[i] == NULL )
			continue;
		if ( IS_STATE(m_vPeerDatas[i]->nPeerState,eRoomPeer_Golden_Ready) )
		{
			++nCnt ;
		}
	}
	return nCnt ;
}

unsigned char CGoldenRoomData::OnUpateActPlayerIdx()
{
	if ( GetDataOnly()->cCurActIdx < 0 )
		return GetDataOnly()->cBankerIdx ;

	unsigned char nMax = GetDataOnly()->cMaxPlayingPeers ;
	for ( int i = GetDataOnly()->cCurActIdx + 1 ; i < nMax * 2 ; ++i )
	{
		unsigned char nRealIdx = i%nMax ;
		if ( nRealIdx == GetDataOnly()->cCurActIdx )
		{
			break;
		}

		if ( nRealIdx == GetDataOnly()->cBankerIdx )
		{
			GetDataOnly()->nRound += 1 ;
		}

		stPeerBaseData* pPeer = m_vPeerDatas[nRealIdx];
		if ( pPeer == NULL )
			continue;
		if ( IS_STATE(pPeer->nPeerState,eRoomPeer_Golden_Playing) == false )
			continue; 

		// update cur act idx ;
		GetDataOnly()->cCurActIdx = pPeer->cRoomIdx ;
		GetDataOnly()->fTimeTick = 0 ;
		return pPeer->cRoomIdx ;
	}
	return -1 ;
}

void CGoldenRoomData::OnStartGame()
{
	tPoker.RestAllPoker() ;
	GetDataOnly()->cCurActIdx = -1 ;
	GetDataOnly()->nAllBetCoin = 0 ;
	GetDataOnly()->nCurMaxBet = GetDataOnly()->nMiniBet ;
	GetDataOnly()->nRound = 0 ;
	GetDataOnly()->fTimeTick = 0 ;
	
	// decide new banker idx ;
	unsigned char nMax = GetDataOnly()->cMaxPlayingPeers ;
	for ( int i = GetDataOnly()->cBankerIdx+ 1 ; i < nMax * 2 ; ++i )
	{
		unsigned char nRealIdx = i%nMax ;

		stPeerBaseData* pPeer = m_vPeerDatas[nRealIdx];
		if ( pPeer == NULL )
			continue;
		if ( IS_STATE(pPeer->nPeerState,eRoomPeer_Golden_Ready) )
			GetDataOnly()->cBankerIdx = i ;
	}

	// update all peer state 
	for ( int i = 0 ; i < nMax ; ++i )
	{
		stGoldenPeerData* pPeer = (stGoldenPeerData*)m_vPeerDatas[i];
		if ( pPeer == NULL )
			continue;
		if ( IS_STATE(pPeer->nPeerState,eRoomPeer_Golden_Ready) )
		{
			pPeer->nPeerState = eRoomPeer_Golden_Playing ;
		}
		else
		{
			pPeer->nPeerState = eRoomPeer_Golden_WaitNextPlay ;
		}

		// reset some member var
		memset(pPeer->vHoldCard,0,sizeof(pPeer->vHoldCard));
		memset(pPeer->vShowedCardIdx,-1,sizeof(pPeer->vShowedCardIdx));
		pPeer->nBetCoin = 0 ;
		pPeer->nShowedCardCnt = 0 ;
		pPeer->nChangeCardTimes = 0 ;
		pPeer->nChangeCardUsedDiamond = 0 ;
		pPeer->nPKTimes = 1 ;
	}
}

void CGoldenRoomData::OnEndGame()
{
	CRoomBaseData::OnEndGame();

	GetDataOnly()->cCurActIdx = -1 ;
	GetDataOnly()->nAllBetCoin = 0 ;
	GetDataOnly()->nCurMaxBet = GetDataOnly()->nMiniBet ;
	GetDataOnly()->nRound = 0 ;
	GetDataOnly()->fTimeTick = 0 ;
	// update all peer state 
	unsigned char nMax = GetDataOnly()->cMaxPlayingPeers ;
	for ( int i = 0 ; i < nMax ; ++i )
	{
		stGoldenPeerData* pPeer = (stGoldenPeerData*)m_vPeerDatas[i];
		if ( pPeer == NULL )
			continue;
		pPeer->nPeerState = eRoomPeer_Golden_WaitToReady ;
		memset(pPeer->vHoldCard,0,sizeof(pPeer->vHoldCard));
		memset(pPeer->vShowedCardIdx,-1,sizeof(pPeer->vShowedCardIdx));
		pPeer->nBetCoin = 0 ;
		pPeer->nShowedCardCnt = 0 ;
		pPeer->nChangeCardTimes = 0 ;
		pPeer->nChangeCardUsedDiamond = 0 ;
		pPeer->nPKTimes = 1 ;
	}
}

char CGoldenRoomData::GameOverCheckAndProcess()  // return > 0 means game over ret value is winner idx ;
{
	unsigned char nMax = GetDataOnly()->cMaxPlayingPeers ;
	stPeerBaseData* pPeer = NULL ,*pWinner = NULL;
	for ( int i = 0 ; i < nMax ; ++i )
	{
		 pPeer = m_vPeerDatas[i];
		if ( pPeer == NULL )
			continue;
		if ( IS_STATE(pPeer->nPeerState,eRoomPeer_Golden_Playing) )
		{
			if ( pWinner )
			{
				return -1 ;
			}
			pWinner = pPeer ;
		}
	}

	// winner get the coin 
	pWinner->nCurCoin += GetDataOnly()->nAllBetCoin ;
	return pWinner->cRoomIdx ;
}

unsigned char CGoldenRoomData::DistributeCard()
{
	unsigned char nPlayingPeerCunt = 0 ;
	for ( int iC = 0 ; iC < GOLDEN_PEER_CARD ; ++iC )
	{
		unsigned char nMax = GetDataOnly()->cMaxPlayingPeers ;
		for ( int i = 0 ; i < nMax ; ++i )
		{
			stGoldenPeerData* pPeer = (stGoldenPeerData*)m_vPeerDatas[i];
			if ( pPeer == NULL || IS_STATE(pPeer->nPeerState,eRoomPeer_Golden_Playing) == false )
				continue;
			pPeer->vHoldCard[iC] = tPoker.GetCardWithCompositeNum();
			if ( iC == 0 )
			{
				++nPlayingPeerCunt ;
			}
		}
	}
	return nPlayingPeerCunt ;
}

unsigned char CGoldenRoomData::OnPlayerReady(unsigned int nSessionID) // 0 success , 1 not your turn ,2 state error 
{
	stGoldenPeerData* pData = (stGoldenPeerData*)GetPeerDataBySessionID(nSessionID);
	if ( !pData)
	{
		return 3 ;
	}

	if (( GetDataOnly()->cCurRoomState != eRoomState_Golden_WaitPeerReady &&  GetDataOnly()->cCurRoomState != eRoomState_Golden_WaitPeerToJoin ) || IS_STATE(pData->nPeerState,eRoomPeer_Golden_WaitToReady) == false)
	{
		return 2 ;
	}

	pData->nPeerState = eRoomPeer_Golden_Ready ;
	return 0 ;
}

unsigned char CGoldenRoomData::OnPlayerLook(unsigned int nSessionID)  // 0 success , 1 not your turn ,2 state error
{
	stGoldenPeerData* pData = (stGoldenPeerData*)GetPeerDataBySessionID(nSessionID);
	if ( !pData)
	{
		return 3 ;
	}

	if ( GetDataOnly()->cCurRoomState != eRoomState_Golden_WaitPeerAction || IS_STATE(pData->nPeerState,eRoomPeer_Golden_Look) || IS_STATE(pData->nPeerState,eRoomPeer_Golden_Playing) == false)
	{
		return 2 ;
	}

	pData->nPeerState = eRoomPeer_Golden_Look ;

	if ( GetDataOnly()->cCurActIdx == pData->cRoomIdx )
	{
		GetDataOnly()->fTimeTick = 0 ;
	}

	return 0 ;
}

unsigned char CGoldenRoomData::OnPlayerGiveUp(unsigned int nSessionID )  // 0 success , 1 not your turn ,2 state error
{
	stGoldenPeerData* pData = (stGoldenPeerData*)GetPeerDataBySessionID(nSessionID);
	if ( !pData)
	{
		return 3 ;
	}

	if ( GetDataOnly()->cCurRoomState != eRoomState_Golden_WaitPeerAction || IS_STATE(pData->nPeerState,eRoomPeer_Golden_GiveUp) || IS_STATE(pData->nPeerState,eRoomPeer_Golden_Playing) == false)
	{
		return 2 ;
	}

	pData->nPeerState = eRoomPeer_Golden_GiveUp ;
	return 0 ;
}

unsigned char CGoldenRoomData::OnPlayerFollow(unsigned int nSessionID,uint64_t& nFinalFollowCoin) // 0 , means error , other is real finaly follow coin, as princeble needed ;
{
	stGoldenPeerData* pData = (stGoldenPeerData*)GetPeerDataBySessionID(nSessionID);
	if ( !pData)
	{
		return 3 ;
	}

	if ( GetDataOnly()->cCurRoomState != eRoomState_Golden_WaitPeerAction || IS_STATE(pData->nPeerState,eRoomPeer_Golden_Playing) == false)
	{
		return 2 ;
	}

	if ( GetDataOnly()->cCurActIdx != pData->cRoomIdx )
	{
		return 1 ;
	}

	nFinalFollowCoin = GetDataOnly()->nCurMaxBet * ( IS_STATE(pData->nPeerState,eRoomPeer_Golden_Look) ? 2 : 1 ) ;

	if ( pData->nCurCoin < nFinalFollowCoin )
	{
		nFinalFollowCoin = 0 ;
		return 4 ;
	}

	pData->nCurCoin -= nFinalFollowCoin ;
	pData->nBetCoin += nFinalFollowCoin ;
	GetDataOnly()->nAllBetCoin += nFinalFollowCoin ;
	return 0 ;
}

unsigned char CGoldenRoomData::OnPlayerAdd(unsigned int nSessionId,uint64_t nAddCoin, uint64_t& nFinalBetCoin ) // when this player looked card then nFinal coin adn add coin is not equal ;
{
	stGoldenPeerData* pData = (stGoldenPeerData*)GetPeerDataBySessionID(nSessionId);
	if ( !pData)
	{
		return 3 ;
	}

	if ( GetDataOnly()->cCurRoomState != eRoomState_Golden_WaitPeerAction || IS_STATE(pData->nPeerState,eRoomPeer_Golden_Playing) == false)
	{
		return 2 ;
	}

	if ( GetDataOnly()->cCurActIdx != pData->cRoomIdx )
	{
		return 1 ;
	}

	if ( nAddCoin <= GetDataOnly()->nCurMaxBet )
	{
		return 5 ;
	}

	nFinalBetCoin = nAddCoin * ( IS_STATE(pData->nPeerState,eRoomPeer_Golden_Look) ? 2 : 1 ) ;

	if ( pData->nCurCoin < nFinalBetCoin )
	{
		nFinalBetCoin = 0 ;
		return 4 ;
	}

	pData->nCurCoin -= nFinalBetCoin ;
	pData->nBetCoin += nFinalBetCoin ;
	GetDataOnly()->nAllBetCoin += nFinalBetCoin ;

	GetDataOnly()->nCurMaxBet = nAddCoin ;
	return 0 ;
}

unsigned char CGoldenRoomData::OnPlayerPK(unsigned char idx , unsigned char cTargetIdx , bool& bWin )  // 0 success , 1 not your turn ,2 state error 
{
	if ( GetDataOnly()->nRound < GOLDEN_PK_ROUND )
	{
		return 6 ;
	}

	stGoldenPeerData* pData = (stGoldenPeerData*)GetPeerDataByIdx(idx);
	if ( !pData)
	{
		return 3 ;
	}

	if ( GetDataOnly()->cCurRoomState != eRoomState_Golden_WaitPeerAction || IS_STATE(pData->nPeerState,eRoomPeer_Golden_Playing) == false)
	{
		return 2 ;
	}

	if ( GetDataOnly()->cCurActIdx != pData->cRoomIdx )
	{
		return 1 ;
	}

	stGoldenPeerData* pTargetData = (stGoldenPeerData*)GetPeerDataByIdx(cTargetIdx);
	if ( !pData )
	{
		return 4 ;
	}

	if (  IS_STATE(pTargetData->nPeerState,eRoomPeer_Golden_Playing) == false)
	{
		return 5 ;
	}

	m_tPeerCard.SetPeerCardByNumber(pData->vHoldCard[0],pData->vHoldCard[1],pData->vHoldCard[2]);
	m_tPeerCardTarget.SetPeerCardByNumber(pTargetData->vHoldCard[0],pTargetData->vHoldCard[1],pTargetData->vHoldCard[2]);
	bWin = m_tPeerCard.PKPeerCard(&m_tPeerCardTarget) ;
	if ( bWin )
	{
		pTargetData->nPeerState = eRoomPeer_Golden_PK_Failed;
	}
	else
	{
		pData->nPeerState = eRoomPeer_Golden_PK_Failed;
	}

	uint64_t nPKCoin = GetDataOnly()->nCurMaxBet * ( IS_STATE(pData->nPeerState,eRoomPeer_Golden_Look) ? 2 : 1 ) ;
	if ( pData->nCurCoin < nPKCoin )
	{
		nPKCoin = pData->nCurCoin ;
	}

	pData->nCurCoin -= nPKCoin ;
	pData->nBetCoin += nPKCoin ;
	GetDataOnly()->nAllBetCoin += nPKCoin ;
	return 0 ;
}

unsigned char CGoldenRoomData::OnPlayerShowCard(unsigned int nSessionID , unsigned char nCardIdx )
{
	stGoldenPeerData* pData = (stGoldenPeerData*)GetPeerDataBySessionID(nSessionID);
	if ( !pData )
	{
		return 10 ;
	}

	if ( IS_STATE(pData->nPeerState,eRoomPeer_Golden_Playing) == false )
	{
		return 1 ;   // state error ;
	}

	if ( pData->nShowedCardCnt >= GOLDEN_PEER_CARD - 1  )
	{
		return 2 ;   // show card cunt up limit ;
	}

	if ( nCardIdx >= GOLDEN_PEER_CARD )
	{
		return 4 ;  // error idx ;
	}

	for ( int i = 0 ; i < pData->nShowedCardCnt ; ++i )
	{
		if ( pData->vShowedCardIdx[i] == nCardIdx )
		{
			return 3 ; // already show 
		}
	}

	// show card 
	pData->vShowedCardIdx[pData->nShowedCardCnt++] = nCardIdx ;
	return 0 ;
}

unsigned char CGoldenRoomData::OnPlayerChangeCard(unsigned int nSessionID , unsigned char nCardIdx , unsigned char& cNewCardNum)
{
	stGoldenPeerData* pData = (stGoldenPeerData*)GetPeerDataBySessionID(nSessionID);
	if ( !pData )
	{
		return 10 ;
	}

	if ( IS_STATE(pData->nPeerState,eRoomPeer_Golden_Playing) == false )
	{
		return 1 ;   // state error ;
	}

	if ( nCardIdx >= GOLDEN_PEER_CARD )
	{
		return 2 ;   // error idx ;
	}

	if ( pData->nChangeCardTimes >= GOLDEN_PEER_CARD )
	{
		return 3 ;   // can not change too many card 
	}

	if ( pData->nDiamond < s_vChangeCardDimonedNeed[pData->nChangeCardTimes] )
	{
		return 4 ;   // diamond not engough ;
	}

	if ( GetDataOnly()->nRound < GetDataOnly()->nChangeCardRound )
	{
		return 5 ; // please wait more round ;
	}

	pData->nDiamond -= s_vChangeCardDimonedNeed[pData->nChangeCardTimes] ;
	pData->nChangeCardUsedDiamond += s_vChangeCardDimonedNeed[pData->nChangeCardTimes] ;
	++pData->nChangeCardTimes;
#ifdef GAME_SERVER
	cNewCardNum = tPoker.GetCardWithCompositeNum();
#endif
	pData->vHoldCard[nCardIdx] = cNewCardNum;

	if ( pData->cRoomIdx == GetDataOnly()->cCurActIdx )
	{
		GetDataOnly()->fTimeTick = 0 ;
	}
	return 0 ;
}

unsigned char CGoldenRoomData::OnPlayerPKTimes(unsigned int nSessionID , unsigned char nNewPKTimes )
{
	stGoldenPeerData* pData = (stGoldenPeerData*)GetPeerDataBySessionID(nSessionID);
	if ( !pData )
	{
		return 10 ;
	}

	if ( IS_STATE(pData->nPeerState,eRoomPeer_Golden_Playing) == false )
	{
		return 1 ;   // state error ;
	}

	if ( pData->nPKTimes >= nNewPKTimes )
	{
		return 2 ; // must big than current ;
	}

	CLogMgr::SharedLogMgr()->ErrorLog("Must check PK Times card") ;
	pData->nPKTimes = nNewPKTimes ;
	return 0 ;
}

bool CGoldenRoomData::MovePeerHoldCardToBuffer(char* pBuffer,unsigned short nSize )
{
	unsigned char nMax = GetDataOnly()->cMaxPlayingPeers ;
	stGoldenHoldPeerCard vCard ;

	for ( int i = 0 ; i < nMax ; ++i )
	{
		stGoldenPeerData* pPeer = (stGoldenPeerData*)m_vPeerDatas[i];
		if ( pPeer == NULL || IS_STATE(pPeer->nPeerState,eRoomPeer_Golden_Playing) == false )
			continue;
		if ( nSize < sizeof(vCard) )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("move peer hold Card to Buffer error, buffer is too small") ;
			return false ;
		}
		vCard.nIdx = pPeer->cRoomIdx ;
		memcpy(vCard.vCard,pPeer->vHoldCard,sizeof(vCard.vCard));
		memcpy(pBuffer,&vCard,sizeof(vCard));
		pBuffer += sizeof(vCard);
		nSize -= sizeof(vCard);
	}
	return true ;
}

unsigned char CGoldenRoomData::GetPeerHoldCardFromBuffer(char* pBuffer,unsigned char nPeerCnt)
{
	stGoldenHoldPeerCard* pCards = (stGoldenHoldPeerCard*)pBuffer ;
	unsigned char nRealCnt = 0 ;
	while(nPeerCnt--)
	{
		stGoldenPeerData* pData = (stGoldenPeerData*)GetPeerDataByIdx(pCards->nIdx) ;
		if ( pData == NULL )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("hold card from buffer error , peer with idx = %d , is NULL",pCards->nIdx) ;
			continue;
		}
		memcpy(pData->vHoldCard,pCards->vCard,sizeof(pData->vHoldCard));
		++pCards ;
		++nRealCnt ;
	}
	return nRealCnt ;
}

bool CGoldenRoomData::CheckCanPlayerBeKickedRightNow(unsigned char nTargetIdx )
{
	CLogMgr::SharedLogMgr()->ErrorLog("please processed here") ;
	return true ;
}

