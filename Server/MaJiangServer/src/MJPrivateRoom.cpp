#include "MJPrivateRoom.h"
#include "log4z.h"
#include "ServerDefine.h"
#include "IMJRoom.h"
#include "IMJPlayer.h"
#include "IMJRoomState.h"
#include "ServerMessageDefine.h"
#include "IGameRoomManager.h"
#include "ISeverApp.h"
#include "AsyncRequestQuene.h"
#include "MJRoomManager.h"
#include <time.h>
#include "XLMJRoom.h"
#include "XZMJRoom.h"
#include "NJMJRoom.h"
#include "SZMJRoom.h"
#include "CYMJRoom.h"
#include "HHMJRoom.h"
#include "JSMJRoom.h"
#include "YZMJRoom.h"
#include "BPMJRoom.h"
#include "MJServer.h"
#include "NJMJPlayer.h"
#define TIME_WAIT_REPLY_DISMISS 90
MJPrivateRoom::~MJPrivateRoom()
{
	delete m_pRoom;
	m_pRoom = nullptr;
}

bool MJPrivateRoom::init(IGameRoomManager* pRoomMgr, stBaseRoomConfig* pConfig, uint32_t nSeialNum, uint32_t nRoomID, Json::Value& vJsValue)
{
	m_nInitCircle = vJsValue["circle"].asUInt();
	m_nInitCoin = vJsValue["initCoin"].asUInt();
	m_nChatID = vJsValue["chatRoomID"].asUInt();
	m_isForFree = false;
	m_isAA = false;
	//m_isCircleType = true;
	memset(&m_stConfig, 0, sizeof(m_stConfig));
	m_stConfig.nConfigID = 0;
	m_stConfig.nBaseBet = 1;//;vJsValue["baseBet"].asUInt();
	m_stConfig.nMaxSeat = 4;///vJsValue["seatCnt"].asUInt();
	m_stConfig.nGameType = vJsValue["roomType"].asUInt();
	if (m_stConfig.nMaxSeat == 0 || m_stConfig.nBaseBet == 0)
	{
		LOGFMTE("create private argument error seatCnt = %u , baseBet = %u", m_stConfig.nMaxSeat, m_stConfig.nBaseBet);
		m_stConfig.nMaxSeat = 4;
		m_stConfig.nBaseBet = 1;
	}

	m_bComsumedRoomCards = false;
	m_mapRecievedReply.clear();
	m_bWaitDismissReply = false;
	m_bDoDismissRoom = false;

	m_eState = eRoomState_Opening;
	m_pRoomMgr = pRoomMgr;
	m_bRoomInfoDiry = true;

	if (vJsValue["createUID"].isNull() == false)
	{
		m_nOwnerUID = vJsValue["createUID"].asUInt();
		LOGFMTD("create private room ownerUID is = %u", m_nOwnerUID);
	}
	else
	{
		m_nOwnerUID = 0;
		LOGFMTD("create private room ownerUID is null ?");
	}

	//if (vJsValue["isCircle"].isNull() == false)
	//{
	//	m_isCircleType = vJsValue["isCircle"].asUInt() == 1;
	//	LOGFMTD("create private room isCircle is = %u", (uint8_t)m_isCircleType);
	//}

	//if ( !m_isCircleType )
	//{
	//	m_nInitCircle *= 4;
	//	vJsValue["circle"] = vJsValue["circle"].asUInt() * 4;
	//}

	if (vJsValue["isFree"].isNull() == false)
	{
		m_isForFree = vJsValue["isFree"].asUInt() == 1 ;
		LOGFMTD("create private room isFree is = %u", m_isForFree);
	}
	else
	{
		LOGFMTD("create private room isFree is null ?");
	}

	if (vJsValue["isAA"].isNull() == false)
	{
		m_isAA = vJsValue["isAA"].asUInt() == 1;
		LOGFMTD("create private room m_isAA is = %u", m_isAA);
	}
	else
	{
		LOGFMTD("create private room m_isAA is null ?");
	}

	m_pRoom = doCreateMJRoom((eRoomType)m_stConfig.nGameType);

	if ( !m_pRoom)
	{
		LOGFMTE("create private room error , room type is null  type = %u",m_stConfig.nGameType);
		return false;
	}
	m_nLeftCircle = m_nInitCircle;
	LOGFMTD("create 1 private room");
	((IMJRoom*)m_pRoom)->setDelegate(this);
	auto bRet = m_pRoom->init(pRoomMgr, &m_stConfig, nSeialNum, nRoomID, vJsValue);
	//if ( eRoom_MJ_NanJing == m_pRoom->getRoomType() )
	//{
	//	((NJMJRoom*)m_pRoom)->bindPrivateRoom(this);
	//}
	return bRet;
}

