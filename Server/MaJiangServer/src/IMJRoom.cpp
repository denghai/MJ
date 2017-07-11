#include "IMJRoom.h"
#include "IMJRoomState.h"
#include "RoomConfig.h"
#include <cassert>
#include "IRoomManager.h"
#include "log4z.h"
#include "IMJPlayer.h"
#include "IMJPlayerCard.h"
#include "IMJPoker.h"
#include "IGameRoomManager.h"
#include "RobotDispatchStrategy.h"
#include "MJCard.h"
#include "MJReplayFrameType.h"
#include "MJServer.h"
IMJRoom::~IMJRoom()
{
	for (auto& ref : m_vMJPlayers)
	{
		delete ref;
		ref = nullptr;
	}

	for (auto& ref : m_vRoomStates)
	{
		if (ref.second)
		{
			delete ref.second;
			ref.second = nullptr;
		}
	}
}

bool IMJRoom::init(IGameRoomManager* pRoomMgr, stBaseRoomConfig* pConfig, uint32_t nSeialNum, uint32_t nRoomID, Json::Value& vJsValue)
{
	// zero data 
	memset(m_vMJPlayers, 0, sizeof(m_vMJPlayers));
	m_pCurState = nullptr;
	m_pRoomMgr = pRoomMgr;
	m_nRoomID = nRoomID;
	m_nSeiralNum = nSeialNum;
	m_pRoomConfig = (stNiuNiuRoomConfig*)pConfig;
	if (MAX_SEAT_CNT < ((stNiuNiuRoomConfig*)m_pRoomConfig)->nMaxSeat)
	{
		assert(0 && "invalid seat cnt");
		((stNiuNiuRoomConfig*)m_pRoomConfig)->nMaxSeat = MAX_SEAT_CNT;
	}
	setBankIdx(-1);
	m_ptrGameRecorder = createRoomRecorder();
	m_ptrGameRecorder->init(nSeialNum, vJsValue["circle"].asUInt(),nRoomID, getRoomType(),vJsValue["createUID"].asUInt() );

	m_ptrGameReplay = std::make_shared<MJReplayGame>();
	return true;
}

bool IMJRoom::onPlayerEnter(stEnterRoomData* pEnterRoomPlayer)
{
	// check if already in room ;
	auto player = getMJPlayerByUID(pEnterRoomPlayer->nUserUID);
	if (player)
	{
		LOGFMTD("player already in this room just reactive Room id = %u , uid = %u, coin do not refresh = %u , enter coin = %u",getRoomID(),pEnterRoomPlayer->nUserUID,player->getCoin(),pEnterRoomPlayer->nCoin);
		pEnterRoomPlayer->nCoin = player->getCoin();
		player->onComeBackRoom(pEnterRoomPlayer);

		// msg ;
		Json::Value jsMsg;
		jsMsg["idx"] = player->getIdx();
		jsMsg["uid"] = player->getUID();
		jsMsg["coin"] = player->getCoin();
		jsMsg["state"] = player->getState();
		jsMsg["isOnLine"] = player->isOnline() ? 1 : 0;
		sendRoomMsg(jsMsg, MSG_ROOM_PLAYER_ENTER);
		return true;
	}

	uint8_t nEmptyIdx = -1;
	for (uint8_t nIdx = 0; nIdx < getSeatCnt(); ++nIdx)
	{
		if (m_vMJPlayers[nIdx] == nullptr)
		{
			nEmptyIdx = nIdx;
			break;
		}
	}

	if (nEmptyIdx == (uint8_t)-1)
	{
		return false;
	}

	auto pMJPlayer = doCreateMJPlayer();
	pMJPlayer->init(pEnterRoomPlayer);
	pMJPlayer->setIdx(nEmptyIdx);
	sitdown(pMJPlayer, nEmptyIdx);
	return true;
}

uint8_t IMJRoom::checkPlayerCanEnter(stEnterRoomData* pEnterRoomPlayer)
{
	auto pPlayer = getMJPlayerByUID(pEnterRoomPlayer->nUserUID);
	if (pPlayer)
	{
		return 0;
	}

	if (isRoomFull())
	{
		return 7;
	}
	return 0;
}

bool IMJRoom::isRoomFull()
{
	uint8_t nSeatCnt = getSeatCnt() < MAX_SEAT_CNT ? getSeatCnt() : MAX_SEAT_CNT;
	for (uint8_t nIdx = 0; nIdx < nSeatCnt; ++nIdx)
	{
		if (m_vMJPlayers[nIdx] == nullptr)
		{
			return false;
		}
	}
	return true;
}

void IMJRoom::sendRoomInfo(uint32_t nSessionID)
{
	Json::Value jsMsg;
	jsMsg["roomID"] = getRoomID();
	jsMsg["configID"] = getRoomConfig()->nConfigID;
	jsMsg["roomState"] = getCurRoomState()->getStateID();
	Json::Value arrPlayers;
	for (auto& pPlayer : m_vMJPlayers )
	{
		if (pPlayer == nullptr)
		{
			continue;
		}
		Json::Value jsPlayer;
		jsPlayer["idx"] = pPlayer->getIdx();
		jsPlayer["uid"] = pPlayer->getUID();
		jsPlayer["coin"] = pPlayer->getCoin();
		jsPlayer["state"] = pPlayer->getState();
		jsPlayer["isOnline"] = pPlayer->isOnline() ? 1 : 0;
		arrPlayers[pPlayer->getIdx()] = jsPlayer;
	}

	jsMsg["waitTimer"] = 0;
	if (getCurRoomState()->getStateID() == eRoomState_WaitPlayerAct)
	{
		jsMsg["waitTimer"] = getCurRoomState()->getStateDuring();
	}

	jsMsg["players"] = arrPlayers;

	jsMsg["bankerIdx"] = getBankerIdx();
	jsMsg["curActIdex"] = getCurRoomState()->getCurIdx();
	jsMsg["leftCardCnt"] = getMJPoker()->getLeftCardCount();

	Json::Value jsSubInfo;
	getSubRoomInfo(jsSubInfo);
	jsMsg["subInfo"] = jsSubInfo;

	sendMsgToPlayer(jsMsg, MSG_MJ_ROOM_INFO, nSessionID);
	LOGFMTD("send msg room info msg to player session id = %u", nSessionID);

	if (getCurRoomState()->getStateID() == eRoomSate_WaitReady || eRoomState_GameEnd == getCurRoomState()->getStateID())
	{
		LOGFMTD("current room not start game , so need not send runtime info msg");
		return;
	}
	sendPlayersCardInfo(nSessionID);
}

