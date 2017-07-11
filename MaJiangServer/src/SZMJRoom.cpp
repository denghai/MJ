#include "SZMJRoom.h"
#include "SZMJPlayer.h"
#include "SZMJPlayerCard.h"
#include "log4z.h"
#include "IMJPoker.h"
#include "ServerMessageDefine.h"
#include "MJRoomStateWaitReady.h"
#include "MJRoomStateWaitPlayerChu.h"
#include "SZRoomStateWaitPlayerAct.h"
#include "MJRoomStateGameEnd.h"
#include "SZRoomStateDoPlayerAct.h"
#include "MJRoomStateAskForPengOrHu.h"
#include "IGameRoomManager.h"
#include "SZRoomStateBuHua.h"
#include "NJRoomStateStartGame.h"
#include "MJRoomStateAskForRobotGang.h"
#include "SZMJPlayerRecorderInfo.h"
#include <ctime>
#include "SZMJPlayerRecorderInfo.h"
bool SZMJRoom::init(IGameRoomManager* pRoomMgr, stBaseRoomConfig* pConfig, uint32_t nSeialNum, uint32_t nRoomID, Json::Value& vJsValue)
{
	IMJRoom::init(pRoomMgr, pConfig, nSeialNum, nRoomID, vJsValue);
	m_isFanBei = false;
	m_isWillFanBei = false;
	m_isBankerHu = false;
	m_nRuleMode = 1;
	if (vJsValue["ruletype"].isNull() || vJsValue["ruletype"].isUInt() == false)
	{
		LOGFMTE("invlid rule type ");
	}
	else
	{
		m_nRuleMode = vJsValue["ruletype"].asUInt();
	}

	if (m_nRuleMode != 1 && 2 != m_nRuleMode)
	{
		LOGFMTE("invalid rule type value = %u",m_nRuleMode );
		m_nRuleMode = 1;
	}

	m_tPoker.initAllCard(eMJ_SuZhou);
	// create state and add state ;
	IMJRoomState* vState[] = {
		new CMJRoomStateWaitReady(), new MJRoomStateWaitPlayerChu(), new SZRoomStateWaitPlayerAct(), new NJRoomStateStartGame(), new SZRoomStateBuHua()
		, new MJRoomStateGameEnd(), new SZRoomStateDoPlayerAct(), new MJRoomStateAskForPengOrHu(), new MJRoomStateAskForRobotGang()
	};
	for (uint8_t nIdx = 0; nIdx < sizeof(vState) / sizeof(IMJRoomState*); ++nIdx)
	{
		addRoomState(vState[nIdx]);
	}
	setInitState(vState[0]);

	// init banker
	m_nBankerIdx = -1;
	auto pRoomRecorder = (SZMJRoomRecorder*)getRoomRecorder().get();
	pRoomRecorder->setRoomOpts(m_nRuleMode);
	return true;
}

void SZMJRoom::willStartGame()
{
	IMJRoom::willStartGame();
	m_isFanBei = false;
	if ( m_isWillFanBei )
	{
		m_isFanBei = true;
	}

	m_isWillFanBei = false;

	if ((uint8_t)-1 == m_nBankerIdx)
	{
		m_nBankerIdx = 0;
	}
	else
	{
		if (m_isBankerHu == false)
		{
			m_nBankerIdx = (m_nBankerIdx + 1) % MAX_SEAT_CNT;
		}
	}


	m_isBankerHu = false;
}

void SZMJRoom::packStartGameMsg(Json::Value& jsMsg)
{
	IMJRoom::packStartGameMsg(jsMsg);
	jsMsg["isFanBei"] = isFanBei() ? 1 : 0;
}

void SZMJRoom::startGame()
{
	IMJRoom::startGame();

	//// bind room to player card 
	//// check di hu 
	//for (auto& pPlayer : m_vMJPlayers)
	//{
	//	if (pPlayer == nullptr)
	//	{
	//		LOGFMTE("room id = %u , start game player is nullptr", getRoomID());
	//		continue;
	//	}
	//	auto pPlayerCard = (SZMJPlayerCard*)pPlayer->getPlayerCard();
	//	pPlayerCard->bindRoom(this);
	//}

	Json::Value jsMsg;
	packStartGameMsg(jsMsg);
	sendRoomMsg(jsMsg, MSG_ROOM_START_GAME);
}

void SZMJRoom::getSubRoomInfo(Json::Value& jsSubInfo)
{
	jsSubInfo["isFanBei"] = isFanBei() ? 1 : 0;
	jsSubInfo["ruletype"] = m_nRuleMode;
}

