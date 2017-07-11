#include "RoomManager.h"
#include "Player.h"
#include "LogManager.h"
#include "PlayerBaseData.h"
#include "RoomTexasPoker.h"
#include "ConfigManager.h"
#include "RoomConfig.h"
#include "GameServerApp.h"
#include "RobotManager.h"
#include "BaccaratRoom.h"
#include "RoomConfig.h"
#include "RoomBaseData.h"
#include "RoomBaseNew.h"
CGameRooms::CGameRooms()
{
	for ( int i = eRoomLevel_None ; i < eRoomLevel_Max; ++ i )
	{
		m_vRooms[i].clear() ;
	}
}

CGameRooms::~CGameRooms()
{
	for ( int i = eRoomLevel_None ; i < eRoomLevel_Max; ++i )
	{
		MAP_ROOM& room = m_vRooms[i] ;
		MAP_ROOM::iterator iter = room.begin() ;
		for ( ; iter != room.end(); ++iter )
		{
			delete iter->second ;
			iter->second = NULL ;
		}
		room.clear() ;
	}
}

void CGameRooms::AddRoom(CRoomBaseNew* pRoom, eRoomLevel eLevel )
{
	if ( pRoom == NULL )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("Add Room Room is NULL ") ;
		return ;
	}

	if ( eLevel < eRoomLevel_None || eLevel >= eRoomLevel_Max )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("illegal RoomLevel level = %d",eLevel)  ;
		return ;
	}
	m_vRooms[eLevel][pRoom->GetRoomID()] = pRoom ;
	RemoveEmptyRoom();
}

void CGameRooms::RemoveEmptyRoom()
{
	for ( int i = eRoomLevel_None; i < eRoomLevel_Max ; ++i )
	{
		RemoveEmptyRoom(m_vRooms[i],8) ;
	}
}

void CGameRooms::RemoveEmptyRoom(MAP_ROOM& vRooms, unsigned short nLeftEmpty )
{
	return ;
	MAP_ROOM::iterator iter = vRooms.begin() ;
	MAP_ROOM vWillRemove ;
	for ( ; iter != vRooms.end(); ++iter )
	{
		if ( iter->second->GetData()->GetEmptySeatCnt() == iter->second->GetData()->GetMaxSeat() )
		{
			if ( nLeftEmpty == 0 )
			{
				vWillRemove[iter->first] = iter->second ;
			}
			else
			{
				--nLeftEmpty ;
			}
		}
	}

	if ( nLeftEmpty > 0 )
	{
		return ;
	}

	MAP_ROOM::iterator iterDel = vWillRemove.begin() ;
	for ( ; iterDel != vWillRemove.end() ; ++iterDel )
	{
		iter = vRooms.begin() ;
		for ( ; iter != vRooms.end() ; ++iter )
		{
			if ( iter->first == iterDel->first )
			{
				delete iterDel->second ;
				vRooms.erase(iter) ;
				break;
			}
		}
	}
	vWillRemove.clear() ;
}

CRoomBaseNew* CGameRooms::GetRoomByID(unsigned int nRoomID, eRoomLevel eLevel )
{
	if ( eLevel < eRoomLevel_None || eLevel >= eRoomLevel_Max )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("GetRoomByID illegal RoomLevel level = %d",eLevel)  ;
		return NULL;
	}

	if ( nRoomID != 0 )
	{
		MAP_ROOM::iterator iter = m_vRooms[eLevel].find(nRoomID);
		if ( iter != m_vRooms[eLevel].end())
		{
			return iter->second ;
		}
	}
	else   // rand a room ;
	{
		unsigned int nSize = m_vRooms[eLevel].size();
		if ( 0 == nSize )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("no room in this level , can not rand a room for you !") ;
			return NULL ;
		}
		int nOmitCnt = rand() % nSize ;
		MAP_ROOM::iterator iter = m_vRooms[eLevel].begin() ;
		for ( ; iter != m_vRooms[eLevel].end() ; ++iter , --nOmitCnt )
		{
			if ( nOmitCnt > 0 )
			{
				continue;
			}

			if ( iter->second->GetData()->GetEmptySeatCnt() > 0 && iter->second->GetData()->GetEmptySeatCnt() != iter->second->GetData()->GetMaxSeat() ) // don't put player in a room, juset have himself ; 
			{
				return iter->second ;
			}
		}

		// second round ;
		iter = m_vRooms[eLevel].begin() ;
		for ( ; iter != m_vRooms[eLevel].end() ; ++iter )
		{
			if ( iter->second->GetData()->GetEmptySeatCnt() > 0 )
			{
				return iter->second ;
			}
		}
	}
	return NULL ;
}