bool MJPrivateRoom::onPlayerEnter(stEnterRoomData* pEnterRoomPlayer)
{
	pEnterRoomPlayer->nPlayerType = ePlayer_Robot; // avoid robot dispatch take effect ; temp let robot join vip room ;
	if (m_pRoom)
	{
		auto iter = m_vAllPlayers.find(pEnterRoomPlayer->nUserUID);
		if (iter == m_vAllPlayers.end())
		{
			stVipPlayer svp;
			svp.nSessionID = pEnterRoomPlayer->nUserSessionID;
			svp.nUID = pEnterRoomPlayer->nUserUID;
			svp.nRealCoin = pEnterRoomPlayer->nCoin;
			svp.nRoomCoin = m_nInitCoin;
			m_vAllPlayers[svp.nUID] = svp;

			pEnterRoomPlayer->nCoin = svp.nRoomCoin;
			LOGFMTD("uid = %u first enter room give coin = %u", pEnterRoomPlayer->nUserUID, svp.nRoomCoin);
		}
		else
		{
			iter->second.nRealCoin = pEnterRoomPlayer->nCoin;
			iter->second.nSessionID = pEnterRoomPlayer->nUserSessionID;
			//pEnterRoomPlayer->nCoin = iter->second.nRoomCoin;
			LOGFMTD("uid = %u  enter room again room coin = %u", pEnterRoomPlayer->nUserUID, iter->second.nRoomCoin);
		}
		return m_pRoom->onPlayerEnter(pEnterRoomPlayer);
	}
	return false;
}

uint8_t MJPrivateRoom::checkPlayerCanEnter(stEnterRoomData* pEnterRoomPlayer)
{
	if ( m_isAA )
	{
		if ( m_bComsumedRoomCards == false && pEnterRoomPlayer->nDiamond < getDiamondNeed() )
		{
			// diamond is not enough 
			return 3;
		}
	}

	if ( m_pRoom )
	{
		return m_pRoom->checkPlayerCanEnter(pEnterRoomPlayer);
	}
	LOGFMTE("private room can not room is null rooom id = %u",getRoomID());
	return 1;
}

bool MJPrivateRoom::onPlayerApplyLeave(uint32_t nPlayerUID)
{
	auto pRoom = (IMJRoom*)m_pRoom;
	auto curState = pRoom->getCurRoomState()->getStateID();
	auto pp = pRoom->getMJPlayerByUID(nPlayerUID);
	if (pp)
	{
		Json::Value jsMsg;
		jsMsg["idx"] = pp->getIdx();
		sendRoomMsg(jsMsg, MSG_ROOM_PLAYER_LEAVE); // tell other player leave ;
		pp->doTempLeaveRoom();
	}
	else
	{
		LOGFMTD("you are not in this room i let you go room id = %u",getRoomID());
		return true;
	}

	if (eRoomSate_WaitReady == curState && this->m_bComsumedRoomCards == false ) // not start game 
	{
		// direct leave just stand up ;
		pRoom->standup(nPlayerUID);

		auto iter = m_vAllPlayers.find(nPlayerUID);
		if (iter == m_vAllPlayers.end())
		{
			LOGFMTE("why player record is null , but still played in private room ? room id = %u , uid = %u , coin = %u", getRoomID(), nPlayerUID, iter->second.nRealCoin);
		}
		else
		{
			// do leave room and tell data svr ;
			stMsgSvrDoLeaveRoom msgdoLeave;
			msgdoLeave.nCoin = iter->second.nRealCoin;
			msgdoLeave.nGameType = getRoomType();
			msgdoLeave.nRoomID = getRoomID();
			msgdoLeave.nUserUID = nPlayerUID;
			msgdoLeave.nGameOffset = 0;
			m_pRoomMgr->sendMsg(&msgdoLeave, sizeof(msgdoLeave), nPlayerUID);

			// remove tag ;
			m_vAllPlayers.erase(iter);
		}
		return true;
	}
	else
	{
		LOGFMTE("private room can not leave befor room is closed");
	}
	return false;
}

