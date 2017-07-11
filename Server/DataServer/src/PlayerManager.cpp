#include "PlayerManager.h"
#include "ServerMessageDefine.h"
#include "log4z.h"
#include "Player.h"
#include "CommonDefine.h"
#include "GameServerApp.h"
#include <assert.h>
#include "EventCenter.h"
#include "PlayerBaseData.h"
#include "AutoBuffer.h"
#include "PlayerMail.h"
#include "PlayerGameData.h"
#include "RewardConfig.h"
#include "RobotCenter.h"

void CSelectPlayerDataCacher::stPlayerDataPrifle::recivedData(stPlayerBrifData* pRecData)
{
	if ( isContentData() )
	{
		LOGFMTI("already have player data uid = %d , recive twice ",pData->nUserUID );
		return ;
	}

	if ( pRecData )
	{
		pData = new stPlayerDetailData ;
		memcpy(pData,pRecData,sizeof(stPlayerDetailData));
	}
	else
	{
		pData = nullptr ;
	}

	if ( vBrifeSubscribers.size() > 0  )
	{
		stMsgRequestPlayerDataRet msgBack ;
		msgBack.nRet = isContentData() ? 0 : 1 ;
		msgBack.isDetail = false ;

		CAutoBuffer auB (sizeof(msgBack) + sizeof(stPlayerDetailDataClient));
		auB.addContent(&msgBack,sizeof(msgBack));
		if ( msgBack.nRet == 0 )
		{
			uint16_t nLen = sizeof(stPlayerBrifData) ;
			auB.addContent((char*)pData,nLen );
		}

		for ( auto pp : vBrifeSubscribers )
		{
			CGameServerApp::SharedGameServerApp()->sendMsg(pp.second.nSessionID,auB.getBufferPtr(),auB.getContentSize()) ;
			LOGFMTD("send data detail profile to subscrible = %d",pp.second.nSessionID) ;
		}

		vBrifeSubscribers.clear() ;
	}

	if ( vDetailSubscribers.size() > 0 )
	{
		stMsgRequestPlayerDataRet msgBack ;
		msgBack.nRet = isContentData() ? 0 : 1 ;
		msgBack.isDetail = true ;

		CAutoBuffer auB (sizeof(msgBack) + sizeof(stPlayerDetailDataClient));
		auB.addContent(&msgBack,sizeof(msgBack));
		if ( msgBack.nRet == 0 )
		{
			uint16_t nLen = sizeof(stPlayerDetailDataClient) ;
			auB.addContent((char*)pData,nLen );
		}

		for ( auto pp : vDetailSubscribers )
		{
			CGameServerApp::SharedGameServerApp()->sendMsg(pp.second.nSessionID,auB.getBufferPtr(),auB.getContentSize()) ;
			LOGFMTD("send data profile detail to subscrible = %d",pp.second.nSessionID) ;
		}
	}

}

void CSelectPlayerDataCacher::stPlayerDataPrifle::addSubscriber( uint32_t nSessionId , bool isDetail )
{
	if ( isDetail )
	{
		auto iter = vDetailSubscribers.find(nSessionId) ;
		if ( iter == vDetailSubscribers.end() )
		{
			stSubscriber vt ;
			vt.isDetail = isDetail ;
			vt.nSessionID = nSessionId ;
			vDetailSubscribers[nSessionId] = vt ;
		}
	}
	else
	{
		auto iter = vBrifeSubscribers.find(nSessionId) ;
		if ( iter == vBrifeSubscribers.end() )
		{
			stSubscriber vt ;
			vt.isDetail = isDetail ;
			vt.nSessionID = nSessionId ;
			vBrifeSubscribers[nSessionId] = vt ;
		}
	}

}

CSelectPlayerDataCacher::CSelectPlayerDataCacher()
{
	m_vDetailData.clear();
}

CSelectPlayerDataCacher::~CSelectPlayerDataCacher()
{
	for ( MAP_ID_DATA::value_type va : m_vDetailData )
	{
		delete va.second ;
		va.second = nullptr ;
	}
	m_vDetailData.clear() ;
}

void CSelectPlayerDataCacher::removePlayerDataCache( uint32_t nUID )
{
	auto iter = m_vDetailData.find(nUID) ;
	if ( iter != m_vDetailData.end() )
	{
		delete iter->second ;
		iter->second = nullptr ;
		m_vDetailData.erase(iter) ;
	}
}

void CSelectPlayerDataCacher::cachePlayerData(stMsgSelectPlayerDataRet* pmsg )
{
	if ( pmsg->nRet )
	{
		LOGFMTD("cahe player data failed");
	}

	stPlayerBrifData* pData = (stPlayerBrifData*)((char*)pmsg + sizeof(stMsgSelectPlayerDataRet));
	if ( !pmsg->isDetail )
	{
		LOGFMTE("here must is detail , uid = %d",pData->nUserUID ) ;
	}

	if ( pmsg->nRet )
	{
		pData = nullptr ;
	}

	auto iter = m_vDetailData.find(pmsg->nDataPlayerUID) ;
	if ( iter == m_vDetailData.end() )
	{
		if ( pData )
		{
			stPlayerDataPrifle* pDataPri = new stPlayerDataPrifle ;
			pDataPri->nPlayerUID = pData->nUserUID ;
			pDataPri->recivedData(pData);
			m_vDetailData[pDataPri->nPlayerUID] = pDataPri  ;
			LOGFMTE("why no cacher foot print uid = %d",pData->nUserUID) ;
		}
		else
		{
			LOGFMTE("what is this situation for data prifle uid = %d",pmsg->nDataPlayerUID) ;
		}
	}
	else
	{
		LOGFMTD("recievd data prifle uid = %d",pData->nUserUID) ;
		iter->second->recivedData(pData) ;
	}
}