void SZMJRoom::onGameDidEnd()
{
	IMJRoom::onGameDidEnd();
	if (getDelegate())
	{
		getDelegate()->onDidGameOver(this);
		return;
	}
}

void SZMJRoom::onGameEnd()
{
	// svr: { isLiuJu : 0 , detail : [ {idx : 0 , offset : 23 }, ...  ], realTimeCal : [ { actType : 23, detial : [ {idx : 2, offset : -23 } ]  } , ... ] } 
	Json::Value jsMsg;
	Json::Value jsDetial;

	auto ptrSingleRecorder = getRoomRecorder()->createSingleRoundRecorder();
	ptrSingleRecorder->init(getRoomRecorder()->getRoundRecorderCnt(), (uint32_t)time(nullptr), 0);
	getRoomRecorder()->addSingleRoundRecorder(ptrSingleRecorder);

	bool isAnyOneHu = false;
	for (auto& ref : m_vMJPlayers)
	{
		Json::Value js;
		if (ref)
		{
			js["idx"] = ref->getIdx();
			js["offset"] = ref->getOffsetCoin();
			js["final"] = ref->getCoin();
			jsDetial[jsDetial.size()] = js;

			auto pPlayerRecorderInfo = std::make_shared<SZMJPlayerRecorderInfo>();
			pPlayerRecorderInfo->init(ref->getUID(), ref->getOffsetCoin());
			ptrSingleRecorder->addPlayerRecorderInfo(pPlayerRecorderInfo);
		}

		if (ref && ref->haveState(eRoomPeer_AlreadyHu))
		{
			isAnyOneHu = true;
			continue;
		}
	}

	jsMsg["isLiuJu"] = isAnyOneHu ? 0 : 1;
	jsMsg["detail"] = jsDetial;

	if (!isAnyOneHu)  
	{
		m_isWillFanBei = true;
	}

	jsMsg["isNextFanBei"] = m_isWillFanBei ? 1 : 0;
	jsMsg["nNextBankIdx"] = m_isBankerHu ? m_nBankerIdx : ((m_nBankerIdx + 1) % MAX_SEAT_CNT);

	sendRoomMsg(jsMsg, MSG_ROOM_SZ_GAME_OVER);
	// send msg to player ;
	IMJRoom::onGameEnd();
}

void SZMJRoom::onPlayerMo(uint8_t nIdx)
{
	IMJRoom::onPlayerMo(nIdx);
	auto player = (SZMJPlayer*)getMJPlayerByIdx(nIdx);
	//player->clearBuHuaFlag();
}

IMJPlayer* SZMJRoom::doCreateMJPlayer()
{
	return new SZMJPlayer();
}

IMJPoker* SZMJRoom::getMJPoker()
{
	return &m_tPoker;
}

bool SZMJRoom::isGameOver()
{
	if (IMJRoom::isGameOver())
	{
		return true;
	}

	for (auto& ref : m_vMJPlayers)
	{
		if (ref && ref->haveState(eRoomPeer_AlreadyHu))
		{
			return true;
		}
	}
	return false;
}

void SZMJRoom::onPlayerBuHua(uint8_t nIdx, uint8_t nHuaCard)
{
	auto player = (SZMJPlayer*)getMJPlayerByIdx(nIdx);
	auto pActCard = (SZMJPlayerCard*)player->getPlayerCard();
	auto nNewCard = getMJPoker()->distributeOneCard();
	pActCard->onBuHua(nHuaCard, nNewCard);
	//player->signBuHuaFlag();
	// send msg ;
	Json::Value msg;
	msg["idx"] = nIdx;
	msg["actType"] = eMJAct_BuHua;
	msg["card"] = nHuaCard;
	msg["gangCard"] = nNewCard;
	sendRoomMsg(msg, MSG_ROOM_ACT);
}

