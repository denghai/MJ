#include "GameRoomCenter.h"
#include "log4z.h"
#include "ServerCommon.h"
#include "AsyncRequestQuene.h"
#include "ISeverApp.h"
#include "GameServerApp.h"
#include "QingJiaModule.h"
#include "SeverUtility.h"
#include "ServerStringTable.h"
#include "Player.h"
CGameRoomCenter::~CGameRoomCenter()
{
	MAP_ROOM_ITEM m_vRoomIDKey ;
	for ( auto& refIter : m_vRoomIDKey )
	{
		delete refIter.second ;
		refIter.second = nullptr ;
	}
	m_vRoomIDKey.clear() ;
	m_vClubsOwner.clear();
	m_vPlayerOwners.clear() ;
	m_nCurSerailNum = 0 ;
	m_vWillUseRoomIDs.clear();
	m_isFinishReadingChatRoomID = false ;
}

void CGameRoomCenter::init( IServerApp* svrApp )
{
	IGlobalModule::init(svrApp) ;
	m_isFinishedReading = false ;
	m_isFinishReadingChatRoomID = false ;
}

void CGameRoomCenter::reqChatRoomIDs()
{
	m_isFinishReadingChatRoomID = true;
	return;
	// get chat room ids ;
	auto pMode = CGameServerApp::SharedGameServerApp()->getQinjiaModule() ;
	Json::Value js ;
	js["last_room_id"] = 0 ;
	if ( m_vReserveChatRoomIDs.empty() == false )
	{
		auto p = m_vReserveChatRoomIDs.back();
		js["last_room_id"] = p ;
	}
	js["count"] = 20 ;
	pMode->sendQinJiaRequest("GetRooms",js,[this](Json::Value& jsResult , Json::Value& jsRet )
	{
		if ( jsResult.isNull() )
		{
			LOGFMTE("req chat room ids is null result") ;
			m_isFinishReadingChatRoomID = true ;
			return ;
		}

		auto jsRooms = jsResult["rooms"];
		if ( jsRooms.isNull() || jsRooms.size() == 0 )
		{
			LOGFMTE("req chat room ids , array is 0 or null") ;
			m_isFinishReadingChatRoomID = true ;
			return ;
		}

		for ( uint8_t nIdx = 0 ; nIdx < jsRooms.size() ; ++nIdx )
		{
			auto jsR = jsRooms[nIdx];
			m_vReserveChatRoomIDs.push(jsR["room_id"].asUInt());
		}

		if ( jsRooms.size() >= 20 )
		{
			LOGFMTD("go on req chat room ids ") ;
			reqChatRoomIDs();
		}
		else
		{
			m_isFinishReadingChatRoomID = true ;
			checkChatRoomIDReserve() ;
			updateRoomItemChatRoomID();
		}
	},js);
}

void CGameRoomCenter::checkChatRoomIDReserve()
{
	return;
	if ( m_vReserveChatRoomIDs.size() > 5 )
	{
		LOGFMTD("reserver chat room id cnt > 5 , ok") ;
		return ;
	}

	LOGFMTD("will create chat roomID cur = %u",m_vReserveChatRoomIDs.size()) ;
	auto pMode = CGameServerApp::SharedGameServerApp()->getQinjiaModule() ;
	Json::Value jsCreateChatRoom ;
	jsCreateChatRoom["room_name"] = std::to_string(rand() % 100000 ) ;
	jsCreateChatRoom["room_type"] = 1 ;
	jsCreateChatRoom["room_create_type"] = 0 ;
	pMode->sendQinJiaRequest("CreateRoom",jsCreateChatRoom,[this](Json::Value& jsResult , Json::Value& jsUs){ if (jsResult["room_id"].isNull() == false ){ m_vReserveChatRoomIDs.push(jsResult["room_id"].asUInt()) ; LOGFMTD("create chat room ok size = %u",m_vReserveChatRoomIDs.size()) ; checkChatRoomIDReserve(); } },jsCreateChatRoom);
}

