#include "MJRoomManager.h"
#include "ISeverApp.h"
#include "log4z.h"
#include "ServerMessageDefine.h"
#include "RoomConfig.h"
#include "MJServer.h"
#include "IMJPlayer.h"
#include "XLMJRoom.h"
#include "MJPrivateRoom.h"
#include "AsyncRequestQuene.h"
#include "XZMJRoom.h"
#include "MJGoldRoom.h"
#include <time.h>

uint32_t MJRoomManager::s_MaxBillID = 0;
void MJRoomManager::init(IServerApp* svrApp)
{
	m_vWillDeleteRoomIDs.clear();

	m_mapUserUIDandRoomID.clear();
	m_lstShorthandedRoomID.clear();
	m_lstUserEnterRoomData.clear();
	m_lstRobotEnterRoomData.clear();
	m_lastRoomCreatedTime = time(NULL);

	IGameRoomManager::init(svrApp);
}

IGameRoom* MJRoomManager::getRoomByID(uint32_t nRoomID)
{
	auto iter = m_vRooms.find(nRoomID);
	if (iter != m_vRooms.end())
	{
		return iter->second;
	}
	return nullptr;
}

void MJRoomManager::sendMsg(stMsg* pmsg, uint32_t nLen, uint32_t nSessionID)
{
	getSvrApp()->sendMsg(nSessionID, (char*)pmsg, nLen);
}

void MJRoomManager::sendMsg(Json::Value& jsContent, unsigned short nMsgType, uint32_t nSessionID, eMsgPort ePort )
{
	getSvrApp()->sendMsg(nSessionID, jsContent, nMsgType, ePort, false);
}

bool MJRoomManager::onMsg(stMsg* prealMsg, eMsgPort eSenderPort, uint32_t nSessionID)
{
	if (MSG_SVR_ENTER_ROOM == prealMsg->usMsgType)
	{
		return processEnterRoomMsg(prealMsg,eSenderPort,nSessionID);
	}

	if (MSG_PLAYER_ENTER_GOLDROOM == prealMsg->usMsgType)
	{
		return processEnterGoldRoomMsg(prealMsg, eSenderPort, nSessionID);
	}

	// msg give to room process 
	stMsgToRoom* pRoomMsg = (stMsgToRoom*)prealMsg;
	auto pRoom = getRoomByID(pRoomMsg->nRoomID);
	if (pRoom == NULL)
	{
		if (MSG_REQUEST_ROOM_INFO == prealMsg->usMsgType) // special solve the problem
		{
			stMsgPlayerEnterRoomRet msgRet;
			msgRet.nRet = 5;
			msgRet.nRoomID = pRoomMsg->nRoomID;
			sendMsg(&msgRet, sizeof(msgRet),nSessionID);
			LOGFMTD("request room info but room is dissmised id = %u , session id = %u",pRoomMsg->nRoomID,nSessionID);
			return true;
		}
		LOGFMTE("can not find room to process id = %d ,from = %d, room id = %d", prealMsg->usMsgType, eSenderPort, pRoomMsg->nRoomID);
		return  false;
	}

	return pRoom->onMessage(prealMsg, eSenderPort, nSessionID);
}