bool CSelectPlayerDataCacher::sendPlayerDataProfile(uint32_t nReqUID ,bool isDetail , uint32_t nSubscriberSessionID )
{
	MAP_ID_DATA::iterator iter = m_vDetailData.find(nReqUID) ;
	if ( iter == m_vDetailData.end() )
	{
		stPlayerDataPrifle* pData = new stPlayerDataPrifle ;
		pData->nPlayerUID = nReqUID ;
		pData->addSubscriber(nSubscriberSessionID,isDetail) ;
		m_vDetailData[pData->nPlayerUID] = pData  ;

		stMsgSelectPlayerData msgReq ;
		msgReq.isDetail = true ;
		msgReq.nTargetPlayerUID = nReqUID ;
		CGameServerApp::SharedGameServerApp()->sendMsg(nSubscriberSessionID,(char*)&msgReq,sizeof(msgReq)) ;
		LOGFMTD("uid = %d not online req form db , add session id = %d to subscrible list" , nReqUID,nSubscriberSessionID) ;
		pData->tRequestDataTime = time(nullptr) ;
		return true ;
	}

	auto pData = iter->second ;
	if ( pData->isContentData() == false )
	{
		pData->addSubscriber(nSubscriberSessionID,isDetail) ;
		LOGFMTD("already req uid = %d data , just add session id  = %d to subscrible list",nReqUID,nSubscriberSessionID) ;

		time_t tNow = time(nullptr) ;
		if ( (tNow - iter->second->tRequestDataTime) > 6 )
		{
			stMsgSelectPlayerData msgReq ;
			msgReq.isDetail = true ;
			msgReq.nTargetPlayerUID = nReqUID ;
			CGameServerApp::SharedGameServerApp()->sendMsg(nSubscriberSessionID,(char*)&msgReq,sizeof(msgReq)) ;
			pData->tRequestDataTime = time(nullptr) ;
			LOGFMTD("request uid = %d data time out , request again",pData->nPlayerUID) ;
		}
	}
	else
	{
		stMsgRequestPlayerDataRet msgBack ;
		msgBack.nRet = 0 ;
		msgBack.isDetail = isDetail ;
	
		CAutoBuffer auB (sizeof(msgBack) + sizeof(stPlayerDetailDataClient));
		auB.addContent(&msgBack,sizeof(msgBack));
		uint16_t nLen = isDetail ? sizeof(stPlayerDetailDataClient) : sizeof(stPlayerBrifData) ;
		auB.addContent((char*)pData->pData,nLen );
		CGameServerApp::SharedGameServerApp()->sendMsg(nSubscriberSessionID,auB.getBufferPtr(),auB.getContentSize()) ;
		LOGFMTD("send data profile uid = %d to subscrible = %d",nReqUID,nSubscriberSessionID) ;
	}
	return true ;
}

CPlayerManager::CPlayerManager()
{
	m_vOfflinePlayers.clear() ;
	m_vAllActivePlayers.clear();
}

CPlayerManager::~CPlayerManager()
{
	MAP_SESSIONID_PLAYERS::iterator iter = m_vAllActivePlayers.begin();
	for ( ; iter != m_vAllActivePlayers.end() ; ++iter )
	{
		iter->second->OnPlayerDisconnect();
		delete iter->second ;
		iter->second = NULL ;
	}
	m_vAllActivePlayers.clear() ;

	MAP_UID_PLAYERS::iterator iter_R = m_vOfflinePlayers.begin() ;
	for ( ; iter_R != m_vOfflinePlayers.end(); ++iter_R )
	{
		delete iter_R->second ;
		iter_R->second = NULL ;
	}
	m_vOfflinePlayers.clear() ;
}

void CPlayerManager::onExit()
{
	MAP_SESSIONID_PLAYERS::iterator iter = m_vAllActivePlayers.begin();
	for ( ; iter != m_vAllActivePlayers.end() ; ++iter )
	{
		iter->second->OnPlayerDisconnect();
	}

	MAP_UID_PLAYERS::iterator iter_R = m_vOfflinePlayers.begin() ;
	for ( ; iter_R != m_vOfflinePlayers.end(); ++iter_R )
	{
		iter_R->second->OnPlayerDisconnect();
	}

	IGlobalModule::onExit();
}

