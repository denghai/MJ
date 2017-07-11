#include "GameRank.h"
#include "GameServerApp.h"
#include <time.h>
#include "LogManager.h"
#include "Player.h"
#include "Player.h"
#include "PlayerBaseData.h"
#include "EventCenter.h"
#define  RANK_REFRESH_ELASP 60 * 60      //an hour
void stRankPeerInfo::OnPlayerOffline( CPlayer*pPlayerOffline )
{
	if ( !pPlayer || pPlayer->GetSessionID() > pPlayerOffline->GetSessionID() )  // pre object  delay offlien 
	{
		return ;
	}

	stDetailData.bIsOnLine = false ;
	pPlayer = NULL ;
}

void stRankPeerInfo::OnPlayerOnline(CPlayer*pPlayerOnline )
{
	pPlayer = pPlayerOnline;
	//pPlayer->GetBaseData()->GetPlayerBrifData(&tBrifData) ;  // can not update brife data , or may rank error ;
	stDetailData.bIsOnLine = true ;
}

void stRankPeerInfo::GetBrifePlayerData(stPlayerBrifData* poutData )
{
	if ( pPlayer )
	{
		pPlayer->GetBaseData()->GetPlayerBrifData(poutData) ;
	}
	else
	{
		memcpy(poutData,&stDetailData,sizeof(stPlayerBrifData));
	}
}

void stRankPeerInfo::SetPeerInfo(stServerGetRankPeerInfo* pData )
{
	memcpy(&stDetailData,&pData->tDetailData,sizeof(stPlayerDetailData));
	nYesterDayWin = pData->nYesterDayWin ;
}

CGamerRank::CGamerRank()
{
	for ( int i = 0 ; i < eRank_Max ; ++i )
	{
		ClearPeerInfos(m_vRank[i]) ;
	}
}

CGamerRank::~CGamerRank()
{
	for ( int i = 0 ; i < eRank_Max ; ++i )
	{
		ClearPeerInfos(m_vRank[i]) ;
	}

	CEventCenter::SharedEventCenter()->RemoveEventListenner(eEvent_PlayerOnline,this,CGamerRank::EventFunc ) ;
	CEventCenter::SharedEventCenter()->RemoveEventListenner(eEvent_PlayerOffline,this,CGamerRank::EventFunc ) ;
	CEventCenter::SharedEventCenter()->RemoveEventListenner(eEvent_NewDay,this,CGamerRank::EventFunc ) ;
}

void CGamerRank::Init()
{
	memset(m_vRefreshTime,0,sizeof(m_vRefreshTime)) ;
	for ( int i = 0 ; i < eRank_Max ; ++i )
	{
		ClearPeerInfos(m_vRank[i]) ;
		RequestNewRank((eRankType)i);
	}

	stMsgGameServerRequestRank msg ;
	msg.eType = eRank_YesterDayWin  ;
	CGameServerApp::SharedGameServerApp()->SendMsgToDBServer((char*)&msg,sizeof(msg)) ;

	CEventCenter::SharedEventCenter()->RegisterEventListenner(eEvent_PlayerOnline,this,CGamerRank::EventFunc ) ;
	CEventCenter::SharedEventCenter()->RegisterEventListenner(eEvent_PlayerOffline,this,CGamerRank::EventFunc ) ;
	CEventCenter::SharedEventCenter()->RegisterEventListenner(eEvent_NewDay,this,CGamerRank::EventFunc ) ;
}

void CGamerRank::RequestNewRank(eRankType eType )
{
	if ( eType < 0 || eType >= eRank_Max )
	{
		CLogMgr::SharedLogMgr()->ErrorLog( "Not proper rank type = %d",eType ) ;
		return ;
	}

	if ( eType == eRank_YesterDayWin )
	{
		return ; // only new day event request ;
	}

	time_t t = time(NULL) ;
	if ( t - m_vRefreshTime[eType] < RANK_REFRESH_ELASP )
	{
		return ;
	}
	m_vRefreshTime[eType] = t ;
	stMsgGameServerRequestRank msg ;
	msg.eType = eType ;
	CGameServerApp::SharedGameServerApp()->SendMsgToDBServer((char*)&msg,sizeof(msg)) ;
}

