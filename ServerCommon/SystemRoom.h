#pragma once
#include "IRoomDelegate.h"
#include "IRoomInterface.h"
#include "ISitableRoom.h"
struct stBaseRoomConfig ;
class IRoomManager ;

template<class TR>
class CSystemRoom
	:public IRoomInterface
	,public IRoomDelegate
{
public:
	typedef TR REAL_ROOM ;
	typedef TR* REAL_ROOM_PTR;
	typedef std::vector<REAL_ROOM*> VEC_ROOMS ;
public:
	CSystemRoom();
	~CSystemRoom();
	bool onFirstBeCreated(IRoomManager* pRoomMgr, uint32_t nRoomID, const Json::Value& vJsValue )override;
	void serializationFromDB(IRoomManager* pRoomMgr,stBaseRoomConfig* pConfig,uint32_t nRoomID , Json::Value& vJsValue )override;
	void serializationToDB()override;

	uint8_t canPlayerEnterRoom( stEnterRoomData* pEnterRoomPlayer )override;  // return 0 means ok ;
	void onPlayerEnterRoom(stEnterRoomData* pEnterRoomPlayer, int8_t& nSubIdx)override;
	void roomItemDetailVisitor(Json::Value& vOutJsValue)override;
	uint32_t getRoomID()override;
	uint8_t getRoomType()override;
	void update(float fDelta)override;
	void onTimeSave()override;
	bool onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID )override;
	bool isDeleteRoom()override;
	void deleteRoom()override ;
	bool onPlayerApplyLeaveRoom(uint32_t nUserUID )override;
	uint32_t getOwnerUID()final{ return MATCH_MGR_UID ; }

	// delegate method
	bool isRoomShouldClose( IRoom* pRoom)override;
	bool isOmitNewPlayerHalo(IRoom* pRoom )override;
	void onRankPlayerChanged( uint32_t nUID , uint16_t nPreIdx , uint16_t nCurIdx )override;
	bool isPlayerLoseReachMax( IRoom* pRoom, uint32_t nUserUID )override;

	int32_t getPlayerOffsetByUID( uint32_t nUserUID )override  ;

	// self method 
protected:
	void onRoomOpen();
	void onRoomClose();
	void resetProfit();
	bool isSubRoomClosed();
	void refreshLastGameRank();
	REAL_ROOM* getRoomBySession(uint32_t nSessionID );
	REAL_ROOM* getRoomByUID(uint32_t nUID );
	REAL_ROOM* getRoomForPlayerToEnter(uint8_t nPlayerType );
	eRoomState getRoomState(){ return m_eState ; }
	const char* getRoomName(){ return m_strName.c_str();}
	uint32_t getProfit();
	time_t getCloseTime(){ return m_tCloseTime ; }
	void sendRoomInfo(uint32_t nSessionID , int8_t nSubRoomIdx = -1 );
protected:
	bool m_bReadOk ; 
	std::string m_strName ;
	IRoomManager* m_pRoomMgr ;
	VEC_ROOMS m_vRooms ;
	stBaseRoomConfig* m_pConfig ;
	uint32_t m_nRoomID ;

	uint32_t m_nTermNumber ;
	eRoomState m_eState ;
	time_t m_tOpenTime ;
	uint32_t m_nDuringSeconds ;
	bool m_bRoomInfoDiry ;
	
	LIST_ROOM_RANK_ITEM m_vLastGameRank ;

	time_t m_tCloseTime ;

	char* m_pRewardInfoBuffer ;
	uint16_t m_nBufferLen ;
};


// define 
#include "log4z.h"
#include "RoomConfig.h"
#include "ServerMessageDefine.h"
#include "IRoomManager.h"
#include "SeverUtility.h"
#include "ServerStringTable.h"
#include "IRoomState.h"
#include "RewardConfig.h"
#define TIME_DURING_ROOM_CLOSE 60*6
#define TIME_DURING_ROOM_OPEN 60*10
template<class TR >
CSystemRoom<TR>::CSystemRoom()
{
	m_vRooms.clear() ;
	m_pConfig = nullptr ;
	m_nRoomID = 0 ;
	m_nTermNumber = 0 ;
	m_eState = eRoomState_None ;
	m_tOpenTime = 0 ;
	m_nDuringSeconds = 0 ;
	m_bRoomInfoDiry = false ;
	m_pRoomMgr = nullptr ;
	m_tCloseTime = 0 ;
	m_strName = "null";
	m_bReadOk = false ;
	m_pRewardInfoBuffer = nullptr ;
	m_nBufferLen = 0 ;
}

template<class TR >
CSystemRoom<TR>::~CSystemRoom()
{
	m_pConfig = nullptr ;
	m_nRoomID = 0 ;
	m_nTermNumber = 0 ;
	m_eState = eRoomState_None ;
	m_tOpenTime = 0 ;
	m_nDuringSeconds = 0 ;

	for ( auto pp : m_vRooms )
	{
		delete pp ;
		pp = nullptr ;
	}
	m_vRooms.clear() ;

	if ( m_pRewardInfoBuffer )
	{
		delete[] m_pRewardInfoBuffer ;
		m_pRewardInfoBuffer = nullptr ;
		m_nBufferLen = 0 ;
	}
}