int CGameRooms::GetRoomCount(eRoomLevel eLevel )
{
	if ( eLevel < eRoomLevel_None || eLevel >= eRoomLevel_Max )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("GetRoomCount illegal RoomLevel level = %d",eLevel)  ;
		return 0;
	}

	return m_vRooms[eLevel].size();
}

int CGameRooms::GetAllRoomCount()
{
	int nCount = 0 ;
	for ( int i = eRoomLevel_None; i < eRoomLevel_Max; ++i )
	{
		nCount += GetRoomCount((eRoomLevel)i) ;
	}
	return nCount ;
}


// Room Manager 
unsigned int CRoomManager::s_RoomID = 1 ;
CRoomManager::CRoomManager()
{

}

CRoomManager::~CRoomManager()
{

}

void CRoomManager::Init()
{ 
	//CreateRoom(eRoom_PaiJiu,eRoomLevel_Junior) ;
	//CreateRoom(eRoom_PaiJiu,eRoomLevel_Advanced) ;
	//CreateRoom(eRoom_PaiJiu,eRoomLevel_Middle) ;

	//// diamoned
	//int nDiamonedRoomCnt = 2 ;
	//while ( nDiamonedRoomCnt-- )
	//{
	//	CreateRoom(eRoom_TexasPoker_Diamoned,eRoomLevel_Junior,true) ;
	//}
	////return ;

	//CreateRoom(eRoom_TexasPoker_Diamoned,eRoomLevel_Middle,true) ;
	//CreateRoom(eRoom_TexasPoker_Diamoned,eRoomLevel_Advanced,true) ;
	//CreateRoom(eRoom_TexasPoker_Diamoned,eRoomLevel_Super,true) ;

	//// coin
	//int nCoinRoomCnt = 4 ;
	//while( nCoinRoomCnt-- )
	//{
	//	CreateRoom(eRoom_TexasPoker,eRoomLevel_Junior) ;
	//	CreateRoom(eRoom_TexasPoker,eRoomLevel_Middle) ;
	//	//CreateRoom(eRoom_TexasPoker,eRoomLevel_Advanced) ;
	//	//CreateRoom(eRoom_TexasPoker,eRoomLevel_Super) ;
	//}

	//nCoinRoomCnt = 2 ;
	//while( nCoinRoomCnt-- )
	//{
	//	//CreateRoom(eRoom_TexasPoker,eRoomLevel_Junior) ;
	//	//CreateRoom(eRoom_TexasPoker,eRoomLevel_Middle) ;
	//	CreateRoom(eRoom_TexasPoker,eRoomLevel_Advanced) ;
	//	CreateRoom(eRoom_TexasPoker,eRoomLevel_Super) ;
	//}
	CRoomConfigMgr* pRoomConfigMgr = (CRoomConfigMgr*)CGameServerApp::SharedGameServerApp()->GetConfigMgr()->GetConfig(CConfigManager::eConfig_Room) ;
	CRoomConfigMgr::LIST_ITER iter = pRoomConfigMgr->GetBeginIter();
	CRoomConfigMgr::LIST_ITER iter_end = pRoomConfigMgr->GetEndIter();
	for ( ; iter != iter_end ; ++iter )
	{
		stBaseRoomConfig* pBase = *iter ;
		int nCount = pBase->nCreateCount ;
		while ( nCount-- )
		{
			CreateRoom(pBase) ;
		}
	}
}

CRoomBaseNew* CRoomManager::GetRoom(char cRoomType , char cRoomLevel, unsigned int nRoomID )
{ 
	if ( cRoomType <= eRoom_None || cRoomType >= eRoom_Max )
		return NULL ;
	return m_vGames[cRoomType].GetRoomByID(nRoomID,(eRoomLevel)cRoomLevel) ;
}