bool CGamerRank::OnMessage(RakNet::Packet* packet )
{
	stMsg* pMsg = (stMsg*)packet->data ;
	if ( pMsg->cSysIdentifer == ID_MSG_DB2GM && MSG_REQUEST_RANK == pMsg->usMsgType )
	{
		// parse the message ;
		stMsgGameServerRequestRankRet* pMsgRet = (stMsgGameServerRequestRankRet*)pMsg ;
		if  ( pMsgRet->nPeerCount == 0 )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("Request rank from db error ; ranktype = %d",pMsgRet->eType ) ;
			return true ;
		}
		VEC_RANKPEER_INFO& vInfos = m_vRank[pMsgRet->eType];
		ClearPeerInfos(vInfos) ;
		char* pBuffer = (char*)pMsgRet ;
		pBuffer += sizeof(stMsgGameServerRequestRankRet);
		unsigned char nRankIdx = 0 ;
		stServerGetRankPeerInfo* pGetInfo = (stServerGetRankPeerInfo*)pBuffer ;
		while( pMsgRet->nPeerCount--)
		{
			stRankPeerInfo* pInfo = new stRankPeerInfo ;
			pInfo->SetPeerInfo(pGetInfo) ;
			++pGetInfo ;
			vInfos.push_back(pInfo) ;
		}
		UpdateOnlineState((eRankType)pMsgRet->eType) ;
		return true ;
	}
	return false ;
}

void CGamerRank::SendRankToPlayer(CPlayer* pPlayer , eRankType eType, unsigned char nFromIdx , unsigned char nCount )
{
	RequestNewRank(eType);
	stMsgPlayerRequestRankRet msg ;
	msg.nRankType = eType ;
	msg.nRet = 0 ;
	if ( eType < 0 || eType >= eRank_Max || nCount == 0 )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("Not Proper rank type = %d",eType) ;
		msg.nCount = 0 ;
		msg.nRet = 1 ;
		pPlayer->SendMsgToClient((char*)&msg,sizeof(msg)) ;
		return ;
	}

	VEC_RANKPEER_INFO& vInfos = m_vRank[eType] ;
	if ( nFromIdx >= vInfos.size() )
	{
		nFromIdx = vInfos.size() - 1 ;
	}
	msg.nCount = (vInfos.size() - nFromIdx) < nCount ? (vInfos.size() - nFromIdx) : nCount ;
	char* pBuffer = new char[msg.nCount * sizeof(stRankPeerBrifInfo) + sizeof(msg)] ;
	memcpy(pBuffer,&msg,sizeof(msg));

	unsigned char nIdx = nFromIdx ;
	nCount = msg.nCount ;
	stRankPeerBrifInfo* pSendInfo = (stRankPeerBrifInfo*)(pBuffer + sizeof(msg));
	while( nCount--)
	{
		if ( vInfos.size() <= nIdx )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("why nIdx is big than count ?") ;
			msg.nCount -= (1+nCount) ;
			memcpy(pBuffer,&msg,sizeof(msg));  // avoid client crash ;
			break;
		}
		vInfos[nIdx]->GetBrifePlayerData(&pSendInfo->tBrifData);
		//memcpy(&pSendInfo->tBrifData , &vInfos[nIdx]->stDetailData,sizeof(stPlayerBrifData));
		pSendInfo->nRankIdx = nIdx ;
		pSendInfo->nSingleWinMost = vInfos[nIdx]->stDetailData.nSingleWinMost ;
		pSendInfo->nYesterDayWin = vInfos[nIdx]->nYesterDayWin ;
		++pSendInfo ;
		++nIdx ;
	}

	pPlayer->SendMsgToClient(pBuffer,msg.nCount * sizeof(stRankPeerBrifInfo) + sizeof(msg)) ;
	delete[] pBuffer ;
}