void IMJRoom::sendPlayersCardInfo(uint32_t nSessionID)
{
	if (getCurRoomState()->getStateID() == eRoomSate_WaitReady || eRoomState_GameEnd == getCurRoomState()->getStateID())
	{
		LOGFMTD("current room not start game , so need not send runtime info msg");
		return;
	}
	Json::Value jsmsg;
	Json::Value vPeerCards;
	for (auto& pp : m_vMJPlayers )
	{
		if (pp == nullptr /*|| pp->haveState(eRoomPeer_CanAct) == false*/)  // lose also have card 
		{
			continue;
		}

		auto pCard = pp->getPlayerCard();
		// svr : { bankerIdx : 2, leftCardCnt : 32 ,playersCard: [ { idx : 2,queType: 2, anPai : [2,3,4,34], mingPai : [ 23,67,32] , huPai : [1,34], chuPai: [2,34,4] },{ anPai : [2,3,4,34], mingPai : [ 23,67,32] , huPai : [1,34] }, .... ] }
		// { idx : 2,queType: 2, anPai : [2,3,4,34], anGangPai: [2,3,5], mingPai : [ 23,67,32] , huPai : [1,34], chuPai: [2,34,4] }
		Json::Value jsCardInfo;
		jsCardInfo["idx"] = pp->getIdx();
		jsCardInfo["queType"] = 0;

		IMJPlayerCard::VEC_CARD vAnPai, vMingPai, vChuPai ,vAnGangedCard, temp;
		pCard->getHoldCard(vAnPai);
		pCard->getChuedCard(vChuPai);
		pCard->getAnGangedCard(vAnGangedCard);
		
		pCard->getEatedCard(vMingPai);
		pCard->getPengedCard(temp);
		for (auto& pc : temp)
		{
			vMingPai.push_back(pc); vMingPai.push_back(pc); vMingPai.push_back(pc);
		}
		temp.clear();
		pCard->getMingGangedCard(temp);
		for (auto& pc : temp)
		{
			vMingPai.push_back(pc); vMingPai.push_back(pc); vMingPai.push_back(pc); vMingPai.push_back(pc);
		}
		temp.clear();

		auto toJs = [](IMJPlayerCard::VEC_CARD& vCards, Json::Value& js )
		{
			for (auto& c : vCards)
			{
				js[js.size()] = c;
			}
		};

		Json::Value jsMingPai, jsAnPai, jsChuPai, jaHupai, jsAngangedPai;
		toJs(vMingPai, jsMingPai); toJs(vAnPai, jsAnPai); toJs(vChuPai, jsChuPai); toJs(vAnGangedCard,jsAngangedPai);
		jsCardInfo["mingPai"] = jsMingPai; jsCardInfo["anPai"] = jsAnPai; jsCardInfo["chuPai"] = jsChuPai; jsCardInfo["huPai"] = jaHupai;
		jsCardInfo["anGangPai"] = jsAngangedPai;
		//vPeerCards[vPeerCards.size()] = jsCardInfo;

		sendMsgToPlayer(jsCardInfo, MSG_ROOM_PLAYER_CARD_INFO, nSessionID);
	}

	//jsmsg["playersCard"] = vPeerCards;
	//jsmsg["bankerIdx"] = getBankerIdx();
	//jsmsg["curActIdex"] = getCurRoomState()->getCurIdx();
	//jsmsg["leftCardCnt"] = getMJPoker()->getLeftCardCount();
	/*sendMsgToPlayer(jsmsg, MSG_ROOM_PLAYER_CARD_INFO, nSessionID);*/
	LOGFMTD("send player card infos !");
}

uint32_t IMJRoom::getRoomID()
{
	return m_nRoomID;
}

uint32_t IMJRoom::getSeiralNum()
{
	return m_nSeiralNum;
}

void IMJRoom::update(float fDelta)
{
	getCurRoomState()->update(fDelta);
}

bool IMJRoom::onMessage(stMsg* prealMsg, eMsgPort eSenderPort, uint32_t nSessionID)
{
	if (MSG_PLAYER_LEAVE_ROOM == prealMsg->usMsgType)
	{
		//LOGFMTE("sub class must process this msg");
		//assert(0&&"sub class must process this msg");
		Json::Value jsMsg;
		auto pPlayer = getMJPlayerBySessionID(nSessionID);
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
		sendMsgToPlayer(jsMsg, prealMsg->usMsgType, nSessionID);
		LOGFMTI("返回玩家离开房间的消息， sesssioniID = %u", nSessionID);
		return true;;
	}
	else if (MSG_INFORM_PLAYER_ONLINE_STATE == prealMsg->usMsgType)
	{
		auto pMsg = (stMsgInformPlayerOnlineState*)prealMsg;
		auto pPlayer = getMJPlayerByUID(pMsg->nUID);
		if (pPlayer == nullptr)
		{
			LOGFMTE("player uid = %u not in room id = %u so can not update player net state",pMsg->nUID,getRoomID());
			return true;
		}
		pPlayer->setIsOnline(pMsg->isOnline);

		Json::Value jsmsg;
		jsmsg["idx"] = pPlayer->getIdx();
		jsmsg["isOnLine"] = pMsg->isOnline ? 1 : 0;
		sendRoomMsg(jsmsg, MSG_ROOM_UPDATE_PLAYER_NET_STATE);
		return true;
	}

	return getCurRoomState()->onMessage(prealMsg, eSenderPort, nSessionID);
}