CRoomBaseNew* CRoomManager::CreateRoom(unsigned int nRoomID)
{
	CRoomConfigMgr* pRoomConfigMgr = (CRoomConfigMgr*)CGameServerApp::SharedGameServerApp()->GetConfigMgr()->GetConfig(CConfigManager::eConfig_Room) ;
	stBaseRoomConfig* pRoomConfig = pRoomConfigMgr->GetRoomConfig(nRoomID) ;
	if ( !pRoomConfig)
	{
		CLogMgr::SharedLogMgr()->ErrorLog( "can not get room config room id = %d",nRoomID ) ;
		return NULL ;
	}

	return CreateRoom(pRoomConfig);
}

CRoomBaseNew* CRoomManager::CreateRoom(stBaseRoomConfig* pRoomConfig )
{
	if ( pRoomConfig == NULL )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("room config is NULL") ;
		return NULL ;
	}

	CRoomBaseNew* pRoom = NULL ;
	unsigned char roomType = pRoomConfig->nRoomType ;
	switch ( roomType )
	{
	//case eRoom_PaiJiu:
	//	{
	//		stPaiJiurRoomConfig* pPJConifg = (stPaiJiurRoomConfig*)pRoomConfig ;
	//		pRoom = new CRoomPaiJiu ;
	//		pRoom->Init( ++s_RoomID,pPJConifg->nMaxSeat) ;
	//		pRoom->SetAntesCoin(pPJConifg->nMinNeedToEnter) ;
	//		((CRoomPaiJiu*)pRoom)->SetBankerEquairedCoin(pPJConifg->nBankerNeedCoin) ;
	//	}
	//	break;
	//case eRoom_Gold:
	//	{

	//	}
	//	break ;
	//case eRoom_TexasPoker_Private:
	//	{
	//		pRoom = new CRoomTexasPoker ;
	//		pRoom->Init( ++s_RoomID,9 ) ;
	//	}
	//	break;
	case eRoom_TexasPoker_Diamoned:
	case eRoom_TexasPoker:
		{
// 			stTaxasRoomConfig* pTPConifg = (stTaxasRoomConfig*)pRoomConfig ;
// 			pRoom = new CRoomTexasPoker ;
// 			pRoom->Init( ++s_RoomID,pTPConifg->nMaxSeat ) ;
// 			pRoom->SetAntesCoin(pTPConifg->nMinNeedToEnter) ;
// 			pRoom->SetMaxTakeInCoin(pTPConifg->nMaxTakeInCoin);
// 			((CRoomTexasPoker*)pRoom)->SetBigBlindBet(pTPConifg->nBigBlind);
		}
		break;
	case eRoom_Gold:
		{
			pRoom = new CRoomGoldenNew ;
			pRoom->Init(pRoomConfig);
		}
		break;
	default:
		{
			CLogMgr::SharedLogMgr()->SystemLog("Unknown room Type Can not create Room , type = %d",roomType ) ;
		}
		return pRoom; 
	}

	if ( pRoom != NULL )
	{
		m_vGames[roomType].AddRoom(pRoom,(eRoomLevel)pRoomConfig->nRoomLevel ) ;
		CRobotManager::SharedRobotMgr()->RequestRobotToJoin(pRoom) ;
	}
	//AddRoomToType(pRoom);
	return pRoom ;
}

CRoomBaseNew* CRoomManager::CreateRoom(char cRoomType , char cRoomLevel)
{
	CRoomConfigMgr* pRoomConfigMgr = (CRoomConfigMgr*)CGameServerApp::SharedGameServerApp()->GetConfigMgr()->GetConfig(CConfigManager::eConfig_Room) ;
	stBaseRoomConfig* pRoomConfig = pRoomConfigMgr->GetRoomConfig(cRoomType,cRoomLevel) ;
	if ( !pRoomConfig)
	{
		CLogMgr::SharedLogMgr()->ErrorLog( "can not get room config room level = %d",cRoomLevel ) ;
		return NULL ;
	}

	return CreateRoom(pRoomConfig);
}