bool MJRoomManager::onMsg(Json::Value& prealMsg, uint16_t nMsgType, eMsgPort eSenderPort, uint32_t nSessionID)
{
	if ( MSG_REQ_MJ_ROOM_BILL_INFO == nMsgType)
	{
		uint32_t nBillID = prealMsg["sieral"].asUInt();
		if (isHaveVipRoomBill(nBillID))
		{
			sendVipRoomBillToPlayer(nBillID, nSessionID);
			return true;
		}

		// add to requesting queue ;
		auto iter = m_vReqingBillInfoPlayers.find(nBillID);
		if (iter == m_vReqingBillInfoPlayers.end())
		{
			auto p = std::shared_ptr<stReqVipRoomBillPlayers>(new stReqVipRoomBillPlayers());
			p->nReqBillID = nBillID;
			p->vReqPlayers.insert(nSessionID);
			m_vReqingBillInfoPlayers[nBillID] = p;
		}
		else
		{
			auto p = iter->second;
			p->vReqPlayers.insert(nSessionID);
		}
		// read from db ;
		auto async = getSvrApp()->getAsynReqQueue();
		Json::Value jsReq;
		std::ostringstream ss;
		ss << "select roomID,roomType,createUID,unix_timestamp(billTime) as bTime,detail,roomInitCoin,circleCnt from viproombills where billID = " << nBillID << " ;";
		jsReq["sql"] = ss.str();
		Json::Value jsUserData;
		jsUserData["billID"] = nBillID;
		async->pushAsyncRequest(ID_MSG_PORT_DB, eAsync_DB_Select, jsReq, [this](uint16_t nReqType, const Json::Value& retContent, Json::Value& jsUserData){
			uint8_t nAfcRow = retContent["afctRow"].asUInt();
			uint32_t nBillID = jsUserData["billID"].asUInt();
			if (nAfcRow == 1)
			{
				auto jsRow = retContent["data"][(uint32_t)0];
				auto pBill = createVipRoomBill();
				--s_MaxBillID;
				pBill->nBillID = nBillID;
				pBill->nBillTime = jsRow["bTime"].asUInt();
				pBill->nCircleCnt = jsRow["circleCnt"].asUInt();
				pBill->nCreateUID = jsRow["createUID"].asUInt();
				pBill->nRoomID = jsRow["roomID"].asUInt();
				pBill->nRoomInitCoin = jsRow["roomInitCoin"].asUInt();
				pBill->nRoomType = jsRow["roomType"].asUInt();

				Json::Reader jsRead;
				jsRead.parse(jsRow["detail"].asString(), pBill->jsDetail, false);
				addVipRoomBill(pBill, false);
			}

			// send all req players ;
			auto iter = m_vReqingBillInfoPlayers.find(nBillID);
			if (iter == m_vReqingBillInfoPlayers.end())
			{
				LOGFMTE("here must error , must have players waiting the result");
				return;
			}

			for (auto& nSessionID : iter->second->vReqPlayers)
			{
				sendVipRoomBillToPlayer(nBillID, nSessionID);
			}
			m_vReqingBillInfoPlayers.erase(iter);

		}, jsUserData);

		return true;
	}

	// msg give to room process 
	if (prealMsg["dstRoomID"].isNull())
	{
		if (nMsgType == MSG_PLAYER_LEAVE_ROOM)
		{
			Json::Value jsMsg;
			jsMsg["ret"] = 1;
			sendMsg(jsMsg, nMsgType, nSessionID);
			LOGFMTE("player leave room msg , lack of dstRoom argument");
			return true;
		}
		return false;
	}

	auto pRoom = getRoomByID(prealMsg["dstRoomID"].asUInt());
	if (pRoom == NULL)
	{
		LOGFMTE("can not find room to process id = %d ,from = %d, room id = %d", nMsgType, eSenderPort, prealMsg["dstRoomID"].asUInt());
		return  false;
	}

	return pRoom->onMsg(prealMsg, nMsgType, eSenderPort, nSessionID);
}

