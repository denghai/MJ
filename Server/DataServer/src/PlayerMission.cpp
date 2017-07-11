#include "PlayerMission.h"
#include "PlayerEvent.h"
#include "MissionConfig.h"
#include "GameServerApp.h"
#include "LogManager.h"
#include "Player.h"
#include "PlayerBaseData.h"
#include "PlayerItem.h"
#include <time.h>
#include "EventCenter.h"
bool stPlayerMissionSate::OnEvent(stPlayerEvetArg* pArg )
{
	if ( bFinish )
	{
		return false ;
	}

	if ( NULL == pArg )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("event arg is NULL") ;
		return false;
	}

	CMissionConfigMgr* pConfigMgr = (CMissionConfigMgr*)CGameServerApp::SharedGameServerApp()->GetConfigMgr()->GetConfig(CConfigManager::eConfig_Mission) ;
	stMissionConfig* pConifg = pConfigMgr->GetMissionConfigByID(nMissionID) ;
	if ( pConifg == NULL )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("lack of mission config id = %d",nMissionID) ;
		return  false;
	}

	bool bReAct = false ;
	switch ( pArg->eEventType )
	{
	case ePlayerEvent_AddFriend:
		{
			if ( pConifg->eType == eMission_AddFriend )
			{
				++nProcessCnt ;
				bReAct = true ;
			}
		}
		break;
	case ePlayerEvent_FirstLogin:
		{
			if ( eMission_FirstLogin == pConifg->eType )
			{
				++nProcessCnt ;
				bReAct = true ;
			}
		}
		break;
	case ePlayerEvent_Recharge:
		{
			stPlayerEventArgRecharge* pRealArg = (stPlayerEventArgRecharge*)pArg ;
			if ( eMission_RMBConsume == pConifg->eType && pRealArg->nRMB >= pConifg->nMissionValue )
			{
				++nProcessCnt ;
				bReAct = true ;
			}
		}
		break;
	case ePlayerEvent_RoundEnd:
		{
			stPlayerEventArgRoundEnd* pRealArg = (stPlayerEventArgRoundEnd*)pArg ;
			switch ( pConifg->eType )
			{
			case eMission_PlayRound:
				{
					++nProcessCnt ;
					bReAct = true ;
				}
				break;
			case eMission_WinRound:
				{
					if ( pRealArg->nCoinOffset > 0 )
					{
						++nProcessCnt ;
						bReAct = true ;
					}
				}
				break;
			case eMission_WinRoundWithCardType:
				{
					if ( pRealArg->nCoinOffset > 0 && pRealArg->cCardType == pConifg->nMissionValue )
					{
						++nProcessCnt ;
						bReAct = true ;
					}
				}
				break;
			case eMission_SingleWinCoin:
				{
					if ( pRealArg->nCoinOffset >= pConifg->nMissionValue )
					{
						++nProcessCnt ;
						bReAct = true ;
					}
				}
				break;
			default:
				//CLogMgr::SharedLogMgr()->ErrorLog("unknown ePlayerEvent_RoundEnd mission type = %d",pConifg->eType ) ;
				break;
			}
		}
		break;
	default:
		//CLogMgr::SharedLogMgr()->PrintLog("Mission do not mind player event type = %d",pArg->eEventType) ;
		break;
	}

	bFinish = pConifg->nMissionProcess <= nProcessCnt ;
	return bReAct ;
}

void stPlayerMissionSate::Reset()
{
	bFinish = false ;
	bGetedReward = false ;
	nProcessCnt = 0 ;
}

