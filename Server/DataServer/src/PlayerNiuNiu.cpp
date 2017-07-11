#include "PlayerNiuNiu.h"
#include "NiuNiuMessageDefine.h"
#include "Player.h"
#include "PlayerBaseData.h"
#include "LogManager.h"
#include "AutoBuffer.h"
void CPlayerNiuNiu::Reset()
{
	IPlayerComponent::Reset();
	m_nCurRoomID = 0 ;
	m_vMyOwnRooms.clear();
	memset(&m_tData,0,sizeof(m_tData));

	stMsgReadPlayerNiuNiuData msg ;
	msg.nUserUID = GetPlayer()->GetUserUID() ;
	SendMsg(&msg,sizeof(msg)) ;

	stMsgReadMyOwnRooms msgReq ;
	msgReq.nUserUID = GetPlayer()->GetUserUID();
	SendMsg(&msgReq,sizeof(msgReq)) ;

	m_bDirty = false ;
}

void CPlayerNiuNiu::Init()
{
	IPlayerComponent::Init();
	m_nCurRoomID = 0 ;
	m_vMyOwnRooms.clear();
	memset(&m_tData,0,sizeof(m_tData));

	stMsgReadMyOwnRooms msgReq ;
	msgReq.nUserUID = GetPlayer()->GetUserUID();
	SendMsg(&msgReq,sizeof(msgReq)) ;

	stMsgReadPlayerNiuNiuData msg ;
	msg.nUserUID = GetPlayer()->GetUserUID() ;
	SendMsg(&msg,sizeof(msg)) ;
	CLogMgr::SharedLogMgr()->PrintLog("requesting player taxas data for uid = %d",msg.nUserUID);

	m_bDirty = false ;
}

bool CPlayerNiuNiu::OnMessage( stMsg* pMessage , eMsgPort eSenderPort)
{
	if ( IPlayerComponent::OnMessage(pMessage,eSenderPort) )
	{
		return true ;
	}
	switch ( pMessage->usMsgType )
	{
	case MSG_READ_PLAYER_NIUNIU_DATA:
		{
			stMsgReadPlayerNiuNiuDataRet* pRet = (stMsgReadPlayerNiuNiuDataRet*)pMessage ;
			memcpy(&m_tData,&pRet->tData,sizeof(m_tData));
		}
		break;
	case MSG_REQUEST_MY_OWN_ROOMS:
		{
			stMsgRequestMyOwnRooms* pRet = (stMsgRequestMyOwnRooms*)pMessage ;
			if ( pRet->nRoomType != eRoom_NiuNiu )
			{
				return false ;
			}

			stMsgRequestMyOwnRoomsRet msgRet ;
			msgRet.nRoomType = pRet->nRoomType ;
			msgRet.nCnt = m_vMyOwnRooms.size() ;
			if ( msgRet.nCnt == 0 )
			{
				SendMsg(&msgRet,sizeof(msgRet)) ;
				return true ;
			}

			CAutoBuffer autoBuffer(sizeof(msgRet) + sizeof(uint32_t)* msgRet.nCnt);
			autoBuffer.addContent((char*)&msgRet,sizeof(msgRet)) ;
			MAP_ID_MYROOW::iterator iter = m_vMyOwnRooms.begin() ;
			for ( ; iter != m_vMyOwnRooms.end() ; ++iter )
			{
				uint32_t n = iter->first ;
				autoBuffer.addContent((char*)&n,sizeof(uint32_t));
			}
			SendMsg((stMsg*)autoBuffer.getBufferPtr(),autoBuffer.getContentSize()) ;
		}
		break;
	case MSG_READ_MY_OWN_ROOMS:
		{
			stMsgReadMyOwnRoomsRet* pRet = (stMsgReadMyOwnRoomsRet*)pMessage ;
			if ( pRet->nRoomType != eRoom_NiuNiu )
			{
				return false;
			}
			stMyOwnRoom* pRoomPtr = (stMyOwnRoom*)((char*)pMessage + sizeof(stMsgReadMyOwnRoomsRet));
			while ( pRet->nCnt-- )
			{
				m_vMyOwnRooms.insert(MAP_ID_MYROOW::value_type(pRoomPtr->nRoomID,*pRoomPtr));
				++pRoomPtr ;
			}
			CLogMgr::SharedLogMgr()->PrintLog("uid = %d ,read own creator room" , GetPlayer()->GetUserUID() ) ;
		}
		break;
	case MSG_NN_ENTER_ROOM:
		{
			stMsgNNEnterRoom* pEnter = (stMsgNNEnterRoom*)pMessage ;
			stMsgNNEnterRoomRet msgBack ;
			if ( GetPlayer()->isNotInAnyRoom() == false  )
			{
				msgBack.nRet = 2 ; 
				CLogMgr::SharedLogMgr()->ErrorLog("check the cur room id");
				SendMsg(&msgBack,sizeof(msgBack)) ;
				return true ;
			}

			stMsgCrossServerRequest msgEnter ;
			msgEnter.cSysIdentifer = ID_MSG_PORT_NIU_NIU ;
			msgEnter.nJsonsLen = 0 ;
			msgEnter.nReqOrigID = GetPlayer()->GetUserUID();
			msgEnter.nRequestSubType = eCrossSvrReqSub_Default ;
			msgEnter.nRequestType = eCrossSvrReq_EnterRoom ;
			msgEnter.nTargetID = pEnter->nTargetID ;
			msgEnter.vArg[0] = GetPlayer()->GetSessionID() ;
			msgEnter.vArg[1] = pEnter->nTargetID ;
			msgEnter.vArg[2] = GetPlayer()->GetBaseData()->GetAllCoin();
			msgEnter.vArg[3] = pEnter->nIDType ;
			SendMsg(&msgEnter,sizeof(msgEnter)) ;

			m_nCurRoomID = pEnter->nTargetID;
			CLogMgr::SharedLogMgr()->PrintLog("uid = %d try to enter niuniu room id = %d",GetPlayer()->GetUserUID(),m_nCurRoomID) ;
		}
		break;
	default:
		return false;
	}
	return true ;
}