void CGameRoomCenter::onConnectedSvr()
{
	reqChatRoomIDs();
	// read max serail number 
	Json::Value jssql ;
	jssql["sql"] = "select max(serialNum) as 'maxSerial' from gameroomcenter ;"  ;
	getSvrApp()->getAsynReqQueue()->pushAsyncRequest(ID_MSG_PORT_DB,eAsync_DB_Select,jssql,[this](uint16_t nReqType ,const Json::Value& retContent,Json::Value& jsUserData){
		uint8_t nRow = retContent["afctRow"].asUInt() ;
		Json::Value jsData = retContent["data"];
		if ( nRow == 0 )
		{
			LOGFMTD("do not read max serial number") ;
		}
		else
		{
			Json::Value jsRow = jsData[(uint32_t)0];
			m_nCurSerailNum = jsRow["maxSerial"].asUInt();
			LOGFMTD("read max serial number = %u",m_nCurSerailNum);
		}
	});

	// read room item info from db svr ;
	readRoomItemsInfo();
}

void CGameRoomCenter::readRoomItemsInfo()
{
	m_isFinishedReading = true;
	return; // temp skip room items ;
	Json::Value jssql ;
	uint32_t nOffset = m_vRoomIDKey.size() ;
	char pBuffer[512] = {0};
	sprintf(pBuffer,"select serialNum,roomID,belongClubID,creatorUID from gameroomcenter where isDelete = 0 order by createDate limit 20 offset %u",nOffset);
	jssql["sql"] = pBuffer ;
	getSvrApp()->getAsynReqQueue()->pushAsyncRequest(ID_MSG_PORT_DB,eAsync_DB_Select,jssql,[this](uint16_t nReqType ,const Json::Value& retContent,Json::Value& jsUserData){
		uint8_t nRow = retContent["afctRow"].asUInt() ;
		Json::Value jsData = retContent["data"];
		for ( uint8_t nIdx = 0 ; nIdx < jsData.size() ; ++nIdx )
		{
			Json::Value jsRow = jsData[nIdx];
			auto* prt = new stRoomItem ;
			prt->nSerialNumber = jsRow["serialNum"].asUInt() ;
			prt->nBelongsToClubUID = jsRow["belongClubID"].asUInt() ;
			prt->nCreator = jsRow["creatorUID"].asUInt() ;
			prt->nRoomID = jsRow["roomID"].asUInt() ;
			addRoomItem(prt,false) ;
		}

		if ( nRow >= 20 ) // go on read more 
		{
			LOGFMTD("go on reader more clubs") ;
			readRoomItemsInfo();
		}
		else
		{
			m_isFinishedReading = true ;
			updateRoomItemChatRoomID();
		}
	});
}

void CGameRoomCenter::updateRoomItemChatRoomID()
{
	if ( m_isFinishedReading == false ||  false == m_isFinishReadingChatRoomID )
	{
		return ;
	}

	if ( m_vReserveChatRoomIDs.size() < m_vRoomIDKey.size() )
	{
		LOGFMTE("why chat id cnt is few than room items ") ;
	}

	for (auto& ref : m_vRoomIDKey )
	{
		if ( m_vReserveChatRoomIDs.empty() )
		{
			break; 
		}
		auto nCharRoomID = m_vReserveChatRoomIDs.front() ;
		ref.second->nChatRoomID = nCharRoomID ;
		m_vReserveChatRoomIDs.pop() ;
	}

	checkChatRoomIDReserve();
	LOGFMTD("assigne all room item chat room id ");
}