void CRoomManager::SendRoomListToPlayer( CPlayer* pTargetPlayer , unsigned char eType, unsigned char cRoomLevel )
{
// 	stMsgRequestRoomListRet msgBack ;
// 	msgBack.cRoomType = eType ;
// 	msgBack.cRoomLevel = cRoomLevel;
// 	if ( eType <= eRoom_None || eType >= eRoom_Max )
// 	{
// 		CLogMgr::SharedLogMgr()->ErrorLog("Quest unknown roomType type = %d, playerName = %s",eType,pTargetPlayer->GetBaseData()->GetPlayerName() ) ;
// 		msgBack.nRoomCount = 0 ;
// 		pTargetPlayer->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
// 		return ;
// 	}
// 	
// 	CGameRooms& pGameRoom = m_vGames[eType] ;
// 	msgBack.nRoomCount = pGameRoom.GetRoomCount((eRoomLevel)cRoomLevel) ;
// 	int nOffset = 0 ;
// 	char* pBuffer = NULL ;
// 	if ( eType == eRoom_TexasPoker_Private )
// 	{
// 		msgBack.nRoomCount = 0 ;
// 		pTargetPlayer->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
// 		CLogMgr::SharedLogMgr()->ErrorLog("no private room this game, playerName = %s",pTargetPlayer->GetBaseData()->GetPlayerName() ) ;
// 		return ;
// 		//pBuffer = new char[ sizeof(msgBack) + sizeof(stPrivateRoomListItem) * msgBack.nRoomCount ] ;
// 		//memcpy(pBuffer,&msgBack, sizeof(msgBack));
// 		//nOffset += sizeof(msgBack);
// 
// 		//stPrivateRoomListItem item ;
// 		//for ( int i = eRoom_None ; i < eRoomLevel_Max ; ++i )
// 		//{
// 		//	CGameRooms::MAP_ROOM& vMap = pGameRoom.m_vRooms[i] ;
// 		//	CGameRooms::MAP_ROOM::iterator iter = vMap.begin();
// 		//	for ( ; iter !=vMap.end(); ++iter )
// 		//	{
// 		//		item.cRoomLevel = i ;
// 		//		item.nCurrentCount = iter->second->GetRoomPeerCount() ;
// 		//		item.nMaxCount = iter->second->GetMaxSeat() ;
// 		//		item.nRoomID = iter->first ;
// 		//		// new add 
// 		//		item.nBigBlind = ((CRoomTexasPoker*)iter->second)->GetBigBlindBet();
// 		//		item.nMinCoinToTake = (iter->second)->GetAntesCoin();
// 		//		item.nMaxCoinToTake = (iter->second)->GetMaxTakeInCoin();
// 		//		item.nWaitOperateTime = iter->second->GetWaitOperateTime();
// 		//		item.bDiamoned = iter->second->IsDiamonedRoom() ;
// 		//		item.bPassword = iter->second->GetPassword() > 0 ;
// 		//		memset(item.cRoomName,0, sizeof(item.cRoomName)) ;
// 		//		sprintf(item.cRoomName,"%s",iter->second->GetRoomName().c_str());
// 		//		memcpy(pBuffer + nOffset , &item, sizeof(item));
// 		//		nOffset += sizeof(item);
// 		//	}
// 		//}
// 	}
// 	else
// 	{
// 		pBuffer = new char[ sizeof(msgBack) + sizeof(stRoomListItem) * msgBack.nRoomCount ] ;
// 		memcpy(pBuffer,&msgBack, sizeof(msgBack));
// 		nOffset += sizeof(msgBack);
// 
// 		stRoomListItem item ;
// 		CGameRooms::MAP_ROOM& vMap = pGameRoom.m_vRooms[cRoomLevel] ;
// 		CGameRooms::MAP_ROOM::iterator iter = vMap.begin();
// 		for ( ; iter !=vMap.end(); ++iter )
// 		{
// 			item.cRoomLevel = cRoomLevel ;
// 			item.nCurrentCount = iter->second->GetRoomPeerCount() ;
// 			item.nMaxCount = iter->second->GetMaxSeat() ;
// 			item.nRoomID = iter->first ;
// 			item.nWaitOperateTime = iter->second->GetWaitOperateTime();
// 			item.nBigBlind = ((CRoomTexasPoker*)iter->second)->GetBigBlindBet();
// 
// 			item.nMinCoinToTake = (iter->second)->GetAntesCoin();
// 			item.nMaxCoinToTake = (iter->second)->GetMaxTakeInCoin();
// 			memcpy(pBuffer + nOffset , &item, sizeof(item));
// 			nOffset += sizeof(item);
// 		}
// 	}
// 	pTargetPlayer->SendMsgToClient(pBuffer,nOffset) ;
// 	if ( pBuffer != NULL )
// 	{
// 		delete[] pBuffer ;
// 		pBuffer = NULL ;
// 	}
}