bool IMJRoom::onMsg(Json::Value& prealMsg, uint16_t nMsgType, eMsgPort eSenderPort, uint32_t nSessionID)
{
	if ( MSG_PLAYER_ACT == nMsgType )
	{
		auto actType = prealMsg["actType"].asUInt();
		auto p = getMJPlayerBySessionID(nSessionID);
		if (p == nullptr)
		{
			LOGFMTE("room id = %u why do act player is null ptr",getRoomID() );
			return true;
		}

		uint8_t nFrameType = -1;
		if (eMJAct_Pass == actType)
		{
			nFrameType = eMJFrame_Pass;
		}
		else if (eMJAct_Hu == actType)
		{
			nFrameType = eMJFrame_Hu;
		}
		else if (eMJAct_Peng == actType)
		{
			nFrameType = eMJFrame_Decl_Peng;
		}
		else if (eMJAct_MingGang == actType)
		{
			nFrameType = eMJFrame_Decl_MingGang;
		}
		else if (eMJAct_BuGang == actType || eMJAct_BuGang_Declare == actType || eMJAct_BuGang_Pre == actType)
		{
			nFrameType = eMJFrame_Decl_BuGang;
		}
		//else if ( eMJAct_Ting == actType)
		//{
		//	nFrameType = eMJFrame_Player_Ting;
		//}
			 
		if ((uint8_t)-1 != nFrameType)
		{
			Json::Value jsFrameArg;
			auto ptrReplay = getGameReplay()->createFrame(nFrameType, (uint32_t)time(nullptr));
			jsFrameArg["idx"] = p->getIdx();
			ptrReplay->setFrameArg(jsFrameArg);
			getGameReplay()->addFrame(ptrReplay);
		}
	}

	if ( MSG_PLAYER_LEAVE_ROOM == nMsgType )
	{
		//LOGFMTE("sub class must process this msg");
		//assert(0&&"sub class must process this msg");
		Json::Value jsMsg;
		auto pPlayer = getMJPlayerBySessionID(nSessionID);
		if (!pPlayer)
		{
			LOGFMTE("you are not in room why you apply leave room id = %u ,session id = %u",getRoomID(),nSessionID );
			jsMsg["ret"] = 1;
		}
		else
		{
			jsMsg["ret"] = 0;
			onPlayerApplyLeave(pPlayer->getUID());
		}
		sendMsgToPlayer(jsMsg, nMsgType, nSessionID);
		LOGFMTI("返回玩家离开房间的消息， sesssioniID = %u", nSessionID);
		return true;;
	}
	else if ( MSG_SET_NEXT_CARD == nMsgType )
	{
		if (prealMsg["card"].isNull() || prealMsg["card"].isUInt() == false)
		{
			LOGFMTE( "MSG_SET_NEXT_CARD key is null or invalid" );
			return true;
		}
		getMJPoker()->pushCardToFron(prealMsg["card"].asUInt());
		return true ;
	}

	if ( MSG_REQUEST_ROOM_INFO == nMsgType)
	{
		LOGFMTD("reback room state and info msg to session id =%u", nSessionID);
		sendRoomInfo(nSessionID);
		return true;
	}
 
	return getCurRoomState()->onMsg(prealMsg,nMsgType,eSenderPort,nSessionID);
}

void IMJRoom::sendRoomMsg(Json::Value& prealMsg, uint16_t nMsgType)
{
	for (auto& ref : m_vMJPlayers)
	{
		if (ref && ref->isTempLeaveRoom() == false )
		{
			sendMsgToPlayer(prealMsg,nMsgType,ref->getSessionID());
		}
	}
}

void IMJRoom::sendMsgToPlayer(Json::Value& prealMsg, uint16_t nMsgType, uint32_t nSessionID)
{
	getRoomMgr()->sendMsg(prealMsg, nMsgType, nSessionID);
}

void IMJRoom::sendMsgToPlayer(stMsg* pmsg, uint16_t nLen, uint32_t nSessionID)
{
	getRoomMgr()->sendMsg(pmsg, nLen, nSessionID);
}

bool IMJRoom::sitdown(IMJPlayer* pPlayer, uint8_t nIdx)
{
	if (nIdx >= getSeatCnt())
	{
		assert("invalid sit down idx , please delete player obj");
		return false;
	}

	if (m_vMJPlayers[nIdx])
	{
		LOGFMTE("this pos already have player , find other seat nidx = %u",nIdx);
		return false;
	}
	m_vMJPlayers[nIdx] = pPlayer;
	pPlayer->setIdx(nIdx);

	// msg ;
	Json::Value jsMsg;
	jsMsg["idx"] = nIdx;
	jsMsg["uid"] = pPlayer->getUID();
	jsMsg["coin"] = pPlayer->getCoin();
	jsMsg["state"] = pPlayer->getState();
	jsMsg["isOnLine"] = pPlayer->isOnline() ? 1 : 0;
	sendRoomMsg(jsMsg, MSG_ROOM_PLAYER_ENTER);
	return true;
}

void IMJRoom::changeIdx()
{
	for (int i = 0; i < MAX_SEAT_CNT; ++i)
	{
		int nRand = rand() % MAX_SEAT_CNT;
		if (i == nRand)
		{
			continue;
		}

		auto pPlayer = m_vMJPlayers[i];
		auto nPlayerIdx = m_vMJPlayers[i]->getIdx();
		
		m_vMJPlayers[i] = m_vMJPlayers[nRand];
		m_vMJPlayers[i]->setIdx(i);

		m_vMJPlayers[nRand] = pPlayer;
		m_vMJPlayers[nRand]->setIdx(nRand);
	}

	//Json::Value jsMsg, jsPlayers;
	//for (int i = 0; i < MAX_SEAT_CNT; ++i)
	//{
	//	Json::Value jsPlayer;
	//	jsPlayer["idx"] = m_vMJPlayers[i]->getIdx();
	//	jsPlayer["uid"] = m_vMJPlayers[i]->getUID();
	//	jsPlayers[jsPlayers.size()] = jsPlayer;
	//}
	//jsMsg["players"] = jsPlayers;
	//sendRoomMsg(jsMsg, MSG_ROOM_CHANGE_IDX);

	for (int i = 0; i < MAX_SEAT_CNT; ++i)
	{
		sendRoomInfo(m_vMJPlayers[i]->getSessionID());
	}
}

bool IMJRoom::standup(uint32_t nUID)
{
	for (auto& ref : m_vMJPlayers)
	{
		if ( ref && ref->getUID() == nUID)
		{
			// msg ;
			Json::Value jsMsg;
			jsMsg["idx"] = ref->getIdx();
			jsMsg["isExit"] = 1;
			sendRoomMsg(jsMsg, MSG_ROOM_PLAYER_LEAVE);

			delete ref;
			ref = nullptr;
			return true;
		}
	}
	LOGFMTE("uid = %u , not sit down can not standup",nUID);
	return false;
}

uint8_t IMJRoom::getSeatCnt()
{
	return (uint8_t)((stSitableRoomConfig*)getRoomConfig())->nMaxSeat;
}

IMJPlayer* IMJRoom::getMJPlayerBySessionID(uint32_t nSessionid)
{
	for (auto& ref : m_vMJPlayers)
	{
		if ( ref && ref->getSessionID() == nSessionid)
		{
			return ref;
		}
	}
	return nullptr;
}

IMJPlayer* IMJRoom::getMJPlayerByUID(uint32_t nUID)
{
	for (auto& ref : m_vMJPlayers)
	{
		if (ref &&  ref->getUID() == nUID)
		{
			return ref;
		}
	}
	return nullptr;
}