bool CGameRoomCenter::onMsg(stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID)
{
	switch (prealMsg->usMsgType )
	{
	case MSG_REQUEST_MY_OWN_ROOMS:
		{
			stMsgRequestMyOwnRooms* pRet = (stMsgRequestMyOwnRooms*)prealMsg ;
			if ( pRet->nRoomType >= eRoom_Max )
			{
				return false;
			}

			auto pPlayer = CGameServerApp::SharedGameServerApp()->GetPlayerMgr()->GetPlayerBySessionID(nSessionID) ;
			if ( nullptr == pPlayer )
			{
				LOGFMTE("session id = %u not login in how to request self create room ",nSessionID);
				break; 
			}

			stMsgRequestMyOwnRoomsRet msgRet ;
			msgRet.nRoomType = pRet->nRoomType ;

			std::vector<uint32_t> vOutRooms ;
			msgRet.nCnt = getPlayerOwenRooms(vOutRooms, pPlayer->GetUserUID(), pRet->nRoomType);
			if ( msgRet.nCnt == 0 )
			{
				getSvrApp()->sendMsg(nSessionID,(char*)&msgRet,sizeof(msgRet));
				return true ;
			}

			CAutoBuffer autoBuffer(sizeof(msgRet) + sizeof(uint32_t)* msgRet.nCnt);
			autoBuffer.addContent((char*)&msgRet,sizeof(msgRet)) ;
			for ( auto& ref : vOutRooms )
			{
				autoBuffer.addContent(&ref,sizeof(uint32_t));
			}
			getSvrApp()->sendMsg(nSessionID,autoBuffer.getBufferPtr(),autoBuffer.getContentSize());
			LOGFMTD("send uid = %u create room to it , cnt = %u",pPlayer->GetUserUID(),msgRet.nCnt);
		}
		break;
	default: 
		return false ;
	}
	return true  ;
}

bool CGameRoomCenter::onMsg(Json::Value& prealMsg ,uint16_t nMsgType, eMsgPort eSenderPort , uint32_t nSessionID)
{
	if (MSG_VIP_ROOM_CLOSED == nMsgType)
	{
		deleteRoomItem(prealMsg["roomID"].asUInt());
		return true;
	}
	return false ;
}

bool CGameRoomCenter::onAsyncRequest(uint16_t nRequestType , const Json::Value& jsReqContent, Json::Value& jsResult )
{
	switch ( nRequestType )
	{
	case eAsync_OnRoomDeleted:
		{
			uint32_t nRoomID = jsReqContent["roomID"].asUInt() ;
			//uint32_t nOwnerUID = jsReqContent["ownerUID"].asUInt() ;
			//uint32_t nClubID = jsReqContent["clubID"].asUInt() ;
			LOGFMTD("room id = %u do deleted",nRoomID) ;
			deleteRoomItem(nRoomID);
		}
		break ;
	case eAsync_ReqRoomSerials:
		{
			if ( m_isFinishedReading == false || m_isFinishReadingChatRoomID == false )
			{
				jsResult["ret"] = 1 ;
				LOGFMTD("still reading from db , please wait game center") ;
				break ;
			}

			jsResult["ret"] = 0 ;

			Json::Value jsRoomIDs ;
			uint32_t nRoomType = jsReqContent["roomType"].asUInt() ;
			auto iter = m_vRoomIDKey.begin();
			while ( m_vRoomIDKey.end() != (iter = std::find_if(iter,m_vRoomIDKey.end(),[nRoomType](MAP_ROOM_ITEM::value_type& refValue){ return getRoomType(refValue.first) == nRoomType; }) ) )
			{
				Json::Value js;
				js["chatRoomID"] = iter->second->nChatRoomID;
				js["serial"] = iter->second->nSerialNumber;
				jsRoomIDs[jsRoomIDs.size()] = js ;
				++iter ;
			}
			jsResult["serials"] = jsRoomIDs ;
			LOGFMTD("req back roomType = %u , roomCnt = %u",nRoomType,jsRoomIDs.size()) ;
		}
		break ;
	case eAsync_CreateGoldRoom:
		{
			eRoomType roomType = (eRoomType)jsReqContent["roomType"].asUInt();
			// async create room ;
			uint32_t nSerailNum = 0;
			uint32_t nNewRoomID = generateRoomID(roomType, nSerailNum);
			jsResult["roomID"] = nNewRoomID;
			jsResult["createUID"] = jsReqContent["createUID"].asUInt();
			jsResult["initCoin"] = jsReqContent["initCoin"].asUInt();
			jsResult["serialNum"] = nSerailNum;
			auto nChatRoomID = getReuseChatRoomID();
			jsResult["chatRoomID"] = nChatRoomID;
		}
		break;
	default:
		return false ;
	}
	return true ;
}