bool CPlayerManager::onMsg( stMsg* pMessage , eMsgPort eSenderPort , uint32_t nSessionID )
{
	if ( ProcessPublicMessage(pMessage,eSenderPort,nSessionID) )
	{
		return true ;
	}

	CPlayer* pTargetPlayer = GetPlayerBySessionID(nSessionID,true );
	if ( pTargetPlayer && pTargetPlayer->OnMessage(pMessage,eSenderPort ) )
	{
		if (pTargetPlayer->IsState(CPlayer::ePlayerState_Offline) )
		{
			pTargetPlayer->OnTimerSave(0,0);
		}
		return true  ;
	}
	else
	{
		if (pTargetPlayer == NULL )
		{
			LOGFMTE("can not find session id = %d to process msg id = %d ,from = %d",nSessionID,pMessage->usMsgType,eSenderPort) ;
		}
		else
		{
			LOGFMTE( "unprocess msg for player uid = %d , msg = %d ,from %d ",pTargetPlayer->GetUserUID(),pMessage->usMsgType,eSenderPort ) ;
		}
	}
	return false ;
}

bool CPlayerManager::onMsg( Json::Value& recvValue , uint16_t nmsgType, eMsgPort eSenderPort , uint32_t nSessionID  )
{
	if ( MSG_REQUEST_PLAYER_IP == nmsgType)
	{
		std::string strIp = "0.0.0.0";
		double dbJ = 0.0f;
		double dbW = 0.0f;
		if (recvValue["reqUID"].isNull() == false && recvValue["reqUID"].isUInt() )
		{
			uint32_t nReqSeq = 0;
			nReqSeq = recvValue["reqUID"].asUInt();
			auto pPlayer = GetPlayerByUserUID(nReqSeq);
			if (pPlayer)
			{
				strIp = pPlayer->GetBaseData()->getIp();
				dbJ = pPlayer->GetBaseData()->getJ();
				dbW = pPlayer->GetBaseData()->getW();
			}
		}
		recvValue["ip"] = strIp;
		recvValue["J"] = dbJ;
		recvValue["W"] = dbW;
		getSvrApp()->sendMsg(nSessionID, recvValue, MSG_REQUEST_PLAYER_IP);
		return true;
	}

	CPlayer* pTargetPlayer = nullptr;
	if (MSG_CONSUM_VIP_ROOM_CARDS == nmsgType)
	{
		pTargetPlayer = GetPlayerByUserUID(recvValue["uid"].asUInt());
	}
	else if (MSG_VIP_ROOM_CLOSED == nmsgType)
	{
		pTargetPlayer = GetPlayerByUserUID(recvValue["uid"].asUInt());
	}
	else
	{
		pTargetPlayer = GetPlayerBySessionID(nSessionID, true);
	}

	if ( pTargetPlayer && pTargetPlayer->OnMessage(recvValue,nmsgType,eSenderPort ) )
	{
		if (pTargetPlayer->IsState(CPlayer::ePlayerState_Offline) )
		{
			pTargetPlayer->OnTimerSave(0,0);
		}
		return true  ;
	}
	else
	{
		if (pTargetPlayer == NULL )
		{
			LOGFMTE("can not find session id = %d to process msg id = %d ,from = %d",nSessionID,nmsgType,eSenderPort) ;
		}
		else
		{
			LOGFMTE( "unprocess msg for player uid = %d , msg = %d ,from %d ",pTargetPlayer->GetUserUID(),nmsgType,eSenderPort ) ;
		}
	}
	return false ;
}

