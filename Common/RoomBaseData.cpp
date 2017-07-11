#include "RoomBaseData.h"
#include <iostream>
#include "LogManager.h"
CRoomBaseData::CRoomBaseData()
	:m_pData(NULL)
{

}

CRoomBaseData::~CRoomBaseData()
{
	ClearAllPeers();
	if ( m_pData )
	{
		delete m_pData ;
		m_pData = NULL ;
	}
}

void CRoomBaseData::Init()
{
	m_pData = NULL ;
	m_vSessionIDs.clear();
}

void CRoomBaseData::OnStartGame(stMsg*pmsg)
{

}

void CRoomBaseData::OnEndGame()
{
	if ( m_vSessionIDs.empty() )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("why not precess this kicked player ") ;
	}
}

unsigned char CRoomBaseData::AddPeer(stPeerBaseData* peerData,unsigned char nRoomIdx)
{
	if ( GetEmptySeatCnt() <= 0 )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("can not add peer to room , id = %d",peerData->nUserUID);
		return -1;
	}

	if ( m_vPeerDatas == 0 && m_pData->cMaxPlayingPeers != 0 )
	{
		m_vPeerDatas = new stPeerBaseData*[m_pData->cMaxPlayingPeers] ;
		memset(m_vPeerDatas,0,sizeof(stPeerBaseData*) * m_pData->cMaxPlayingPeers) ;
	}
	
	if ( nRoomIdx < m_pData->cMaxPlayingPeers && m_vPeerDatas[nRoomIdx] == NULL )
	{
		m_vPeerDatas[nRoomIdx] = peerData ;
		peerData->cRoomIdx = nRoomIdx ;
		return nRoomIdx ;
	}

	for ( unsigned char idx = 0 ; idx < m_pData->cMaxPlayingPeers ; ++idx )
	{
		if ( m_vPeerDatas[idx] == NULL )
		{
			m_vPeerDatas[idx] = peerData ;
			peerData->cRoomIdx = idx ;
			return idx; 
		}
	}
	return 0 ;
}

void CRoomBaseData::RemovePeer(unsigned int nSessionID )
{
	for ( unsigned char idx = 0 ; idx < m_pData->cMaxPlayingPeers ; ++idx )
	{
		if ( m_vPeerDatas[idx] && m_vPeerDatas[idx]->nSessionID == nSessionID )
		{
			delete m_vPeerDatas[idx] ;
			m_vPeerDatas[idx] = NULL ;
			break; 
		}
	}

	VEC_KICKED_SESSIONID::iterator iter = m_vSessionIDs.begin();
	for ( ; iter != m_vSessionIDs.end(); ++iter )
	{
		if ( (*iter) == nSessionID )
		{
			m_vSessionIDs.erase(iter) ;
			break;
		}
	}
}

stPeerBaseData* CRoomBaseData::GetPeerDataBySessionID(unsigned int nSessionID )
{
	for ( unsigned char idx = 0 ; idx < m_pData->cMaxPlayingPeers ; ++idx )
	{
		if ( m_vPeerDatas[idx] && m_vPeerDatas[idx]->nSessionID == nSessionID )
		{
			return m_vPeerDatas[idx] ;  
		}
	}
	return NULL ;
}

stPeerBaseData* CRoomBaseData::GetPeerDataByIdx(unsigned char cIdx )
{
	if ( cIdx >= m_pData->cMaxPlayingPeers )
		return NULL ;
	return m_vPeerDatas[cIdx] ;
}

stPeerBaseData* CRoomBaseData::GetPeerDataByUserUID(unsigned int nUserUID)
{
	for ( unsigned char idx = 0 ; idx < m_pData->cMaxPlayingPeers ; ++idx )
	{
		if ( m_vPeerDatas[idx] && m_vPeerDatas[idx]->nUserUID == nUserUID )
		{
			return m_vPeerDatas[idx] ;  
		}
	}
	return NULL ;
}

unsigned char CRoomBaseData::GetEmptySeatCnt()
{
	unsigned char nCnt = 0 ;
	for ( unsigned char idx = 0 ; idx < m_pData->cMaxPlayingPeers ; ++idx )
	{
		if ( m_vPeerDatas[idx] == NULL  )
		{
			++nCnt ;  
		}
	}
	return nCnt ;
}


unsigned char CRoomBaseData::GetPlayingSeatCnt()
{
	return m_pData->cMaxPlayingPeers - GetEmptySeatCnt() ;
}

unsigned char CRoomBaseData::GetMaxSeat()
{
	return m_pData->cMaxPlayingPeers ;
}