void CRoomManager::stSpeedRoom::AddRoom(CRoomBaseNew*pRoomBase)
{
	CRoomTexasPoker* pRoom = dynamic_cast<CRoomTexasPoker*>(pRoomBase);
	if ( pRoom == NULL )
	{
		return ;
	}

	MAP_BLIND_ROOMS* pMapVecRooms = NULL ;
	if ( 5 == pRoom->GetMaxSeat() )
	{
		pMapVecRooms = &vSeatRooms[eSeatCount_5];
	}
	else
	{
		pMapVecRooms = &vSeatRooms[eSeatCount_9] ;
	}
	
	MAP_BLIND_ROOMS::iterator iter = pMapVecRooms->find(pRoom->GetBigBlindBet()) ;
	if ( iter != pMapVecRooms->end() )
	{
		iter->second.push_back(pRoomBase) ;
	}
	else
	{
		VEC_ROOM vRooms;
		vRooms.push_back(pRoomBase);
		pMapVecRooms->insert(MAP_BLIND_ROOMS::value_type((unsigned int)pRoom->GetBigBlindBet(),vRooms));
	}
}

void CRoomManager::AddRoomToType(CRoomBaseNew* pRoomBase)
{
	CRoomTexasPoker* pRoom = dynamic_cast<CRoomTexasPoker*>(pRoomBase);
	if ( pRoom == NULL )
	{
		return ;
	}

	if ( pRoom->GetWaitOperateTime() >= TIME_LOW_LIMIT_FOR_NORMAL_ROOM )
	{
		m_vSpeedRooms[eSpeed_Normal].AddRoom(pRoomBase) ;
	}
	else
	{
		m_vSpeedRooms[eSpeed_Quick].AddRoom(pRoomBase) ;
	}
}

CRoomBaseNew* CRoomManager::GetProperRoomToJoin(unsigned char cSpeed , unsigned char cSeatType, unsigned int nBlindBet , unsigned int nExptedRoomID  )
{
	if ( cSpeed < 0 || cSpeed >= eSpeed_Max )
	{
		cSpeed = eSpeed_Normal ;
	}

	if ( cSeatType < 0 || cSeatType >= eSeatCount_Max )
	{
		cSeatType = eSeatCount_9 ;
	}

	MAP_BLIND_ROOMS& vRooms = m_vSpeedRooms[cSpeed].vSeatRooms[cSeatType];
	MAP_BLIND_ROOMS::iterator iter = vRooms.find(nBlindBet) ;
	if ( iter == vRooms.end() )
	{
		iter = vRooms.begin();
	}
	
	VEC_ROOM& vec = iter->second ;

	// find not full , and not empty rooms 
	VEC_ROOM vecTemp ;
	for ( int i = 0 ; i < vec.size(); ++i )
	{
		if ( vec[i]->GetEmptySeatCount() <= 0 || vec[i]->GetMaxSeat() == vec[i]->GetEmptySeatCount() || vec[i]->GetRoomID() == nExptedRoomID )
		{
			continue;
		}
		vecTemp.push_back(vec[i]) ;
	}

	if ( vecTemp.size() == 0 )
	{
		for ( int i = 0 ; i < vec.size(); ++i )
		{
			if ( vec[i]->GetMaxSeat() == vec[i]->GetEmptySeatCount() )
			{
				return vec[i] ;
			}
		}
		return NULL ;
	}
 
	int idx = rand() % vecTemp.size() ;
	return vecTemp[idx];
}