bool CPlayerManager::ProcessPublicMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID  )
{
	switch ( prealMsg->usMsgType )
	{
	case MSG_CROSS_SERVER_REQUEST:
		{
			stMsgCrossServerRequest* pRet = (stMsgCrossServerRequest*)prealMsg ;

			Json::Value* pJsValue = nullptr ;
			Json::Value rootValue ;
			if ( pRet->nJsonsLen )
			{
				Json::Reader reader;
				char* pstr = ((char*)&pRet->nJsonsLen) + sizeof(pRet->nJsonsLen) ;
				reader.parse(pstr,pstr + pRet->nJsonsLen,rootValue,false);
				pJsValue = &rootValue ;
			}

			if ( onCrossServerRequest(pRet,eSenderPort,pJsValue) == false )
			{
				CPlayer* pPlayer = GetPlayerByUserUID(pRet->nTargetID);
				assert(pPlayer&&"this request no one to process or target id error");
				if ( pPlayer && pPlayer->onCrossServerRequest(pRet,eSenderPort,pJsValue) )
				{
					return true ;
				}
				LOGFMTE("cross request type = %d , subType = %d ,unprocessed",pRet->nRequestType,pRet->nRequestSubType);
				return false;
			}

			return true ;
		}
		break;
	case MSG_CROSS_SERVER_REQUEST_RET:
		{
			stMsgCrossServerRequestRet* pRet = (stMsgCrossServerRequestRet*)prealMsg ;
			Json::Value* pJsValue = nullptr ;
			Json::Value rootValue ;
			if ( pRet->nJsonsLen )
			{
				Json::Reader reader;
				char* pstr = ((char*)&pRet->nJsonsLen) + sizeof(pRet->nJsonsLen) ;
				reader.parse(pstr,pstr + pRet->nJsonsLen,rootValue,false);
				pJsValue = &rootValue ;
			}

			if ( onCrossServerRequestRet(pRet,pJsValue) == false )
			{
				CPlayer* pPlayer = GetPlayerByUserUID(pRet->nTargetID);
				assert(pPlayer&&"this request no one to process or target id error");
				if ( pPlayer && pPlayer->onCrossServerRequestRet(pRet,pJsValue) )
				{
					return true ;
				}
				LOGFMTE("cross request result type = %d , subType = %d ,unprocessed",pRet->nRequestType,pRet->nRequestSubType);
				return false;
			}
			return true ;
		}
		break;
	case MSG_SVR_DO_LEAVE_ROOM:
		{
			stMsgSvrDoLeaveRoom* pRet = (stMsgSvrDoLeaveRoom*)prealMsg ;
			CPlayer* pp = GetPlayerByUserUID(pRet->nUserUID) ;
			if (!pp)
			{
				LOGFMTE("uid = %d not find , so can not process do leave room",pRet->nUserUID);
			}
			else
			{
				LOGFMTD("uid = %d do leave room ",pRet->nUserUID) ;
				pp->OnMessage(prealMsg,eSenderPort);
			}
		}
		break;
	case MSG_SYNC_PRIVATE_ROOM_RESULT:
		{
			stMsgSyncPrivateRoomResult* pRet = (stMsgSyncPrivateRoomResult*)prealMsg ;
			CPlayer* pp = GetPlayerByUserUID(pRet->nTargetPlayerUID) ;
			if (!pp || pp->IsState(CPlayer::ePlayerState_Online) == false )
			{
				LOGFMTE("uid = %d not find , so can not process MSG_SYNC_PRIVATE_ROOM_RESULT, seiral = %u ",pRet->nTargetPlayerUID,pRet->nSiealNum );
				Json::Value jsArg ;
				jsArg["createUID"] = pRet->nCreatorUID ;
				jsArg["duiringTime"] = pRet->nDuringTimeSeconds ;
				jsArg["finalCoin"] = pRet->nFinalCoin ;
				jsArg["offset"] = pRet->nOffset ;
				jsArg["roomID"] = pRet->nRoomID ;
				jsArg["finishTime"] = (uint32_t)time(nullptr);
				jsArg["buyIn"] = pRet->nBuyIn ;
				jsArg["baseBet"] = pRet->nBaseBet ;
				jsArg["roomName"] = pRet->cRoomName ;
				jsArg["clubID"] = pRet->nClubID ;
				jsArg["serialNum"] = pRet->nSiealNum;
				CPlayerMailComponent::PostOfflineEvent(CPlayerMailComponent::Event_SyncGameResult,jsArg,pRet->nTargetPlayerUID);
			}
			else
			{
				pp->OnMessage(prealMsg,eSenderPort);
			}
		}
		break ;
	case MSG_REQUEST_PLAYER_DATA:
		{
			stMsgRequestPlayerData* pRet = (stMsgRequestPlayerData*)prealMsg ;
			stMsgRequestPlayerDataRet msgBack ;
			msgBack.nRet = 0 ;
			msgBack.isDetail = pRet->isDetail ;
			CPlayer* pPlayer = GetPlayerByUserUID(pRet->nPlayerUID);

			stPlayerDetailDataClient stData ;
			stData.nCurrentRoomID = 0 ;
			CAutoBuffer auB (sizeof(msgBack) + sizeof(stPlayerDetailData));
			if ( pPlayer )
			{
				//CPlayerTaxas* pTaxasData = (CPlayerTaxas*)pPlayer->GetComponent(ePlayerComponent_PlayerTaxas);
				//stData.nCurrentRoomID = pTaxasData->getCurRoomID() ;

				//if ( stData.nCurrentRoomID )  // select take in
				//{
				//	stMsgCrossServerRequest msgReq ;
				//	msgReq.nJsonsLen = 0 ;
				//	msgReq.nReqOrigID = nSessionID ;
				//	msgReq.nRequestSubType = eCrossSvrReqSub_SelectPlayerData ;
				//	msgReq.nRequestType = eCrossSvrReq_SelectTakeIn ;
				//	msgReq.nTargetID = stData.nCurrentRoomID ;
				//	msgReq.cSysIdentifer = ID_MSG_PORT_TAXAS ;
				//	msgReq.vArg[0] = pRet->nPlayerUID;
				//	msgReq.vArg[1] = pRet->isDetail ;
				//	CGameServerApp::SharedGameServerApp()->sendMsg(nSessionID,(char*)&msgReq,sizeof(msgReq)) ;
				//	LOGFMTD("select take in for player detail") ;
				//	return true ;
				//}

				if ( pRet->isDetail )
				{
					pPlayer->GetBaseData()->GetPlayerDetailData(&stData);
					//pTaxasData->getTaxasData(&stData.tTaxasData);
				}
				else
				{
					pPlayer->GetBaseData()->GetPlayerBrifData(&stData) ;
				}
				
				auB.addContent(&msgBack,sizeof(msgBack));
				auB.addContent(&stData,pRet->isDetail ? sizeof(stPlayerDetailDataClient) : sizeof(stPlayerBrifData) ) ;
				CGameServerApp::SharedGameServerApp()->sendMsg(nSessionID,auB.getBufferPtr(),auB.getContentSize()) ;
				return true ;
			}

			m_tPlayerDataCaher.sendPlayerDataProfile(pRet->nPlayerUID,pRet->isDetail,nSessionID);
		}
		break;
	case MSG_SELECT_DB_PLAYER_DATA:
		{
			stMsgSelectPlayerDataRet* pRet = (stMsgSelectPlayerDataRet*)prealMsg ;
			m_tPlayerDataCaher.cachePlayerData(pRet);
		}
		break;
	case MSG_PLAYER_LOGIN:
		{
			stMsgOnPlayerLogin* pmsgenter = (stMsgOnPlayerLogin*)prealMsg ;
			CPlayer* pPlayer = GetPlayerBySessionID(nSessionID) ;
			if ( pPlayer != NULL && pPlayer->GetUserUID() == pmsgenter->nUserUID )
			{
				LOGFMTE("double nSession, this nSessionID already have player, already login , do not login again  id = %d? ",nSessionID) ;
				return true ;
			}

			if ( pPlayer != NULL && pPlayer->GetUserUID() != pmsgenter->nUserUID ) // switch account in main scene 
			{
				// disconnect pre player 
				OnPlayerOffline(pPlayer);
			}

			if ( ProcessIsAlreadyLogin(pmsgenter->nUserUID,nSessionID) )
			{
				return true ;
			}

			// is offline peer 
			MAP_UID_PLAYERS::iterator iterOfflien = m_vOfflinePlayers.begin();
			for ( ; iterOfflien != m_vOfflinePlayers.end(); ++iterOfflien )
			{
				if ( iterOfflien->second && iterOfflien->first == pmsgenter->nUserUID )
				{
					iterOfflien->second->OnReactive(nSessionID) ;
					AddPlayer(iterOfflien->second) ;
					m_vOfflinePlayers.erase(iterOfflien) ;
					return true ;
				}
			}

			CPlayer* pNew = new CPlayer ;
			pNew->Init(pmsgenter->nUserUID,nSessionID ) ;
			AddPlayer(pNew) ;
		}
		break;
	case MSG_DISCONNECT_CLIENT:
	case MSG_PLAYER_LOGOUT:
		{
			CPlayer* pPlayer = GetPlayerBySessionID(nSessionID) ;
			if ( pPlayer )
			{
				// post online event ;
				LOGFMTD("player disconnect session id = %d",nSessionID);
				OnPlayerOffline(pPlayer) ;
			}
			else
			{
				LOGFMTE("client disconnect ! client is NULL session id = %d",nSessionID) ;
			}
			//#ifdef _DEBUG
			LogState();
			//#endif
		}
		break;
	//case MSG_TP_ORDER_LEAVE:
	//	{
	//		stMsgOrderTaxasPlayerLeaveRet* pRet = (stMsgOrderTaxasPlayerLeaveRet*)prealMsg ;
	//		CPlayer* pp = GetPlayerByUserUID(pRet->nUserUID) ;
	//		if (!pp)
	//		{
	//			LOGFMTE("uid = %d not find , so can not inform leave",pRet->nUserUID);
	//		}
	//		else
	//		{
	//			pp->OnMessage(prealMsg,eSenderPort);
	//		}
	//	}
	//	break;
	//case MSG_TP_INFORM_LEAVE:
	//	{
	//		stMsgInformTaxasPlayerLeave* pRet = (stMsgInformTaxasPlayerLeave*)prealMsg ;
	//		CPlayer* pp = GetPlayerByUserUID(pRet->nUserUID) ;
	//		if (!pp)
	//		{
	//			LOGFMTE("uid = %d not find , so can not inform leave",pRet->nUserUID);
	//		}
	//		else
	//		{
	//			pp->OnMessage(prealMsg,eSenderPort);
	//		}
	//	}
	//	break;
	case MSG_TP_SYNC_PLAYER_DATA:
		{
			stMsgSyncTaxasPlayerData* pRet = (stMsgSyncTaxasPlayerData*)prealMsg ;
			CPlayer* pp = GetPlayerByUserUID(pRet->nUserUID) ;
			if (!pp)
			{
				LOGFMTE("uid = %d not find , so can not sys data",pRet->nUserUID);
			}
			else
			{
				pp->OnMessage(prealMsg,eSenderPort);
			}
		}
		break;
	case MSG_SVR_DELAYED_LEAVE_ROOM:
		{
			stMsgSvrDelayedLeaveRoom* pRet = (stMsgSvrDelayedLeaveRoom*)prealMsg ;
			CPlayer* pp = GetPlayerByUserUID(pRet->nUserUID) ;
			if (!pp)
			{
				LOGFMTE("uid = %d not find , so can not delay leave room",pRet->nUserUID);
			}
			else
			{
				pp->OnMessage(prealMsg,eSenderPort);
			}
		}
		break;
	case MSG_VERIFY_TANSACTION:
	{
		stMsgFromVerifyServer* pRet = (stMsgFromVerifyServer*)prealMsg;
		CPlayer* pp = GetPlayerByUserUID(pRet->nBuyerPlayerUserUID);
		if (!pp)
		{
			LOGFMTE("uid = %d not find ,can not give shopItemID = %u", pRet->nBuyerPlayerUserUID, pRet->nShopItemID);
		}
		else
		{
			return pp->OnMessage(prealMsg, eSenderPort);
		}
		break;
	}
	default:
		return false;
	}

	return true ;
}