void CGameRoomCenter::addRoomItem(stRoomItem* pItem , bool isNewAdd )
{
	auto iter = m_vRoomIDKey.find(pItem->nRoomID) ;
	if ( iter != m_vRoomIDKey.end() )
	{
		LOGFMTD("already have this room id object id = %u, create by uid = %u ",pItem->nRoomID,iter->second->nCreator ) ;
		delete pItem ;
		pItem = nullptr ;
		return ;
	}
	m_vRoomIDKey[pItem->nRoomID] = pItem ;
	
	// add to player owners 
	addRoomItemToOwner(m_vPlayerOwners,pItem->nCreator,pItem->nRoomID);

	// add to club owners 
	if ( pItem->nBelongsToClubUID != 0 )
	{
		addRoomItemToOwner(m_vClubsOwner,pItem->nBelongsToClubUID,pItem->nRoomID);
	}
	else
	{
		// tell club players ;
		CSendPushNotification::getInstance()->reset() ;
		CSendPushNotification::getInstance()->addTarget(pItem->nBelongsToClubUID) ;
		CSendPushNotification::getInstance()->setContent(CServerStringTable::getInstance()->getStringByID(3),1);
		CSendPushNotification::getInstance()->postApns( getSvrApp()->getAsynReqQueue(),true,"newRoom") ;
	}

	// remove from will use ids 
	auto iter_willUse = std::find(m_vWillUseRoomIDs.begin(),m_vWillUseRoomIDs.end(),pItem->nRoomID);
	if ( iter_willUse != m_vWillUseRoomIDs.end() )
	{
		m_vWillUseRoomIDs.erase(iter_willUse) ;
	}
	else
	{
		if ( isNewAdd )
		{
			LOGFMTE("why this room id = %u , not in will use list ?",pItem->nRoomID) ;
		};
	}

	if ( isNewAdd ) // save to db 
	{
		LOGFMTD("save this item to db ") ;
		Json::Value jssql ;
		char pBuffer[512] = {0};
		sprintf(pBuffer,"insert into gameroomcenter ( serialNum,roomID,roomType,belongClubID,creatorUID,createDate ) values ('%u','%u','%u','%u','%u',now());",pItem->nSerialNumber,pItem->nRoomID,getRoomType(pItem->nRoomID),pItem->nBelongsToClubUID,pItem->nCreator);
		jssql["sql"] = pBuffer ;
		getSvrApp()->getAsynReqQueue()->pushAsyncRequest(ID_MSG_PORT_DB,eAsync_DB_Add,jssql);
	}
}