bool MJRoomManager::addUserToRoom(MJGoldRoom* pRoom)
{
	auto nPlayerCnt = m_lstUserEnterRoomData.size();
	auto nRobotCnt = m_lstRobotEnterRoomData.size();
	auto nShorthandedCnt = pRoom->getShorhandedPlayerCount();
	if (nPlayerCnt - nRobotCnt >= nShorthandedCnt)
	{
		while (nShorthandedCnt > 0)
		{
			if (m_lstUserEnterRoomData.size() > 0)
			{
				auto userData = m_lstUserEnterRoomData.front();
				auto b = pRoom->onPlayerEnter(&userData.tPlayerData);
				if (b)
				{
					pRoom->sendRoomInfo(userData.tPlayerData.nUserSessionID);
					m_lstUserEnterRoomData.pop_front();
					LOGFMTI("有人进入房间成功 ，会有room info 返回 , 进入房间成功消息也发 uid = %u", userData.tPlayerData.nUserUID);
				}
				else
				{
					LOGFMTE("enter gold room failed! uid = %u", userData.tPlayerData.nUserUID);
					return false;
				}
			}
			else
			{
				auto robotData = m_lstRobotEnterRoomData.front();
				auto b = pRoom->onPlayerEnter(&robotData.tPlayerData);
				if (b)
				{
					pRoom->sendRoomInfo(robotData.tPlayerData.nUserSessionID);
					m_lstRobotEnterRoomData.pop_front();
					LOGFMTI("机器人进入房间成功 ，会有room info 返回 , 进入房间成功消息也发 uid = %u", robotData.tPlayerData.nUserUID);
				}
				else
				{
					LOGFMTE("enter gold room failed! uid = %u", robotData.tPlayerData.nUserUID);
					return false;
				}
			}
		}
	}
	else
	{
		LOGFMTD("The shorthanded gold room is still missing, playerCnt = %u, robotCnt = %u, shorthandedCnt = %u", nPlayerCnt, nRobotCnt, nShorthandedCnt);
		return false;
	}
	return true;
}

void MJRoomManager::createGoldRoomAndAddUserIn()
{
	auto nPlayerCnt = m_lstUserEnterRoomData.size();
	auto nRobotCnt = m_lstRobotEnterRoomData.size();
	if (nPlayerCnt > 0 /*&& nPlayerCnt + nRobotCnt > 4*/)
	{
		auto userData = m_lstUserEnterRoomData.front();
		m_lstUserEnterRoomData.pop_front();

		Json::Value jsReq;
		jsReq["roomType"] = userData.nGameType;
		jsReq["createUID"] = userData.tPlayerData.nUserUID;
		jsReq["initCoin"] = userData.tPlayerData.nCoin;

		Json::Value jsUserData;

		getSvrApp()->getAsynReqQueue()->pushAsyncRequest(ID_MSG_PORT_DATA, eAsync_CreateGoldRoom, jsReq, [this, userData](uint16_t nReqType, const Json::Value& retContent, Json::Value& jsUserData)
		{
			jsUserData["circle"] = uint32_t(-1);

			MJGoldRoom* pRoom = static_cast<MJGoldRoom*>(doCreatePublicRoom(jsUserData));

			// save to DB ;
			Json::Value jsreq;
			char pBuffer[2014] = { 0 };
			Json::StyledWriter jsWrite;
			std::string str = jsWrite.write(jsUserData);
#ifdef GAME_365
			sprintf_s(pBuffer, sizeof(pBuffer), "insert into rooms (serialNum,roomType,ownerUID,createTime,jsCreateJson,leftTime ) values ('%u','%u','%u',now(),'%s','%u'); ",
				jsReqContent["serialNum"].asUInt(), pRoom->getRoomType(), jsReqContent["createUID"].asUInt(), str.c_str(), jsReqContent["circle"].asUInt());
#else
			sprintf_s(pBuffer, sizeof(pBuffer), "insert into rooms (serialNum,roomType,ownerUID,createTime,jsCreateJson,leftTime ) values ('%u','%u','%u',now(),'%s','%u'); ",
				jsUserData["serialNum"].asUInt(), pRoom->getRoomType(), jsUserData["createUID"].asUInt(), str.c_str(), jsUserData["circle"].asUInt());
#endif 
			jsreq["sql"] = pBuffer;
			getSvrApp()->getAsynReqQueue()->pushAsyncRequest(ID_MSG_PORT_DB, eAsync_DB_Add, jsreq);

			auto nRoomId = pRoom->getRoomID();

			if (processEnterGoldRoomMsg((stMsg*)&userData, ID_MSG_PORT_DATA, userData.tPlayerData.nUserSessionID) == false)
			{
				addWillDeleteRoomID(nRoomId);
				LOGFMTE("create gold room and add createUID player to room failed! room will be destroyed. roomId = %u", nRoomId);
				return;
			}

			if (addUserToRoom(pRoom) == false)
			{
				std::list<uint32_t>::iterator it = std::find(m_lstShorthandedRoomID.begin(), m_lstShorthandedRoomID.end(), nRoomId);
				if (it == m_lstShorthandedRoomID.end())
				{
					m_lstShorthandedRoomID.push_back(nRoomId);
				}
				LOGFMTE("create gold room and add user to room failed! roomId = %u", nRoomId);
			}
		}, jsUserData);

		m_lastRoomCreatedTime = time(NULL);
	}
}