void IMJRoom::goToState(IMJRoomState* pTargetState, Json::Value* jsValue )
{
	if (pTargetState == getCurRoomState() )
	{
		LOGFMTE("go to the same state %d , room id = %d ? ", pTargetState->getStateID(), getRoomID());
	}

	LOGFMTI("roomID = %u 进入房间状态： %u", getRoomID(), pTargetState->getStateID());

	getCurRoomState()->leaveState();
	m_pCurState = pTargetState;
	m_pCurState->enterState(this, *jsValue);

	//stMsgRoomEnterNewState msgNewState;
	//msgNewState.m_fStateDuring = m_pCurState->getStateDuring();
	//msgNewState.nNewState = m_pCurState->getStateID();
	//LOGFMTI("not tell client state changed");
}

void IMJRoom::goToState(uint16_t nStateID, Json::Value* jsValue )
{
	auto pSateIter = m_vRoomStates.find(nStateID);
	if (pSateIter == m_vRoomStates.end())
	{
		LOGFMTE("can not find state id = %u , so can not to it",nStateID );
		return;
	}
	goToState(pSateIter->second, jsValue);
}

uint8_t IMJRoom::getBankerIdx()
{
	return m_nBankerIdx;
}

void IMJRoom::setBankIdx(uint8_t nIdx)
{
	m_nBankerIdx = nIdx;
}

void IMJRoom::onPlayerSetReady(uint8_t nIdx)
{
	auto pPlayer = getMJPlayerByIdx(nIdx);
	if (!pPlayer)
	{
		LOGFMTE("idx = %u target player is null ptr can not set ready",nIdx);
		return;
	}
	pPlayer->setState(eRoomPeer_Ready);
	// msg ;
	Json::Value jsMsg;
	jsMsg["idx"] = nIdx;
	sendRoomMsg(jsMsg, MSG_ROOM_PLAYER_READY);
}

IMJPlayer* IMJRoom::getMJPlayerByIdx(uint8_t nIdx)
{
	if ( nIdx >= getSeatCnt())
	{
		LOGFMTE("invalid seat idx = %u get player",nIdx );
		return nullptr;
	}
	return m_vMJPlayers[nIdx];
}

void IMJRoom::startGame()
{
	for (auto& pPlayer : m_vMJPlayers)
	{
		if (pPlayer)
		{
			pPlayer->onStartGame();
		}
	}

	// distribute card 
	auto pPoker = getMJPoker();
	//LOGFMTD("room id = %u start game shuffle card ",getRoomID());
	pPoker->shuffle();
	//LOGFMTD("room id = %u shuffle end", getRoomID());
	Json::Value jsFrameArg, jsPlayers;
	jsFrameArg["bankIdx"] = getBankerIdx();
	for (auto& pPlayer : m_vMJPlayers)
	{
		if (!pPlayer)
		{
			LOGFMTE("why player is null hz mj must all player is not null");
			continue;
		}

		if (pPlayer->getCoin() < getRoomConfig()->nDeskFee)
		{
			LOGFMTE("uid = %u coin = %u, 你的钱太少了，还是别玩了吧，这次就不扣你的台费了",pPlayer->getUID(),pPlayer->getCoin());
		}
		else
		{
			pPlayer->setCoin(pPlayer->getCoin() - (int32_t)getRoomConfig()->nDeskFee);
		}

		//LOGFMTD("distribute card for player idx = %u and decrease desk fee = %u",pPlayer->getIdx(),getRoomConfig()->nDeskFee );
		Json::Value jsPlayer;
		jsPlayer["idx"] = pPlayer->getIdx();
		Json::Value jsHoldCard;
		for (uint8_t nIdx = 0; nIdx < 13; ++nIdx)
		{
			auto nCard = pPoker->distributeOneCard();
			pPlayer->getPlayerCard()->addDistributeCard(nCard);
			jsHoldCard[jsHoldCard.size()] = nCard;
			//LOGFMTD("card idx = %u card number = %u", nIdx,nCard);
		}

		if (getBankerIdx() == pPlayer->getIdx())
		{
			auto nCard = pPoker->distributeOneCard();
			pPlayer->getPlayerCard()->onMoCard(nCard);
			jsHoldCard[jsHoldCard.size()] = nCard;
		}
		jsPlayer["cards"] = jsHoldCard;
		jsPlayer["coin"] = pPlayer->getCoin();
		jsPlayer["uid"] = pPlayer->getUID();
		jsPlayers[jsPlayers.size()] = jsPlayer;
	}
	jsFrameArg["players"] = jsPlayers;

	auto pFrame = getGameReplay()->createFrame(eMJFrame_StartGame, (uint32_t)time(0));
	pFrame->setFrameArg(jsFrameArg);
	getGameReplay()->addFrame(pFrame);
	LOGFMTI("room id = %u start game !",getRoomID());
}

void IMJRoom::packStartGameMsg(Json::Value& msg)
{
	Json::Value peerCards[4]; // used for sign for msg ;
	for (auto& pPlayer : m_vMJPlayers)
	{
		if (!pPlayer)
		{
			LOGFMTE("why player is null hz mj must all player is not null");
			continue;
		}
		auto pPlayerCard = pPlayer->getPlayerCard();
		IMJPlayerCard::VEC_CARD vCard;
		pPlayerCard->getHoldCard(vCard);
		for (auto& vC : vCard)
		{
			auto& pArray = peerCards[pPlayer->getIdx()];
			pArray[pArray.size()] = vC;
		}
	}

	uint8_t nDice = 2 + rand() % 11;
	msg["dice"] = nDice;
	msg["banker"] = m_nBankerIdx;
	Json::Value arrPeerCards;
	for (uint8_t nIdx = 0; nIdx < getSeatCnt(); ++nIdx)
	{
		Json::Value peer;
		peer["cards"] = peerCards[nIdx];
		arrPeerCards[nIdx] = peer;
	}
	msg["peerCards"] = arrPeerCards;
}

void IMJRoom::willStartGame()
{
	for (auto& pPlayer : m_vMJPlayers)
	{
		if (pPlayer)
		{
			pPlayer->onWillStartGame();
		}
	}

	// add game replay
	auto p = (MJGameReplayManager*)m_pRoomMgr->getSvrApp()->getModuleByType(CMJServerApp::eMod_ReplayMgr);
	getGameReplay()->reset();
	getGameReplay()->setReplayID(p->generateReplayID());
}

void IMJRoom::onGameEnd()
{
	// add Entery ;
	for (auto& pPlayer : m_vMJPlayers)
	{
		if (pPlayer)
		{
			pPlayer->onGameEnd();
		}
	}

	// add game replay
	auto p = (MJGameReplayManager*)m_pRoomMgr->getSvrApp()->getModuleByType(CMJServerApp::eMod_ReplayMgr);
	p->addGameReplay(getGameReplay());
	getGameReplay()->reset();
}

void IMJRoom::onGameDidEnd()
{
	for (auto& pPlayer : m_vMJPlayers)
	{
		if (pPlayer)
		{
			pPlayer->onGameDidEnd();
		}
	}
}

