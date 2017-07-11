#pragma once
#include "IRoomInterface.h"
#include "ISitableRoom.h"
#include "IRoomDelegate.h"
#include <cassert>
#include "ISitableRoomPlayer.h"
#include "ConfigDefine.h"
struct stBuyInEntry
{
	uint32_t nBuyInCoin ;
	time_t tBuyInTime ;
};

struct stPrivateRoomPlayerItem
{
	uint32_t nUserUID ;
	uint32_t nToTalCoin ;
	uint32_t nCoinInRoom ;
	uint32_t nToTalBuyIn ;
	std::vector<stBuyInEntry> vBuyInRecord ;
	uint32_t nCheckedCoin ;
	bool isApplyIng ;
	bool isDirty ;
	uint8_t nPayDeskFeeState ;  // 0 not pay , 1 paying , 2  paied 

	stPrivateRoomPlayerItem(uint32_t nPlayerUID, uint32_t nAllCoin )
	{
		nUserUID = nPlayerUID ;
		nToTalCoin = nAllCoin ;
		nCoinInRoom = 0 ;
		nToTalBuyIn = 0 ;
		vBuyInRecord.clear() ;
		isDirty = false ;
		isApplyIng = false ;
		nCheckedCoin = 0 ;
		nPayDeskFeeState = 0 ;
	}

	bool isPayingDeskFee(){ return nPayDeskFeeState == 1 ;}
	bool isPayedDeskFee(){ return 2 == nPayDeskFeeState ; }
	void doPayingDeskFee(){ if ( isPayedDeskFee() == false ){ nPayDeskFeeState = 1 ; return ;} LOGFMTE("can not do paying more than once"); } ;
	void doPayedDeskFee(){ if (isPayingDeskFee() ){ nPayDeskFeeState = 2 ; return ;} LOGFMTE("not paying desk fee , how to already pay");}
	void payDeskFeeFailed(){ if (isPayingDeskFee() ){ nPayDeskFeeState = 0 ; return ;} LOGFMTE("not paying desk fee  how to pay failed"); }

	uint16_t getBuyEntryCnt(){ return vBuyInRecord.size() ;}
	void toJsvalue(Json::Value& jsValue )
	{
		jsValue["nUserUID"] = nUserUID ;
		jsValue["nToTalCoin"] = nToTalCoin ;
		jsValue["nToTalBuyIn"] = nToTalBuyIn ;
		jsValue["nCoinInRoom"] = nCoinInRoom ;

		Json::Value arrayBuEntry ;
		for ( auto& ref  : vBuyInRecord )
		{
			Json::Value entry ;
			entry["nBuyInCoin"] = ref.nBuyInCoin ;
			entry["tBuyInTime"] = (uint32_t)ref.tBuyInTime ;
			arrayBuEntry[arrayBuEntry.size()] = entry ;
		}

		if ( arrayBuEntry.empty() == false )
		{
			jsValue["vBuyInRecord"] = arrayBuEntry ;
		}
	}

	void fromjsValue( Json::Value& jsValue )
	{
		nUserUID = jsValue["nUserUID"].asUInt();
		nToTalCoin = jsValue["nToTalCoin"].asUInt();
		nToTalBuyIn = jsValue["nToTalBuyIn"].asUInt();
		nCoinInRoom = jsValue["nCoinInRoom"].asUInt();

		if ( jsValue["vBuyInRecord"].isNull() )
		{
			return ;
		}

		Json::Value vJsBuyRecorder  = jsValue["vBuyInRecord"] ;
		for ( uint16_t nIdx = 0 ; nIdx < vJsBuyRecorder.size() ; ++nIdx )
		{
			Json::Value jsEntry = vJsBuyRecorder[nIdx] ;

			stBuyInEntry entry ;
			entry.tBuyInTime = jsEntry["tBuyInTime"].asUInt() ;
			entry.nBuyInCoin = jsEntry["nBuyInCoin"].asUInt() ;
			vBuyInRecord.push_back(entry) ;
		}
	}
	
	bool buyIn(uint32_t nCoin )
	{
		if ( nToTalCoin < (uint32_t)(float(nCoin) * 1.1 ) )
		{
			return false ;
		}

		nToTalCoin -= (uint32_t)(float(nCoin) * 1.1 );
		nToTalBuyIn += nCoin ;

		stBuyInEntry tEntry  ;
		tEntry.nBuyInCoin = nCoin ;
		tEntry.tBuyInTime = time(nullptr) ;
		vBuyInRecord.push_back(tEntry) ;
		isDirty = true ;
		return true ;
	}
};
template<class T >
class CPrivateRoom
	:public IRoomInterface
	,public IRoomDelegate
{
public:
	typedef T REAL_ROOM ;
	typedef T* REAL_ROOM_PTR;
public:
	CPrivateRoom();
	~CPrivateRoom();

	// interface 
	bool onFirstBeCreated(IRoomManager* pRoomMgr,uint32_t nRoomID, const Json::Value& vJsValue )override;
	void serializationFromDB(IRoomManager* pRoomMgr,stBaseRoomConfig* pConfig,uint32_t nRoomID , Json::Value& vJsValue ) override;
	void serializationToDB() override;
	bool onCreateFromDB(IRoomManager* pRoomMgr, uint32_t nRoomID, const Json::Value& vJsValue)override;

	uint8_t canPlayerEnterRoom( stEnterRoomData* pEnterRoomPlayer )override;  // return 0 means ok ;
	void onPlayerEnterRoom(stEnterRoomData* pEnterRoomPlayer,int8_t& nSubIdx ) override;
	bool onPlayerApplyLeaveRoom(uint32_t nUserUID )override;
	void roomItemDetailVisitor(Json::Value& vOutJsValue) override;
	uint32_t getRoomID()override;
	uint8_t getRoomType() override ;
	void update(float fDelta) override;
	void onTimeSave() override;
	bool onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID )override;
	bool onMessage( Json::Value& prealMsg ,uint16_t nMsgType, eMsgPort eSenderPort , uint32_t nSessionID )override;
	bool isDeleteRoom()override;
	void deleteRoom()override ;
	uint32_t getOwnerUID()override;
	void onRoomDoClosed();

	// delegate msg ;
	uint8_t canPlayerEnterRoom( IRoom* pRoom,stEnterRoomData* pEnterRoomPlayer )override;  // return 0 means ok ;
	bool isRoomShouldClose( IRoom* pRoom)override;
	bool isOmitNewPlayerHalo(IRoom* pRoom )override;
	void onRankPlayerChanged( uint32_t nUID , uint16_t nPreIdx , uint16_t nCurIdx )override;
	bool isPlayerLoseReachMax( IRoom* pRoom, uint32_t nUserUID )override;
	bool onPlayerWillDoLeaveRoom(IRoom* pRoom , IRoom::stStandPlayer* pPlayer )override;
	bool onDelayPlayerWillLeaveRoom(IRoom* pRoom , ISitableRoomPlayer* pPlayer )override;
	bool onOneRoundEnd(IRoom* pRoom)override;

	// self 
	uint32_t getRoomState(){ return m_eState ; }
	bool isRoomClosed();
	void sendRoomInfo(uint32_t nSessionID );
	uint32_t getCardNeed()
	{
		auto t = time(nullptr);
		struct tm tt;
		auto tr = localtime_s(&tt,&t);
		if ((tt.tm_mday >= 27 && tt.tm_mon == 0) || (tt.tm_mday <= 3 && tt.tm_mon == 1))
		{
			if (tt.tm_hour >= 8 && tt.tm_hour <= 12)
			{
				LOGFMTD("temp free for room card id = %u",getRoomID());
				return 0;
			}
		}
#ifndef GAME_365
		return (( m_nDuringSeconds / 60 / 15 ) ) ;
#else
		switch ((uint32_t)m_nDuringSeconds)
		{
		case 10:
			return 1;
		case 20:
			return 2;
		case 30 :
			return 3;
		default:
			LOGFMTD("unknown total round cnt = %u , so need 1 card", (uint32_t)m_nDuringSeconds);
			return 1;
		}
#endif
		return 1;
	}
	stPrivateRoomPlayerItem* getPlayerByUID(uint32_t nUserUID )
	{
		auto iter = m_mapPrivateRoomPlayers.find(nUserUID) ;
		if ( iter != m_mapPrivateRoomPlayers.end() )
		{
			return iter->second ;
		}
		return nullptr ;
	}

	int32_t getPlayerOffsetByUID( uint32_t nUserUID )override ;
	bool isPlaying()override ;
	uint32_t getMaxTakeIn(){ return m_nMaxTakeIn;}

	void setLeftTime(uint32_t nLeftSec )override{ m_fLeftTimeSec = (float)nLeftSec ; }
	void setRoomState(uint32_t nRoomState )override{ m_eState = (eRoomState)nRoomState ;}