bool CPlayerMission::OnMessage( stMsg* pMsg , eMsgPort eSenderPort )
{
	if ( IPlayerComponent::OnMessage(pMsg,eSenderPort) )
	{
		return true ;
	}

	switch ( pMsg->usMsgType )
	{
	case MSG_PLAYER_REQUEST_MISSION_LIST:
		{
			SendStateListToClient();
		}
		break;
	case MSG_PLAYER_REQUEST_MISSION_REWORD:
		{
			//stMsgPlayerRequestMissionReward* pMsgRet = (stMsgPlayerRequestMissionReward*)pMsg ;
			//stPlayerMissionSate* pMissionState = GetMissionState(pMsgRet->nMissionID) ;
			//stMsgPlayerRequestMissionRewardRet msgBack ;
			//msgBack.nRet = 0 ;
			//msgBack.nMissionID = pMsgRet->nMissionID ;
			//if ( !pMissionState )
			//{
			//	msgBack.nRet = 3 ;
			//	SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
			//	return true ;
			//}
			//else if ( pMissionState->bFinish == false )
			//{
			//	msgBack.nRet = 1 ;
			//	SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
			//	return true ;
			//}
			//else if ( pMissionState->bGetedReward )
			//{
			//	msgBack.nRet = 2 ;
			//	SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
			//	return true ;
			//}

			//// give reward ;
			//CMissionConfigMgr* pConfigMgr = (CMissionConfigMgr*)CGameServerApp::SharedGameServerApp()->GetConfigMgr()->GetConfig(CConfigManager::eConfig_Mission) ;
			//stMissionConfig* pConifg = pConfigMgr->GetMissionConfigByID(pMissionState->nMissionID) ;
			//GetPlayer()->GetBaseData()->ModifyMoney(pConifg->nRewardCoin,false);
			// 
			//CPlayerItemComponent* pPlayerItemComp = (CPlayerItemComponent*)GetPlayer()->GetComponent(ePlayerComponent_PlayerItemMgr);
			//std::map<unsigned short, unsigned short>::iterator iter = pConifg->vItemsAndCount.begin() ;
			//for ( ;iter != pConifg->vItemsAndCount.end(); ++iter )
			//{
			//	pPlayerItemComp->AddItemByID(iter->first,iter->second) ;
			//}
			//pMissionState->bGetedReward = true ;
			//
			//msgBack.nDiamoned = GetPlayer()->GetBaseData()->GetAllDiamoned() ;
			//msgBack.nFinalCoin = GetPlayer()->GetBaseData()->GetAllCoin() ;
			//SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
		}
		break;
	case MSG_GAME_SERVER_GET_MISSION_DATA:
		{
			//stMsgGameServerGetMissionDataRet* pMsgRet = (stMsgGameServerGetMissionDataRet*)pMsg;
			//time_t tLast = pMsgRet->nLastSaveTime ;
			//struct tm tmLastSave = *localtime( &tLast) ;
			//time_t nNow = time(NULL) ;
			//struct tm tmNow = *localtime(&nNow) ;
			//if ( pMsgRet->nMissonCount != 0 && tmNow.tm_yday == tmLastSave.tm_yday )
			//{
			//	char* pBuffer = (char*)pMsgRet ;
			//	pBuffer += sizeof(stMsgGameServerGetMissionDataRet);
			//	short nCount = pMsgRet->nMissonCount ;
			//	while ( nCount-- )
			//	{
			//		stMissionSate* pMissionState = (stMissionSate*)pBuffer ;
			//		stPlayerMissionSate* pM = new stPlayerMissionSate ;
			//		memcpy(pM,pMissionState,sizeof(stMissionSate));
			//		m_vAllMissionStates[pM->nMissionID] = pM ;
			//		pBuffer += sizeof(stMissionSate);
			//	}
			//}
			//else
			//{
			//	CMissionConfigMgr* pConfigMgr = (CMissionConfigMgr*)CGameServerApp::SharedGameServerApp()->GetConfigMgr()->GetConfig(CConfigManager::eConfig_Mission) ;
			//	CMissionConfigMgr::MAP_MISSION::iterator iter_M = pConfigMgr->m_vAllMission.begin() ;
			//	for ( ; iter_M != pConfigMgr->m_vAllMission.end(); ++iter_M )
			//	{
			//		stPlayerMissionSate* pM = new stPlayerMissionSate ;
			//		pM->nMissionID = iter_M->first ;
			//		pM->Reset();
			//		m_vAllMissionStates[pM->nMissionID] = pM ;
			//	}
			//}

			//// first login event ;   //keep first login mission always finished ;
			//stPlayerEvetArg evet ;
			//evet.eEventType = ePlayerEvent_FirstLogin ;
			//GetPlayer()->PostPlayerEvent(&evet);
			//m_bDirty = false ;
		}
		break;
	default:
		return false ;
	}
	return true;
}

void CPlayerMission::OnPlayerDisconnect()
{
	// save mission Data ;
	TimerSave();
	CEventCenter::SharedEventCenter()->RemoveEventListenner(eEvent_NewDay,this,CPlayerMission::EventFunc ) ;
}

void CPlayerMission::Reset()
{
	Clear() ;
	// send msg to db to get mission info ;
	stMsgGameServerGetMissionData msg ;
	msg.nSessionID = GetPlayer()->GetSessionID();
	msg.nUserUID = GetPlayer()->GetUserUID() ;
	SendMsg(&msg,sizeof(msg)) ;
	m_bDirty = false ;
	CEventCenter::SharedEventCenter()->RegisterEventListenner(eEvent_NewDay,this,CPlayerMission::EventFunc ) ;
}