void SZMJRoom::onPlayerHu(std::vector<uint8_t>& vHuIdx, uint8_t nCard, uint8_t nInvokeIdx)
{
	if (vHuIdx.empty())
	{
		LOGFMTE("why hu vec is empty ? room id = %u", getRoomID());
		return;
	}

	auto iterBankWin = std::find(vHuIdx.begin(), vHuIdx.end(), getBankerIdx());
	m_isBankerHu = iterBankWin != vHuIdx.end();

	Json::Value jsDetail;
	Json::Value jsMsg;


	bool isZiMo = vHuIdx.front() == nInvokeIdx;
	jsMsg["isZiMo"] = isZiMo ? 1 : 0;
	jsMsg["huCard"] = nCard;
	jsMsg["isFanBei"] = isFanBei() ? 1 : 0 ;
	if (isZiMo)
	{
		onPlayerZiMo(nInvokeIdx, nCard, jsDetail);
		jsMsg["detail"] = jsDetail;
		sendRoomMsg(jsMsg, MSG_ROOM_SZ_PLAYER_HU );
		return;
	}

	// check dian piao ;
	if (vHuIdx.size() > 1)  // yi pao duo xiang 
	{
		m_isWillFanBei = true;
	}

	auto pLosePlayer = getMJPlayerByIdx(nInvokeIdx);
	if (!pLosePlayer)
	{
		LOGFMTE("room id = %u lose but player idx = %u is nullptr", getRoomID(), nInvokeIdx);
		return;
	}

	//{ dianPaoIdx : 23 , isRobotGang : 0 , nLose : 23, huPlayers : [{ idx : 234 , win : 234 , baoPaiIdx : 2 , huardSoftHua : 23, vhuTypes : [ eFanxing , ] } , .... ] } 
	jsDetail["dianPaoIdx"] = pLosePlayer->getIdx();
	jsDetail["isRobotGang"] = pLosePlayer->haveDecareBuGangFalg() ? 1 : 0;
	pLosePlayer->addDianPaoCnt();
	Json::Value jsHuPlayers;
	uint32_t nTotalLose = 0;
	// adjust caculate order 
	std::vector<uint8_t> vOrderHu;
	if (vHuIdx.size() > 1)
	{
		for (uint8_t offset = 1; offset <= 3; ++offset)
		{
			auto nCheckIdx = nInvokeIdx + offset;
			nCheckIdx = nCheckIdx % 4;
			auto iter = std::find(vHuIdx.begin(), vHuIdx.end(), nCheckIdx);
			if (iter != vHuIdx.end())
			{
				vOrderHu.push_back(nCheckIdx);
			}
		}
	}
	else
	{
		vOrderHu.swap(vHuIdx);
	}

	for (auto& nHuIdx : vOrderHu)
	{
		auto pHuPlayer = getMJPlayerByIdx(nHuIdx);
		if (pHuPlayer == nullptr)
		{
			LOGFMTE("room id = %u hu player idx = %u , is nullptr", getRoomID(), nHuIdx);
			continue;
		}
		pHuPlayer->addHuCnt();

		Json::Value jsHuPlayer;
		jsHuPlayer["idx"] = pHuPlayer->getIdx();
		pHuPlayer->setState(eRoomPeer_AlreadyHu);

		auto pHuPlayerCard = (SZMJPlayerCard*)pHuPlayer->getPlayerCard();

		std::vector<uint16_t> vType;
		uint16_t nHuHuaCnt = 0;
		uint16_t nHardSoftHua = 0;
		pHuPlayerCard->onDoHu(false,false, nCard, vType, nHuHuaCnt, nHardSoftHua);
		auto nAllHuaCnt = nHuHuaCnt + nHardSoftHua;
		if (isFanBei())
		{
			nAllHuaCnt *= 2;
		}

		jsHuPlayer["holdHuaCnt"] = nHardSoftHua;
		jsHuPlayer["huHuaCnt"] = nHuHuaCnt;
		Json::Value jsHuTyps;
		for (auto& refHu : vType)
		{
			jsHuTyps[jsHuTyps.size()] = refHu;
		}
		jsHuPlayer["vhuTypes"] = jsHuTyps;

		// process bao pai qing kuang ;
		if (pLosePlayer->haveDecareBuGangFalg()) // robot gang ;
		{
			nAllHuaCnt *= 3;  // robot gang means bao pai, and zi mo ; menas zi mo 
			LOGFMTD("room id = %u , ploseplayer = %u have gang dec ", getRoomID(), pLosePlayer->getUID());
		}

		LOGFMTD("room id = %u winner = %u all huaCnt = %u lose uid =%u", getRoomID(), pHuPlayer->getUID(), nAllHuaCnt, pLosePlayer->getUID());
		if (nAllHuaCnt > pLosePlayer->getCoin())
		{
			nAllHuaCnt = pLosePlayer->getCoin();
		}
		pLosePlayer->addOffsetCoin(-1 * (int32_t)nAllHuaCnt);
		nTotalLose += nAllHuaCnt;

		pHuPlayer->addOffsetCoin(nAllHuaCnt);
		jsHuPlayer["win"] = nAllHuaCnt;

		jsHuPlayers[jsHuPlayers.size()] = jsHuPlayer;
	}

	jsDetail["nLose"] = nTotalLose;
	jsDetail["huPlayers"] = jsHuPlayers;
	jsMsg["detail"] = jsDetail;
	sendRoomMsg(jsMsg, MSG_ROOM_SZ_PLAYER_HU);
	LOGFMTD("room id = %u hu end ", getRoomID());
}