bool MJPrivateRoom::isRoomFull()
{
	return m_pRoom->isRoomFull();
}

//virtual void roomItemDetailVisitor(Json::Value& vOutJsValue) = 0;
uint32_t MJPrivateRoom::getRoomID()
{
	return m_pRoom->getRoomID();
}

uint8_t MJPrivateRoom::getRoomType()
{
	return m_pRoom->getRoomType();
}

void MJPrivateRoom::update(float fDelta)
{
	if (m_pRoom && m_eState != eRoomState_Close )
	{
		m_pRoom->update(fDelta);
	}
}

bool MJPrivateRoom::onMessage(stMsg* prealMsg, eMsgPort eSenderPort, uint32_t nPlayerSessionID)
{
	switch (prealMsg->usMsgType)
	{
	case MSG_REQUEST_ROOM_INFO:
	{
		stMsgToRoom* pRet = (stMsgToRoom*)prealMsg;
		sendRoomInfo(nPlayerSessionID);
	}
	break;
	case MSG_PLAYER_LEAVE_ROOM:
	{
		Json::Value jsMsg;
		auto pPlayer = ((IMJRoom*)m_pRoom)->getMJPlayerBySessionID(nPlayerSessionID);
		if (!pPlayer)
		{
			LOGFMTE("you are not in room why you apply leave room id = %u ,session id = %u", getRoomID(), nPlayerSessionID);
			jsMsg["ret"] = 1;
		}
		else
		{
			jsMsg["ret"] = 0;
			onPlayerApplyLeave(pPlayer->getUID());
		}
		sendMsgToPlayer(jsMsg, MSG_PLAYER_LEAVE_ROOM, nPlayerSessionID);
	}
	break;
	default:
	{
		if (m_pRoom)
		{
			return m_pRoom->onMessage(prealMsg, eSenderPort, nPlayerSessionID);
		}
		else
		{

		}
	}
	return false;
	}

	return true;
}