void CGamerRank::SendRankDetailToPlayer(CPlayer* pPlayer , unsigned int nRankPeerUserUID, eRankType eType )
{
	stRankPeerInfo* pInfoPeer = GetRankPeerInfo(nRankPeerUserUID,eType) ;
	stMsgPlayerRequestRankPeerDetailRet msg ;
	msg.nRet = 0 ;
	if ( !pInfoPeer )
	{
		msg.nRet = 1 ;
		pPlayer->SendMsgToClient((char*)&msg,sizeof(msg)) ;
		return ;
	}
	if ( pInfoPeer->pPlayer )
	{
		pInfoPeer->pPlayer->GetBaseData()->GetPlayerDetailData(&msg.stDetailInfo);
	}
	else
	{
		memcpy(&msg.stDetailInfo,&pInfoPeer->stDetailData,sizeof(stPlayerDetailData));
	}
	pPlayer->SendMsgToClient((char*)&msg,sizeof(msg)) ;
}

void CGamerRank::ClearPeerInfos(VEC_RANKPEER_INFO& vInfos )
{
	VEC_RANKPEER_INFO::iterator iter = vInfos.begin() ;
	for ( ; iter != vInfos.end(); ++iter )
	{
		delete *iter ;
		*iter = NULL ;
	}
	vInfos.clear() ;
}

stRankPeerInfo* CGamerRank::GetRankPeerInfo(unsigned int nUserUID, eRankType eType )
{
	if ( eType < 0 || eType >= eRank_Max )
	{
		return NULL ;
	}

	VEC_RANKPEER_INFO& vInfo = m_vRank[eType] ;
	VEC_RANKPEER_INFO::iterator iter = vInfo.begin() ;
	for ( ; iter != vInfo.end(); ++iter )
	{
		if ( (*iter)->stDetailData.nUserUID == nUserUID )
		{
			return (*iter);
		}
	}
	return NULL ;
}

void CGamerRank::UpdateOnlineState(  eRankType eType )
{
	if ( eType < 0 || eType >= eRank_Max )
	{
		CLogMgr::SharedLogMgr()->ErrorLog( "Not proper rank type = %d",eType ) ;
		return ;
	}

	VEC_RANKPEER_INFO::iterator iter = m_vRank[eType].begin() ;
	for ( ; iter != m_vRank[eType].end(); ++iter )
	{
		stRankPeerInfo* pInofr = *iter ;
		CPlayer* pPlaye = CGameServerApp::SharedGameServerApp()->GetPlayerMgr()->GetPlayerByUserUID(pInofr->stDetailData.nUserUID) ;
		if ( pPlaye)
		{
			pInofr->OnPlayerOnline(pPlaye) ;
		}
	}
}

bool CGamerRank::EventFunc(void* pUserData,stEventArg* pArg)
{
	CGamerRank* pF = (CGamerRank*)pUserData ;
	pF->OnProcessEvent(pArg) ;
	return false ;
}

void CGamerRank::OnProcessEvent(stEventArg* pArg)
{
	switch ( pArg->cEvent )
	{
	case eEvent_NewDay:
		{
			stMsgGameServerRequestRank msg ;
			msg.eType = eRank_YesterDayWin  ;
			CGameServerApp::SharedGameServerApp()->SendMsgToDBServer((char*)&msg,sizeof(msg)) ;
		}
		break;
	case eEvent_PlayerOffline:
	case eEvent_PlayerOnline:
		{
			CPlayer* p = (CPlayer*)pArg->pData ;
			for ( int i = 0 ; i < eRank_Max ; ++i )
			{
				stRankPeerInfo* pinfo = GetRankPeerInfo(p->GetUserUID(),(eRankType)i) ;
				if ( pinfo )
				{
					if (pArg->cEvent == eEvent_PlayerOffline )
					{
						pinfo->OnPlayerOffline(p) ;
						CLogMgr::SharedLogMgr()->PrintLog("rank a peer offline");
					}
					else if ( eEvent_PlayerOnline == pArg->cEvent )
					{
						pinfo->OnPlayerOnline(p) ;
						CLogMgr::SharedLogMgr()->PrintLog("rank a peer online");
					}
				}
			}
		}
		break;
	default:
		break;
	}
}