void SZMJRoom::onPlayerZiMo(uint8_t nIdx, uint8_t nCard, Json::Value& jsDetail)
{
	auto pZiMoPlayer = (SZMJPlayer*)getMJPlayerByIdx(nIdx);
	if (pZiMoPlayer == nullptr)
	{
		LOGFMTE("room id = %u zi mo player is nullptr idx = %u ", getRoomID(), nIdx);
		return;
	}
	pZiMoPlayer->addZiMoCnt();
	pZiMoPlayer->setState(eRoomPeer_AlreadyHu);
	// svr :{ huIdx : 234 , baoPaiIdx : 2 , winCoin : 234,huardSoftHua : 23, isGangKai : 0 ,vhuTypes : [ eFanxing , ], LoseIdxs : [ {idx : 1 , loseCoin : 234 }, .... ]   }
	jsDetail["huIdx"] = nIdx;

	auto pHuPlayerCard = (SZMJPlayerCard*)pZiMoPlayer->getPlayerCard();
	std::vector<uint16_t> vType;
	uint16_t nHuHuaCnt = 0;
	uint16_t nHardSoftHua = 0;
	pHuPlayerCard->onDoHu(true, getMJPoker()->getLeftCardCount() < getSeatCnt() ,nCard,vType, nHuHuaCnt, nHardSoftHua);

	Json::Value jsHuTyps;
	for (auto& refHu : vType)
	{
		jsHuTyps[jsHuTyps.size()] = refHu;
	}
	jsDetail["vhuTypes"] = jsHuTyps;
	jsDetail["isGangKai"] = 0;
	// xiao gang kai hua 
	if ( pZiMoPlayer->haveGangFalg() /*|| pZiMoPlayer->haveBuHuaFlag()*/ )
	{
		nHuHuaCnt += 5;
		jsDetail["isGangKai"] = 1;
	}

	jsDetail["holdHuaCnt"] = nHardSoftHua;
	jsDetail["huHuaCnt"] = nHuHuaCnt;
	// da gang kai hua 
	auto nAllHuaCnt = nHuHuaCnt + nHardSoftHua ;
	if (isFanBei())
	{
		nAllHuaCnt *= 2;
	}

	jsDetail["invokerGangIdx"] = nIdx;
	auto nBaoPaiIdx = pHuPlayerCard->getSongGangIdx();
	auto nTotalWin = 0;
	if ((uint8_t)-1 != nBaoPaiIdx)
	{
		nTotalWin = nAllHuaCnt * 3; // bao pai 
		auto pPlayerBao = getMJPlayerByIdx(nBaoPaiIdx);

		if (nTotalWin > pPlayerBao->getCoin())
		{
			nTotalWin = pPlayerBao->getCoin();
		}
		pPlayerBao->addOffsetCoin(-1 * (int32_t)nTotalWin);
		jsDetail["invokerGangIdx"] = nBaoPaiIdx;
	}
	else
	{
		Json::Value jsVLoses;
		for (auto& pLosePlayer : m_vMJPlayers)
		{
			if (pLosePlayer == pZiMoPlayer)
			{
				continue;
			}

			auto nKouHua = nAllHuaCnt;

			if (nKouHua > pLosePlayer->getCoin())
			{
				nKouHua = pLosePlayer->getCoin();
			}
			pLosePlayer->addOffsetCoin(-1 * (int32_t)nKouHua);

			nTotalWin += nKouHua;
			//Json::Value jsLose;
			//jsLose["loseCoin"] = nKouHua;
			//jsLose["idx"] = pLosePlayer->getIdx();
			//jsVLoses[jsVLoses.size()] = jsLose;
		}
		/*jsDetail["LoseIdxs"] = jsVLoses;*/
	}
	pZiMoPlayer->addOffsetCoin(nTotalWin);
	jsDetail["winCoin"] = nTotalWin;

	LOGFMTD("room id = %u hu end ", getRoomID());
}