bool IMJRoom::canStartGame()
{
	uint8_t nReadyCnt = 0;
	for (auto& pPlayer : m_vMJPlayers)
	{
		if (pPlayer && pPlayer->haveState(eRoomPeer_Ready) )
		{
			++nReadyCnt;
		}
	}
	return nReadyCnt == getSeatCnt();
}

// mj function ;
void IMJRoom::onWaitPlayerAct(uint8_t nIdx, bool& isCanPass)
{
	auto pPlayer = getMJPlayerByIdx(nIdx);
	if (!pPlayer)
	{
		LOGFMTE("player idx = %u is null can not tell it wait act",nIdx);
		return;
	}
	auto pMJCard = pPlayer->getPlayerCard();
	// send msg to tell player do act 
	Json::Value jsArrayActs;
	Json::Value jsFrameActs;
	if ( isCanGoOnMoPai() )
	{
		// check bu gang .
		IMJPlayerCard::VEC_CARD vCards;
		pMJCard->getHoldCardThatCanBuGang(vCards);
		for (auto& ref : vCards)
		{
			Json::Value jsAct;
			jsAct["act"] = eMJAct_BuGang;
			jsAct["cardNum"] = ref;
			jsArrayActs[jsArrayActs.size()] = jsAct;
			jsFrameActs[jsFrameActs.size()] = eMJAct_BuGang;
		}
		// check an gang .
		vCards.clear();
		pMJCard->getHoldCardThatCanAnGang(vCards);
		for (auto& ref : vCards)
		{
			Json::Value jsAct;
			jsAct["act"] = eMJAct_AnGang;
			jsAct["cardNum"] = ref;
			jsArrayActs[jsArrayActs.size()] = jsAct;
			jsFrameActs[jsFrameActs.size()] = eMJAct_AnGang;
		}
	}

	// check hu .
	if ( pMJCard->isHoldCardCanHu())
	{
		Json::Value jsAct;
		jsAct["act"] = eMJAct_Hu;
		jsAct["cardNum"] = pMJCard->getNewestFetchedCard();
		jsArrayActs[jsArrayActs.size()] = jsAct;
		jsFrameActs[jsFrameActs.size()] = eMJAct_Hu;
	}

	isCanPass = jsArrayActs.empty() == false;
	jsFrameActs[jsFrameActs.size()] = eMJAct_Chu;

	// add default alwasy chu , infact need not add , becaust it alwasy in ,but compatable with current client ;
	Json::Value jsAct;
	jsAct["act"] = eMJAct_Chu;
	jsAct["cardNum"] = getAutoChuCardWhenWaitActTimeout(nIdx);
	jsArrayActs[jsArrayActs.size()] = jsAct;

	Json::Value jsMsg;
	jsMsg["acts"] = jsArrayActs;
	sendMsgToPlayer(jsMsg, MSG_PLAYER_WAIT_ACT_AFTER_RECEIVED_CARD, pPlayer->getSessionID());
	
	if ( isCanPass )  // player do have option do select or need not give frame ;
	{
		Json::Value jsFrameArg;
		auto ptrReplay = getGameReplay()->createFrame(eMJFrame_WaitPlayerAct, (uint32_t)time(nullptr));
		jsFrameArg["idx"] = nIdx;
		jsFrameArg["act"] = jsFrameActs;
		ptrReplay->setFrameArg(jsFrameArg);
		getGameReplay()->addFrame(ptrReplay);
	}

	//LOGFMTD("tell player idx = %u do act size = %u",nIdx,jsArrayActs.size());
}

uint8_t IMJRoom::getAutoChuCardWhenWaitActTimeout(uint8_t nIdx)
{
	auto pPlayer = getMJPlayerByIdx(nIdx);
	if (!pPlayer)
	{
		LOGFMTE("why this player is null idx = %u , can not chu card",nIdx);
		return 0;
	}
	return pPlayer->getPlayerCard()->getNewestFetchedCard();
}

uint8_t IMJRoom::getAutoChuCardWhenWaitChuTimeout(uint8_t nIdx)
{
	auto pPlayer = getMJPlayerByIdx(nIdx);
	if (!pPlayer)
	{
		LOGFMTE("why this player is null idx = %u , can not chu card", nIdx);
		return 0;
	}
	IMJPlayerCard::VEC_CARD vCard;
	pPlayer->getPlayerCard()->getHoldCard(vCard);
	if (vCard.empty())
	{
		LOGFMTE("hold card can not be empty");
		assert(0&&"hold card must no be empty");
		return 0;
	}
	return vCard.back();
}

void IMJRoom::onPlayerMo(uint8_t nIdx)
{
	auto pPlayer = getMJPlayerByIdx(nIdx);
	if (!pPlayer)
	{
		LOGFMTE("why this player is null idx = %u , can not mo", nIdx);
		return;
	}

	auto nNewCard = getMJPoker()->distributeOneCard();
	if (nNewCard == 0)
	{
		Assert(0,"invlid card" );
	}
	pPlayer->getPlayerCard()->onMoCard(nNewCard);
	pPlayer->clearGangFlag();
	pPlayer->clearDecareBuGangFlag();
	// send msg ;
	Json::Value msg;
	msg["idx"] = nIdx;
	msg["actType"] = eMJAct_Mo;
	msg["card"] = nNewCard;
	msg["invokerIdx"] = nIdx;
	sendRoomMsg(msg, MSG_ROOM_ACT);

	Json::Value jsFrameArg;
	auto ptrReplay = getGameReplay()->createFrame(eMJFrame_Mo, (uint32_t)time(nullptr));
	jsFrameArg["idx"] = nIdx;
	jsFrameArg["card"] = nNewCard;
	ptrReplay->setFrameArg(jsFrameArg);
	getGameReplay()->addFrame(ptrReplay);
}

void IMJRoom::onPlayerPeng(uint8_t nIdx, uint8_t nCard, uint8_t nInvokeIdx)
{
	auto pPlayer = getMJPlayerByIdx(nIdx);
	auto pInvoker = getMJPlayerByIdx(nInvokeIdx);
	if (!pPlayer || !pInvoker)
	{
		LOGFMTE("why this player is null idx = %u , can not peng", nIdx);
		return;
	}

	if (pPlayer->getPlayerCard()->onPeng(nCard) == false)
	{
		LOGFMTE( "nidx = %u peng card = %u error",nIdx,nCard );
	}
	pInvoker->getPlayerCard()->onCardBeGangPengEat(nCard);

	Json::Value jsmsg;
	jsmsg["idx"] = nIdx;
	jsmsg["actType"] = eMJAct_Peng;
	jsmsg["card"] = nCard;
	jsmsg["invokerIdx"] = nInvokeIdx;
	sendRoomMsg(jsmsg, MSG_ROOM_ACT);

	Json::Value jsFrameArg;
	auto ptrReplay = getGameReplay()->createFrame(eMJFrame_Peng, (uint32_t)time(nullptr));
	jsFrameArg["idx"] = nIdx;
	jsFrameArg["card"] = nCard;
	jsFrameArg["invokerIdx"] = nInvokeIdx;
	ptrReplay->setFrameArg(jsFrameArg);
	getGameReplay()->addFrame(ptrReplay);
}