template<class TR >
int32_t CSystemRoom<TR>::getPlayerOffsetByUID( uint32_t nUserUID )
{
	for ( auto ri : m_vSortedRankItems )
	{
		if ( ri->nUserUID == nUserUID )
		{
			return ri->nGameOffset ;
		}
	}
	return 0 ;
}


template<class TR >
bool CSystemRoom<TR>::onFirstBeCreated(IRoomManager* pRoomMgr, uint32_t nRoomID, const Json::Value& vJsValue )
{
	time_t tNow = time(nullptr) ;
	m_pConfig = pConfig ;
	m_nRoomID = nRoomID ;
	m_tOpenTime = tNow ;
	m_nDuringSeconds = TIME_DURING_ROOM_OPEN;
	m_eState = eRoomState_Opening ;
	m_pRoomMgr = pRoomMgr ;
	m_bRoomInfoDiry = true ;
	if ( vJsValue["duringTime"].isNull() == false )
	{
		m_nDuringSeconds = vJsValue["duringTime"].asUInt() ;
	}

	if ( vJsValue["openTime"].isNull() == false )
	{
		m_tOpenTime = vJsValue["openTime"].asUInt() ;
	}


	if ( m_tOpenTime < tNow )
	{
		m_tOpenTime = tNow ;
	}

	if ( m_tOpenTime > tNow )
	{
		m_eState = eRoomState_Close ;
	}
	m_tCloseTime = m_tOpenTime + (time_t)m_nDuringSeconds ;

	if ( vJsValue["name"].isNull() == false )
	{
		m_strName = vJsValue["name"].asString();
		LOGFMTD("new create room name = %s",m_strName.c_str()) ;
	}
	uint16_t nSubRoomCnt = 2 ;
	if ( vJsValue["subRoomCnt"].isNull() == false )
	{
		nSubRoomCnt = vJsValue["subRoomCnt"].asInt();
		if ( nSubRoomCnt <= 0 )
		{
			LOGFMTE("why set sub room cnt = 0 ") ;
			nSubRoomCnt = 3 ;
		}
	}

	vJsValue["parentRoomID"] = getRoomID() ;
	LOGFMTE("temp open 3 count room") ;
	nSubRoomCnt = 1 ;
	for ( uint16_t nIdx = 0 ; nIdx < nSubRoomCnt ; ++nIdx )
	{
		auto * pRoom = new REAL_ROOM ;
		pRoom->onFirstBeCreated(pRoomMgr,pConfig,nIdx ,vJsValue);
		pRoomMgr->reqeustChatRoomID(pRoom);
		pRoom->setDelegate(this);
		m_vRooms.push_back(pRoom) ;
	}
	LOGFMTD("create %u sub room",nSubRoomCnt) ;
	return true ;
}

template<class TR >
void CSystemRoom<TR>::serializationFromDB(IRoomManager* pRoomMgr,stBaseRoomConfig* pConfig,uint32_t nRoomID , Json::Value& vJsValue )
{
	m_nRoomID = nRoomID ;
	m_pConfig = pConfig ;
	m_pRoomMgr = pRoomMgr ;
	m_nTermNumber = vJsValue["termNum"].asUInt();
	m_eState = (eRoomState)vJsValue["state"].asUInt();
	m_tOpenTime = (time_t)vJsValue["openT"].asUInt();
	m_nDuringSeconds = vJsValue["duringT"].asUInt();
	m_tCloseTime = (time_t)vJsValue["closeT"].asUInt();
	m_strName = vJsValue["name"].asString();

	Json::Value arraySubRoom = vJsValue["sub"];
	LOGFMTE("temp open 3 count room") ;
	uint8_t nSubRoomCnt = 1 ;
	for ( uint16_t nIdx = 0 ; nIdx < arraySubRoom.size() && nIdx < nSubRoomCnt; ++nIdx )
	{
		Json::Value sbuRoom = arraySubRoom[nIdx] ;
		sbuRoom["parentRoomID"] = getRoomID() ;
		auto pRoom = new REAL_ROOM ;
		pRoom->serializationFromDB(pRoomMgr,pConfig,nIdx,sbuRoom);
		pRoom->setDelegate(this);
		m_vRooms.push_back(pRoom);
	}

	// read rank data 
	stMsgReadRoomPlayer msgRead ;
	msgRead.nRoomID = getRoomID();
	msgRead.nRoomType = getRoomType() ;
	msgRead.nTermNumber = m_nTermNumber ;
	m_pRoomMgr->sendMsg(&msgRead,sizeof(msgRead),getRoomID()) ;

	LOGFMTD("read room id = %u ternm = %u rank player",getRoomID(),m_nTermNumber) ;
	if ( m_nTermNumber > 0 )
	{
		stMsgReadRoomPlayer msgRead ;
		msgRead.nRoomID = getRoomID();
		msgRead.nRoomType = getRoomType() ;
		msgRead.nTermNumber = m_nTermNumber - 1 ;
		m_pRoomMgr->sendMsg(&msgRead,sizeof(msgRead),getRoomID()) ;
		LOGFMTD("read room id = %u last rank players ",getRoomID()) ;
	}
}