bool MJPrivateRoom::onMsg(Json::Value& prealMsg, uint16_t nMsgType, eMsgPort eSenderPort, uint32_t nSessionID)
{
	if (MSG_PLAYER_LEAVE_ROOM == nMsgType)
	{
		//LOGFMTE("sub class must process this msg");
		//assert(0&&"sub class must process this msg");
		Json::Value jsMsg;
		auto pPlayer = ((IMJRoom*)m_pRoom)->getMJPlayerBySessionID(nSessionID);
		if (!pPlayer)
		{
			LOGFMTE("you are not in room why you apply leave room id = %u ,session id = %u", getRoomID(), nSessionID);
			jsMsg["ret"] = 1;
		}
		else
		{
			jsMsg["ret"] = 0;
			onPlayerApplyLeave(pPlayer->getUID());
		}
		sendMsgToPlayer(jsMsg, nMsgType, nSessionID);
		return true;;
	}

	switch (nMsgType)
	{
	case MSG_APPLY_DISMISS_VIP_ROOM:
	{
		auto pp = ((IMJRoom*)m_pRoom)->getMJPlayerBySessionID(nSessionID);
		if (pp == nullptr)
		{
			LOGFMTE("pp is null why , you apply dismiss , but , you are not sit in room, session id = %u", nSessionID);
			return true;
		}

		m_mapRecievedReply[pp->getUID()] = 1;
		if (m_bWaitDismissReply)
		{
			LOGFMTE("already waiting reply %u why you go on apply ?", pp->getUID() );
			onCheckDismissReply(false);
		}
		else
		{
			auto pRoom = (IMJRoom*)m_pRoom;
			bool bNotOpen = (m_bComsumedRoomCards == false) && (pRoom->getCurRoomState()->getStateID() == eRoomSate_WaitReady);
			if ( bNotOpen == false )
			{
				Json::Value jsMsg;
				jsMsg["applyerIdx"] = pp->getIdx();
				sendRoomMsg(jsMsg, MSG_ROOM_APPLY_DISMISS_VIP_ROOM);
			}

			m_tWaitRepklyTimer.reset();
			m_tWaitRepklyTimer.setInterval(TIME_WAIT_REPLY_DISMISS);
			m_tWaitRepklyTimer.setIsAutoRepeat(false);
			m_tWaitRepklyTimer.setCallBack([this](CTimer*p, float f){

				onCheckDismissReply(true);
			});
			m_tWaitRepklyTimer.start();
			m_bWaitDismissReply = true;
			onCheckDismissReply(false);
			m_tInvokerTime = time(nullptr);
			m_nInvokerDismissUID = pp->getUID();
		}
	}
	break;
	case MSG_PLAYER_CHAT_MSG:
	{
		auto pp = ((IMJRoom*)m_pRoom)->getMJPlayerBySessionID(nSessionID);
		if (pp == nullptr)
		{
			prealMsg["ret"] = 1;
			sendMsgToPlayer(prealMsg, MSG_PLAYER_CHAT_MSG, nSessionID);
			break;
		}

		prealMsg["playerIdx"] = pp->getIdx();
		sendRoomMsg(prealMsg, MSG_ROOM_CHAT_MSG);
	}
	break;
	case MSG_REPLY_DISSMISS_VIP_ROOM_APPLY:
	{
		if (!m_bWaitDismissReply)
		{
			LOGFMTE("nobody apply to dismiss room ,why you reply ? session id = %u", nSessionID);
			return true;
		}

		auto pp = ((IMJRoom*)m_pRoom)->getMJPlayerBySessionID(nSessionID);
		if (pp == nullptr)
		{
			LOGFMTE("pp is null why , you apply dismiss , but , you are not sit in room, session id = %u", nSessionID);
			return true;
		}

		LOGFMTD("received player session id = %u , reply dismiss ret = %u", nSessionID, prealMsg["reply"].asUInt());
		m_mapRecievedReply[pp->getUID()] = prealMsg["reply"].asUInt();

		Json::Value jsMsg;
		jsMsg["idx"] = pp->getIdx();
		jsMsg["reply"] = prealMsg["reply"];
		sendRoomMsg(jsMsg, MSG_ROOM_REPLY_DISSMISS_VIP_ROOM_APPLY);

		onCheckDismissReply(false);
	}
	break;
	default:
		if (m_pRoom)
		{
			return m_pRoom->onMsg(prealMsg, nMsgType, eSenderPort, nSessionID);
		}
	}

	return true;
}

void MJPrivateRoom::sendRoomMsg(Json::Value& prealMsg, uint16_t nMsgType)
{
	if (m_pRoom)
	{
		m_pRoom->sendRoomMsg(prealMsg,nMsgType);
	}
}

void MJPrivateRoom::sendMsgToPlayer(Json::Value& prealMsg, uint16_t nMsgType, uint32_t nSessionID)
{
	if (m_pRoom)
	{
		m_pRoom->sendMsgToPlayer(prealMsg, nMsgType, nSessionID);
	}
}

stBaseRoomConfig* MJPrivateRoom::getRoomConfig()
{
	return &m_stConfig;
}