char CRoomBaseData::GetRoomIdxBySessionID(unsigned int nSessionID )
{
	stPeerBaseData* pData = GetPeerDataBySessionID(nSessionID) ;
	if ( !pData )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("this peer may not in this room session id = %d",nSessionID) ;
		return -1 ;
	}
	return pData->cRoomIdx ;
}

unsigned int CRoomBaseData::GetSessionIDByIdx(unsigned char nIdx)
{
	stPeerBaseData* pData = GetPeerDataByIdx(nIdx) ;
	if ( !pData )
	{
		return 0 ;
	}
	return pData->nSessionID ;
}

void CRoomBaseData::SetRoomState(unsigned short nRoomState )
{
	m_pData->cCurRoomState = nRoomState ;
	m_pData->fTimeTick = 0 ;
}

unsigned char CRoomBaseData::OnPlayerKick(unsigned int nSessionID,unsigned char nTargetIdx,bool* bRightKicked )
{
	stPeerBaseData* pData = GetPeerDataBySessionID(nSessionID);
	if ( !pData )   
	{
		// you are not sit 
		return 2 ; 
	}

	stPeerBaseData* pTargetData = GetPeerDataByIdx(nTargetIdx) ;
	if ( !pData )
	{
		return 3 ;  // target not sit 
	}

	if ( pData == pTargetData )
	{
		return 4 ; // can not kick self 
	}

	bool bRightNow = CheckCanPlayerBeKickedRightNow(nTargetIdx) ;
	if (bRightKicked)
	{
		*bRightKicked = bRightNow  ;
	}

	if (bRightKicked )
	{
		RemovePeer(pTargetData->nSessionID) ;
	}
	else
	{
		m_vSessionIDs.push_back(pTargetData->nSessionID) ;
	}

	return 0 ;
}

unsigned char CRoomBaseData::OnCheckDelayKickPlayers(std::vector<unsigned int>& vKickedPlayer )
{
	if ( m_vSessionIDs.empty() )
	{
		vKickedPlayer.clear();
		return 0 ;
	}

	VEC_KICKED_SESSIONID vKickTem (m_vSessionIDs.begin(),m_vSessionIDs.end());
	VEC_KICKED_SESSIONID::iterator iter = vKickTem.begin();
	for ( ; iter != vKickTem.end(); ++iter )
	{
		stPeerBaseData* pData = GetPeerDataBySessionID( *iter );
		if ( !pData )   
		{
			continue; ;
		}
		unsigned int nValue = *iter ;
#ifndef GAME_SERVER
		nValue = pData->cRoomIdx ;
#endif
		if ( DoBeKickedPlayer(*iter) )
		{
			vKickedPlayer.push_back(nValue) ;
		}
	}
	return vKickedPlayer.size();
}

//bool CRoomBaseData::CheckCanPlayerBeKickedRightNow(unsigned char nTargetIdx )
//{
//	return false ;
//}

bool CRoomBaseData::DoBeKickedPlayer(unsigned int nSessionID )
{
	VEC_KICKED_SESSIONID::iterator iter = m_vSessionIDs.begin() ;
	bool bKick = false ;
	for ( ; iter != m_vSessionIDs.end() ; ++iter )
	{
		if ( *iter == nSessionID )
		{
			m_vSessionIDs.erase(iter) ;
			bKick = true ;
			break; ;
		}
	}

	if ( !bKick )
	{
		return false ;
	}

	stPeerBaseData* pData = GetPeerDataBySessionID(nSessionID);
	if ( !pData )   
	{
		false ;
	}

	bool bRightNow = CheckCanPlayerBeKickedRightNow(pData->cRoomIdx) ;
	if ( bRightNow == false )
	{
		m_vSessionIDs.push_back(nSessionID);
		return false ;
	}
	RemovePeer(nSessionID) ;
	return true ;
}

unsigned short CRoomBaseData::GetKickSessionIDsToBuffer(const char* pBuffer )
{
	unsigned short nSize = m_vSessionIDs.size() * sizeof(unsigned int);
	VEC_KICKED_SESSIONID::iterator iter = m_vSessionIDs.begin() ;
	unsigned int* pIntTem = (unsigned int*)pBuffer ;
	for ( ; iter != m_vSessionIDs.end() ; ++iter )
	{
		*pIntTem = *iter ;
		++pIntTem ;
	}
	return nSize ;
}

void CRoomBaseData::ClearAllPeers()
{
	for ( unsigned char idx = 0 ; idx < m_pData->cMaxPlayingPeers ; ++idx )
	{
		if ( m_vPeerDatas[idx] )
		{
			delete m_vPeerDatas[idx] ;
			m_vPeerDatas[idx] = NULL ;
		}
	}
	delete[] m_vPeerDatas ;
}