template<class TR >
void CSystemRoom<TR>::serializationToDB()
{
	Json::StyledWriter jsWrite ;
	Json::Value vValue ;

	vValue["termNum"] = m_nTermNumber ;
	vValue["state"] = m_eState ;
	vValue["openT"] = (uint32_t)m_tOpenTime ;
	vValue["duringT"] = m_nDuringSeconds ;
	vValue["closeT"] = (uint32_t)m_tCloseTime ;
	vValue["name"] = getRoomName();

	Json::Value arraySubRoom ;
	for ( uint16_t nIdx = 0 ; nIdx < (uint16_t)m_vRooms.size(); ++nIdx )
	{
		Json::Value subRoom ;
		m_vRooms[nIdx]->willSerializtionToDB(subRoom);
		arraySubRoom[nIdx] = subRoom ;
	}
	vValue["sub"] = arraySubRoom ;

	std::string strJson = jsWrite.write(vValue);

	stMsgSaveUpdateRoomInfo msgSave ;
	msgSave.nRoomType = getRoomType() ;
	msgSave.nRoomID = getRoomID() ;
	msgSave.nJsonLen = strJson.size() ;
	msgSave.nConfigID = m_pConfig->nConfigID;
	msgSave.nRoomOwnerUID = MATCH_MGR_UID ;

	CAutoBuffer autoBuffer(sizeof(msgSave) + msgSave.nJsonLen);
	autoBuffer.addContent((char*)&msgSave,sizeof(msgSave)) ;
	autoBuffer.addContent(strJson.c_str(),msgSave.nJsonLen) ;
	m_pRoomMgr->sendMsg((stMsg*)autoBuffer.getBufferPtr(),autoBuffer.getContentSize(),0) ;
}

template<class TR >
uint8_t CSystemRoom<TR>::canPlayerEnterRoom( stEnterRoomData* pEnterRoomPlayer )  // return 0 means ok ;
{
	if ( pEnterRoomPlayer->nPlayerType == ePlayer_Robot )
	{
		LOGFMTE("temp let robot enter any room");
		return 0 ;
	}

	if ( getRoomState() != eRoomState_Opening )
	{
		return 7 ;  // room not open 
	}

	if ( m_pConfig->bIsNeedRegistered )
	{
		if ( pEnterRoomPlayer->isRegisted == false )
		{
			LOGFMTD("player is visitor , son can not enter room , uid = %d",pEnterRoomPlayer->nUserUID);
			return 2 ; // not register player  can not enter ;
		}
	}

	if ( m_pConfig->nCoinLowLimit > pEnterRoomPlayer->nCoin )
	{
		LOGFMTD("player coin is too few so can not enter room , uid = %d",pEnterRoomPlayer->nUserUID);
		return 3 ; // player coin is too few ;
	}

	if ( m_pConfig->nCoinTopLimit && m_pConfig->nCoinTopLimit < pEnterRoomPlayer->nCoin )
	{
		LOGFMTD("player coin is too many, so can not enter room , uid = %d",pEnterRoomPlayer->nUserUID);
		return 4 ; // player coin is too many ;
	}

	if ( isPlayerLoseReachMax(nullptr,pEnterRoomPlayer->nUserUID) )
	{
		LOGFMTD("player lose reach limit, so can not enter room , uid = %d",pEnterRoomPlayer->nUserUID);
		return 5 ; // lose too many ;
	}

	auto alReadyRoom = getRoomBySession(pEnterRoomPlayer->nUserSessionID);
	auto alReadyRoom2 = getRoomByUID(pEnterRoomPlayer->nUserUID);
	if ( alReadyRoom != nullptr || alReadyRoom2 != nullptr )
	{
		LOGFMTD("you already in some room, so can not enter room , uid = %d",pEnterRoomPlayer->nUserUID);
		return 1 ;
	}
	return 0 ;
}

template<class TR >
void CSystemRoom<TR>::onPlayerEnterRoom(stEnterRoomData* pEnterRoomPlayer, int8_t& nSubIdx )
{
	if ( nSubIdx >= (int8_t)m_vRooms.size() )
	{
		LOGFMTE("invalid sub idx = %u ,room id = %u",nSubIdx, getRoomID());
		nSubIdx = -1 ;
	}

	REAL_ROOM_PTR roomEnter = nullptr;
	if ( nSubIdx >= (int8_t)0  )
	{
		roomEnter = m_vRooms[nSubIdx] ;
	}
	else
	{
		roomEnter = getRoomForPlayerToEnter(pEnterRoomPlayer->nPlayerType) ;
		nSubIdx = roomEnter->getRoomID();
	}
	
	roomEnter->onPlayerEnterRoom(pEnterRoomPlayer,nSubIdx);
	sendRoomInfo(pEnterRoomPlayer->nUserSessionID,roomEnter->getRoomID());
	//roomEnter->sendRoomPlayersInfo(pEnterRoomPlayer->nUserSessionID);
	LOGFMTD("uid = %u , enter room id = %u , subIdx = %u",pEnterRoomPlayer->nUserUID, getRoomID(),roomEnter->getRoomID()) ;
}