void MJPrivateRoom::sendRoomInfo(uint32_t nSessionID)
{
	if (m_pRoom)
	{
		m_pRoom->sendRoomInfo(nSessionID);
	}
	else
	{
		LOGFMTE("private room core is null , can not send detail info");
		return;
	}

	LOGFMTD("send vip room info ext to player session id = %u", nSessionID);
	Json::Value jsMsg;
	jsMsg["leftCircle"] = m_nLeftCircle;
	jsMsg["baseBet"] = m_stConfig.nBaseBet;
	jsMsg["creatorUID"] = m_nOwnerUID;
	jsMsg["initCoin"] = m_nInitCoin;
	jsMsg["initCircle"] = m_nInitCircle;
	jsMsg["roomType"] = m_pRoom->getRoomType();
	jsMsg["chatID"] = m_nChatID;
	jsMsg["isAA"] = m_isAA ? 1 : 0;
	//jsMsg["isCircle"] = m_isCircleType ? 1 : 0;
	// is waiting vote dismiss room ;
	jsMsg["isWaitingDismiss"] = m_bWaitDismissReply ? 1 : 0;
	int32_t nLeftSec = 0;
	if (m_bWaitDismissReply)
	{
		jsMsg["applyDismissUID"] = m_nInvokerDismissUID;
		// find argee idxs ;
		Json::Value jsArgee;
		for (auto& ref : m_mapRecievedReply)
		{
			auto p = ((IMJRoom*)m_pRoom)->getMJPlayerByUID(ref.first);
			if (!p)
			{
				LOGFMTE("%u you are not in room but you reply dissmiss room ", ref.first);
				continue;
			}
			jsArgee[jsArgee.size()] = p->getIdx();
		}

		jsMsg["agreeIdxs"] = jsArgee;

		// caclulate wait time ;
		auto nEsT = time(nullptr) - m_tInvokerTime;
		if (nEsT > TIME_WAIT_REPLY_DISMISS)
		{
			nLeftSec = 1;
		}
		else
		{
			nLeftSec = TIME_WAIT_REPLY_DISMISS - nEsT;
		}
	}

	jsMsg["leftWaitTime"] = nLeftSec;

	sendMsgToPlayer(jsMsg, MSG_VIP_ROOM_INFO_EXT, nSessionID);
}

void MJPrivateRoom::onCheckDismissReply(bool bTimerOut)
{
	auto pRoom = (IMJRoom*)m_pRoom;
	bool bNotOpen = (m_bComsumedRoomCards == false) && (pRoom->getCurRoomState()->getStateID() == eRoomSate_WaitReady);
	if (bNotOpen)
	{
		auto iter = m_mapRecievedReply.find(m_nOwnerUID);
		if (iter != m_mapRecievedReply.end())
		{
			if (iter->second == 1 )
			{
				onRoomGameOver(true);
				m_mapRecievedReply.clear();
				m_bWaitDismissReply = false;
				m_tWaitRepklyTimer.canncel();
				LOGFMTD("room id = %u room owner dismiss room when not open direct dissmiss ", getRoomID());
				return;
			}
		}
	}

	// normal tou piao 
	uint8_t nAgreeCnt = 0;
	uint8_t nDisAgreeCnt = 0;
	for (auto& ref : m_mapRecievedReply)
	{
		if ( ref.second == 0 )
		{
			++nDisAgreeCnt;
		}
		else
		{
			++nAgreeCnt;
		}
	}

	uint8_t nSeatCnt = ((IMJRoom*)m_pRoom)->getSeatCnt();
	for (uint8_t nIdx = 0; nIdx < nSeatCnt; ++nIdx)
	{
		auto pp = pRoom->getMJPlayerByIdx(nIdx);
		if (nullptr == pp)
		{
			++nAgreeCnt;
		}
	}

	//process result
	if (nAgreeCnt * 2 > nSeatCnt || bTimerOut )
	{
		LOGFMTD("most player want dismiss room");
		onRoomGameOver(true);
	}
	else if (nDisAgreeCnt >= 1)
	{
		LOGFMTD("most player do not want dismiss room");
	}
	else
	{
		// go on wait more player reply ;
		return;
	}

	m_mapRecievedReply.clear();
	m_bWaitDismissReply = false;
	m_tWaitRepklyTimer.canncel();
	return;

	//if (bTimerOut)
	//{
	//	if ((nSeatCnt - nDisAgreeCnt) * 2 > nSeatCnt)  // not disagree means agree ;
	//	{
	//		LOGFMTD("most player want dismiss room time out");
	//		onRoomGameOver(true);
	//	}
	//}
	//else
	//{
	//	if (nAgreeCnt * 2 > nSeatCnt)
	//	{
	//		LOGFMTD("most player want dismiss room");
	//		onRoomGameOver(true);
	//	}
	//	else if (nDisAgreeCnt >= 1 )
	//	{
	//		LOGFMTD("most player do not want dismiss room");
	//		m_mapRecievedReply.clear();
	//		m_bWaitDismissReply = false;
	//		m_tWaitRepklyTimer.canncel();
	//		return;
	//	}
	//	else
	//	{
	//		// go on wait more player reply ;
	//		return;
	//	}
	//}

	//m_mapRecievedReply.clear();
	//m_bWaitDismissReply = false;
	//m_tWaitRepklyTimer.canncel();
}