void CGameRoomCenter::deleteRoomItem( uint32_t nRoomID )
{
	auto iter = m_vRoomIDKey.find(nRoomID) ;
	if ( iter == m_vRoomIDKey.end() )
	{
		LOGFMTE("can not find room id = %u to delete",nRoomID) ;
		return ;
	}

	// recycle chat room id ;
	auto nChatRoomID = iter->second->nChatRoomID ;
	if ( nChatRoomID != 0 )
	{
		m_vReserveChatRoomIDs.push(nChatRoomID);
	}
	
	uint32_t nClubID = iter->second->nBelongsToClubUID ;
	uint32_t nCreatorID = iter->second->nCreator ;
	uint32_t nSeailNum = iter->second->nSerialNumber ;

	delete iter->second ;
	iter->second = nullptr ;
	m_vRoomIDKey.erase(iter) ;

	// do delete from db ;
	Json::Value jssql ;
	char pBuffer[512] = {0};
	sprintf(pBuffer,"update gameroomcenter set isDelete = 1 where serialNum = '%u';",nSeailNum);
	jssql["sql"] = pBuffer;
	getSvrApp()->getAsynReqQueue()->pushAsyncRequest(ID_MSG_PORT_DB,eAsync_DB_Update,jssql);
	
	// delete from player owners ;
	bool b = deleteRoomItemFromOwner(m_vPlayerOwners,nCreatorID,nRoomID) ;
	if ( b == false )
	{
		LOGFMTE("delete room from player owner uid = %u , roomId = %u , failed",nCreatorID,nRoomID) ;
	}

	if ( nClubID == 0 )
	{
		return ;
	}

	b = deleteRoomItemFromOwner(m_vClubsOwner,nClubID,nRoomID) ;
	if ( !b )
	{
		LOGFMTE("delete room from nClub owner is failed club id = %u , room id = %u",nClubID,nRoomID) ;
	}
}

bool CGameRoomCenter::addRoomItemToOwner(MAP_ROOM_OWNERS& vOwners ,uint32_t nOwnerUID ,uint32_t nRoomID)
{
	auto iterOwner = vOwners.find(nOwnerUID);
	stRoomOwnerInfo* pOwnerInfo = nullptr ;
	if ( iterOwner == vOwners.end() )
	{
		pOwnerInfo = new stRoomOwnerInfo ;
		pOwnerInfo->nOwnerUID = nOwnerUID ;
		vOwners[pOwnerInfo->nOwnerUID] = pOwnerInfo ;
	}
	else
	{
		pOwnerInfo = iterOwner->second ;
	}

	auto iterRoomID = std::find(pOwnerInfo->vRoomIDs.begin(),pOwnerInfo->vRoomIDs.end(),nRoomID );
	if ( iterRoomID == pOwnerInfo->vRoomIDs.end() )
	{
		pOwnerInfo->vRoomIDs.push_back(nRoomID) ;
	}
	else
	{
		LOGFMTE("why already add this room id = %u",nRoomID) ;
	}
	return true ;
}

bool CGameRoomCenter::deleteRoomItemFromOwner(MAP_ROOM_OWNERS& vOwners ,uint32_t nOwnerUID , uint32_t nRoomID )
{
	// remove from owner
	auto iterOwner = vOwners.find(nOwnerUID) ;
	if (iterOwner == vOwners.end() )
	{
		LOGFMTE("why do not have the creator uid = %u recorder in owner container ? roomID = %u " , nOwnerUID,nRoomID ) ;
		return false;
	}

	auto pOwner = iterOwner->second ;
	auto iterID = std::find(pOwner->vRoomIDs.begin(),pOwner->vRoomIDs.end(),nRoomID) ;
	if ( iterID != pOwner->vRoomIDs.end() )
	{
		pOwner->vRoomIDs.erase(iterID);
		if ( pOwner->vRoomIDs.empty() )
		{
			delete pOwner ;
			pOwner = nullptr ;
			vOwners.erase(iterOwner) ;
		}
	}
	else
	{
		LOGFMTE("uid = %u ,do not own room id = %u , why delete it ",nOwnerUID,nRoomID) ;
	}
	return true ;
}

uint8_t CGameRoomCenter::getRoomType(uint32_t nRoomID)
{
	uint32_t nTypeNumber = nRoomID / 100000 ;
	uint32_t nRealID = nRoomID % 100000 ;
	uint32_t narg = nRealID % 4 + 1;
	return (nTypeNumber - narg) ;
}