void IMJRoom::onPlayerEat(uint8_t nIdx, uint8_t nCard, uint8_t nWithA, uint8_t nWithB, uint8_t nInvokeIdx)
{
	auto pPlayer = getMJPlayerByIdx(nIdx);
	auto pInvoker = getMJPlayerByIdx(nInvokeIdx);
	if (!pPlayer || !pInvoker )
	{
		LOGFMTE("why this player is null idx = %u , can not eat", nIdx);
		return;
	}

	if (pPlayer->getPlayerCard()->onEat(nCard,nWithA,nWithB) == false)
	{
		LOGFMTE("nidx = %u eat card = %u error, with a = %u ,b = %u", nIdx, nCard,nWithA,nWithB);
	}
	pInvoker->getPlayerCard()->onCardBeGangPengEat(nCard);

	// send msg ;
	Json::Value jsmsg;
	jsmsg["idx"] = nIdx;
	jsmsg["actType"] = eMJAct_Chi;
	jsmsg["card"] = nCard;
	Json::Value jseatwith;
	jseatwith[jseatwith.size()] = nWithA;
	jseatwith[jseatwith.size()] = nWithB;
	jsmsg["eatWith"] = jseatwith;
	jsmsg["invokerIdx"] = nInvokeIdx;
	sendRoomMsg(jsmsg, MSG_ROOM_ACT);
}

void IMJRoom::onPlayerMingGang(uint8_t nIdx, uint8_t nCard, uint8_t nInvokeIdx)
{
	auto pPlayer = getMJPlayerByIdx(nIdx);
	auto pInvoker = getMJPlayerByIdx(nInvokeIdx);
	if (!pPlayer || !pInvoker)
	{
		LOGFMTE("why this player is null idx = %u , can not ming gang", nIdx);
		return;
	}
	pPlayer->signGangFlag();
	pPlayer->addMingGangCnt();

	auto nGangGetCard = getMJPoker()->distributeOneCard();
	if (pPlayer->getPlayerCard()->onMingGang(nCard, nGangGetCard) == false)
	{
		LOGFMTE("nidx = %u ming gang card = %u error,", nIdx, nCard );
	}
	pInvoker->getPlayerCard()->onCardBeGangPengEat(nCard);

	// send msg 
	Json::Value msg;
	msg["idx"] = nIdx;
	msg["actType"] = eMJAct_MingGang;
	msg["card"] = nCard;
	msg["gangCard"] = nGangGetCard;
	msg["invokerIdx"] = nInvokeIdx;
	sendRoomMsg(msg, MSG_ROOM_ACT);

	Json::Value jsFrameArg;
	auto ptrReplay = getGameReplay()->createFrame(eMJFrame_MingGang, (uint32_t)time(nullptr));
	jsFrameArg["idx"] = nIdx;
	jsFrameArg["gang"] = nCard;
	jsFrameArg["newCard"] = nGangGetCard;
	jsFrameArg["invokerIdx"] = nInvokeIdx;
	ptrReplay->setFrameArg(jsFrameArg);
	getGameReplay()->addFrame(ptrReplay);
}

void IMJRoom::onPlayerAnGang(uint8_t nIdx, uint8_t nCard)
{
	auto pPlayer = getMJPlayerByIdx(nIdx);
	if (!pPlayer)
	{
		LOGFMTE("why this player is null idx = %u , can not an gang", nIdx);
		return;
	}
	pPlayer->signGangFlag();
	pPlayer->addAnGangCnt();
	auto nGangGetCard = getMJPoker()->distributeOneCard();
	if (pPlayer->getPlayerCard()->onAnGang(nCard, nGangGetCard) == false)
	{
		LOGFMTE("nidx = %u an gang card = %u error,", nIdx, nCard);
	}

	// send msg ;
	Json::Value msg;
	msg["idx"] = nIdx;
	msg["actType"] = eMJAct_AnGang;
	msg["card"] = nCard;
	msg["gangCard"] = nGangGetCard;
	msg["invokerIdx"] = nIdx;
	sendRoomMsg(msg, MSG_ROOM_ACT);

	Json::Value jsFrameArg;
	auto ptrReplay = getGameReplay()->createFrame(eMJFrame_AnGang, (uint32_t)time(nullptr));
	jsFrameArg["idx"] = nIdx;
	jsFrameArg["gang"] = nCard;
	jsFrameArg["newCard"] = nGangGetCard;
	ptrReplay->setFrameArg(jsFrameArg);
	getGameReplay()->addFrame(ptrReplay);
}

void IMJRoom::onPlayerBuGang(uint8_t nIdx, uint8_t nCard)
{
	auto pPlayer = getMJPlayerByIdx(nIdx);
	if (!pPlayer)
	{
		LOGFMTE("why this player is null idx = %u , can not bu gang", nIdx);
		return;
	}
	pPlayer->signGangFlag();
	pPlayer->clearDecareBuGangFlag();
	auto nGangCard = getMJPoker()->distributeOneCard();
	if (pPlayer->getPlayerCard()->onBuGang(nCard, nGangCard) == false)
	{
		LOGFMTE("nidx = %u bu gang card = %u error,", nIdx, nCard);
	}
	pPlayer->addMingGangCnt();
	// send msg 
	Json::Value msg;
	msg["idx"] = nIdx;
	msg["actType"] = eMJAct_BuGang_Done;
	msg["card"] = nCard;
	msg["gangCard"] = nGangCard;
	msg["invokerIdx"] = nIdx;
	sendRoomMsg(msg, MSG_ROOM_ACT);

	Json::Value jsFrameArg;
	auto ptrReplay = getGameReplay()->createFrame(eMJFrame_BuGang, (uint32_t)time(nullptr));
	jsFrameArg["idx"] = nIdx;
	jsFrameArg["gang"] = nCard;
	jsFrameArg["newCard"] = nGangCard;
	ptrReplay->setFrameArg(jsFrameArg);
	getGameReplay()->addFrame(ptrReplay);
}