bool CPlayerManager::onAsyncRequest(uint16_t nRequestType , const Json::Value& jsReqContent, Json::Value& jsResult )
{
	switch (nRequestType)
	{
	case eAsync_ComsumDiamond:
		{
			uint32_t nUID = jsReqContent["targetUID"].asUInt() ;
			uint32_t nDiamond = jsReqContent["diamond"].asUInt();
			auto pPlayer = GetPlayerByUserUID(nUID);	
			jsResult["diamond"] = nDiamond ;
			if ( pPlayer == nullptr || pPlayer->GetBaseData()->GetAllDiamoned() < nDiamond )
			{
				jsResult["ret"] = 1 ;
				LOGFMTW("palyer uid = %u , consum diamond error , cnt = %u",nUID,nDiamond );
			}
			else
			{
				pPlayer->GetBaseData()->decressMoney(nDiamond,true ) ;
				jsResult["ret"] = 0 ;
				LOGFMTD("palyer uid = %u , consum diamond success , cnt = %u",nUID,nDiamond );
			}
		}
		break;
	case eAsync_GiveBackDiamond:
		{
			uint32_t nUID = jsReqContent["targetUID"].asUInt() ;
			uint32_t nDiamond = jsReqContent["diamond"].asUInt();
			auto pPlayer = GetPlayerByUserUID(nUID);	
			if ( pPlayer == nullptr )
			{
				Json::Value jsContent;
				jsContent["targetUID"] = nUID;
				jsContent["addCard"] = nDiamond;
				jsContent["addCardNo"] = nUID;
				Json::StyledWriter jsWrite;
				auto str = jsWrite.write(jsContent);
				CPlayerMailComponent::PostMailToPlayer(eMailType::eMail_AddRoomCard, str.c_str(), str.size(), nUID);
				LOGFMTE("uid = %u not online can not give back diamond = %u, via agent add card mail ",nUID,nDiamond);
			}
			else
			{
				pPlayer->GetBaseData()->AddMoney(nDiamond,true);
				LOGFMTD("give back diamond uid = %u , cnt = %u",nUID,nDiamond);
			}
		}
		break ;
	case eAsync_AgentAddRoomCard:
	{
		uint32_t nUserUID = jsReqContent["targetUID"].asUInt();
		int32_t nAddCnt = jsReqContent["addCard"].asInt();
		uint32_t nSeailNumber = jsReqContent["addCardNo"].asUInt();
		auto pPlayer = GetPlayerByUserUID(nUserUID);
		if (nullptr == pPlayer)
		{
			LOGFMTI("player not online agents add card to uid = %u , cnt = %d , addCardNo = %u", nUserUID, nAddCnt, nSeailNumber);
			Json::StyledWriter jsWrite;
			auto str = jsWrite.write(jsReqContent);
			CPlayerMailComponent::PostMailToPlayer(eMailType::eMail_AddRoomCard,str.c_str(),str.size(),nUserUID);
		}
		else
		{
			LOGFMTI("player agents add card to uid = %u , cnt = %d , addCardNo = %u",nUserUID,nAddCnt,nSeailNumber);
			pPlayer->GetBaseData()->AddMoney(nAddCnt, true);
			stMsg msgU;
			msgU.usMsgType = MSG_PLAYER_UPDATE_MONEY;
			pPlayer->GetBaseData()->OnMessage(&msgU, ID_MSG_PORT_CLIENT);
		}

		jsResult = jsReqContent;
	}
	break;
	case eAsync_AgentGetPlayerInfo:
	{
		uint32_t nUserUID = jsReqContent["targetUID"].asUInt();
		jsResult["targetUID"] = jsReqContent["targetUID"];
		auto pPlayer = GetPlayerByUserUID(nUserUID);
		if (nullptr == pPlayer)
		{
			jsResult["isOnline"] = 0;
		}
		else
		{
			jsResult["isOnline"] = 1; 
			jsResult["name"] = pPlayer->GetBaseData()->GetPlayerName();
			jsResult["leftCardCnt"] = pPlayer->GetBaseData()->GetAllDiamoned();
		}
	}
	break;
	default:
		return false;
	}
	return true ;
}