void CPlayerMission::Init()
{
	Reset();
}

bool CPlayerMission::OnPlayerEvent(stPlayerEvetArg* pArg)
{
	MAP_PLAYER_MISSION_STATE::iterator iter = m_vAllMissionStates.begin() ;
	for ( ; iter != m_vAllMissionStates.end(); ++iter )
	{
		if ( iter->second && iter->second->bFinish == false )
		{
			if ( iter->second->OnEvent(pArg) )
			{
				m_bDirty = true ;
			}

			if ( iter->second->bFinish )
			{
				OnMissionFinish(iter->second) ;
				m_bDirty = true ;
			}
		}
	}
	return false ;
}

void CPlayerMission::Clear()
{
	MAP_PLAYER_MISSION_STATE::iterator iter = m_vAllMissionStates.begin() ;
	for ( ; iter != m_vAllMissionStates.end(); ++iter )
	{
		if ( iter->second )
		{
			delete iter->second ;
		}
		iter->second = NULL ;
	}
	m_vAllMissionStates.clear() ;
	m_bDirty = false ;
}

void CPlayerMission::SendStateListToClient()
{
	stMsgPlayerRequestMissionListRet msg ;
	msg.nMissionCount = m_vAllMissionStates.size() ;
	char* pBuffer = new char[sizeof(msg) + msg.nMissionCount * sizeof(stMissionSate)] ;
	unsigned short nOffset = 0 ;
	memcpy(pBuffer,&msg,sizeof(msg));
	nOffset += sizeof(msg);
	MAP_PLAYER_MISSION_STATE::iterator iter = m_vAllMissionStates.begin() ;
	for ( ; iter != m_vAllMissionStates.end(); ++iter )
	{
		memcpy(pBuffer + nOffset , iter->second , sizeof(stMissionSate));
		nOffset += sizeof(stMissionSate);
	}
	SendMsg((stMsg*)pBuffer,nOffset) ;
	delete[] pBuffer ;
}

stPlayerMissionSate* CPlayerMission::GetMissionState(unsigned short nMissionID ) 
{
	MAP_PLAYER_MISSION_STATE::iterator iter = m_vAllMissionStates.find(nMissionID) ;
	if ( iter != m_vAllMissionStates.end() )
	{
		return iter->second ;
	}
	return NULL ;
}

void CPlayerMission::OnMissionFinish(stPlayerMissionSate* pMission)
{
	stMsgPlayerNewMissionFinished msg ;
	msg.nMissionID = pMission->nMissionID ;
	SendMsg(&msg,sizeof(msg)) ;
	m_bDirty = true ;
}

void CPlayerMission::TimerSave()
{
	if ( m_bDirty = false )
	{
		return ;
	}
	stMsgGameServerSaveMissionData msg ;
	msg.nSavetime = (unsigned int)time(NULL) ;
	msg.nMissonCount = m_vAllMissionStates.size() ; 
	msg.nUserUID = GetPlayer()->GetUserUID() ;
	char* pBuffer = new char[sizeof(msg) + msg.nMissonCount * sizeof(stMissionSate) ] ;
	unsigned short nOfffset = 0 ;
	memcpy(pBuffer, &msg, sizeof(msg));
	nOfffset += sizeof(msg);

	MAP_PLAYER_MISSION_STATE::iterator iter = m_vAllMissionStates.begin() ;
	for ( ; iter != m_vAllMissionStates.end(); ++iter )
	{
		stPlayerMissionSate* pMissionData = iter->second ;
		memcpy(pBuffer + nOfffset , pMissionData,sizeof(stMissionSate));
		nOfffset += sizeof(stMissionSate);
	}
	SendMsg((stMsg*)pBuffer,nOfffset) ;
	delete[] pBuffer ;
	m_bDirty = false ;
}
bool CPlayerMission::EventFunc(void* pUserData,stEventArg* pArg)
{
	CPlayerMission* pBaseData = (CPlayerMission*)pUserData ;
	pBaseData->OnNewDay(pArg);
	return false ;
}

void CPlayerMission::OnNewDay(stEventArg* pArg)
{
	MAP_PLAYER_MISSION_STATE::iterator iter = m_vAllMissionStates.begin();
	for ( ; iter != m_vAllMissionStates.end() ; ++iter )
	{
		if ( !iter->second )
		{
			continue;
		}

		if ( iter->second->bGetedReward == false && iter->second->bFinish )
		{
			continue;
		}

		iter->second->Reset();
		m_bDirty = true ;
	}
}