void MJPrivateRoom::onDidGameOver(IMJRoom* pRoom)
{
	if (m_bDoDismissRoom)
	{
		LOGFMTD("do dismiss room so skip this game over event");
		return;
	}

	if (m_bComsumedRoomCards == false)
	{
		m_bComsumedRoomCards = true;
		// comsum room card ;
		auto nCardCnt = getDiamondNeed();
		if ( nCardCnt > 0 )
		{
			LOGFMTD("send msg to consumed vip room card = %u", nCardCnt);
			Json::Value jsConsumed;
			jsConsumed["cardCnt"] = nCardCnt;
			jsConsumed["uid"] = m_nOwnerUID;
			m_pRoomMgr->sendMsg(jsConsumed, MSG_CONSUM_VIP_ROOM_CARDS, 0, ID_MSG_PORT_DATA);

			// if aa kou ka  , kou every one
			if ( m_isAA )
			{
				for (uint8_t nSeatIdx = 0; nSeatIdx < m_pRoom->getSeatCnt(); ++nSeatIdx)
				{
					auto pPlayer = m_pRoom->getMJPlayerByIdx(nSeatIdx);
					if (pPlayer == nullptr)
					{
						LOGFMTE( "player idx = %u is null in room id = %u , comsume diamond error",nSeatIdx,m_pRoom->getRoomID() );
						continue;
					}

					if (m_nOwnerUID == pPlayer->getUID())
					{
						continue;
					}

					Json::Value jsConsumed;
					jsConsumed["cardCnt"] = nCardCnt;
					jsConsumed["uid"] = pPlayer->getUID();
					m_pRoomMgr->sendMsg(jsConsumed, MSG_CONSUM_VIP_ROOM_CARDS, 0, ID_MSG_PORT_DATA);
				}
			}
		}
	}

	// decrease circle ;
	std::vector<uint8_t> vLoseOver;
	if (m_pRoom->isOneCirleEnd())
	{
		--m_nLeftCircle;
	}

	if (m_nLeftCircle > 0 && m_pRoom->isInternalShouldClosedAll() == false )
	{
		LOGFMTD("vip room not over , leftCircle = %u ", m_nLeftCircle );
		return;
	}

	LOGFMTD("vip room over room id = %u ", getRoomID());
	// on game over ;
	onRoomGameOver(false);
}

bool MJPrivateRoom::isLastCircle()
{
	return m_nLeftCircle == 1;
}

uint8_t MJPrivateRoom::getDiamondNeed()
{
	if ( m_isForFree)
	{
		return 0;
	}

	uint16_t nCardCnt = 2;//m_nInitCircle * ROOM_CARD_CNT_PER_CIRLE_NJMJ; // { 2, 3, 6}
	if (m_pRoom->getRoomType() == eRoom_MJ_CaoYang || m_pRoom->getRoomType() == eRoom_MJ_BeiPiao)
	{
		if (2 == m_nInitCircle || (8 == m_nInitCircle /*&& !m_isCircleType*/))
		{
			nCardCnt = 3;
		}
		else // 4 quan
		{
			nCardCnt = 6;
		}
	} 
	else
	{
		if (1 == m_nInitCircle || (4 == m_nInitCircle /*&& !m_isCircleType*/))
		{
			nCardCnt = 2;
		}
		else if (2 == m_nInitCircle || (8 == m_nInitCircle /*&& !m_isCircleType*/))
		{
			nCardCnt = 4;
		}
		else // 4 quan
		{
			nCardCnt = 8;
		}
	}

	if ( m_isAA )
	{
		nCardCnt = 1;
		if ( 16 == m_nInitCircle /*&& !m_isCircleType*/)
		{
			nCardCnt = 2;
		}
	}
	return nCardCnt;
}