protected:
	IRoomManager* m_pRoomMgr ;
	uint32_t m_nRoomID ;
	uint32_t m_nOwnerUID ;
	uint32_t m_nClubID ;
	float m_fLeftTimeSec ;
	uint32_t m_nDuringSeconds ;
	eRoomState m_eState ;
	REAL_ROOM_PTR m_pRoom ;
	uint32_t m_nBaseBet ;
	uint32_t m_nBaseTakeIn ;
	std::string m_strRoomName ;
	bool m_isControlTakeIn ;
	uint32_t m_nMaxTakeIn ;
	uint32_t m_nSerialNum ;
	bool m_bRoomInfoDiry ;
	bool m_isRequestingChatID ;
	float m_fWaitOpenTicket ;
	float m_fWaitPlayerJoinTicket;

	std::map<uint32_t,stPrivateRoomPlayerItem*> m_mapPrivateRoomPlayers ;
};




// implement 
#include "log4z.h"
#include "RoomConfig.h"
#include "ServerMessageDefine.h"
#include "IRoomManager.h"
#include "SeverUtility.h"
#include "ServerStringTable.h"
#include "IRoomState.h"
#include "RewardConfig.h"
#include "AsyncRequestQuene.h"

template<class T >
CPrivateRoom<T>::CPrivateRoom()
{
	m_pRoomMgr = nullptr ;
	m_isControlTakeIn = false ;
	m_nRoomID = 0 ;
	m_nOwnerUID = 0;
	m_fLeftTimeSec = 10 ;
	m_nDuringSeconds = 10;
	m_eState = eRoomState_None ;
	m_pRoom = nullptr;
	m_nMaxTakeIn = 999999999 ;
	m_nClubID = 0 ;
	m_nSerialNum = 0 ;
	m_bRoomInfoDiry = false ;
	m_isRequestingChatID = false ;
}

template<class T >
CPrivateRoom<T>::~CPrivateRoom()
{
	if ( m_pRoom )
	{
		m_pRoomMgr->deleteRoomChatID(m_pRoom->getChatRoomID());

		delete m_pRoom ;
		m_pRoom = nullptr ;
	}

	for ( auto ref : m_mapPrivateRoomPlayers )
	{
		delete ref.second ;
		ref.second = nullptr ;
	}
	m_mapPrivateRoomPlayers.clear() ;
}

// interface 
template<class T >
bool CPrivateRoom<T>::onFirstBeCreated(IRoomManager* pRoomMgr,uint32_t nRoomID, const Json::Value& vJsValue )
{
	m_nRoomID = nRoomID ;
	m_nDuringSeconds = 2 * 60;
#ifdef GAME_365
	m_nDuringSeconds = 10;
#endif
	m_eState = eRoomState_WaitOpen ;
	m_pRoomMgr = pRoomMgr ;
	m_strRoomName = vJsValue["name"].asString();
	m_nBaseBet = vJsValue["baseBet"].asUInt() ;
	m_nBaseTakeIn = vJsValue["baseTakeIn"].asUInt() ;
	m_isControlTakeIn = vJsValue["isControlTakeIn"].asBool() ;
	m_nClubID = vJsValue["clubID"].asUInt() ;
	m_nSerialNum = vJsValue["serialNum"].asUInt();
	m_fWaitOpenTicket = 0 ;
	m_fWaitPlayerJoinTicket = 0;
	if ( vJsValue["duringMin"].isNull() == false )
	{
		m_nDuringSeconds = vJsValue["duringMin"].asUInt() * 60 ;
#ifdef GAME_365
		m_nDuringSeconds = vJsValue["duringMin"].asUInt();
#endif
		LOGFMTD("create private room duiring is = %u",m_nDuringSeconds) ;
	}
	else
	{
		LOGFMTE("create private room duringMin is null ?") ;
	}

	if ( vJsValue["createUID"].isNull() == false )
	{
		m_nOwnerUID = vJsValue["createUID"].asUInt() ;
		LOGFMTD("create private room ownerUID is = %u",m_nOwnerUID) ;
	}
	else
	{
		LOGFMTE("create private room ownerUID is null ?") ;
	}

	m_fLeftTimeSec = (float)m_nDuringSeconds ;

	m_pRoom = new REAL_ROOM ;
	m_pRoom->onFirstBeCreated(pRoomMgr,getRoomID() ,vJsValue);
	pRoomMgr->reqeustChatRoomID(m_pRoom);
	m_pRoom->setDelegate(this);
	
	m_nMaxTakeIn = m_pRoom->getMaxTakeIn();
	LOGFMTD("create 1 private room") ;


	return true ;
}

template<class T >
bool CPrivateRoom<T>::onCreateFromDB(IRoomManager* pRoomMgr, uint32_t nRoomID, const Json::Value& vJsValue)
{
	onFirstBeCreated(pRoomMgr, nRoomID, vJsValue);
	// read private room data 
	stMsgReadPrivateRoomPlayer msgReadPrivate;
	msgReadPrivate.nRoomSerialNum = m_nSerialNum;
	msgReadPrivate.nRoomID = getRoomID();
	m_pRoomMgr->sendMsg(&msgReadPrivate, sizeof(msgReadPrivate), getRoomID());
	LOGFMTD("read room private item");

	// read rank data 
	stMsgReadRoomPlayer msgRead;
	msgRead.nRoomSerialNum = m_nSerialNum;
	msgRead.nRoomID = getRoomID();  
	msgRead.nTermNumber = 0;
	m_pRoomMgr->sendMsg(&msgRead, sizeof(msgRead), getRoomID());
	LOGFMTD("read room rank");
	return true;
}

template<class T >
void CPrivateRoom<T>::serializationFromDB(IRoomManager* pRoomMgr,stBaseRoomConfig* pConfig,uint32_t nRoomID , Json::Value& vJsValue )
{
	//m_bRoomInfoDiry = false ;
	//m_nRoomID = nRoomID ;
	//m_pConfig = (stSitableRoomConfig*)pConfig ;
	//m_pRoomMgr = pRoomMgr ;
	//m_eState = (eRoomState)vJsValue["state"].asUInt();
	//m_tCloseTime = (time_t)vJsValue["closeTime"].asUInt();
	//m_nDeadTime = m_tCloseTime + 1;
	//m_nDuringSeconds = vJsValue["duringTime"].asUInt();
	//
	//m_nOwnerUID = vJsValue["ownerUID"].asUInt();
	//m_tCreateTime = (time_t)vJsValue["createTime"].asUInt() ;

	//vJsValue["parentRoomID"] = getRoomID() ;

	//Json::Value subRoom = vJsValue["subRoom"];
	//m_pRoom = new REAL_ROOM ;

	//float fDiveRate = pConfig->fDividFeeRate ;
	//uint32_t nDeskFee = pConfig->nDeskFee ;
	//pConfig->fDividFeeRate = 0 ;
	//pConfig->nDeskFee = 0 ;
	//m_pRoom->serializationFromDB(pRoomMgr,pConfig,0,subRoom);
	//pConfig->fDividFeeRate = fDiveRate ;
	//pConfig->nDeskFee = nDeskFee ;
	//m_pRoom->setDelegate(this);

	//// read rank data 
	//stMsgReadRoomPlayer msgRead ;
	//msgRead.nRoomID = getRoomID();
	//msgRead.nRoomType = getRoomType() ;
	//msgRead.nTermNumber = 0 ;
	//m_pRoomMgr->sendMsg(&msgRead,sizeof(msgRead),getRoomID()) ;

	//// read private room data 
	//stMsgReadPrivateRoomPlayer msgReadPrivate ;
	//msgReadPrivate.nRoomID = getRoomID() ;
	//msgReadPrivate.nRoomType = getRoomType() ;
	//m_pRoomMgr->sendMsg(&msgReadPrivate,sizeof(msgReadPrivate),getRoomID()) ;

	//LOGFMTD("read room id = %u ternm = %u rank player",getRoomID(),0) ;
}