CPlayer* CPlayerManager::GetPlayerBySessionID( unsigned int nSessionID , bool bInclueOffline )
{
	MAP_SESSIONID_PLAYERS::iterator iter = m_vAllActivePlayers.find(nSessionID) ;
	if ( iter != m_vAllActivePlayers.end())
	{
		return iter->second ;
	}

	if ( bInclueOffline )
	{
		MAP_UID_PLAYERS::iterator iterOffline = m_vOfflinePlayers.begin();
		for ( ; iterOffline != m_vOfflinePlayers.end(); ++iterOffline )
		{
			if ( iterOffline->second->GetSessionID() == nSessionID  )
			{
				return iterOffline->second ;
			}
		}
	}
	return NULL ;
}

void CPlayerManager::update(float fDeta )
{
	// process player delay delete
	 LIST_PLAYERS vListWillDelete ;
	 time_t tNow = time(nullptr);
	 for ( auto& pair : m_vOfflinePlayers )
	 {
		 if ( pair.second->getCanDelayTime() <= tNow )
		 {
			 vListWillDelete.push_back(pair.second) ;
		 }
	 }

	 for ( auto& pp : vListWillDelete )
	 {
		 for ( auto& pair : m_vOfflinePlayers )
		 {
			 if ( pair.second->GetUserUID() == pp->GetUserUID() )
			 {
				 pp->OnTimerSave(0,0) ;
				 m_vOfflinePlayers.erase(m_vOfflinePlayers.find(pair.first)) ;
				 LOGFMTD("player uid = %d do delete player object",pp->GetUserUID() ) ;
				 delete pp ;
				 pp = nullptr ;

				 break;
			 }
		 }
	 }

	 IGlobalModule::update(fDeta);
}