uint32_t CGameRoomCenter::generateRoomID(eRoomType eType,uint32_t& nserailNum )
{
	uint32_t nRoomID = 0 ;
	auto iter = m_vRoomIDKey.begin() ;
	auto iter_willUse = m_vWillUseRoomIDs.end();
	uint32_t nTryTimes = 0 ;
	nserailNum = ++m_nCurSerailNum ;
	do 
	{
		uint32_t nRealID = rand() % 100000 ;
		uint32_t narg = nRealID % 4 + 1;
		uint32_t nTypeNumber = eType + narg ;
		nRoomID = nTypeNumber * 100000 + nRealID ;	

		iter = m_vRoomIDKey.find(nRoomID) ;
		++nTryTimes ;
		if ( nTryTimes > 1 )
		{
			LOGFMTD("try times = %u to generate room id ",nTryTimes);
		}

		if ( !m_vWillUseRoomIDs.empty() )
		{
			iter_willUse = std::find(m_vWillUseRoomIDs.begin(),m_vWillUseRoomIDs.end(),nRoomID) ;
		}
	}
	while (iter != m_vRoomIDKey.end() || iter_willUse != m_vWillUseRoomIDs.end() ) ;

	m_vWillUseRoomIDs.push_back(nRoomID) ;
	return nRoomID ;
}

CGameRoomCenter::stRoomItem* CGameRoomCenter::getRoomItemByRoomID(uint32_t nRoomID )
{
	auto iter = m_vRoomIDKey.find(nRoomID) ;
	if ( iter != m_vRoomIDKey.end() )
	{
		return iter->second ;
	}
	return nullptr ;
}

uint16_t CGameRoomCenter::getPlayerOwnRoomCnt(uint32_t nPlayerUID,uint8_t nGameType )
{
	//auto iter = m_vPlayerOwners.find(nPlayerUID) ;
	//if ( iter != m_vPlayerOwners.end() )
	//{
	//	return iter->second->vRoomIDs.size() ;
	//}
	//return 0 ;

	std::vector<uint32_t> v;
	getPlayerOwenRooms(v, nPlayerUID, nGameType);
	return v.size();
}

uint16_t CGameRoomCenter::getClubOwnRoomCnt(uint32_t nClubID )
{
	auto iter = m_vClubsOwner.find(nClubID) ;
	if ( iter != m_vClubsOwner.end() )
	{
		return iter->second->vRoomIDs.size(); 
	}
	return 0 ;
}

uint16_t CGameRoomCenter::getClubOwnRooms(std::vector<uint32_t>& vRoomIDs , uint32_t nClubID )
{
	auto iter = m_vClubsOwner.find(nClubID) ;
	if ( iter != m_vClubsOwner.end() )
	{
		auto pOwnInfor = iter->second ;
		vRoomIDs.clear();
		vRoomIDs.assign(pOwnInfor->vRoomIDs.begin(),pOwnInfor->vRoomIDs.end()) ;
		return vRoomIDs.size();
	}
	return 0 ;
}

uint32_t CGameRoomCenter::getReuseChatRoomID()
{
	if ( m_vReserveChatRoomIDs.empty() )
	{
		return 0 ;
	}

	auto iter = m_vReserveChatRoomIDs.front() ;
	m_vReserveChatRoomIDs.pop() ;
	checkChatRoomIDReserve();
	return iter ;
}

uint16_t CGameRoomCenter::getPlayerOwenRooms(std::vector<uint32_t>& vRoomIDs, uint32_t nPlayerUID, uint8_t nRoomType)
{
	auto iter = m_vPlayerOwners.find(nPlayerUID) ;
	if ( iter != m_vPlayerOwners.end() )
	{
		auto pOwnInfor = iter->second ;
		vRoomIDs.clear();
		for (auto& ref : pOwnInfor->vRoomIDs)
		{
			if (getRoomType(ref) == nRoomType)
			{
				vRoomIDs.push_back(ref);
			}
		}
		//vRoomIDs.assign(pOwnInfor->vRoomIDs.begin(),pOwnInfor->vRoomIDs.end()) ;
		return vRoomIDs.size();
	}
	return 0 ;
}