template<class T >
void CPrivateRoom<T>::serializationToDB()
{
	//Json::StyledWriter jsWrite ;
	//Json::Value vValue ;

	//vValue["state"] = m_eState ;
	//vValue["duringTime"] = m_nDuringSeconds ;
	//vValue["closeTime"] = (uint32_t)m_tCloseTime ;
	//vValue["ownerUID"] = getOwnerUID() ;
	////vValue["deadTime"] = (uint32_t)m_nDeadTime ;
	//vValue["createTime"] = (uint32_t)m_tCreateTime ;

	//Json::Value subRoom ;
	//m_pRoom->willSerializtionToDB(subRoom);
	//vValue["subRoom"] = subRoom ;

	//std::string strJson = jsWrite.write(vValue);

	//stMsgSaveUpdateRoomInfo msgSave ;
	//msgSave.nRoomType = getRoomType() ;
	//msgSave.nRoomID = getRoomID() ;
	//msgSave.nJsonLen = strJson.size() ;
	//msgSave.nConfigID = m_pConfig->nConfigID;
	//msgSave.nRoomOwnerUID = getOwnerUID() ;

	//CAutoBuffer autoBuffer(sizeof(msgSave) + msgSave.nJsonLen);
	//autoBuffer.addContent((char*)&msgSave,sizeof(msgSave)) ;
	//autoBuffer.addContent(strJson.c_str(),msgSave.nJsonLen) ;
	//m_pRoomMgr->sendMsg((stMsg*)autoBuffer.getBufferPtr(),autoBuffer.getContentSize(),0) ;
	char pBuffer[215] = {0};
	sprintf_s(pBuffer,sizeof(pBuffer),"update rooms set leftTime = %u , roomState = %u where serialNum = %u ;",(uint32_t)m_fLeftTimeSec,(uint32_t)m_eState,m_nSerialNum);
	Json::Value jsReq ;
	jsReq["sql"] = pBuffer ;
	m_pRoomMgr->getSvrApp()->getAsynReqQueue()->pushAsyncRequest(ID_MSG_PORT_DB,eAsync_DB_Update,jsReq);
}

template<class T >
uint8_t CPrivateRoom<T>::canPlayerEnterRoom( stEnterRoomData* pEnterRoomPlayer )  // return 0 means ok ;
{
	if ( getRoomState() == eRoomState_Close )
	{
		return 7 ;  // room not open 
	}

	if ( m_pRoom )
	{
		return m_pRoom->canPlayerEnterRoom(pEnterRoomPlayer);
	}
	return 7 ;
}

template<class T >
bool CPrivateRoom<T>::isPlaying()
{
	if ( getRoomState() == eRoomState_WaitOpen )
	{
		return false ;
	}

	return m_pRoom->isPlaying();
}

template<class T >
void CPrivateRoom<T>::onPlayerEnterRoom(stEnterRoomData* pEnterRoomPlayer,int8_t& nSubIdx )
{
	nSubIdx = 0 ;
	if ( nullptr == m_pRoom )
	{
		return ;
	}
	
	stPrivateRoomPlayerItem* pPlayerItem = nullptr ;
	auto iter = m_mapPrivateRoomPlayers.find(pEnterRoomPlayer->nUserUID) ;
	if ( iter == m_mapPrivateRoomPlayers.end() )
	{
		pPlayerItem = new stPrivateRoomPlayerItem(pEnterRoomPlayer->nUserUID,pEnterRoomPlayer->nCoin) ;
		m_mapPrivateRoomPlayers[pPlayerItem->nUserUID] = pPlayerItem ;
	}
	else
	{
		pPlayerItem = iter->second ;
	}

	pPlayerItem->nToTalCoin = pEnterRoomPlayer->nCoin ;

	// check offline checked coin 
	if ( pPlayerItem->nCheckedCoin > 0 )
	{
		auto bRet = pPlayerItem->buyIn(pPlayerItem->nCheckedCoin);
		if ( bRet == false )
		{
			LOGFMTE("when you not in the room owener allow you coin = %u , but now you do not have enought coin to take in uid = %u ",pPlayerItem->nCheckedCoin,pPlayerItem->nUserUID);
		}
		else
		{
			pPlayerItem->nCoinInRoom += pPlayerItem->nCheckedCoin ;
			pPlayerItem->nCheckedCoin = 0 ;
			onUpdatePlayerGameResult(m_pRoom,pPlayerItem->nUserUID,0);
		}
	}

	stEnterRoomData refEnterData ;
	memcpy_s(&refEnterData,sizeof(stEnterRoomData),pEnterRoomPlayer,sizeof(stEnterRoomData));
	refEnterData.nCoin = pPlayerItem->nCoinInRoom ;
	m_pRoom->onPlayerEnterRoom(&refEnterData,nSubIdx) ;
	sendRoomInfo(pEnterRoomPlayer->nUserSessionID);
	LOGFMTD("uid = %u , enter room id = %u , subIdx = %u inRoom coin = %u , total coin = %u",pEnterRoomPlayer->nUserUID, getRoomID(),0,pPlayerItem->nCoinInRoom,pPlayerItem->nToTalCoin) ;
}

template<class T >
bool CPrivateRoom<T>::onPlayerApplyLeaveRoom(uint32_t nUserUID )
{
	if ( m_pRoom )
	{
		return m_pRoom->onPlayerApplyLeaveRoom(nUserUID) ;
	}
	return false ;
}

template<class T >
void CPrivateRoom<T>::roomItemDetailVisitor(Json::Value& vOutJsValue)
{
	vOutJsValue["name"] = m_strRoomName ;
	vOutJsValue["creatorUID"] = getOwnerUID() ;
	vOutJsValue["baseBet"] = m_nBaseBet ;
	vOutJsValue["playerCnt"] = m_pRoom->getSitDownPlayerCount() ;
	vOutJsValue["roomID"] = getRoomID();
	vOutJsValue["roomType"] = getRoomType() ;
	vOutJsValue["initTime"] = m_nDuringSeconds / 60 ; ;
	vOutJsValue["playedTime"] = (uint32_t)((m_nDuringSeconds - m_fLeftTimeSec)/60);
#ifdef GAME_365
	vOutJsValue["initTime"] = (uint32_t)m_nDuringSeconds;
	vOutJsValue["playedTime"] = (uint32_t)((m_nDuringSeconds - m_fLeftTimeSec));
#endif
	vOutJsValue["seatCnt"] = (uint16_t)m_pRoom->getSeatCount();
	vOutJsValue["clubID"] = m_nClubID ;
}

template<class T >
uint32_t CPrivateRoom<T>::getRoomID()
{
	return m_nRoomID ;
}

template<class T >
uint8_t CPrivateRoom<T>::getRoomType()
{
	if ( m_pRoom )
	{
		return m_pRoom->getRoomType();
	}
	return eRoom_None ;
}

template<class T >
bool CPrivateRoom<T>::isRoomClosed()
{
	if ( m_pRoom )
	{
		return eRoomState_Close == m_pRoom->getCurRoomState()->getStateID();
	}
	return false ;
}