CPlayer* CPlayerManager::GetFirstActivePlayer()
{
	MAP_SESSIONID_PLAYERS::iterator iter = m_vAllActivePlayers.begin() ;
	if ( iter == m_vAllActivePlayers.end() || iter->second == NULL )
	{
		LOGFMTE("first actvie player is NULL") ;
		return NULL ;
	}
	return iter->second ;
}

CPlayer* CPlayerManager::GetPlayerByUserUID( unsigned int nUserUID, bool bInclueOffline )
{
	MAP_SESSIONID_PLAYERS::iterator iter = m_vAllActivePlayers.begin() ;
	for ( ; iter != m_vAllActivePlayers.end(); ++iter )
	{
		if ( iter->second )
		{
			if ( nUserUID == iter->second->GetUserUID() )
			{
				return iter->second ;
			}
		}
	}

	if ( bInclueOffline )
	{
		MAP_UID_PLAYERS::iterator iter_L = m_vOfflinePlayers.find(nUserUID) ;
		if ( iter_L != m_vOfflinePlayers.end() )
		{
			return iter_L->second ;
		}
	}
	return NULL ;
}

void CPlayerManager::OnPlayerOffline( CPlayer*pPlayer )
{
	if ( !pPlayer )
	{
		LOGFMTE("Can not Remove NULL player !") ;
		return ;
	}

	if ( pPlayer->GetBaseData()->getPlayerType() == ePlayer_Robot )
	{
		auto robotCenter = (CRobotCenter*)CGameServerApp::SharedGameServerApp()->getModuleByType(CRobotCenter::eModule_Type);
		robotCenter->onRobotDisconnect(pPlayer->GetUserUID()) ;
		LOGFMTD("robot uid = %u disconnected",pPlayer->GetUserUID()) ;
	}

	CEventCenter::SharedEventCenter()->PostEvent(eEvent_PlayerOffline,pPlayer) ;
	pPlayer->OnPlayerDisconnect() ;

	MAP_SESSIONID_PLAYERS::iterator iter = m_vAllActivePlayers.find(pPlayer->GetSessionID() ) ;
	if ( iter == m_vAllActivePlayers.end() )
	{
		LOGFMTE("why pPlayer uid = %d , not active ? shuold active " , pPlayer->GetUserUID() ) ;
		delete pPlayer ;
		pPlayer = NULL ;
		return ;
	}
	m_vAllActivePlayers.erase(iter) ;
	 
	// cache to offine line 
	MAP_UID_PLAYERS::iterator iterOffline = m_vOfflinePlayers.find(pPlayer->GetUserUID()) ;
	if ( iterOffline != m_vOfflinePlayers.end() )
	{
		LOGFMTE("why player uid = %d already in offline list ? " , iterOffline->first ) ;
		
		delete pPlayer ;
		pPlayer = NULL ;
		return ;
	}
	m_vOfflinePlayers[pPlayer->GetUserUID()] = pPlayer ;
}

void CPlayerManager::AddPlayer(CPlayer*pPlayer)
{
	if ( !pPlayer )
	{
		LOGFMTE("Can not Add NULL player !") ;
		return ;
	}
	MAP_SESSIONID_PLAYERS::iterator iter = m_vAllActivePlayers.find(pPlayer->GetSessionID() ) ;
	if ( iter != m_vAllActivePlayers.end() )
	{
		LOGFMTE("Player to add had existed in active map ! , player UID = %d",pPlayer->GetUserUID() ) ;
		delete pPlayer ;
		pPlayer = NULL ;
	}
	else
	{
		m_vAllActivePlayers[pPlayer->GetSessionID()]= pPlayer ;
	}
}

void CPlayerManager::LogState()
{
	LOGFMTI( "Active Player: %d   Offline Player: %d " ,m_vAllActivePlayers.size(),m_vOfflinePlayers.size() ) ;
}

bool CPlayerManager::ProcessIsAlreadyLogin(unsigned int nUserID ,unsigned nSessionID )
{
	// is active player
	CPlayer* pPlayer = NULL ;
	MAP_SESSIONID_PLAYERS::iterator  iter_ma = m_vAllActivePlayers.begin() ;
	for ( ; iter_ma != m_vAllActivePlayers.end(); ++iter_ma )
	{
		pPlayer = iter_ma->second ;
		if ( !pPlayer )
		{
			LOGFMTE("Why empty active player here ?") ;
			continue; 
		}

		if ( pPlayer->GetUserUID() == nUserID )
		{
			pPlayer->OnAnotherClientLoginThisPeer(nSessionID) ;
			m_vAllActivePlayers.erase(iter_ma) ;
			AddPlayer(pPlayer) ;
			LOGFMTD("other decivec login");
			return true ;
		}
	}
	return false ;
}