template<typename TR >
void CSystemRoom<TR>::sendRoomInfo(uint32_t nSessionID , int8_t nSubRoomIdx )
{
	typename REAL_ROOM_PTR pRoom = nullptr ;
	if ( nSubRoomIdx < 0 )
	{
		pRoom = getRoomBySession(nSessionID) ;
	}
	else if ( nSubRoomIdx < (int16_t)m_vRooms.size() )
	{
		pRoom = m_vRooms[nSubRoomIdx] ;
	}

	if ( pRoom == nullptr )
	{
		LOGFMTE("session id = %u requesiont room info room id = %u, subIdx = %d is null",nSessionID , getRoomID(),nSubRoomIdx) ;
		return ;
	}

	stMsgRoomInfo msgInfo ;
	msgInfo.eCurRoomState = pRoom->getCurRoomState()->getStateID() ;
	msgInfo.fChouShuiRate = m_pConfig->fDividFeeRate ;
	msgInfo.nChatRoomID = pRoom->getChatRoomID() ;
	msgInfo.nCloseTime = (uint32_t)getCloseTime() ;
	msgInfo.nDeskFee =  m_pConfig->nDeskFee ;
	msgInfo.nMaxSeat = (uint8_t)pRoom->getSeatCount();
	msgInfo.nRoomID = getRoomID() ;
	msgInfo.nRoomType = getRoomType() ;
	msgInfo.nSubIdx = pRoom->getRoomID() ;

	Json::StyledWriter wr ;
	Json::Value vOut ;
	pRoom->roomInfoVisitor(vOut);
	std::string str = wr.write(vOut) ;
	msgInfo.nJsonLen = str.size() ;
	CAutoBuffer sBuf(sizeof(msgInfo) + msgInfo.nJsonLen );
	sBuf.addContent(&msgInfo,sizeof(msgInfo)) ;
	sBuf.addContent(str.c_str(),msgInfo.nJsonLen) ;
	m_pRoomMgr->sendMsg((stMsg*)sBuf.getBufferPtr(),sBuf.getContentSize(),nSessionID) ;
	LOGFMTD("send room info to session id = %u js:%s",nSessionID, str.c_str()) ;

	pRoom->sendRoomPlayersInfo(nSessionID);
}

template<class TR >
void CSystemRoom<TR>::roomItemDetailVisitor(Json::Value& vOutJsValue)
{
	vOutJsValue["configID"] = m_pConfig->nConfigID ;
	vOutJsValue["openTime"] = (uint32_t)m_tOpenTime ;
	vOutJsValue["closeTime"] = (uint32_t)m_tCloseTime ;
	vOutJsValue["state"] = (uint32_t)getRoomState() ;
	vOutJsValue["name"] = getRoomName();
}

template<class TR >
uint32_t CSystemRoom<TR>::getRoomID()
{
	return m_nRoomID ;
}

template<class TR >
uint8_t CSystemRoom<TR>::getRoomType()
{
	return m_vRooms[0]->getRoomType();
}

template<class TR >
void CSystemRoom<TR>::update(float fDelta)
{
	if ( !m_bReadOk )
	{
		//return ;
	}

	for ( auto pp : m_vRooms )
	{
		pp->update(fDelta);
	}

	// update room state 
	switch ( getRoomState())
	{
	case eRoomState_Opening:
		{
			time_t tNow = time(nullptr);
			if ( tNow >= getCloseTime() )
			{
				m_eState = eRoomState_WillClose ;
				m_bRoomInfoDiry = true ;
				LOGFMTD("uid = %d change do will close",getRoomID() );
			}
		}
		break;
	case eRoomState_WillClose:
		{
			if ( isSubRoomClosed() )
			{
				m_eState = eRoomState_Close ;
				m_bRoomInfoDiry = true ;
				LOGFMTD("uid = %d change do close",getRoomID() );
				onRoomClose();
			}
		}
		break;
	case eRoomState_Close:
		{
			time_t tNow = time(nullptr);
			if ( tNow >= m_tOpenTime )
			{
				m_eState = eRoomState_Opening ;
				m_bRoomInfoDiry = true ;
				LOGFMTD("uid = %d change do open",getRoomID() );
				onRoomOpen();
			}
		}
		break;
	case eRoomState_WillDead:
		{
			if ( isSubRoomClosed() )
			{
				m_bRoomInfoDiry = true ;
				LOGFMTD("uid = %d change do dead",getRoomID() );
				m_eState = eRoomState_Dead ;
				for ( auto uu : m_vRooms )
				{
					uu->forcePlayersLeaveRoom();
					m_pRoomMgr->deleteRoomChatID(uu->getChatRoomID()) ;
				}
			}
		}
		break;
	case eRoomState_Dead:
	case eRoomState_None:
		{

		}
		break;
	default:
		LOGFMTE("unknonw room state = %u room id = %u",getRoomState(),getRoomID()) ;
		break;
	}
}

template<class TR >
void CSystemRoom<TR>::onTimeSave()
{
	LOGFMTD("time save room info room id = %u",getRoomID());
	// save room rank ;
	stMsgSaveRoomPlayer msgSave ;
	msgSave.nRoomID = getRoomID() ;
	msgSave.nRoomType = getRoomType() ;
	msgSave.nTermNumber = m_nTermNumber ;
	for ( auto pp : m_vSortedRankItems )
	{
		if ( !pp->bIsDiryt )
		{
			continue;
		}
		pp->bIsDiryt = false ;

		msgSave.savePlayer.nUserUID = pp->nUserUID ;
		msgSave.savePlayer.nGameOffset = pp->nGameOffset ;
		msgSave.savePlayer.nOtherOffset = pp->nOtherOffset ;
		m_pRoomMgr->sendMsg(&msgSave,sizeof(msgSave),0) ;
		LOGFMTD("update rank uid = %u , offset = %d",pp->nUserUID,pp->nGameOffset) ;
	}

	for ( auto pR : m_vRooms )
	{
		if ( pR->isRoomInfoDirty() )
		{
			m_bRoomInfoDiry = true ;
		}
	}

	if ( m_bRoomInfoDiry )
	{
		serializationToDB();
		m_bRoomInfoDiry = false ;
		for ( auto pR : m_vRooms )
		{
			pR->setRoomInfoDirty(false);
		}
	}
}