void MJPrivateRoom::onRoomGameOver(bool isDismissed)
{
	// do close room ;
	m_eState = eRoomState_Close;
	if (isDismissed)
	{
		m_bDoDismissRoom = true;
	}
	// all player leave and update coin 
	auto pRoom = (IMJRoom*)m_pRoom;
	bool bCanncelBill = (m_bComsumedRoomCards == false) && (pRoom->getCurRoomState()->getStateID() == eRoomSate_WaitReady);
	if ( m_bDoDismissRoom && ( pRoom->getCurRoomState()->getStateID() != eRoomSate_WaitReady ) )
	{
		pRoom->onGameEnd();
	}

	for (uint8_t nIdx = 0; nIdx < pRoom->getSeatCnt(); ++nIdx)
	{
		auto pp = (IMJPlayer*)pRoom->getMJPlayerByIdx(nIdx);
		if (pp == nullptr)
		{
			LOGFMTE("why private player is null room id = %u idx = %u" ,getRoomID(),nIdx);
			continue;
		}

		auto iter = m_vAllPlayers.find(pp->getUID());
		if (iter == m_vAllPlayers.end())
		{
			LOGFMTE("why player record is null , but still played in private room ? room id = %u , uid = %u , coin = %u",getRoomID(),pp->getUID(),pp->getCoin() );
			continue;
		}

		// kou chu room fee , if is jing yuan zi 95
		iter->second.nRoomCoin = pp->getCoin();
		iter->second.nAnGangCnt = pp->getAnGangCnt();
		iter->second.nDianPaoCnt = pp->getDianPaoCnt();
		iter->second.nHuCnt = pp->getHuCnt();
		iter->second.nWaiBaoCoin = 0;
		//if (eRoom_MJ_NanJing == pRoom->getRoomType())
		//{
		//	iter->second.nWaiBaoCoin = ((NJMJPlayer*)pp)->getWaiBaoCoin();
		//}
		iter->second.nMingGangCnt = pp->getMingGangCnt();
		iter->second.nZiMoCnt = pp->getZiMoCnt();

		// do leave room and tell data svr ;
		stMsgSvrDoLeaveRoom msgdoLeave;
		msgdoLeave.nCoin = iter->second.nRealCoin;
		msgdoLeave.nGameType = getRoomType();
		msgdoLeave.nRoomID = getRoomID();
		msgdoLeave.nUserUID = pp->getUID();
		m_pRoomMgr->sendMsg(&msgdoLeave, sizeof(msgdoLeave), pp->getUID());
	}

	
	
	// send room bills ;
	if ( !bCanncelBill )
	{
		Json::Value jsMsg;
		jsMsg["ret"] = isDismissed ? 1 : 0;
		jsMsg["initCoin"] = m_nInitCoin;

		// sys bill id to data svr 
		stMsgSyncPrivateRoomResult msgResult;
		msgResult.nRoomID = getRoomID();
		msgResult.nCreatorUID = m_nOwnerUID;
		msgResult.nClubID = 0;
		msgResult.nDuringTimeSeconds = m_nInitCircle;
		msgResult.nSiealNum = getSeiralNum();
		memset(msgResult.cRoomName, 0, sizeof(msgResult.cRoomName));

		// add room recorder 
		auto pRoomRecorder = getRoomRecorder();

		Json::Value jsVBills;
		Json::Value jsPlayedPlayers;
		for (auto& ref : m_vAllPlayers)
		{
			Json::Value jsPlayer;
			jsPlayer["uid"] = ref.second.nUID;
			jsPlayer["curCoin"] = ref.second.nRoomCoin;
			jsPlayer["ziMoCnt"] = ref.second.nZiMoCnt;
			jsPlayer["huCnt"] = ref.second.nHuCnt;
			jsPlayer["dianPaoCnt"] = ref.second.nDianPaoCnt;
			jsPlayer["mingGangCnt"] = ref.second.nMingGangCnt;
			jsPlayer["AnGangCnt"] = ref.second.nAnGangCnt;
			jsPlayer["waiBaoCoin"] = ref.second.nWaiBaoCoin;

			jsVBills[jsVBills.size()] = jsPlayer;

			jsPlayedPlayers[jsPlayedPlayers.size()] = ref.second.nUID;

			// to data svr msg 
			msgResult.nOffset = ref.second.nRoomCoin - m_nInitCoin;
			msgResult.nTargetPlayerUID = ref.second.nUID;
			msgResult.nFinalCoin = ref.second.nRoomCoin;
			msgResult.nBuyIn = m_nInitCoin;
			msgResult.nBaseBet = 1;
			m_pRoomMgr->sendMsg(&msgResult, sizeof(msgResult), 0);
		}
		jsMsg["bills"] = jsVBills;
		// add room recorder to game recorder mg ;
		auto pRecorderMgr = CMJServerApp::getInstance()->getRecorderMgr();
		pRecorderMgr->addRoomRecorder(pRoomRecorder,true);

		for (auto& ref : m_vAllPlayers)
		{
			if (ref.second.nSessionID)
			{
				LOGFMTD("send game room over bill to session id = %u", ref.second.nSessionID);
				m_pRoomMgr->sendMsg(jsMsg, MSG_VIP_ROOM_GAME_OVER, ref.second.nSessionID);
			}
		}

		// add vip room bill 
		auto pBill = ((MJRoomManager*)m_pRoomMgr)->createVipRoomBill();
		pBill->nBillID = pRoom->getSeiralNum();
		pBill->jsDetail = jsVBills;
		pBill->nBillTime = (uint32_t)time(nullptr);
		pBill->nCreateUID = m_nOwnerUID;
		pBill->nRoomID = getRoomID();
		pBill->nRoomType = getRoomType();
		pBill->nRoomInitCoin = m_nInitCoin;
		pBill->nCircleCnt = m_nInitCircle /*- m_nLeftCircle*/;
		((MJRoomManager*)m_pRoomMgr)->addVipRoomBill(pBill, true);
	}
	
	Json::Value jsClosed;
	jsClosed["uid"] = m_nOwnerUID;
	jsClosed["roomID"] = getRoomID();
	jsClosed["eType"] = getRoomType();
	m_pRoomMgr->sendMsg(jsClosed, MSG_VIP_ROOM_CLOSED, 0, ID_MSG_PORT_DATA);

	if ( !bCanncelBill )
	{
		// will delete this room ;
		((MJRoomManager*)m_pRoomMgr)->addWillDeleteRoomID(getRoomID());
		return;
	}
	// tell client closed room ;
	Json::Value jsDoClosed;
	jsDoClosed["roomID"] = getRoomID();
	jsDoClosed["eType"] = getRoomType();
	jsDoClosed["isDismiss"] = isDismissed ? 1 : 0;
	pRoom->sendRoomMsg(jsDoClosed, MSG_VIP_ROOM_DO_CLOSED);
	// will delete this room ;
	((MJRoomManager*)m_pRoomMgr)->addWillDeleteRoomID(getRoomID());
}