void MJRoomManager::update(float fDeta)
{
	IGameRoomManager::update(fDeta);
	for (auto& ref : m_vRooms)
	{
		if (ref.second)
		{
			ref.second->update(fDeta);
		}
	}

	time_t tNow = time(NULL);
	if (tNow - m_lastRoomCreatedTime > 10)
	{
		//是否有缺人的房间
		while (m_lstShorthandedRoomID.size() > 0)
		{
			auto nRoomId = m_lstShorthandedRoomID.front();

			MJGoldRoom* pRoom = static_cast<MJGoldRoom*>(getRoomByID(nRoomId));
			if (pRoom == nullptr)
			{
				LOGFMTE("shorthanded gold room not found, roomId = %u", nRoomId);
				m_lstShorthandedRoomID.pop_front();
				break;
			}
			if (addUserToRoom(pRoom) == false)
			{
				break;
			}
			m_lstShorthandedRoomID.pop_front();
			m_lastRoomCreatedTime = time(NULL);
		}
		//创建房间
		createGoldRoomAndAddUserIn();
	}

	if (m_vWillDeleteRoomIDs.empty())
	{
		return;
	}

	for (auto& ref : m_vWillDeleteRoomIDs)
	{
		auto iter = m_vRooms.find(ref);
		if (iter == m_vRooms.end())
		{
			LOGFMTE("why delete the room id = %u , room is null ",ref);
			continue;
		}
		delete iter->second;
		iter->second = nullptr;
		m_vRooms.erase(iter);
	}
	m_vWillDeleteRoomIDs.clear();
}