bool SZMJRoom::onPlayerApplyLeave(uint32_t nPlayerUID)
{
	auto pPlayer = getMJPlayerByUID(nPlayerUID);
	if (!pPlayer)
	{
		LOGFMTE("you are not in room id = %u , how to leave this room ? uid = %u", getRoomID(), nPlayerUID);
		return false;
	}

	Json::Value jsMsg;
	jsMsg["idx"] = pPlayer->getIdx();
	sendRoomMsg(jsMsg, MSG_ROOM_PLAYER_LEAVE); // tell other player leave ;

	auto curState = getCurRoomState()->getStateID();
	if (eRoomSate_WaitReady == curState || eRoomState_GameEnd == curState)
	{
		// direct leave just stand up ;
		//auto pXLPlayer = (XLMJPlayer*)pPlayer;
		stMsgSvrDoLeaveRoom msgdoLeave;
		msgdoLeave.nCoin = pPlayer->getCoin();
		msgdoLeave.nGameType = getRoomType();
		msgdoLeave.nRoomID = getRoomID();
		msgdoLeave.nUserUID = pPlayer->getUID();
		msgdoLeave.nGameOffset = pPlayer->getOffsetCoin();
		getRoomMgr()->sendMsg(&msgdoLeave, sizeof(msgdoLeave), pPlayer->getSessionID());
		LOGFMTD("player uid = %u , leave room id = %u", pPlayer->getUID(), getRoomID());

		if (eRoomSate_WaitReady == curState || eRoomState_GameEnd == curState)  // when game over or not start , delte player in room data ;
		{
			// tell robot dispatch player leave 
			auto ret = standup(nPlayerUID);
			return ret;
		}
		else
		{
			LOGFMTE("decide player already sync data uid = %u room id = %u", pPlayer->getUID(), getRoomID());
		}
	}
	pPlayer->doTempLeaveRoom();
	onPlayerTrusteedStateChange(pPlayer->getIdx(), true);
	return true;
}

void SZMJRoom::sendPlayersCardInfo(uint32_t nSessionID)
{
	Json::Value jsmsg;
	Json::Value vPeerCards;
	for (auto& pp : m_vMJPlayers)
	{
		if (pp == nullptr /*|| pp->haveState(eRoomPeer_CanAct) == false*/)  // lose also have card 
		{
			continue;
		}

		auto pCard = (SZMJPlayerCard*)pp->getPlayerCard();
		Json::Value jsCardInfo;
		jsCardInfo["idx"] = pp->getIdx();
		jsCardInfo["newMoCard"] = 0;
		if (getCurRoomState()->getStateID() == eRoomState_WaitPlayerAct && getCurRoomState()->getCurIdx() == pp->getIdx())
		{
			jsCardInfo["newMoCard"] = pp->getPlayerCard()->getNewestFetchedCard();
		}

		pCard->getCardInfo(jsCardInfo);
		sendMsgToPlayer(jsCardInfo, MSG_ROOM_PLAYER_CARD_INFO, nSessionID);
	}

	//jsmsg["playersCard"] = vPeerCards;
	//jsmsg["bankerIdx"] = getBankerIdx();
	//jsmsg["curActIdex"] = getCurRoomState()->getCurIdx();
	//jsmsg["leftCardCnt"] = getMJPoker()->getLeftCardCount();
	/*sendMsgToPlayer(jsmsg, MSG_ROOM_PLAYER_CARD_INFO, nSessionID);*/
	LOGFMTD("send player card infos !");
}

bool SZMJRoom::isOneCirleEnd()
{
	return true;
}

void SZMJRoom::onPlayerMingGang( uint8_t nIdx, uint8_t nCard, uint8_t nInvokeIdx )
{
	IMJRoom::onPlayerMingGang(nIdx, nCard, nInvokeIdx);
	auto pActPlayer = getMJPlayerByIdx(nIdx);
	auto pActCard = (SZMJPlayerCard*)pActPlayer->getPlayerCard();
	pActCard->setSongGangIdx(nInvokeIdx);
}

void SZMJRoom::onPlayerChu(uint8_t nIdx, uint8_t nCard)
{
	IMJRoom::onPlayerChu(nIdx, nCard);
	auto pActPlayer = getMJPlayerByIdx(nIdx);
	auto pActCard = (SZMJPlayerCard*)pActPlayer->getPlayerCard();
	pActCard->setSongGangIdx(-1); // reset song gang ;
}

std::shared_ptr<IGameRoomRecorder> SZMJRoom::createRoomRecorder()
{
	return std::make_shared<SZMJRoomRecorder>();
}

uint8_t SZMJRoom::getZiMoHuaRequire()
{
	if (1 == m_nRuleMode)
	{
		return 2;
	}
	else if (2 == m_nRuleMode)
	{
		return 3;
	}
	return 3;
}

uint8_t SZMJRoom::getDianPaoHuHuaRequire()
{
	if (1 == m_nRuleMode)
	{
		return 3;
	}
	else if (2 == m_nRuleMode)
	{
		return 4;
	}
	return 4;
}