bool CPlayerManager::onCrossServerRequest(stMsgCrossServerRequest* pRequest , eMsgPort eSenderPort,Json::Value* vJsValue)
{
	if ( pRequest->nRequestType == eCrossSvrReq_Inform )
	{
		CPlayerMailComponent::PostMailToPlayer(eMail_PlainText,((char*)pRequest) + sizeof(stMsgCrossServerRequest),pRequest->nJsonsLen,pRequest->nTargetID) ;
		return true ;
	}
	else if ( eCrossSvrReq_GameOver == pRequest->nRequestType )
	{
		if ( vJsValue->isNull() )
		{
			LOGFMTE("why game over result is null room type = %d room id = %d",(uint32_t)pRequest->vArg[0],pRequest->nReqOrigID) ;
			return true ;
		}

		const char* pName = "null" ;
		if ( (*vJsValue)["roomName"].isNull() == false )
		{
			pName = (*vJsValue)["roomName"].asCString();
		}
		
		LOGFMTD("%s game over ",pName);
		Json::Value vPlayers = (*vJsValue)["players"];
		for ( uint8_t nIdx = 0 ; nIdx < vPlayers.size(); ++nIdx )
		{
			Json::Value item = vPlayers[nIdx] ;
			uint32_t nUID = item["userUID"].asInt() ;
			uint16_t nRewardID = item["rewardID"].asInt() ;
			CPlayer* pp = GetPlayerByUserUID(nUID) ;
			if ( pp && pp->IsState(CPlayer::ePlayerState_Online) )
			{
				LOGFMTD("uid = %u , get reward id = %u ",nUID,nRewardID ) ;
				pp->GetBaseData()->onGetReward(nIdx,nRewardID,(uint16_t)pRequest->vArg[0],pName) ;
			}
			else
			{	
				LOGFMTD("uid = %u , not online post msg get reward id = %u ",nUID,nRewardID ) ;
				Json::Value jEventArg ;
				jEventArg["rewardID"] = nRewardID;
				jEventArg["gameType"] = (uint16_t)pRequest->vArg[0];
				jEventArg["roomName"] = pName;
				jEventArg["rankIdx"] = nIdx ;
				CPlayerMailComponent::PostOfflineEvent(CPlayerMailComponent::Event_Reward,jEventArg,nUID);
			}

		}

		return true ;
	}
	return false ;
}

bool CPlayerManager::onCrossServerRequestRet(stMsgCrossServerRequestRet* pResult,Json::Value* vJsValue)
{
	if ( eCrossSvrReq_SelectTakeIn == pResult->nRequestType && eCrossSvrReqSub_SelectPlayerData == pResult->nRequestSubType )
	{
		//stMsgRequestPlayerDataRet msgBack ;
		//msgBack.nRet = 0 ;
		//msgBack.isDetail = pResult->vArg[3] ;
		//CPlayer* pPlayer = GetPlayerByUserUID(pResult->vArg[0]);
		//if ( pPlayer == nullptr )
		//{
		//	LOGFMTE("why this player is null , can not , funck!") ;
		//	return true ;
		//}

		//stPlayerDetailDataClient stData ;
		//stData.nCurrentRoomID = 0 ;
		//CAutoBuffer auB (sizeof(msgBack) + sizeof(stPlayerDetailDataClient));
		//if ( pPlayer )
		//{
		//	CPlayerTaxas* pTaxasData = (CPlayerTaxas*)pPlayer->GetComponent(ePlayerComponent_PlayerTaxas);
		//	stData.nCurrentRoomID = pResult->nReqOrigID;
		//	if ( msgBack.isDetail )
		//	{
		//		pPlayer->GetBaseData()->GetPlayerDetailData(&stData);
		//		pTaxasData->getTaxasData(&stData.tTaxasData);
		//		if ( vJsValue )
		//		{
		//			stData.tTaxasData.nPlayTimes += (*vJsValue)["playTimes"].asUInt();
		//			stData.tTaxasData.nWinTimes += (*vJsValue)["winTimes"].asUInt();
		//			stData.tTaxasData.nSingleWinMost = (*vJsValue)["singleMost"].asUInt() < stData.tTaxasData.nSingleWinMost ? stData.tTaxasData.nSingleWinMost : (*vJsValue)["singleMost"].asUInt();
		//		}
		//		else
		//		{
		//			LOGFMTD("targe player not sit down uid = %llu",pResult->vArg[0]);
		//		}
		//	}
		//	else
		//	{
		//		pPlayer->GetBaseData()->GetPlayerBrifData(&stData) ;
		//	}

		//	if ( pResult->vArg[1] )
		//	{
		//		stData.nCoin += pResult->vArg[2] ;
		//	}
		//	else
		//	{
		//		stData.nDiamoned += pResult->vArg[2] ;
		//	}
		//	stData.nCurrentRoomID = pResult->nReqOrigID;
		//	auB.addContent(&msgBack,sizeof(msgBack));
		//	auB.addContent(&stData,msgBack.isDetail ? sizeof(stPlayerDetailDataClient) : sizeof(stPlayerBrifData) ) ;
		//	CGameServerApp::SharedGameServerApp()->sendMsg(pResult->nTargetID,auB.getBufferPtr(),auB.getContentSize()) ;
		//	LOGFMTD("select take in ret , send player data");
		//	return true ;
		//}
		//return true ;
	}
	return false ;
}