template<class T >
void CPrivateRoom<T>::onRoomDoClosed()
{
	LOGFMTD("uid = %d change do close, prepare recorders ",getRoomID() );
	if ( m_pRoom )
	{
		stMsgRoomEnterNewState msgNewState ;
		msgNewState.m_fStateDuring = 999999;
		msgNewState.nNewState = eRoomState_Close;
		m_pRoom->sendRoomMsg(&msgNewState,sizeof(msgNewState));

		m_pRoom->forcePlayersLeaveRoom();
		m_pRoomMgr->deleteRoomChatID(m_pRoom->getChatRoomID()) ;
	}

	// sync player coin  ;
	stPrivateRoomRecorder* pRecoder = new stPrivateRoomRecorder ;
	pRecoder->nConfigID = 0 ;
	pRecoder->nSieralNum = m_nSerialNum;
	pRecoder->nCreaterUID = getOwnerUID() ;
	pRecoder->nRoomID = getRoomID() ;
	pRecoder->nTime = time(nullptr) ;
	pRecoder->nDuringSeconds = m_nDuringSeconds ;
				
	// produce room bill info ;
	sortRoomRankItem();
	// reset apns target 
	CSendPushNotification::getInstance()->reset() ;

	stMsgSyncPrivateRoomResult msgResult ;
	msgResult.nRoomID = getRoomID() ;
	msgResult.nCreatorUID = getOwnerUID() ;
	msgResult.nClubID = m_nClubID ; 
	msgResult.nDuringTimeSeconds = m_nDuringSeconds ;
	msgResult.nSiealNum = m_nSerialNum;
	memset(msgResult.cRoomName,0,sizeof(msgResult.cRoomName));
	sprintf_s(msgResult.cRoomName,sizeof(msgResult.cRoomName),"%s",m_strRoomName.c_str());
				
	LIST_ROOM_RANK_ITEM::iterator iter = m_vSortedRankItems.begin();
	for ( uint16_t nIdx = 0 ; iter != m_vSortedRankItems.end(); ++iter, ++nIdx )
	{
		// add target to apns 
		CSendPushNotification::getInstance()->addTarget((*iter)->nUserUID);

		// push rank 5 to send to client ;
		stRoomRankItem* pItem = (*iter) ;
		auto pP = getPlayerByUID(pItem->nUserUID);
		if ( pP == nullptr )
		{
			continue;
		}
		Json::Value vValue ;
		vValue["uid"] = pP->nUserUID ;
		vValue["buyIn"] = pP->nToTalBuyIn ;
		vValue["offset"] = pItem->nGameOffset ;
		pRecoder->playerDetail[nIdx] = vValue ;

		msgResult.nOffset = pItem->nGameOffset ;
		msgResult.nTargetPlayerUID = pItem->nUserUID ;
		msgResult.nFinalCoin = pP->nCoinInRoom ;
		msgResult.nBuyIn = pP->nToTalBuyIn ;
		msgResult.nBaseBet = m_nBaseBet ;
					
		m_pRoomMgr->sendMsg(&msgResult,sizeof(msgResult),0);
	}

	// audients also have zhan ji 
	if ( m_vSortedRankItems.empty() == false )
	{
		for (auto& ref : m_mapPrivateRoomPlayers )
		{
			auto pp = ref.second ;
			if ( pp == nullptr || pp->nToTalBuyIn != 0 )  // not audients ;
			{
				continue;
			}
			// add target to apns 
			CSendPushNotification::getInstance()->addTarget(pp->nUserUID);

			msgResult.nOffset = 0 ;
			msgResult.nTargetPlayerUID = pp->nUserUID ;
			msgResult.nFinalCoin = 0 ;
			msgResult.nBuyIn = 0 ;
			msgResult.nBaseBet = m_nBaseBet ;

			m_pRoomMgr->sendMsg(&msgResult,sizeof(msgResult),0);
		}
	}

	if ( m_vSortedRankItems.empty() || pRecoder->playerDetail.isNull())
	{
		delete pRecoder ; 
		pRecoder = nullptr ;
		LOGFMTE("private room log is null id = %u",getRoomID()) ;
	}
	else 
	{
		m_pRoomMgr->addPrivateRoomRecorder(pRecoder) ;
	}
				
	// send apns tell close 
	CSendPushNotification::getInstance()->setContent(CServerStringTable::getInstance()->getStringByID(10),1);
	CSendPushNotification::getInstance()->postApns( m_pRoomMgr->getSvrApp()->getAsynReqQueue(),false,"room closed") ;
	LOGFMTD("send close end apns uid = %u",getRoomID()) ;

	m_bRoomInfoDiry = true ;
	onTimeSave();
}

template<class T >
void CPrivateRoom<T>::update(float fDelta)
{
	if ( m_eState == eRoomState_Close || eRoomState_WaitOpen == m_eState )
	{
		if ( eRoomState_WaitOpen == m_eState )
		{
			m_fWaitOpenTicket += fDelta ;
			if ( m_fWaitOpenTicket >= 60*60 )
			{
				m_eState = eRoomState_Close ;
				onRoomDoClosed();
				// inform data svr clear room info ;
				Json::Value jsReq ;
				jsReq["roomID"] = getRoomID() ;
				jsReq["ownerUID"] = getOwnerUID() ;
				jsReq["clubID"] = m_nClubID ;
				m_pRoomMgr->getSvrApp()->getAsynReqQueue()->pushAsyncRequest(ID_MSG_PORT_DATA,eAsync_OnRoomDeleted,jsReq);
				return ;
			}
		}
		return ;
	}

	if ( eRoomState_WaitJoin != m_eState)
	{
		m_fWaitPlayerJoinTicket = 0 ;
	}
	else
	{
		m_fWaitPlayerJoinTicket += fDelta;
		if (m_fWaitOpenTicket >= 60 * 60)
		{
			m_eState = eRoomState_Close;
			onRoomDoClosed();
			// inform data svr clear room info ;
			Json::Value jsReq;
			jsReq["roomID"] = getRoomID();
			jsReq["ownerUID"] = getOwnerUID();
			jsReq["clubID"] = m_nClubID;
			m_pRoomMgr->getSvrApp()->getAsynReqQueue()->pushAsyncRequest(ID_MSG_PORT_DATA, eAsync_OnRoomDeleted, jsReq);
			return;
		}
	}

	m_bRoomInfoDiry = true ;
#ifndef GAME_365
	m_fLeftTimeSec -= fDelta ;
#endif
	if ( m_fLeftTimeSec <= 0 )
	{
		m_fLeftTimeSec = 0 ;
		if ( m_eState == eRoomState_Pasue || m_pRoom->getCurRoomState()->getStateID() == eRoomState_WaitJoin )
		{
			m_eState = eRoomState_Close ;
			onRoomDoClosed();
			// inform data svr clear room info ;
			Json::Value jsReq ;
			jsReq["roomID"] = getRoomID() ;
			jsReq["ownerUID"] = getOwnerUID() ;
			jsReq["clubID"] = m_nClubID ;
			m_pRoomMgr->getSvrApp()->getAsynReqQueue()->pushAsyncRequest(ID_MSG_PORT_DATA,eAsync_OnRoomDeleted,jsReq);
			return ;
		}
	}

	if ( m_pRoom && getRoomState() == eRoomState_Opening )
	{
		m_pRoom->update(fDelta);
	}
}