void IMJRoom::onPlayerHu(std::vector<uint8_t>& vHuIdx, uint8_t nCard, uint8_t nInvokeIdx)
{
	// send msg 
	Json::Value msg;
	msg["idx"] = vHuIdx.front();
	msg["actType"] = eMJAct_Hu;
	msg["card"] = nCard;
	msg["invokerIdx"] = nInvokeIdx;
	sendRoomMsg(msg, MSG_ROOM_ACT);
}

void IMJRoom::onPlayerChu(uint8_t nIdx, uint8_t nCard)
{
	auto pPlayer = getMJPlayerByIdx(nIdx);
	if (!pPlayer)
	{
		LOGFMTE("why this player is null idx = %u , can not chu", nIdx);
		return;
	}

	// send msg ;
	Json::Value msg;
	msg["idx"] = nIdx;
	msg["actType"] = eMJAct_Chu;
	msg["card"] = nCard;
	msg["invokerIdx"] = nIdx;
	sendRoomMsg(msg, MSG_ROOM_ACT);

	if (!pPlayer->getPlayerCard()->onChuCard(nCard))
	{
		LOGFMTE("chu card error idx = %u , card = %u",nIdx,nCard );
	}

	Json::Value jsFrameArg;
	auto ptrReplay = getGameReplay()->createFrame(eMJFrame_Chu, (uint32_t)time(nullptr));
	jsFrameArg["idx"] = nIdx;
	jsFrameArg["card"] = nCard;
	ptrReplay->setFrameArg(jsFrameArg);
	getGameReplay()->addFrame(ptrReplay);
}

bool IMJRoom::isAnyPlayerPengOrHuThisCard(uint8_t nInvokeIdx, uint8_t nCard)
{
	for (auto& ref : m_vMJPlayers)
	{
		if (ref == nullptr || nInvokeIdx == ref->getIdx() )
		{
			continue;
		}

		auto pMJCard = ref->getPlayerCard();
		if (pMJCard->canPengWithCard(nCard) || pMJCard->canHuWitCard(nCard))
		{
			return true;
		}

		if (isCanGoOnMoPai() && pMJCard->canMingGangWithCard(nCard) ) // must can gang , will not run here , will return when check peng ;
		{
			return true;
		}

		if (ref->getIdx() == (nInvokeIdx + 1) % getSeatCnt())
		{
			uint8_t a = 0, b = 0;
			if (pMJCard->canEatCard(nCard, a, b))
			{
				return true;
			}
		}
	}

	return false;
}

void IMJRoom::onAskForPengOrHuThisCard(uint8_t nInvokeIdx, uint8_t nCard, std::vector<uint8_t>& vWaitHuIdx, std::vector<uint8_t>& vWaitPengGangIdx, bool& isNeedWaitEat)
{
	Json::Value jsFrameArg;

	for (auto& ref : m_vMJPlayers)
	{
		if (ref == nullptr || nInvokeIdx == ref->getIdx())
		{
			continue;
		}

		Json::Value jsMsg;
		jsMsg["invokerIdx"] = nInvokeIdx;
		jsMsg["cardNum"] = nCard;

		Json::Value jsActs;
		auto pMJCard = ref->getPlayerCard();

		// check peng 
		if (pMJCard->canPengWithCard(nCard))
		{
			jsActs[jsActs.size()] = eMJAct_Peng;
			vWaitPengGangIdx.push_back(ref->getIdx());
		}

		// check ming gang 
		if ( isCanGoOnMoPai() && pMJCard->canMingGangWithCard(nCard))
		{
			jsActs[jsActs.size()] = eMJAct_MingGang;
			// already add in peng ;  vWaitPengGangIdx
			if (vWaitPengGangIdx.empty())
			{
				vWaitPengGangIdx.push_back(ref->getIdx());
			}
		}

		if (ref->getIdx() == (nInvokeIdx + 1) % getSeatCnt())
		{
			uint8_t a = 0, b = 0;
			isNeedWaitEat = false;
			if (pMJCard->canEatCard(nCard, a, b))
			{
				isNeedWaitEat = true;
				jsActs[jsActs.size()] = eMJAct_Chi;
			}
		}

		// check hu ;
		if (pMJCard->canHuWitCard(nCard))
		{
			jsActs[jsActs.size()] = eMJAct_Hu;
			vWaitHuIdx.push_back(ref->getIdx());
		}

		if (jsActs.size() > 0)
		{
			jsActs[jsActs.size()] = eMJAct_Pass;
		}

		if ( jsActs.size() == 0 )
		{
			continue;
		}
		jsMsg["acts"] = jsActs;
		sendMsgToPlayer(jsMsg, MSG_PLAYER_WAIT_ACT_ABOUT_OTHER_CARD, ref->getSessionID());

		//LOGFMTD("inform uid = %u act about other card room id = %u card = %u", ref->getUID(), getRoomID(),nCard );

		Json::Value jsFramePlayer;
		jsFramePlayer["idx"] = ref->getIdx();
		jsFramePlayer["acts"] = jsActs;

		jsFrameArg[jsFrameArg.size()] = jsFramePlayer;
	}

	// add frame 
	auto ptrReplay = getGameReplay()->createFrame(eMJFrame_WaitPlayerActAboutCard, (uint32_t)time(nullptr));
	ptrReplay->setFrameArg(jsFrameArg);
	getGameReplay()->addFrame(ptrReplay);
}

bool IMJRoom::isAnyPlayerRobotGang(uint8_t nInvokeIdx, uint8_t nCard)
{
	for (auto& ref : m_vMJPlayers)
	{
		if (ref == nullptr || nInvokeIdx == ref->getIdx())
		{
			continue;
		}

		auto pMJCard = ref->getPlayerCard();
		uint8_t a = 0, b = 0;
		if (pMJCard->canHuWitCard(nCard))
		{
			return true;
		}
	}

	return false;
}