bool MJRoomManager::onAsyncRequest(uint16_t nRequestType, const Json::Value& jsReqContent, Json::Value& jsResult)
{
	if (eAsync_ApplyLeaveRoom == nRequestType)
	{
		uint32_t nRoomID = jsReqContent["roomID"].asUInt();
		uint32_t nUID = jsReqContent["uid"].asUInt();
		auto pRoom = getRoomByID(nRoomID) ;
		if (nullptr == pRoom)
		{
			jsResult["ret"] = 3;
			jsResult["coin"] = 0;
			return true;
		}

		auto p = pRoom->onPlayerApplyLeave(nUID);
		if ( true == p)
		{
			jsResult["ret"] = 2;
			jsResult["coin"] = 0;
			return true;
		}

		jsResult["ret"] = 0;
		jsResult["coin"] = 0;
		// do apply leave ;
		
		//jsResult["coin"] = p->getCoin() ;
		//if ()
		//{
		//	jsResult["ret"] = 0;   // direct leave ;
		//}
		//else
		//{
		//	jsResult["ret"] = 1;  // delay leave ;
		//}
		return true;
	}

	/*if (eAsync_SendUpdateCoinToClient == nRequestType)
	{
		uint32_t nSessionID = jsReqContent["sessionID"].asUInt();
		uint32_t nUID = jsReqContent["uid"].asUInt();
		uint32_t nCoin = jsReqContent["coin"].asUInt();
		uint32_t nDiamond = jsReqContent["diamond"].asUInt();
		uint32_t nRoomID = jsReqContent["roomID"].asUInt();

		auto pRoom = getRoomByID(nRoomID);
		if (pRoom)
		{
			Json::Value js = jsReqContent;
			bool bProcessed = pRoom->onMsg(js, MSG_INTERAL_ROOM_SEND_UPDATE_COIN, ID_MSG_PORT_DATA, nSessionID);
			if (!bProcessed)
			{
				LOGFMTE("find the room you stay in to update coin msg, but can not find you uid = %u", nUID);
				Json::Value jsmsgBack;
				jsmsgBack["coin"] = nCoin;
				jsmsgBack["diamond"] = nDiamond;
				sendMsg(jsmsgBack, MSG_REQ_UPDATE_COIN, nSessionID);
			}
		}
		else
		{
			LOGFMTE("can not find the room you stay in to update coin msg  uid = %u", nUID);
			Json::Value jsmsgBack;
			jsmsgBack["coin"] = nCoin;
			jsmsgBack["diamond"] = nDiamond;
			sendMsg(jsmsgBack, MSG_REQ_UPDATE_COIN, nSessionID);

		}
		return true;
	}*/

	if (eAsync_CreateRoom == nRequestType )
	{
		Json::Value js = jsReqContent;
		jsResult["ret"] = 0;
		LOGFMTD("received create room id = %u", jsReqContent["roomID"].asUInt());

		auto pRoom = doCreatePrivateRoom(0, js);;
		if (pRoom == nullptr)
		{
			LOGFMTE("why create room is null");
			jsResult["ret"] = 0;
			return true;
		}
		// save to DB ;
		Json::Value jsreq;
		char pBuffer[2014] = { 0 };
		Json::StyledWriter jsWrite;
		std::string str = jsWrite.write(jsReqContent);
#ifdef GAME_365
		sprintf_s(pBuffer, sizeof(pBuffer), "insert into rooms (serialNum,roomType,ownerUID,createTime,jsCreateJson,leftTime ) values ('%u','%u','%u',now(),'%s','%u'); ",
			jsReqContent["serialNum"].asUInt(), pRoom->getRoomType(), jsReqContent["createUID"].asUInt(), str.c_str(), jsReqContent["circle"].asUInt());
#else
		sprintf_s(pBuffer, sizeof(pBuffer), "insert into rooms (serialNum,roomType,ownerUID,createTime,jsCreateJson,leftTime ) values ('%u','%u','%u',now(),'%s','%u'); ",
			jsReqContent["serialNum"].asUInt(),pRoom->getRoomType(), jsReqContent["createUID"].asUInt(), str.c_str(), jsReqContent["circle"].asUInt() );
#endif 
		jsreq["sql"] = pBuffer;
		getSvrApp()->getAsynReqQueue()->pushAsyncRequest(ID_MSG_PORT_DB, eAsync_DB_Add, jsreq);
		return true;
	}

	return false;
}

void MJRoomManager::onConnectedSvr()
{
	IGameRoomManager::onConnectedSvr();
	auto asyq = getSvrApp()->getAsynReqQueue();
	Json::Value jsReq;
	jsReq["sql"] = "SELECT max(billID) as maxBillID FROM viproombills ;";
	asyq->pushAsyncRequest(ID_MSG_PORT_DB, eAsync_DB_Select, jsReq, [](uint16_t nReqType, const Json::Value& retContent, Json::Value& jsUserData){
		uint32_t nAft = retContent["afctRow"].asUInt();
		auto jsData = retContent["data"];
		if (nAft == 0 || jsData.isNull())
		{
			LOGFMTE("read max bill id error ");
			return;
		}

		auto jsRow = jsData[(uint32_t)0];
		s_MaxBillID = jsRow["maxBillID"].asUInt();
		++s_MaxBillID;
		LOGFMTD("max bill id  = %u", s_MaxBillID);
	});
}