template<class T >
void CPrivateRoom<T>::onTimeSave()
{
	LOGFMTD("time save room info room id = %u",getRoomID());
	// save room rank ;
	stMsgSaveRoomPlayer msgSave ;
	msgSave.nRoomSerialNum = m_nSerialNum;
	msgSave.nTermNumber = 0 ;
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
		//LOGFMTD("update rank uid = %u , offset = %d",pp->nUserUID,pp->nGameOffset) ;
	}

	if ( m_bRoomInfoDiry == true )
	{
		serializationToDB();
		m_bRoomInfoDiry = false ;
	}
	// save private room player 

	CAutoBuffer auBuffer( sizeof( stMsgSavePrivateRoomPlayer ) + 200 );
	for ( auto& refPrivatePlayer : m_mapPrivateRoomPlayers )
	{
		uint32_t nCoinInRoom = 0 ;
		bool isPlayerInRoom = false ;
		bool isDelayLeave = false;
		ISitableRoomPlayer* pSit = (ISitableRoomPlayer*)m_pRoom->getSitdownPlayerByUID(refPrivatePlayer.second->nUserUID);
		if ( pSit )
		{
			nCoinInRoom = pSit->getCoin();
			isPlayerInRoom = true ;
			auto pp = m_pRoom->getPlayerByUserUID(refPrivatePlayer.second->nUserUID);
			isDelayLeave = (nullptr == pp);
			if (isDelayLeave && (pSit->isDelayStandUp() == false))
			{
				LOGFMTE("save you delay leave , but you not delay stand up ? big error uid = %u", refPrivatePlayer.second->nUserUID );
			}
		}
		else
		{
			auto pp = m_pRoom->getPlayerByUserUID(refPrivatePlayer.second->nUserUID);
			if ( pp )
			{
				isPlayerInRoom = true ;
				nCoinInRoom = pp->nCoin ;
			}
		}

		if ( refPrivatePlayer.second->isDirty == false && (!( isPlayerInRoom && nCoinInRoom != refPrivatePlayer.second->nCoinInRoom )) )
		{
			continue;
		}

		auto nInCoinBackUp = refPrivatePlayer.second->nCoinInRoom;
		if ( isPlayerInRoom )
		{
			refPrivatePlayer.second->nCoinInRoom = nCoinInRoom ;
		}
		

		refPrivatePlayer.second->isDirty = false;

		Json::Value jsValue ;
		refPrivatePlayer.second->toJsvalue(jsValue);

		if (isDelayLeave)
		{
			LOGFMTD("uid delay leave backup coin = %u coinInRoom = %u", nInCoinBackUp, refPrivatePlayer.second->nCoinInRoom );
			refPrivatePlayer.second->nCoinInRoom = nInCoinBackUp;
		}

		Json::StyledWriter jsWrite ;
		std::string str = jsWrite.write(jsValue) ;

		stMsgSavePrivateRoomPlayer msgSavePrivatePlayer ;
		msgSavePrivatePlayer.nJsonLen = str.size() ;
		msgSavePrivatePlayer.nRoomSerialNum = m_nSerialNum;
		msgSavePrivatePlayer.nUserUID = refPrivatePlayer.second->nUserUID ;

		auBuffer.clearBuffer() ;
		auBuffer.addContent(&msgSavePrivatePlayer,sizeof(msgSavePrivatePlayer)) ;
		auBuffer.addContent(str.c_str(),msgSavePrivatePlayer.nJsonLen) ;
		m_pRoomMgr->sendMsg((stMsg*)auBuffer.getBufferPtr(),auBuffer.getContentSize(),0) ;
	}
}