template<class TR >
bool CSystemRoom<TR>::onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID )
{
	switch ( prealMsg->usMsgType )
	{
	case MSG_REQUEST_ROOM_REWARD_INFO:
		{
			LOGFMTD("session id = %u request room id = %u reward info",nPlayerSessionID,getRoomID()) ;
			if ( nullptr != m_pRewardInfoBuffer )
			{
				m_pRoomMgr->sendMsg((stMsg*)m_pRewardInfoBuffer,m_nBufferLen,nPlayerSessionID) ;
				break ;
			}

			Json::Value vDetail ;
			for ( uint8_t nIdx = 0 ; nIdx < m_pConfig->vRewardID.size(); ++nIdx )
			{
				auto pr = CRewardConfig::getInstance()->getRewardByID(m_pConfig->vRewardID[nIdx]) ;
				if ( !pr )
				{
					vDetail[nIdx] = "none";
					continue;
				}

				vDetail[nIdx] = pr->strRewardDesc ; 
			}

			Json::StyledWriter write ;
			std::string strDe = write.write(vDetail);

			stMsgRequestRoomRewardInfoRet msgBack ;
			msgBack.nJsonLen = strDe.size() ;
			msgBack.nRoomID = getRoomID() ;
			msgBack.nRoomType = getRoomType() ;

			m_nBufferLen = sizeof(msgBack) + msgBack.nJsonLen;
			m_pRewardInfoBuffer = new char[m_nBufferLen] ;
			memcpy_s(m_pRewardInfoBuffer,m_nBufferLen,&msgBack,sizeof(msgBack));
			memcpy_s(m_pRewardInfoBuffer + sizeof(msgBack),m_nBufferLen - sizeof(msgBack),strDe.c_str(),msgBack.nJsonLen);
			m_pRoomMgr->sendMsg((stMsg*)m_pRewardInfoBuffer,m_nBufferLen,nPlayerSessionID) ;
		}
		break;
	case MSG_READ_ROOM_PLAYER:
		{
			stMsgReadRoomPlayerRet* pRet = (stMsgReadRoomPlayerRet*)prealMsg ; 
			if ( pRet->nTermNumber + 1 < m_nTermNumber )
			{
				LOGFMTI("recieved last last term player data , skip id  room id = %u",getRoomID()) ;
				break; 
			}

			if ( pRet->bIsLast && m_nTermNumber == pRet->nTermNumber )
			{
				LOGFMTI("room id = %u read ok ",getRoomID()) ;
				m_bReadOk = true ;
			}

			LOGFMTD("room id = %d recive room player data cnt = %d",getRoomID(),pRet->nCnt) ;
			stSaveRoomPlayerEntry* pp = (stSaveRoomPlayerEntry*)(((char*)prealMsg) + sizeof(stMsgReadRoomPlayerRet));
			while ( pRet->nCnt-- )
			{
				stRoomRankItem* p = new stRoomRankItem ;
				p->bIsDiryt = false ;
				p->nUserUID = pp->nUserUID ;
				p->nGameOffset = pp->nGameOffset ;
				p->nOtherOffset = pp->nOtherOffset ;
				if ( pRet->nTermNumber == m_nTermNumber )  // current term rank item ;
				{
					m_vRoomRankHistroy[p->nUserUID] = p ;
					m_vSortedRankItems.push_back(p) ;
				}
				else if ( pRet->nTermNumber + 1 == m_nTermNumber ) // last term rank 
				{
					m_vLastGameRank.push_back(p) ;
				}
				else
				{
					LOGFMTE("recived too old rank data room id = %u",getRoomID());
					return true ;
				}

				++pp;
			}

			if ( pRet->nTermNumber + 1 == m_nTermNumber && m_vLastGameRank.size() >= 2 && pRet->bIsLast ) // last term rank 
			{
				extern bool sortFuncRankItem(IRoomDelegate::stRoomRankItem* pLeft , IRoomDelegate::stRoomRankItem* pRight );

				m_vLastGameRank.sort(sortFuncRankItem);
				LOGFMTD("sort last rank player room id = %u",getRoomID());
			}
		}
		break;
	case MSG_REQUEST_ROOM_RANK:
		{
			uint32_t nUserID = 0 ;
			auto tRoom = getRoomBySession(nPlayerSessionID);
			if ( tRoom )
			{
				auto pp = tRoom->getPlayerBySessionID(nPlayerSessionID) ;
				if ( pp )
				{
					nUserID = pp->nUserUID ;
				}
			}

			std::map<uint32_t,stRoomRankEntry> vWillSend ;
			sortRoomRankItem();
			int16_t nSelfRankIdx = -1 ;
			uint16_t nSendRankCnt = 5 ;
			int16_t nMaxCheckCount = 30 ;
			// add 5 player into list ;
			LIST_ROOM_RANK_ITEM::iterator iter = m_vSortedRankItems.begin();
			for ( uint8_t nIdx = 0 ; iter != m_vSortedRankItems.end(); ++iter,++nIdx )
			{
				// push rank 5 to send to client ;
				stRoomRankItem* pItem = (*iter) ;
				if ( pItem->nUserUID == nUserID )
				{
					nSelfRankIdx = nIdx ;
				}

				if ( nIdx < nSendRankCnt || pItem->nUserUID == nUserID )
				{
					stRoomRankEntry entry ;
					entry.nGameOffset = pItem->nGameOffset ;
					entry.nOtherOffset = pItem->nOtherOffset ;
					entry.nUserUID = pItem->nUserUID ;
					vWillSend[pItem->nUserUID] = entry ;
				}
				else
				{
					if ( nSelfRankIdx != -1 || nUserID == 0 )  // find self or not in this room ;
					{
						break;
					}
				}

				--nMaxCheckCount ;
				if ( nMaxCheckCount <= 0 )
				{
					break;
				}
			}

			LOGFMTD("uid = %d request rank room id = %u",nUserID,getRoomID());
			// send room info to player ;
			stMsgRequestRoomRankRet msgRet ;
			msgRet.nCnt = vWillSend.size() ;
			msgRet.nSelfRankIdx = nSelfRankIdx ;
			CAutoBuffer msgBuffer(sizeof(msgRet) + msgRet.nCnt * sizeof(stRoomRankEntry));
			msgBuffer.addContent(&msgRet,sizeof(msgRet));
			for ( auto& itemSendPlayer : vWillSend )
			{
				msgBuffer.addContent(&itemSendPlayer.second,sizeof(stRoomRankEntry));
				LOGFMTD("room id = %u rank player uid = %u offset = %d",getRoomID(),itemSendPlayer.second.nUserUID,itemSendPlayer.second.nGameOffset);
			}
			m_pRoomMgr->sendMsg((stMsg*)msgBuffer.getBufferPtr(),msgBuffer.getContentSize(),nPlayerSessionID) ;
		}
		break;
	case MSG_REQUEST_LAST_TERM_ROOM_RANK:
		{
			std::map<uint32_t,stRoomRankEntry> vWillSend ;
			// add at most 3 player into list ;
			LIST_ROOM_RANK_ITEM::iterator iter = m_vLastGameRank.begin();
			for ( uint8_t nIdx = 0 ; iter != m_vLastGameRank.end() && nIdx < 3; ++iter,++nIdx )
			{
				// push rank 3 to send to client ;
				stRoomRankItem* pItem = (*iter) ;
				stRoomRankEntry entry ;
				entry.nGameOffset = pItem->nGameOffset ;
				entry.nOtherOffset = pItem->nOtherOffset ;
				entry.nUserUID = pItem->nUserUID ;
				vWillSend[pItem->nUserUID] = entry ;

			}

			// send room info to player ;
			stMsgRequestLastTermRoomRankRet msgRet ;
			msgRet.nCnt = vWillSend.size() ;
			CAutoBuffer msgBuffer(sizeof(msgRet) + msgRet.nCnt * sizeof(stRoomRankEntry));
			msgBuffer.addContent(&msgRet,sizeof(msgRet));
			for ( auto& itemSendPlayer : vWillSend )
			{
				msgBuffer.addContent(&itemSendPlayer.second,sizeof(stRoomRankEntry));
			}
			m_pRoomMgr->sendMsg((stMsg*)msgBuffer.getBufferPtr(),msgBuffer.getContentSize(),nPlayerSessionID) ;
			LOGFMTD("session id = %u request last game rank , room id = %u",nPlayerSessionID,getRoomID());
		}
		break;
	case MSG_REQUEST_ROOM_INFO:
		{
			stMsgToRoom* pRet = (stMsgToRoom*)prealMsg ;
			sendRoomInfo(nPlayerSessionID,pRet->nSubRoomIdx);
		}
		break;
	default:
		{
			stMsgToRoom* pRet = (stMsgToRoom*)prealMsg ;
			REAL_ROOM_PTR pRoom = nullptr ;
			if (pRet->nSubRoomIdx < (int8_t)0 )
			{
				pRoom = getRoomBySession(nPlayerSessionID) ;

			}
			else if ( pRet->nSubRoomIdx < (int8_t)m_vRooms.size() )
			{
				pRoom = m_vRooms[pRet->nSubRoomIdx] ;
			}

			if ( pRoom )
			{
				return pRoom->onMessage(prealMsg,eSenderPort,nPlayerSessionID) ;
			}
			else
			{
				LOGFMTE("sessionid = %u send msg = %u, subIdx = %d room is null room id = %u", nPlayerSessionID,pRet->usMsgType,pRet->nSubRoomIdx,getRoomID()) ;
			}
		}
		return false ;
	}

	return true ;
}