bool MJRoomManager::processEnterRoomMsg(stMsg* prealMsg, eMsgPort eSenderPort, uint32_t nSessionID)
{
	stMsgSvrEnterRoomRet msgBack;
	msgBack.nRet = 0;
	stMsgSvrEnterRoom* pRet = (stMsgSvrEnterRoom*)prealMsg;
	msgBack.nGameType = 0;
	msgBack.nRoomID = pRet->nRoomID;
	msgBack.nSubIdx = 0;

	LOGFMTD("session id = %u enter room type = %u , roomID = %u", nSessionID, pRet->nSubIdx, pRet->nRoomID);

	// find a room to enter 
	IGameRoom* pRoomEnter = nullptr;
	pRoomEnter = getRoomByID(pRet->nRoomID);
	if (pRoomEnter == nullptr)
	{
		msgBack.nRet = 5;
		sendMsg(&msgBack, sizeof(msgBack), nSessionID);
		LOGFMTD("target room id = %u is null", pRet->nRoomID);
		return false;
	}

	// when come here will not nullptr of room ;
	// check enter xian zhi 
	msgBack.nRet = pRoomEnter->checkPlayerCanEnter(&pRet->tPlayerData);

	msgBack.nRoomID = pRoomEnter->getRoomID();
	if (msgBack.nRet)
	{
		sendMsg(&msgBack, sizeof(msgBack), nSessionID);
		LOGFMTD("you are not proper to enter this room target id = %u , ret = %d", pRet->nRoomID, msgBack.nRet);
		return true;
	}

	int8_t nidx = 0;
	msgBack.nGameType = pRoomEnter->getRoomType();
	msgBack.nRoomID = pRoomEnter->getRoomID();
	msgBack.nSubIdx = 0;
	msgBack.nRet = 0;
	sendMsg(&msgBack, sizeof(msgBack), nSessionID);

	auto b = pRoomEnter->onPlayerEnter(&pRet->tPlayerData);
	if (b)
	{
		pRoomEnter->sendRoomInfo(nSessionID);
		LOGFMTI("有人进入房间成功 ，会有room info 返回 , 进入房间成功消息也发 uid = %u", pRet->tPlayerData.nUserUID);
	}	
	return true;
}

bool MJRoomManager::processEnterGoldRoomMsg(stMsg* prealMsg, eMsgPort eSenderPort, uint32_t nSessionID)
{
	stMsgSvrEnterGoldRoomRet msgBack;
	msgBack.nRet = 0;
	stMsgSvrEnterGoldRoom* pRet = (stMsgSvrEnterGoldRoom*)prealMsg;
	msgBack.nGameType = 0;
	msgBack.nRoomID = pRet->nRoomID;
	msgBack.nSubIdx = 0;

	LOGFMTD("session id = %u enter room type = %u , roomID = %u", nSessionID, pRet->nSubIdx, pRet->nRoomID);

	if ((int32_t)(pRet->tPlayerData.nCoin) <= 0)
	{
		// coin is not enough
		msgBack.nRet = 3;
		sendMsg(&msgBack, sizeof(msgBack), nSessionID);
		LOGFMTD("enter gold room failed, player coin(%d) is zero!", (int32_t)pRet->tPlayerData.nCoin);
		return false;
	}

	// find a room to enter 
	IGameRoom* pRoomEnter = getRoomByID(pRet->nRoomID);
	if (pRoomEnter == nullptr)
	{
		// 检查这个人是否已有金币房间
		auto nRoomID = getRoomIdByUserUID(pRet->tPlayerData.nUserUID);
		if (nRoomID != uint32_t(-1))
		{
			pRoomEnter = getRoomByID(nRoomID);
		}
	}
	if (pRoomEnter == nullptr)
	{
		//没有房间，进入排队队列
		enterWaitingQueue(pRet);
		msgBack.nRet = 8;
		sendMsg(&msgBack, sizeof(msgBack), nSessionID);
		LOGFMTD("target room id = %u is null", pRet->nRoomID);
		return false;
	}

	//有房间，回到原来的房间
	// when come here will not nullptr of room ;
	// check enter xian zhi 
	msgBack.nRet = pRoomEnter->checkPlayerCanEnter(&pRet->tPlayerData);

	msgBack.nRoomID = pRoomEnter->getRoomID();
	if (msgBack.nRet)
	{
		sendMsg(&msgBack, sizeof(msgBack), nSessionID);
		LOGFMTD("you are not proper to enter this room target id = %u , ret = %d", pRet->nRoomID, msgBack.nRet);
		return false;
	}

	int8_t nidx = 0;
	msgBack.nGameType = pRoomEnter->getRoomType();
	msgBack.nRoomID = pRoomEnter->getRoomID();
	msgBack.nSubIdx = 0;
	msgBack.nRet = 0;
	sendMsg(&msgBack, sizeof(msgBack), nSessionID);

	auto b = pRoomEnter->onPlayerEnter(&pRet->tPlayerData);
	if (b)
	{
		pRoomEnter->sendRoomInfo(nSessionID);
		LOGFMTI("有人进入房间成功 ，会有room info 返回 , 进入房间成功消息也发 uid = %u", pRet->tPlayerData.nUserUID);
		return true;
	}
	return false;
}