bool CPlayerNiuNiu::onCrossServerRequest(stMsgCrossServerRequest* pRequest , eMsgPort eSenderPort,Json::Value* vJsValue)
{
	if ( IPlayerComponent::onCrossServerRequest(pRequest,eSenderPort,vJsValue) )
	{
		return true ;
	}

	switch ( pRequest->nRequestType )
	{
	case eCrossSvrReq_SyncNiuNiuData:
		{
			m_tData.nPlayTimes += pRequest->vArg[0] ;
			m_tData.nWinTimes += pRequest->vArg[1] ;
			if ( pRequest->vArg[2] > m_tData.nSingleWinMost )
			{
				m_tData.nSingleWinMost = pRequest->vArg[2] ;
			}

			m_bDirty = true ;
		}
		break;
	default:
		return false ;
	}
	return true ;
}

bool CPlayerNiuNiu::onCrossServerRequestRet(stMsgCrossServerRequestRet* pResult,Json::Value* vJsValue )
{
	if ( IPlayerComponent::onCrossServerRequestRet(pResult,vJsValue) )
	{
		return true ;
	}

	switch ( pResult->nRequestType )
	{
	default:
		return false;
	}
	return true ;
}

void CPlayerNiuNiu::OnPlayerDisconnect()
{

}

void CPlayerNiuNiu::OnOtherWillLogined()
{

}

void CPlayerNiuNiu::TimerSave()
{
	if ( m_bDirty )
	{
		stMsgSavePlayerNiuNiuData msgSave ;
		msgSave.nUserUID = GetPlayer()->GetUserUID() ;
		memcpy(&msgSave.tData,&m_tData,sizeof(msgSave.tData));
		SendMsg(&msgSave,sizeof(msgSave)) ;
	}
	m_bDirty = false ;
}

void CPlayerNiuNiu::OnReactive(uint32_t nSessionID )
{
	sendNiuNiuDataToClient();
}

void CPlayerNiuNiu::OnOtherDoLogined()
{
	sendNiuNiuDataToClient();
}

void CPlayerNiuNiu::addOwnRoom(uint32_t nRoomID , uint16_t nConfigID )
{
	stMyOwnRoom myroom ;
	myroom.nRoomID = nRoomID ;
	myroom.nConfigID = nConfigID;
	m_vMyOwnRooms.insert(MAP_ID_MYROOW::value_type(myroom.nRoomID,myroom));
}

bool CPlayerNiuNiu::isCreateRoomCntReachLimit()
{
	if ( GetPlayer()->GetUserUID() == MATCH_MGR_UID )
	{
		return false ;
	}

	return m_vMyOwnRooms.size() >= 5 ;
}

bool CPlayerNiuNiu::deleteOwnRoom(uint32_t nRoomID )
{
	auto iter = m_vMyOwnRooms.find(nRoomID) ;
	if ( iter != m_vMyOwnRooms.end() )
	{
		m_vMyOwnRooms.erase(iter) ;
		return true ;
	}
	return false ;
}

uint16_t CPlayerNiuNiu::getMyOwnRoomConfig( uint32_t nRoomID ) 
{
	MAP_ID_MYROOW::iterator iter = m_vMyOwnRooms.find(nRoomID) ;
	if ( iter != m_vMyOwnRooms.end() )
	{
		return iter->second.nConfigID ;
	}
	return 0 ;
}

bool CPlayerNiuNiu::isRoomIDMyOwn(uint32_t nRoomID)
{
	MAP_ID_MYROOW::iterator iter = m_vMyOwnRooms.find(nRoomID) ;
	return iter != m_vMyOwnRooms.end() ;
}

void CPlayerNiuNiu::sendNiuNiuDataToClient()
{
	stMsgPlayerBaseDataNiuNiu msg ;
	memcpy(&msg.tNiuNiuData,&m_tData,sizeof(m_tData)) ;
	SendMsg(&msg,sizeof(msg)) ;
}