template<class TR >
bool CSystemRoom<TR>::isDeleteRoom()
{
	return m_eState == eRoomState_Dead ;
}

template<class TR >
void CSystemRoom<TR>::deleteRoom()
{
	m_eState = eRoomState_WillDead ;
}

template<class TR >
bool CSystemRoom<TR>::onPlayerApplyLeaveRoom(uint32_t nUserUID )
{
	auto room = getRoomByUID(nUserUID) ;
	if ( room )
	{
		return room->onPlayerApplyLeaveRoom(nUserUID) ;
	}
	return false ;
}

// delegate method
template<class TR >
bool CSystemRoom<TR>::isRoomShouldClose( IRoom* pRoom)
{
	return (getRoomState() != eRoomState_Opening );
}

template<class TR >
bool CSystemRoom<TR>::isOmitNewPlayerHalo(IRoom* pRoom )
{
	return m_pConfig->bIsOmitNewPlayerHalo ;
}

template<class TR >
void CSystemRoom<TR>::onRankPlayerChanged( uint32_t nUID , uint16_t nPreIdx , uint16_t nCurIdx )
{
	CSendPushNotification::getInstance()->reset() ;
	CSendPushNotification::getInstance()->addTarget(nUID);

	uint16_t nStrID = 7 ;
	if ( nPreIdx < 3 && nCurIdx >= 3 )  // leave qian 3 
	{
		nStrID = 8 ;
		if ( eRoom_NiuNiu == getRoomType() )
		{
			nStrID = 9 ;
		}
	}
	else if ( nPreIdx >= 3 && nCurIdx < 3 )  // enter qian 3 
	{
		nStrID = 6 ;
		if ( eRoom_NiuNiu == getRoomType() )
		{
			nStrID = 7 ;
		}
	}
	else
	{
		return ;
	}

	char pBuffer[256] = {0} ;
	sprintf_s(pBuffer,sizeof(pBuffer),CServerStringTable::getInstance()->getStringByID(nStrID),getRoomName());
	CSendPushNotification::getInstance()->setContent(pBuffer,1);
	CSendPushNotification::getInstance()->postApns( m_pRoomMgr->getSvrApp()->getAsynReqQueue(),false,"rank changed") ;
	LOGFMTD("send rank change apns uid = %u",nUID) ;
}