template<class T >
bool CPrivateRoom<T>::onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID )
{
	switch ( prealMsg->usMsgType )
	{
	case MSG_PLAYER_SITDOWN:
		{
			stMsgPlayerSitDown* pRet = (stMsgPlayerSitDown*)prealMsg ;
			auto stStandPlayer = m_pRoom->getPlayerBySessionID(nPlayerSessionID);
			if ( !stStandPlayer )
			{
				LOGFMTE("you are not in room ,how to sit down ") ;
				if ( m_pRoom )
				{
					return m_pRoom->onMessage(prealMsg,eSenderPort,nPlayerSessionID) ;
				}
				return false;
			}

			ISitableRoomPlayer* stiDownPlayer = m_pRoom->getSitdownPlayerBySessionID(nPlayerSessionID);
			auto iter = m_mapPrivateRoomPlayers.find(stStandPlayer->nUserUID) ;
			if ( iter == m_mapPrivateRoomPlayers.end() )
			{
				LOGE("not at entry player how to sit dwon ?");
				if ( m_pRoom )
				{
					return m_pRoom->onMessage(prealMsg,eSenderPort,nPlayerSessionID) ;
				}
				return true;
			}
			assert(iter != m_mapPrivateRoomPlayers.end() && "not at entry player how to rebuy ?" );
			stPrivateRoomPlayerItem* pPrivatePlayer = iter->second ;
			if ( pPrivatePlayer->isPayedDeskFee() || stStandPlayer->nCoin < m_pRoom->coinNeededToSitDown() || m_pRoom->getPlayerByIdx(pRet->nIdx) )
			{
				if ( m_pRoom )
				{
					return m_pRoom->onMessage(prealMsg,eSenderPort,nPlayerSessionID) ;
				}
				return true;
			}
			else if ( pPrivatePlayer->isPayingDeskFee() )
			{
				LOGI("already doing pay desk fee , don't do more than once ");
				return true ;
			}
			else // do pay
			{
				pPrivatePlayer->doPayingDeskFee();
				auto pAsync = m_pRoomMgr->getSvrApp()->getAsynReqQueue();

				Json::Value jsUserData ;
				jsUserData["session"] = nPlayerSessionID ;
				jsUserData["idx"] = pRet->nIdx ;
				jsUserData["uid"] = pPrivatePlayer->nUserUID ;
				
				Json::Value jsReqData ;
				jsReqData["targetUID"] = pPrivatePlayer->nUserUID ;
				jsReqData["diamond"] = getCardNeed() ;

				pAsync->pushAsyncRequest(ID_MSG_PORT_DATA,eAsync_ComsumDiamond,jsReqData,[this,pAsync,pPrivatePlayer](uint16_t nReqType ,const Json::Value& retContent,Json::Value& jsUserData){
					uint32_t nDiamond = retContent["diamond"].asUInt() ;
					uint32_t nRet = retContent["ret"].asUInt();

					uint32_t nSessionID = jsUserData["session"].asUInt() ;
					uint32_t nIdx = jsUserData["idx"].asUInt();
					uint32_t nUID = jsUserData["uid"].asUInt();
					
					auto stStandPlayer = m_pRoom->getPlayerBySessionID(nSessionID);
					auto pSitDown = m_pRoom->getPlayerByIdx(nIdx);
					bool bSitSuccess = nRet == 0 && stStandPlayer && pSitDown == nullptr && m_pRoom && getRoomState() != eRoomState_Close ;
					if ( bSitSuccess )
					{
						stMsgPlayerSitDown msg ;
						msg.nIdx = nIdx ;
						msg.nTakeInCoin = 0 ;
						msg.nRoomID = getRoomID() ;
						if ( m_pRoom )
						{
							m_pRoom->onMessage(&msg,ID_MSG_PORT_CLIENT,nSessionID) ;
						}
						pPrivatePlayer->doPayedDeskFee() ;
						return  ;
					}
					else
					{
						pPrivatePlayer->payDeskFeeFailed() ;
						if ( nRet == 0 )
						{
							// give back diamond 
							Json::Value jsReq ;
							jsReq["targetUID"] = nUID ;
							jsReq["diamond"] = nDiamond ;
							pAsync->pushAsyncRequest(ID_MSG_PORT_DATA,eAsync_GiveBackDiamond,jsReq);
						}

						stMsgPlayerSitDownRet msgBack ;
						msgBack.nRet = 5 ;
						m_pRoom->sendMsgToPlayer(&msgBack,sizeof(msgBack),nSessionID);
						LOGFMTE("player sit down failed daimond casue uid = %u",nUID);
					}

				},jsUserData);
				return true ;
			}
		}
		break;
	case MSG_READ_PRIVATE_ROOM_PLAYER:
		{
			stMsgReadPrivateRoomPlayerRet* pRet = (stMsgReadPrivateRoomPlayerRet*)prealMsg ;
			char* pBuffer = (char*)pRet ;
			pBuffer += sizeof(stMsgReadPrivateRoomPlayerRet);
			Json::Reader jsReader ;
			Json::Value jsValue ;
			jsReader.parse(pBuffer,pBuffer + pRet->nJsonLen,jsValue,false) ;
			stPrivateRoomPlayerItem* pItem = new stPrivateRoomPlayerItem(0,0);
			pItem->fromjsValue(jsValue) ;
			m_mapPrivateRoomPlayers[pItem->nUserUID] = pItem ;
		}
		break ;
	case MSG_READ_ROOM_PLAYER:
		{
			stMsgReadRoomPlayerRet* pRet = (stMsgReadRoomPlayerRet*)prealMsg ; 
			if ( pRet->nTermNumber != 0 )
			{
				LOGFMTI("recieved last wrong term player data , skip id  room id = %u",getRoomID()) ;
				break; 
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
				 // current term rank item ;
				m_vRoomRankHistroy[p->nUserUID] = p ;
				m_vSortedRankItems.push_back(p) ;
				++pp;
			}
		}
		break;
	case MSG_REQUEST_ROOM_RANK:
		{
			uint32_t nUserID = 0 ;
			if ( m_pRoom )
			{
				auto pp = m_pRoom->getPlayerBySessionID(nPlayerSessionID) ;
				if ( pp )
				{
					nUserID = pp->nUserUID ;
				}
			}

			std::map<uint32_t,stRoomRankEntry> vWillSend ;
			sortRoomRankItem();
			int16_t nSelfRankIdx = -1 ;
			uint16_t nSendRankCnt = 80 ;
			int16_t nMaxCheckCount = 80 ;
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

				if ( nIdx < nSendRankCnt || pItem->nUserUID == nUserID  )
				{
					if ( getPlayerByUID(pItem->nUserUID) == nullptr )
					{
						continue; 
					}

					stRoomRankEntry entry ;
					entry.nGameOffset = pItem->nGameOffset ;
					entry.nOtherOffset = getPlayerByUID(pItem->nUserUID)->nToTalBuyIn;
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
	case MSG_REQUEST_ROOM_INFO:
		{
			stMsgToRoom* pRet = (stMsgToRoom*)prealMsg ;
			sendRoomInfo(nPlayerSessionID);

			if (m_pRoom)
			{
				m_pRoom->sendResultToPlayerWhenDuringResultState(nPlayerSessionID);
			}
		}
		break;
	default:
		{
			if ( m_pRoom )
			{
				return m_pRoom->onMessage(prealMsg, eSenderPort, nPlayerSessionID);
			}
			else
			{

			}
		}
		return false ;
	}

	return true ;
}

template<class T >
bool CPrivateRoom<T>::onMessage( Json::Value& prealMsg ,uint16_t nMsgType, eMsgPort eSenderPort , uint32_t nSessionID )
{
	switch (nMsgType)
	{
	case MSG_SET_GAME_STATE:
		{
			uint32_t nUID = prealMsg["uid"].asUInt() ;
			uint32_t nst = prealMsg["state"].asUInt() ;
			
			Json::Value jsMsg ;
			if ( nUID != getOwnerUID() )
			{
				jsMsg["ret"] = 4 ; 
				m_pRoomMgr->sendMsg(nSessionID,jsMsg,nMsgType) ;
				break ;
			}

			if ( nst != eRoomState_Opening && eRoomState_Pasue != nst )
			{
				jsMsg["ret"] = 5 ; 
				m_pRoomMgr->sendMsg(nSessionID,jsMsg,nMsgType) ;
				break ;
			}

			if ( getRoomState() == nst )
			{
				jsMsg["ret"] = 3 ; 
				m_pRoomMgr->sendMsg(nSessionID,jsMsg,nMsgType) ;
				break ;
			}

			m_eState = (eRoomState)nst ;

			Json::Value jsresult ;
			jsresult["state"] = nst ;
			m_pRoom->sendRoomMsg(jsresult,MSG_ROOM_GAME_STATE_CHANGED) ;
			LOGFMTD("room id = %u , set state to %u",getRoomID(),getRoomState()) ;
			m_bRoomInfoDiry = true ;

			// temp code 
			serializationToDB() ;
			// temp code 
		}
		break;
	case MSG_APPLY_TAKE_IN:
		{
			uint32_t nTakeIn = prealMsg["takeIn"].asUInt() ;
#ifdef GAME_365
			if (!m_isControlTakeIn && ( nTakeIn == 1000u || nTakeIn == 5000u ) )
			{
				if (m_pRoom->getRoomType() != eRoom_NiuNiu)
				{
					nTakeIn = 1500;
					LOGFMTD("temp let player default coin = 5000 , room id = %u, session id = %u", getRoomID(), nSessionID);
				}
			}
#endif 
			// check coin state ;
			auto stStandPlayer = m_pRoom->getPlayerBySessionID(nSessionID);
			if ( !stStandPlayer )
			{
				LOGFMTE("you are not in room ,can not take in ") ;
				break;
			}
			assert( stStandPlayer && "not enter room how to rebuy ?" );
			ISitableRoomPlayer* stiDownPlayer = m_pRoom->getSitdownPlayerBySessionID(nSessionID);

			auto iter = m_mapPrivateRoomPlayers.find(stStandPlayer->nUserUID) ;

			assert(iter != m_mapPrivateRoomPlayers.end() && "not at entry player how to rebuy ?" );
			stPrivateRoomPlayerItem* pPrivatePlayer = iter->second ;

			uint32_t nCurInRoomCoin = stStandPlayer->nCoin ;
			if ( stiDownPlayer )
			{
				assert("when sit down must take in all " && nCurInRoomCoin == 0 );
				nCurInRoomCoin += stiDownPlayer->getCoin();
			}

			Json::Value jsMsgBack ;
			jsMsgBack["ret"] = 0 ;
			jsMsgBack["isApply"] = 0 ;
			jsMsgBack["inRoomCoin"] = nCurInRoomCoin ;

			if ( pPrivatePlayer->isApplyIng )
			{
				jsMsgBack["ret"] = 4 ;
				m_pRoom->sendMsgToPlayer(nSessionID,jsMsgBack,nMsgType) ;
				//LOGFMTE("temp let double applying take in");
				break ; //(uint32_t)(float(nCoin) * 1.1 )
			}

#ifdef GAME_365
			if (pPrivatePlayer->nToTalCoin < 20000)
			{
				pPrivatePlayer->nToTalCoin = 20000;
			}
#endif 
			
			if ( (uint32_t)(float(nTakeIn) * 1.1 ) > pPrivatePlayer->nToTalCoin )
			{
				jsMsgBack["ret"] = 3 ;
				m_pRoom->sendMsgToPlayer(nSessionID,jsMsgBack,nMsgType) ;
				break ;
			}

			if ( nCurInRoomCoin + nTakeIn > getMaxTakeIn() || ( nTakeIn % m_nBaseTakeIn != 0 ) )
			{
				jsMsgBack["ret"] = 2 ;
				m_pRoom->sendMsgToPlayer(nSessionID,jsMsgBack,nMsgType) ;
				break ;
			}

			// check is control ;
			if ( !m_isControlTakeIn || nTakeIn <= pPrivatePlayer->nCheckedCoin ) // do take in 
			{
				if ( !m_isControlTakeIn )
				{
					pPrivatePlayer->nCheckedCoin = nTakeIn ;
				}
				 
				if ( nTakeIn <= pPrivatePlayer->nCheckedCoin )
				{
					LOGFMTD("uid = %u , do takeIn = %u , checkCoin = %u session id = %u", pPrivatePlayer->nUserUID, nTakeIn, pPrivatePlayer->nCheckedCoin, nSessionID);
					nTakeIn = pPrivatePlayer->nCheckedCoin;
					pPrivatePlayer->nCheckedCoin -= nTakeIn ;
				}
				else
				{
					jsMsgBack["ret"] = 3 ;
					m_pRoom->sendMsgToPlayer(nSessionID,jsMsgBack,nMsgType) ;
					break ;
				}
				
				if ( pPrivatePlayer->buyIn(nTakeIn) == false )
				{
					jsMsgBack["ret"] = 3 ;
					m_pRoom->sendMsgToPlayer(nSessionID,jsMsgBack,nMsgType) ;
					pPrivatePlayer->nCheckedCoin += nTakeIn ;
					break ;
				}

				if ( stiDownPlayer )
				{
					stiDownPlayer->setCoin(stiDownPlayer->getCoin() + nTakeIn ) ;

					stMsgPrivateRoomReBuyIn msgRoom ;
					msgRoom.nBuyInCoin = nTakeIn ;
					msgRoom.nFinalCoin = stiDownPlayer->getCoin() ;
					msgRoom.nIdx = stiDownPlayer->getIdx() ;
					m_pRoom->sendRoomMsg(&msgRoom,sizeof(msgRoom)) ;
					LOGFMTD("uid = %u ,sit down coin = %u , stand coin = %u , totalBuyin = %u , newBuyin = %u ",stStandPlayer->nUserUID,stiDownPlayer->getCoin(),stStandPlayer->nCoin,pPrivatePlayer->nToTalBuyIn,nTakeIn) ;
					jsMsgBack["inRoomCoin"] = msgRoom.nFinalCoin ;
				}
				else
				{
					stStandPlayer->nCoin += nTakeIn ;
		
					jsMsgBack["inRoomCoin"] = stStandPlayer->nCoin ;
				}

				m_pRoom->sendMsgToPlayer(nSessionID,jsMsgBack,nMsgType) ;

				// add tag for give back coin , when closed 
				onUpdatePlayerGameResult(m_pRoom,pPrivatePlayer->nUserUID,0);
				break ;
			}

			// send notice ; do apply ;
			pPrivatePlayer->isApplyIng = true ;

			Json::Value jsReqContent ;
			jsReqContent["dlgType"] = eNoticeType::eNotice_ApplyTakeIn ;
			jsReqContent["targetUID"] = getOwnerUID() ;
			Json::Value jsDlgArg ;
			jsDlgArg["roomID"] = getRoomID() ;
			jsDlgArg["applyUID"] = pPrivatePlayer->nUserUID ;
			jsDlgArg["takeIn"] = nTakeIn ;
			jsDlgArg["roomName"] = m_strRoomName ;
			jsReqContent["arg"] = jsDlgArg ;
			m_pRoomMgr->getSvrApp()->getAsynReqQueue()->pushAsyncRequest(ID_MSG_PORT_DATA,eAsync_PostDlgNotice,jsReqContent) ;

			LOGFMTD("uid = %u apply to take in coin = %u , do send notice dlg",pPrivatePlayer->nUserUID,nTakeIn) ;
			jsMsgBack["isApply"] = 1 ;
			m_pRoom->sendMsgToPlayer(nSessionID,jsMsgBack,nMsgType) ;

			auto pp = m_pRoom->getPlayerByUserUID(getOwnerUID());
			if ( !pp )
			{
				// apns tell creator have player enter room 
				CSendPushNotification::getInstance()->reset() ;
				CSendPushNotification::getInstance()->addTarget(getOwnerUID()) ;
				CSendPushNotification::getInstance()->setContent(CServerStringTable::getInstance()->getStringByID(11),1);
				CSendPushNotification::getInstance()->postApns( m_pRoomMgr->getSvrApp()->getAsynReqQueue(),false,"applyTakeIn") ;
			}
			else
			{
				LOGFMTD("room owner in room , need not send apns") ;
			}
		}
		break ;
	case MSG_REPLY_APPLY_TAKE_IN:
		{
			uint32_t nReplyUID = prealMsg["replyToUID"].asUInt() ;
			bool isAgree = prealMsg["isAgree"].asBool() ;
			uint32_t nApplyCoin = prealMsg["coin"].asUInt() ;

			auto iter = m_mapPrivateRoomPlayers.find(nReplyUID) ;
			if ( iter == m_mapPrivateRoomPlayers.end() )
			{
				LOGFMTE("reply take in  uid = %u , not exist",nReplyUID) ;
				prealMsg["ret"] = 1;
				m_pRoomMgr->sendMsg(nSessionID, prealMsg, nMsgType);
				break ;
			}

			stPrivateRoomPlayerItem* pPrivatePlayer = iter->second ;
			if ( pPrivatePlayer->isApplyIng == false )
			{
				LOGFMTE("uid = %u not applying , why reply to it",nReplyUID) ;
				prealMsg["ret"] = 2;
				m_pRoomMgr->sendMsg(nSessionID, prealMsg, nMsgType);
				break ;
			}
			pPrivatePlayer->isApplyIng = false ;
			if ( isAgree )
			{

				ISitableRoomPlayer* stiDownPlayer = m_pRoom->getSitdownPlayerByUID(pPrivatePlayer->nUserUID);
				auto pStandPlayer = m_pRoom->getPlayerByUserUID(pPrivatePlayer->nUserUID);
				if ( stiDownPlayer )
				{
					Json::Value jsMsgBack ;
					jsMsgBack["ret"] = 0 ;
					if ( pPrivatePlayer->buyIn(nApplyCoin) == false )
					{
						pPrivatePlayer->nCheckedCoin = nApplyCoin ;
						jsMsgBack["ret"] = 3 ;
						LOGFMTE("UID = %u need buy coin = %u may be coin not enough",pPrivatePlayer->nUserUID,nApplyCoin);
						m_pRoom->sendMsgToPlayer(stiDownPlayer->getSessionID(),jsMsgBack,MSG_APPLY_TAKE_IN) ;

						prealMsg["ret"] = 3;
						m_pRoomMgr->sendMsg(nSessionID, prealMsg, nMsgType);
						break ;
					}

					stiDownPlayer->setCoin(stiDownPlayer->getCoin() + nApplyCoin ) ;

					stMsgPrivateRoomReBuyIn msgRoom ;
					msgRoom.nBuyInCoin = nApplyCoin ;
					msgRoom.nFinalCoin = stiDownPlayer->getCoin() ;
					msgRoom.nIdx = stiDownPlayer->getIdx() ;
					m_pRoom->sendRoomMsg(&msgRoom,sizeof(msgRoom)) ;
					LOGFMTD("reply uid = %u ,sit down coin = %u , stand coin = %u , totalBuyin = %u , newBuyin = %u ",pPrivatePlayer->nUserUID,stiDownPlayer->getCoin(),0,pPrivatePlayer->nToTalBuyIn,nApplyCoin) ;
					

					jsMsgBack["isApply"] = 0 ;
			
					jsMsgBack["inRoomCoin"] = msgRoom.nFinalCoin ;
					//m_pRoom->sendMsgToPlayer(stiDownPlayer->getSessionID(),jsMsgBack,MSG_APPLY_TAKE_IN) ;
					LOGFMTD("room id = %u reply uid = %u , result = %u , coin = %u in room direct give",getRoomID(),nReplyUID,isAgree,nApplyCoin) ;
					pPrivatePlayer->nCheckedCoin = 0 ;
					//break;
				}
				else if ( pStandPlayer )
				{
					Json::Value jsMsgBack ;
					jsMsgBack["ret"] = 0 ;
					if ( pPrivatePlayer->buyIn(nApplyCoin) == false )
					{
						pPrivatePlayer->nCheckedCoin = nApplyCoin ;
						jsMsgBack["ret"] = 3 ;
						LOGFMTE("UID = %u need buy coin = %u",pPrivatePlayer->nUserUID,nApplyCoin);
						m_pRoom->sendMsgToPlayer(pStandPlayer->nUserSessionID,jsMsgBack,MSG_APPLY_TAKE_IN) ;

						prealMsg["ret"] = 3;
						m_pRoomMgr->sendMsg(nSessionID, prealMsg, nMsgType);
						break ;
					}
					pPrivatePlayer->nCheckedCoin = 0 ;

					pStandPlayer->nCoin += nApplyCoin ;

					jsMsgBack["isApply"] = 0 ;
					jsMsgBack["inRoomCoin"] = pStandPlayer->nCoin ;
					m_pRoom->sendMsgToPlayer(pStandPlayer->nUserSessionID,jsMsgBack,MSG_APPLY_TAKE_IN) ;
					// add tag for give back coin , when closed 
					onUpdatePlayerGameResult(m_pRoom,pPrivatePlayer->nUserUID,0);
				}
				else
				{
					pPrivatePlayer->nCheckedCoin = nApplyCoin ;
				}
			}
			
			// send dlg 
			Json::Value jsReqContent ;
			jsReqContent["dlgType"] = eNoticeType::eNotice_ReplyTakeIn ;
			jsReqContent["targetUID"] = nReplyUID ;
			Json::Value jsDlgArg ;
			jsDlgArg["roomID"] = getRoomID() ;
			jsDlgArg["coin"] = nApplyCoin ;
			jsDlgArg["isAgree"] = isAgree ;
			jsDlgArg["roomName"] = m_strRoomName ;
			jsReqContent["arg"] = jsDlgArg ;
			m_pRoomMgr->getSvrApp()->getAsynReqQueue()->pushAsyncRequest(ID_MSG_PORT_DATA,eAsync_PostDlgNotice,jsReqContent) ;
			LOGFMTD("room id = %u reply uid = %u , result = %u , coin = %u",getRoomID(),nReplyUID,isAgree,nApplyCoin) ;

			prealMsg["ret"] = 0;
			m_pRoomMgr->sendMsg(nSessionID, prealMsg, nMsgType);
		}
		break ;
	default:
		if ( m_pRoom )
		{
			return m_pRoom->onMessage(prealMsg,nMsgType,eSenderPort,nSessionID) ;
		}
		else
		{

		}
		return false;
	}
	return true ;
}

template<class T >
bool CPrivateRoom<T>::isDeleteRoom()
{
#ifdef _DEBUG
	return m_eState == eRoomState_Close; // when room is request chat room id , can not delete the object , it is call back arg ;
#else
	return m_eState == eRoomState_Close && ( m_pRoom->getChatRoomID() > 0 ); // when room is request chat room id , can not delete the object , it is call back arg ;
#endif
	
}

template<class T >
void CPrivateRoom<T>::deleteRoom()
{
	if (eRoomState_WaitOpen == getRoomState())
	{
		// give back room card here ;
		LIST_ROOM_RANK_ITEM::iterator iter = m_vSortedRankItems.begin();
		for (uint16_t nIdx = 0; iter != m_vSortedRankItems.end(); ++iter, ++nIdx)
		{
			// push rank 5 to send to client ;
			stRoomRankItem* pItem = (*iter);
			auto pP = getPlayerByUID(pItem->nUserUID);
			if (pP == nullptr)
			{
				continue;
			}

			auto nCnt = getCardNeed();

			Json::Value jsReq;
			jsReq["targetUID"] = pItem->nUserUID;
			jsReq["diamond"] = nCnt;
			m_pRoomMgr->getSvrApp()->getAsynReqQueue()->pushAsyncRequest(ID_MSG_PORT_DATA, eAsync_GiveBackDiamond, jsReq);
			LOGFMTI("delete room give back room card = %u , uid =%u", nCnt, pItem->nUserUID);
		}
	}

	m_eState = eRoomState_Close;
	onRoomDoClosed();
}

template<class T >
uint32_t CPrivateRoom<T>::getOwnerUID()
{
	return m_nOwnerUID ;
}

// delegate msg ;
template<class T >
uint8_t CPrivateRoom<T>::canPlayerEnterRoom( IRoom* pRoom,stEnterRoomData* pEnterRoomPlayer )  // return 0 means ok ;
{
	return 0 ;
}

template<class T >
bool CPrivateRoom<T>::isRoomShouldClose( IRoom* pRoom)
{
	return eRoomState_Close == m_eState ;
}

template<class T >
bool CPrivateRoom<T>::isOmitNewPlayerHalo(IRoom* pRoom )
{
	return true ;
}

template<class T >
void CPrivateRoom<T>::onRankPlayerChanged( uint32_t nUID , uint16_t nPreIdx , uint16_t nCurIdx )
{
	return ;
}

template<class T >
bool CPrivateRoom<T>::isPlayerLoseReachMax( IRoom* pRoom, uint32_t nUserUID )
{
	return false ;
}

template<class T >
bool CPrivateRoom<T>::onPlayerWillDoLeaveRoom(IRoom* pRoom , IRoom::stStandPlayer* pPlayer )
{
	auto pRoomPlayer = getPlayerByUID(pPlayer->nUserUID) ;
	if ( pRoomPlayer->nCoinInRoom != pPlayer->nCoin )
	{
		pRoomPlayer->nCoinInRoom = pPlayer->nCoin ;
		pRoomPlayer->isDirty = true ;
	}

	pPlayer->nCoin = pRoomPlayer->nToTalCoin ;
	IRoomDelegate::onPlayerWillDoLeaveRoom(pRoom,pPlayer) ;
	pRoomPlayer->nToTalCoin = 0 ;
	LOGFMTD("uid = %u leave private room sys coin = %u , coin in this room = %u",pPlayer->nUserUID,pPlayer->nCoin, pRoomPlayer->nCoinInRoom ) ;
	return true ;
}

template<class T >
bool CPrivateRoom<T>::onDelayPlayerWillLeaveRoom(IRoom* pRoom , ISitableRoomPlayer* pPlayer )
{
	auto pRoomPlayer = getPlayerByUID(pPlayer->getUserUID()) ;
	pRoomPlayer->nCoinInRoom += pPlayer->getCoin() ;
	pRoomPlayer->isDirty = true ;

	pPlayer->setCoin( 0 );
	IRoomDelegate::onDelayPlayerWillLeaveRoom(pRoom,pPlayer) ;

	LOGFMTD("uid = %u leave DELAY private room sys coin = %u , coin in this room = %u",pPlayer->getUserUID(),pPlayer->getCoin(), pRoomPlayer->nCoinInRoom ) ;
	return true ;
}

template<class T >
bool CPrivateRoom<T>::onOneRoundEnd(IRoom* pRoom)
{
	m_fLeftTimeSec -= 1;
	return true;
}

template<class T >
void CPrivateRoom<T>::sendRoomInfo(uint32_t nSessionID )
{
	typename REAL_ROOM_PTR pRoom = m_pRoom ;
	if ( pRoom == nullptr )
	{
		LOGFMTE("session id = %u request room info room id = %u, subIdx = %d is null",nSessionID , getRoomID(),0) ;
		return ;
	}

	Json::Value jsMsgRoomInfo ;
	jsMsgRoomInfo["ownerUID"] = getOwnerUID();
	jsMsgRoomInfo["roomID"] = getRoomID();
	jsMsgRoomInfo["seatCnt"] = (uint8_t)pRoom->getSeatCount();
	jsMsgRoomInfo["chatID"] = pRoom->getChatRoomID();
	jsMsgRoomInfo["curState"] = getRoomState();
	jsMsgRoomInfo["cardNeed"] =( (uint32_t)m_nDuringSeconds) / 10; ///getCardNeed(); 
	LOGFMTD("card need = %u",getCardNeed());
	if ( getRoomState() == eRoomState_Opening )
	{
		jsMsgRoomInfo["curState"] = pRoom->getCurRoomState()->getStateID() ;
	}

	jsMsgRoomInfo["leftTimeSec"]  = (uint32_t)m_fLeftTimeSec ;

	auto iPlayer = pRoom->getPlayerBySessionID(nSessionID);
	if ( iPlayer == nullptr )
	{
		LOGFMTE("you are not in this room can not req room info session id = %u", nSessionID );
		return;
	}
	auto iter = m_mapPrivateRoomPlayers.find(iPlayer->nUserUID) ;
	assert(iter != m_mapPrivateRoomPlayers.end() && "why this is null ?" );
	jsMsgRoomInfo["selfCoin"] = iter->second->nCoinInRoom ;
	jsMsgRoomInfo["baseTakeIn"] = m_nBaseTakeIn;
	jsMsgRoomInfo["isCtrlTakeIn"] = (uint32_t)this->m_isControlTakeIn ;
	jsMsgRoomInfo["sieralNum"] = m_nSerialNum;

	Json::Value jsGame ;
	pRoom->roomInfoVisitor(jsGame);
	jsMsgRoomInfo["game"] = jsGame ;
	pRoom->sendMsgToPlayer(nSessionID,jsMsgRoomInfo,MSG_ROOM_INFO) ;
	pRoom->sendRoomPlayersInfo(nSessionID);
}

template<class T >
int32_t CPrivateRoom<T>::getPlayerOffsetByUID( uint32_t nUserUID )
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