void MJRoomManager::enterWaitingQueue(stMsgSvrEnterGoldRoom* data)
{
	std::list<stMsgSvrEnterGoldRoom>::iterator it = m_lstUserEnterRoomData.begin();
	for (; it != m_lstUserEnterRoomData.end(); )
	{
		if (it->tPlayerData.nUserUID == data->tPlayerData.nUserUID)
		{
			it = m_lstUserEnterRoomData.erase(it);
			//break;
		}
		else
		{
			++it;
		}
	}
	m_lstUserEnterRoomData.push_back(*data);
}

uint32_t MJRoomManager::getRoomIdByUserUID(uint32_t nUserUID)
{
	std::map<uint32_t, uint32_t>::iterator it = m_mapUserUIDandRoomID.find(nUserUID);
	if (it != m_mapUserUIDandRoomID.end())
	{
		return it->second;
	}
	return uint32_t(-1);
}

//IGameRoom* MJRoomManager::randRoomToEnterByConfigID(uint32_t nRoomConfigID)
//{
//	auto iter = m_vPublicRooms.find(nRoomConfigID);
//	std::vector<IGameRoom*> vecRoom;
//	if (iter != m_vPublicRooms.end())
//	{
//		auto& vecRoomIDs = iter->second;
//		for (auto& ref : vecRoomIDs)
//		{
//			auto pRoom = getRoomByID(ref);
//			if (pRoom == nullptr || pRoom->isRoomFull())
//			{
//				continue;
//			}
//			vecRoom.push_back(pRoom);
//		}
//	}
//	
//	if (vecRoom.empty())
//	{
//		return doCreatePublicRoom(nRoomConfigID);
//	}
//	
//	return vecRoom[rand() % vecRoom.size()];
//}

IGameRoom* MJRoomManager::doCreatePublicRoom(Json::Value& jsArg)
{
	auto p = new MJGoldRoom();
	auto ret = p->init(this, nullptr, jsArg["serialNum"].asUInt(), jsArg["roomID"].asUInt(), jsArg);
	if (ret == false)
	{
		delete p;
		p = nullptr;
		LOGFMTE("create public room failed");
	}
	else
	{
		auto iter = m_vRooms.find(p->getRoomID());
		if (iter != m_vRooms.end())
		{
			LOGFMTE("why have duplicate room id when create public room, room id = %u", p->getRoomID());
			delete p;
			p = nullptr;
			return p;
		}
		m_vRooms[p->getRoomID()] = p;
		LOGFMTD("create public room id = %u success", p->getRoomID());
	}
	return nullptr;
}

IGameRoom* MJRoomManager::doCreatePrivateRoom(uint16_t nConfigID, Json::Value& jsArg)
 {
	auto p = new MJPrivateRoom();
	auto ret = p->init(this, nullptr, jsArg["serialNum"].asUInt(), jsArg["roomID"].asUInt() , jsArg);
	if (ret == false)
	{
		delete p;
		p = nullptr;
		LOGFMTE("create private room failed");
	}
	else
	{
		auto iter = m_vRooms.find(p->getRoomID());
		if (iter != m_vRooms.end())
		{
			LOGFMTE("why have duplicate room id when create public room configid = %u , room id = %u", nConfigID, p->getRoomID());
			delete p;
			p = nullptr;
			return p;
		}
		m_vRooms[p->getRoomID()] = p;
		LOGFMTD("create private room id = %u success", p->getRoomID() );
	}
	return p;
}