void IMJRoom::onAskForRobotGang(uint8_t nInvokeIdx, uint8_t nCard, std::vector<uint8_t>& vCandinates)
{
	// send decalre gang msg ;
	Json::Value msg;
	msg["idx"] = nInvokeIdx;
	msg["actType"] = eMJAct_BuGang_Pre;
	msg["card"] = nCard;
	msg["invokerIdx"] = nInvokeIdx;
	sendRoomMsg(msg, MSG_ROOM_ACT);

	// inform target player do this things 
	Json::Value jsFrameArg;
	for (auto& ref : m_vMJPlayers)
	{
		if (ref == nullptr || nInvokeIdx == ref->getIdx())
		{
			continue;
		}

		Json::Value jsMsg;
		jsMsg["invokerIdx"] = nInvokeIdx;
		jsMsg["cardNum"] = nCard;

		Json::Value jsActs;
		auto pMJCard = ref->getPlayerCard();
		// check hu 
		if (pMJCard->canHuWitCard(nCard))
		{
			jsActs[jsActs.size()] = eMJAct_Hu;
			vCandinates.push_back(ref->getIdx());

			jsFrameArg[jsFrameArg.size()] = ref->getIdx();
		}

		if (jsActs.size() > 0)
		{
			jsActs[jsActs.size()] = eMJAct_Pass;
		}

		jsMsg["acts"] = jsActs;
		sendMsgToPlayer(jsMsg, MSG_PLAYER_WAIT_ACT_ABOUT_OTHER_CARD, ref->getSessionID());
		//LOGFMTD("inform uid = %u robot gang card = %u room id = %u ", ref->getUID(),nCard, getRoomID());
	}

	// add frame 
	auto ptrReplay = getGameReplay()->createFrame( eMJFrame_WaitRobotGang, (uint32_t)time(nullptr));
	ptrReplay->setFrameArg(jsFrameArg);
	getGameReplay()->addFrame(ptrReplay);
}

uint8_t IMJRoom::getNextActPlayerIdx(uint8_t nCurActIdx)
{
	return (nCurActIdx + 1) % getSeatCnt();
}

bool IMJRoom::isGameOver()
{
	return !isCanGoOnMoPai();
}

bool IMJRoom::isCanGoOnMoPai()
{
	return getMJPoker()->getLeftCardCount() > 0 ;
}

void IMJRoom::onPlayerLouHu(uint8_t nIdx, uint8_t nInvokerIdx)
{
	if ( isHaveLouHu() == false )
	{
		return;
	}
}

void IMJRoom::onPlayerLouPeng(uint8_t nIdx, uint32_t nLouCard)
{
	if (isHaveLouPeng() == false)
	{
		return;
	}
}

bool IMJRoom::addRoomState(IMJRoomState* pState)
{
	auto iter = m_vRoomStates.find(pState->getStateID());
	if (iter == m_vRoomStates.end())
	{
		m_vRoomStates[pState->getStateID()] = pState;
		return true;
	}
	LOGFMTE("already add this state id =%u , be remember delete failed add obj",pState->getStateID());
	return false;
}

void IMJRoom::setInitState(IMJRoomState* pState)
{
	m_pCurState = pState; 
	Json::Value js;
	m_pCurState->enterState(this,js);
}

uint32_t IMJRoom::getCoinNeedToSitDown()
{
	return 0;
}

void IMJRoom::onCheckTrusteeForWaitPlayerAct(uint8_t nIdx, bool isMayBeHu)
{
	auto pPlayer = getMJPlayerByIdx(nIdx);
	if (pPlayer == nullptr)
	{
		LOGFMTE("waht player act ,but player is nullptr");
		return;
	}

	if (pPlayer->isTrusteed() == false)
	{
		return;
	}

	pPlayer->setTrusteeActFunc([this, nIdx, isMayBeHu](CTimer* pT, float ft)
	{
		auto pPlayer = getMJPlayerByIdx(nIdx);
		if (pPlayer == nullptr)
		{
			LOGFMTE("setTrusteeActFunc waht player act ,but player is nullptr");
			return;
		}

		if (pPlayer->isTrusteed() == false)
		{
			LOGFMTD("player cannecled trustee act ");
			return;
		}

		auto playerCard = pPlayer->getPlayerCard();
		Json::Value jsmsg;
		if (isMayBeHu && playerCard->isHoldCardCanHu())
		{
			jsmsg["actType"] = eMJAct_Hu;
			jsmsg["card"] = playerCard->getNewestFetchedCard();
		}
		else
		{
			jsmsg["actType"] = eMJAct_Chu;
			jsmsg["card"] = getAutoChuCardWhenWaitActTimeout(nIdx);;
		}
		//LOGFMTD("%u player tuo guan do act = %u", pPlayer->getUID(), jsmsg["actType"].asUInt());
		onMsg(jsmsg, MSG_PLAYER_ACT, ID_MSG_PORT_CLIENT, pPlayer->getSessionID());
	}
	);
}

void IMJRoom::onCheckTrusteeForHuOtherPlayerCard(std::vector<uint8_t> vPlayerIdx, uint8_t nTargetCard)
{
	for (auto& nIdx : vPlayerIdx)
	{
		auto pPlayer = getMJPlayerByIdx(nIdx);
		if (pPlayer == nullptr)
		{
			LOGFMTE("onCheckTrusteeForHuOtherPlayerCard player act ,but player is nullptr");
			continue;
		}

		if (pPlayer->isTrusteed() == false)
		{
			continue;
		}

		pPlayer->setTrusteeActFunc([this, nIdx, nTargetCard](CTimer* pT, float ft)
		{
			auto pPlayer = getMJPlayerByIdx(nIdx);
			if (pPlayer == nullptr)
			{
				LOGFMTE("setTrusteeActFunc waht player act ,but player is nullptr");
				return;
			}

			if (pPlayer->isTrusteed() == false)
			{
				LOGFMTD("player cannecled trustee act ");
				return;
			}

			auto playerCard = pPlayer->getPlayerCard();
			Json::Value jsmsg;
			if ( playerCard->canHuWitCard(nTargetCard) )
			{
				jsmsg["actType"] = eMJAct_Hu;
				jsmsg["card"] = nTargetCard;
			}
			else
			{
				jsmsg["actType"] = eMJAct_Pass;
				jsmsg["card"] = 0;
			}
			LOGFMTD("%u player tuo guan do act = %u  about other card", pPlayer->getUID(), jsmsg["actType"].asUInt());
			onMsg(jsmsg, MSG_PLAYER_ACT, ID_MSG_PORT_CLIENT, pPlayer->getSessionID());
		}
		);
	}
}

void IMJRoom::onPlayerTrusteedStateChange(uint8_t nPlayerIdx, bool isTrusteed)
{
	//if (getDelegate())
	//{
	//	LOGFMTD("vip room should not have tuo guan function %u",getRoomID() );
	//	return;
	//}

	//auto pPlayer = getMJPlayerByIdx(nPlayerIdx);
	//if (!pPlayer)
	//{
	//	LOGFMTE("room id = %u , player idx = %u is nulljptr trusteed state change",getRoomID(),nPlayerIdx);
	//	return;
	//}

	//pPlayer->switchTrusteed(isTrusteed);
	//Json::Value js;
	//js["idx"] = pPlayer->getIdx();
	//js["isTrusteed"] = isTrusteed ? 1 : 0 ;
	//sendRoomMsg(js, MSG_ROOM_REQUEST_TRUSTEED);
	//LOGFMTD("room id = %u , player idx = %u update trusteed state = %u " ,getRoomID(),nPlayerIdx,isTrusteed );
}