template<class TR >
bool CSystemRoom<TR>::isPlayerLoseReachMax( IRoom* pRoom, uint32_t nUserUID )
{
	if ( m_pConfig->nMaxLose == 0  )
	{
		return false ;
	}

	auto pp = getRankItemByUID(nUserUID) ;
	if ( nullptr == pp )
	{
		return false ;
	}

	int32_t nRet = pp->nGameOffset + m_pConfig->nMaxLose ;
	return nRet <= 0 ;
}

// self method 
template<class TR >
void CSystemRoom<TR>::onRoomOpen()
{
	m_eState = eRoomState_Opening ;
	m_tCloseTime = m_tOpenTime + m_nDuringSeconds ;
	LOGFMTI("room id = %d opened",getRoomID());
	// refresh last game rank ;
	refreshLastGameRank();
	// remove all histroy 
	removeAllRankItemPlayer();
	++m_nTermNumber ;
	m_bRoomInfoDiry = true ;
	LOGFMTI("room id = %u star to termNumber = %u",getRoomID(),m_nTermNumber) ;
}

template<class TR >
void CSystemRoom<TR>::resetProfit()
{
	for ( auto pr : m_vRooms )
	{
		pr->setTotalProfit(0);
	}
}

template<class TR >
void CSystemRoom<TR>::onRoomClose()
{
	m_eState = eRoomState_Close ;
	LOGFMTI("room id = %d closed",getRoomID());
	sortRoomRankItem();

	m_tOpenTime = time(nullptr) + TIME_DURING_ROOM_CLOSE ; // half an hour later reopen ;
	m_bRoomInfoDiry = true;

	// tong ji guanjun 
	if ( m_vSortedRankItems.empty() )
	{
		// save log ;
		stMsgSaveLog msgLog ;
		msgLog.nJsonExtnerLen = 0 ;
		msgLog.nLogType = eLog_MatchResult ;
		msgLog.nTargetID = getRoomID() ;
		memset(msgLog.vArg,0,sizeof(msgLog.vArg)) ;
		msgLog.vArg[0] = getRoomType() ;
		msgLog.vArg[1] = m_nTermNumber ;
		msgLog.vArg[2] = getProfit();
		m_pRoomMgr->sendMsg(&msgLog,sizeof(msgLog),0);
		resetProfit();
		onTimeSave();
		return  ;
	}

	// process reward ;
	stMsgCrossServerRequest msgReq ;
	msgReq.cSysIdentifer = ID_MSG_PORT_DATA ;
	msgReq.nReqOrigID = getRoomID();
	msgReq.nTargetID = getRoomID();
	msgReq.nRequestType = eCrossSvrReq_GameOver ;
	msgReq.nRequestSubType = eCrossSvrReqSub_Default ;
	msgReq.vArg[0] = getRoomType();

	Json::Value vArray ;
	uint8_t nIdx = 0 ;
	for ( auto pRanker : m_vSortedRankItems )
	{
		if ( nIdx >= m_pConfig->vRewardID.size() )
		{
			break;
		}

		Json::Value item ;
		item["userUID"] = pRanker->nUserUID ;
		item["rewardID"] = m_pConfig->vRewardID[nIdx] ;
		vArray[nIdx] = item ;
		++nIdx ;
	}

	Json::Value vArgument ;
	vArgument["roomName"] = getRoomName();
	vArgument["players"] = vArray ;
	Json::StyledWriter wWrite ; 
	std::string strJson = wWrite.write(vArgument);
	msgReq.nJsonsLen = strJson.size() ;
	CAutoBuffer aub(sizeof(msgReq) + msgReq.nJsonsLen );
	aub.addContent(&msgReq,sizeof(msgReq)) ;
	aub.addContent(strJson.c_str(),msgReq.nJsonsLen) ;
	m_pRoomMgr->sendMsg((stMsg*)aub.getBufferPtr(),aub.getContentSize(),0) ;
	LOGFMTD("room id = %d game over , result js = %s",getRoomID(),strJson.c_str()) ;

	// save log ;
	stMsgSaveLog msgLog ;
	msgLog.nJsonExtnerLen = 0 ;
	msgLog.nLogType = eLog_MatchResult ;
	msgLog.nTargetID = getRoomID() ;
	memset(msgLog.vArg,0,sizeof(msgLog.vArg)) ;
	msgLog.vArg[0] = getRoomType() ;
	msgLog.vArg[1] = m_nTermNumber ;
	msgLog.vArg[2] = getProfit();
	resetProfit();

	Json::Value arrayLog ;
	uint8_t logIdx = 0 ;
	for ( auto pR : m_vSortedRankItems )
	{
		if ( logIdx >= 3 )
		{
			break ;
		}

		Json::Value item ;
		item["userUID"] = pR->nUserUID ;
		item["gameOffset"] = pR->nGameOffset;
		item["otherOffset"] = pR->nOtherOffset ;
		arrayLog[logIdx++] = item ;
	}
	Json::StyledWriter logWrite ;
	std::string logJson = logWrite.write(arrayLog) ;
	msgLog.nJsonExtnerLen = logJson.size() ;

	CAutoBuffer logBuffer(sizeof(msgLog) + msgLog.nJsonExtnerLen);
	logBuffer.addContent(&msgLog,sizeof(msgLog)) ;
	logBuffer.addContent(logJson.c_str(),msgLog.nJsonExtnerLen) ;
	m_pRoomMgr->sendMsg((stMsg*)logBuffer.getBufferPtr(),logBuffer.getContentSize(),getRoomID()) ;

	// send push notification 
	const char* pContent = CServerStringTable::getInstance()->getStringByID(3); // taxas 
	if ( getRoomType() == eRoom_NiuNiu )
	{
		pContent = CServerStringTable::getInstance()->getStringByID(4); // niu niu  
	}

	char pBuffer[256] = { 0 } ;
	sprintf_s(pBuffer,sizeof(pBuffer),pContent,getRoomName()) ;
	if ( strlen(pBuffer) > 219 )
	{
		LOGFMTE("msg too len : %s",pBuffer) ;
		onTimeSave();
		return ;
	}

	CSendPushNotification::getInstance()->reset();
	CSendPushNotification::getInstance()->setContent(pBuffer,1) ;
	for ( auto pp : m_vSortedRankItems )
	{
		CSendPushNotification::getInstance()->addTarget(pp->nUserUID) ;
	}

	CSendPushNotification::getInstance()->postApns( m_pRoomMgr->getSvrApp()->getAsynReqQueue(),false,"room closed") ;
	onTimeSave();
}