uint32_t MJPrivateRoom::getCoinNeedToSitDown()
{
	return m_pRoom->getCoinNeedToSitDown();
}

IMJRoom* MJPrivateRoom::doCreateMJRoom(eRoomType eMJType)
{
	switch (eMJType )
	{
	//case eRoom_MJ_Blood_River:
	//{
	//	return new XLMJRoom();
	//}
	//break;
	//case eRoom_MJ_Blood_End:
	//{
	//	return new XZMJRoom();
	//}
	//break;
	//case eRoom_MJ_NanJing:
	//{
	//	return new NJMJRoom();
	//}
	//break;
	//case eRoom_MJ_SuZhou:
	//{
	//	return new SZMJRoom();
	//}
	//break;
	case eRoom_MJ_CaoYang:
	{
		return new CYMJRoom();
	}
	case eRoom_MJ_HongHe:
	{
		return new HHMJRoom();
	}
	case eRoom_MJ_JianShui:
	{
		return new JSMJRoom();
	}
	case eRoom_MJ_YangZhou:
	{
		return new YZMJRoom();
	}
	case eRoom_MJ_BeiPiao:
	{
		return new BPMJRoom();
	}
	default:
		LOGFMTE("unknown mj room type = %u, can not create room for private room ",eMJType);
		return nullptr;
	}
	return nullptr;
}

uint32_t MJPrivateRoom::getSeiralNum()
{
	return m_pRoom->getSeiralNum();
}

std::shared_ptr<IGameRoomRecorder> MJPrivateRoom::getRoomRecorder()
{
	return m_pRoom->getRoomRecorder();
}