uint32_t MJRoomManager::genPrivateRoomID()
{
	IGameRoom* pCheckRoom = nullptr;
	uint32_t nRoomID = 0;
	do
	{
		nRoomID = (1 + rand() % 9) * 100000 + rand() % 100000;
		pCheckRoom = getRoomByID(nRoomID);
	} while (pCheckRoom);
	return nRoomID;
}

static uint32_t s_id = 1000000;
uint32_t MJRoomManager::generateRoomID()
{
	++s_id;
	if (s_id == uint32_t(-1))
	{
		s_id = 1000000;
	}
	return s_id;
}

void MJRoomManager::addWillDeleteRoomID(uint32_t nDelRoomID)
{
	m_vWillDeleteRoomIDs.push_back(nDelRoomID);
}

MJRoomManager::~MJRoomManager()
{
	for (auto& ref : m_vRooms)
	{
		delete ref.second;
		ref.second = nullptr;
	}
	m_vRooms.clear();
}

void MJRoomManager::sendVipRoomBillToPlayer(uint32_t nBillID, uint32_t nTargetSessionD)
{
	Json::Value jsMsg;
	jsMsg["ret"] = 0;
	jsMsg["sieral"] = nBillID;

	if (!isHaveVipRoomBill(nBillID))
	{
		jsMsg["ret"] = 1;
		sendMsg(jsMsg, MSG_REQ_MJ_ROOM_BILL_INFO, nTargetSessionD);
		return;
	}

	auto pBill = m_vVipRoomBills.find(nBillID)->second;
	jsMsg["billTime"] = pBill->nBillTime;
	jsMsg["circle"] = pBill->nCircleCnt;
	jsMsg["creatorUID"] = pBill->nCreateUID;
	jsMsg["roomID"] = pBill->nRoomID;
	jsMsg["initCoin"] = pBill->nRoomInitCoin;
	jsMsg["roomType"] = pBill->nRoomType;
	jsMsg["detail"] = pBill->jsDetail;
	sendMsg(jsMsg, MSG_REQ_MJ_ROOM_BILL_INFO, nTargetSessionD);
}

void MJRoomManager::addVipRoomBill(std::shared_ptr<stVipRoomBill>& pBill, bool isAddtoDB)
{
	if (isHaveVipRoomBill(pBill->nBillID))
	{
		LOGFMTE("already have this bill id = %u", pBill->nBillID);
		return;
	}

	m_vVipRoomBills[pBill->nBillID] = pBill;

	if (isAddtoDB)
	{
		auto asy = getSvrApp()->getAsynReqQueue();
		Json::Value jsReq;
		char pBuffer[1800] = { 0 };
		Json::StyledWriter jsWrite;
		auto str = jsWrite.write(pBill->jsDetail);
		LOGFMTD("detail len = %u",str.size());
		sprintf_s(pBuffer, sizeof(pBuffer), "insert into viproombills (billID,roomID,roomType,createUID,billTime,detail,roomInitCoin ,circleCnt ) values( %u,%u,%u,%u,now(),'%s',%u,%u);"
			, pBill->nBillID, pBill->nRoomID, pBill->nRoomType, pBill->nCreateUID, str.c_str(), pBill->nRoomInitCoin, pBill->nCircleCnt);
		jsReq["sql"] = pBuffer;
		asy->pushAsyncRequest(ID_MSG_PORT_DB, eAsync_DB_Add, jsReq);
	}
}

bool MJRoomManager::isHaveVipRoomBill(uint32_t nVipBillID)
{
	auto iter = m_vVipRoomBills.find(nVipBillID);
	return iter != m_vVipRoomBills.end();
}

std::shared_ptr<MJRoomManager::stVipRoomBill> MJRoomManager::createVipRoomBill()
{
	auto p = VIP_ROOM_BILL_SHARED_PTR(new stVipRoomBill());
	p->nBillID = ++s_MaxBillID;
	return p;
}