template<class TR >
void CSystemRoom<TR>::refreshLastGameRank()
{
	// clear data ;
	for ( auto rankItem : m_vLastGameRank )
	{
		delete rankItem ;
		rankItem = nullptr ;
	}
	m_vLastGameRank.clear() ;
	sortRoomRankItem();
	int8_t nCnt = 10 ;
	for ( auto pRank : m_vSortedRankItems )
	{
		stRoomRankItem* p = new stRoomRankItem ;
		memcpy(p,pRank,sizeof(stRoomRankItem));
		p->bIsDiryt = false ;
		m_vLastGameRank.push_back(p) ;
		if ( --nCnt <= 0 )
		{
			break;
		}
	}
}

template<class TR >
bool CSystemRoom<TR>::isSubRoomClosed()
{
	for ( auto pRoom : m_vRooms )
	{
		if ( pRoom->getCurRoomState()->getStateID() != eRoomState_Close )
		{
			return false ;
		}
	}

	return true ;
}

template<class TRR >
typename CSystemRoom<TRR>::REAL_ROOM_PTR  CSystemRoom<TRR>::getRoomBySession(uint32_t nSessionID )
{
	for ( auto pRoom : m_vRooms )
	{
		if ( pRoom->isPlayerInRoomWithSessionID(nSessionID) )
		{
			return pRoom ;
		}
	}
	return nullptr ;
}

template<class TR >
typename CSystemRoom<TR>::REAL_ROOM_PTR CSystemRoom<TR>::getRoomByUID(uint32_t nUID )
{
	for ( auto pRoom : m_vRooms )
	{
		if ( pRoom->isPlayerInRoomWithUserUID(nUID) )
		{
			return pRoom ;
		}
	}
	return nullptr ;
}

template<class TR >
typename CSystemRoom<TR>::REAL_ROOM_PTR CSystemRoom<TR>::getRoomForPlayerToEnter(uint8_t nPlayerType )
{

	//for ( auto pRoom : m_vRooms )
	//{
	//	if ( pRoom->getSitDownPlayerCount() < pRoom->getSeatCount() )
	//	{
	//		return pRoom ;
	//	}
	//}
	return m_vRooms[rand()%m_vRooms.size()] ;
}

template<class TR >
uint32_t CSystemRoom<TR>::getProfit()
{
	uint32_t nProfit = 0 ;
	for ( auto pRoom : m_vRooms )
	{
		nProfit += pRoom->getTotalProfit();
	}
	return nProfit ;
}