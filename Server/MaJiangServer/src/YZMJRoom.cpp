#include "YZMJRoom.h"
#include "YZMJPlayerCard.h"
#include "log4z.h"
#include "IMJPoker.h"
#include "YZMJPlayer.h"
#include "IGameRoomManager.h"
#include "ServerMessageDefine.h"
#include "MJRoomStateWaitReady.h"
#include "MJRoomStateWaitPlayerChu.h"
#include "MJRoomStateWaitPlayerAct.h"
#include "MJRoomStateStartGame.h"
#include "MJRoomStateGameEnd.h"
#include "MJRoomStateDoPlayerAct.h"
#include "YZRoomStateAskForPengOrHu.h"
#include "MJRoomStateAskForRobotGang.h"
#include "MJRoomStateWaitPlayerChu.h"
#include "YZRoomStateWaitPlayerChu.h"
#include "YZRoomStateDoPlayerAct.h"
#include "YZMJPlayerRecorderInfo.h"
#include "MJReplayFrameType.h"

#include <ctime>

bool YZMJRoom::init(IGameRoomManager* pRoomMgr, stBaseRoomConfig* pConfig, uint32_t nSeialNum, uint32_t nRoomID, Json::Value& vJsValue)
{
	IMJRoom::init(pRoomMgr, pConfig, nSeialNum, nRoomID, vJsValue);

	m_nInitCoin = vJsValue["initCoin"].asUInt();

	m_isJingYuanZi = vJsValue["isJingYuanZi"].asUInt() == 0 ? false : true; // 1:进园子；0:紧淌
	m_nJingYuanZiFen = vJsValue["nJingYuanZiFen"].asUInt(); // 10、20、30
	m_isPeiZi = vJsValue["isPeiZi"].asUInt() == 0 ? false : true; // 1:配子玩法；0:不是配子玩法
	m_isBaiBanPeiZi = vJsValue["isBaiBanPeiZi"].asUInt() == 0 ? false : true; // 1:白板配子玩法；0:不是白板配子玩法
	m_haveYiTiaoLong = vJsValue["haveYiTiaoLong"].asUInt() == 0 ? false : true; // 1:有一条龙番；0:没有一条龙番
	m_have7Pair = vJsValue["have7Pair"].asUInt() == 0 ? false : true; // 1:7对可胡；0:7对不可胡

	m_tPoker.initAllCard(eMJ_COMMON);
	// create state and add state ;
	IMJRoomState* vState[] = {
		new CMJRoomStateWaitReady(), new YZRoomStateWaitPlayerChu(), new MJRoomStateWaitPlayerAct(), new MJRoomStateStartGame()
		, new MJRoomStateGameEnd(), new YZRoomStateDoPlayerAct(), new YZRoomStateAskForPengOrHu(), new MJRoomStateAskForRobotGang()
	};
	for (uint8_t nIdx = 0; nIdx < sizeof(vState) / sizeof(IMJRoomState*); ++nIdx)
	{
		addRoomState(vState[nIdx]);
	}
	setInitState(vState[0]);

	// init banker
	m_nBankerIdx = -1;
	auto pRoomRecorder = (YZMJRoomRecorder*)getRoomRecorder().get();
	//pRoomRecorder->setRoomOpts(m_isOnePay ? 1 : 0);

	m_vSettle.clear();

	return true;
}

void YZMJRoom::willStartGame()
{
	IMJRoom::willStartGame();

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

	m_nBanZiCard = uint8_t(-1);

	m_vSettle.clear();
}

void YZMJRoom::startGame()
{
	IMJRoom::startGame();

	Json::Value jsMsg;
	IMJRoom::packStartGameMsg(jsMsg);

	if (m_isPeiZi)
	{
		//翻开搬子
		if (m_isBaiBanPeiZi)
		{
			m_nBanZiCard = uint8_t(-1);
		} 
		else
		{
			m_nBanZiCard = getMJPoker()->distributeOneCard();
		}
		jsMsg["nBanZiCard"] = m_nBanZiCard;

		for (auto& pPlayer : m_vMJPlayers)
		{
			auto pPlayerCard = (YZMJPlayerCard*)pPlayer->getPlayerCard();
			pPlayerCard->setBanZiCard(m_nBanZiCard);
		}
	}

	for (auto& pPlayer : m_vMJPlayers)
	{
		auto pPlayerCard = (YZMJPlayerCard*)pPlayer->getPlayerCard();
		if (m_haveYiTiaoLong)
		{
			pPlayerCard->setHaveYiTiaoLong();
		}
		if (m_have7Pair)
		{
			pPlayerCard->setHave7Pair();
		}
	}

	sendRoomMsg(jsMsg, MSG_ROOM_YZ_START_GAME);

	// replay arg 
	Json::Value jsReplayInfo;
	jsReplayInfo["roomID"] = getRoomID();
	jsReplayInfo["time"] = (uint32_t)time(nullptr);
	jsReplayInfo["isJingYuanZi"] = m_isJingYuanZi ? 1 : 0;
	jsReplayInfo["JingYuanZiFen"] = m_nJingYuanZiFen;
	jsReplayInfo["isPeiZi"] = m_isPeiZi ? 1 : 0;
	jsReplayInfo["isBaiBanPeiZi"] = m_isBaiBanPeiZi ? 1 : 0;
	jsReplayInfo["haveYiTiaoLong"] = m_haveYiTiaoLong ? 1 : 0;
	jsReplayInfo["have7Pair"] = m_have7Pair ? 1 : 0;
	if (m_isPeiZi)
	{
		jsReplayInfo["BanZiCard"] = m_nBanZiCard;
	}
	getGameReplay()->setReplayRoomInfo(jsReplayInfo);
}

void YZMJRoom::getSubRoomInfo(Json::Value& jsSubInfo)
{
	//MSG_MJ_ROOM_INFO subInfo
	jsSubInfo["isJingYuanZi"] = m_isJingYuanZi ? 1 : 0;
	jsSubInfo["JingYuanZiFen"] = m_nJingYuanZiFen;
	jsSubInfo["isPeiZi"] = m_isPeiZi ? 1 : 0;
	jsSubInfo["isBaiBanPeiZi"] = m_isBaiBanPeiZi ? 1 : 0;
	jsSubInfo["haveYiTiaoLong"] = m_haveYiTiaoLong ? 1 : 0;
	jsSubInfo["have7Pair"] = m_have7Pair ? 1 : 0;
	if (m_isPeiZi)
	{
		jsSubInfo["BanZiCard"] = m_nBanZiCard;
	}
}

void YZMJRoom::onGameDidEnd()
{
	IMJRoom::onGameDidEnd();
	if (getDelegate())
	{
		getDelegate()->onDidGameOver(this);
		return;
	}
}

void YZMJRoom::onGameEnd()
{
	// svr: { isLiuJu : 0 , detail : [ {idx : 0 , offset : 23 }, ...  ], realTimeCal : [ { actType : 23, detial : [ {idx : 2, offset : -23 } ]  } , ... ] } 
	Json::Value jsMsg;
	Json::Value jsDetial;

	bool isAnyOneHu = false;
	for (auto& ref : m_vMJPlayers)
	{
		if (ref && ref->haveState(eRoomPeer_AlreadyHu))
		{
			isAnyOneHu = true;
			continue;
		}
	}

	if (!isAnyOneHu)  // huang zhuang ;
	{
		std::vector<uint32_t> vLoseCoin, vWinCoin, vGangCoinGet, vGangCoinOut;
		for (auto i = 0; i < MAX_SEAT_CNT; ++i)
		{
			vGangCoinGet.push_back(0);
			vGangCoinOut.push_back(0);
		}

		Json::Value jsReal;
		settleInfoToJson(jsReal, vGangCoinGet, vGangCoinOut);
		jsMsg["realTimeCal"] = jsReal;
	}

	jsMsg["nNextBankIdx"] = m_isBankerHu ? m_nBankerIdx : ((m_nBankerIdx + 1) % MAX_SEAT_CNT);

	auto ptrSingleRecorder = getRoomRecorder()->createSingleRoundRecorder();
	ptrSingleRecorder->init(getRoomRecorder()->getRoundRecorderCnt(), (uint32_t)time(nullptr), getGameReplay()->getReplayID());
	getRoomRecorder()->addSingleRoundRecorder(ptrSingleRecorder);

	for (auto& ref : m_vMJPlayers)
	{
		Json::Value js;
		if (ref)
		{
			js["idx"] = ref->getIdx();
			js["offset"] = ref->getOffsetCoin();
			js["final"] = ref->getCoin();
			jsDetial[jsDetial.size()] = js;

			auto pPlayerRecorderInfo = std::make_shared<YZMJPlayerRecorderInfo>();
			pPlayerRecorderInfo->init(ref->getUID(), ref->getOffsetCoin());
			ptrSingleRecorder->addPlayerRecorderInfo(pPlayerRecorderInfo);
		}
	}

	jsMsg["isLiuJu"] = isAnyOneHu ? 0 : 1;
	jsMsg["detail"] = jsDetial;

	sendRoomMsg(jsMsg, MSG_ROOM_YZ_GAME_OVER);
	// send msg to player ;
	IMJRoom::onGameEnd();
}

IMJPlayer* YZMJRoom::doCreateMJPlayer()
{
	return new YZMJPlayer();
}

IMJPoker* YZMJRoom::getMJPoker()
{
	return &m_tPoker;
}

bool YZMJRoom::isGameOver()
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

void YZMJRoom::addSettle(stSettle& tSettle)
{
	m_vSettle.push_back(tSettle);
	// do send message ;
	Json::Value jsMsg;
	jsMsg["actType"] = tSettle.eSettleReason;

	Json::Value jsWin;
	for (auto& ref : tSettle.vWinIdxs)
	{
		Json::Value js;
		js["idx"] = ref.first;
		js["offset"] = ref.second;
		js["isWin"] = 1;
		jsWin[jsWin.size()] = js;
	}
	jsMsg["winers"] = jsWin;

	Json::Value jsLose;
	for (auto& ref : tSettle.vLoseIdx)
	{
		Json::Value js;
		js["idx"] = ref.first;
		js["offset"] = ref.second;
		js["isWin"] = 0;
		jsLose[jsLose.size()] = js;
	}
	jsMsg["loserIdxs"] = jsLose;

	sendRoomMsg(jsMsg, MSG_ROOM_NJ_REAL_TIME_SETTLE);
}

void YZMJRoom::onPlayerMingGang(uint8_t nIdx, uint8_t nCard, uint8_t nInvokeIdx)
{
	IMJRoom::onPlayerMingGang(nIdx, nCard, nInvokeIdx);
	auto pActPlayer = getMJPlayerByIdx(nIdx);
	auto pActPlayerCard = (YZMJPlayerCard*)pActPlayer->getPlayerCard();
	pActPlayerCard->checkCanHuPao();

	//// do cacualte ;
	//stSettle st;
	//st.eSettleReason = eMJAct_MingGang;
	//uint16_t nWin = 0;
	//for (uint8_t nCheckIdx = 0; nCheckIdx < MAX_SEAT_CNT; ++nCheckIdx)
	//{
	//	if (nIdx == nCheckIdx)
	//	{
	//		continue;
	//	}

	//	auto pPlayer = getMJPlayerByIdx(nCheckIdx);
	//	uint16_t nLose = 1;

	//	if (m_isJingYuanZi && pPlayer->getCoin() - nLose <= m_nInitCoin - m_nJingYuanZiFen)
	//	{
	//		if (pPlayer->getCoin() + m_nJingYuanZiFen > m_nInitCoin)
	//		{
	//			nLose = pPlayer->getCoin() + m_nJingYuanZiFen - m_nInitCoin;
	//		}
	//		else
	//		{
	//			nLose = 0;
	//			continue;
	//		}
	//	}

	//	pPlayer->addOffsetCoin(-1 * (int32_t)nLose);
	//	st.addLose(nCheckIdx, nLose);

	//	nWin += nLose;
	//}
	//pActPlayer->addOffsetCoin(nWin);
	//st.addWin(nIdx, nWin);
	//addSettle(st);

	auto pInvokerPlayer = getMJPlayerByIdx(nInvokeIdx);
	// do cacualte ;
	stSettle st;
	st.eSettleReason = eMJAct_MingGang;
	uint16_t nLose = 2;
	if (m_isJingYuanZi && pInvokerPlayer->getCoin() - nLose <= m_nInitCoin - m_nJingYuanZiFen)
	{
		if (pInvokerPlayer->getCoin() + m_nJingYuanZiFen > m_nInitCoin)
		{
			nLose = pInvokerPlayer->getCoin() + m_nJingYuanZiFen - m_nInitCoin;
		}
		else
		{
			nLose = 0;
			return;
		}
	}

	pInvokerPlayer->addOffsetCoin(-1 * (int32_t)nLose);
	st.addLose(nInvokeIdx, nLose);
	pActPlayer->addOffsetCoin(nLose);
	st.addWin(nIdx, nLose);
	addSettle(st);

	if (isInternalShouldClosedAll())
	{
		goToState(eRoomState_GameEnd);
	}
}

void YZMJRoom::onPlayerAnGang(uint8_t nIdx, uint8_t nCard)
{
	IMJRoom::onPlayerAnGang(nIdx, nCard);

	// do caculate ;
	stSettle st;
	st.eSettleReason = eMJAct_AnGang;
	uint16_t nWin = 0;
	for (uint8_t nCheckIdx = 0; nCheckIdx < MAX_SEAT_CNT; ++nCheckIdx)
	{
		if (nIdx == nCheckIdx)
		{
			continue;
		}

		auto pPlayer = getMJPlayerByIdx(nCheckIdx);
		uint16_t nLose = 2;

		if (m_isJingYuanZi && pPlayer->getCoin() - nLose <= m_nInitCoin - m_nJingYuanZiFen)
		{
			if (pPlayer->getCoin() + m_nJingYuanZiFen > m_nInitCoin)
			{
				nLose = pPlayer->getCoin() + m_nJingYuanZiFen - m_nInitCoin;
			}
			else
			{
				nLose = 0;
				continue;
			}
		}

		pPlayer->addOffsetCoin(-1 * (int32_t)nLose);
		st.addLose(nCheckIdx, nLose);

		nWin += nLose;
	}
	auto pPlayerWin = getMJPlayerByIdx(nIdx);
	pPlayerWin->addOffsetCoin(nWin);
	st.addWin(nIdx, nWin);
	addSettle(st);

	auto pActPlayerCard = (YZMJPlayerCard*)pPlayerWin->getPlayerCard();
	pActPlayerCard->checkCanHuPao();

	if (isInternalShouldClosedAll())
	{
		goToState(eRoomState_GameEnd);
	}
}

void YZMJRoom::onPlayerBuGang(uint8_t nIdx, uint8_t nCard)
{
	IMJRoom::onPlayerBuGang(nIdx, nCard);
	auto pActPlayer = getMJPlayerByIdx(nIdx);
	auto pActPlayerCard = (YZMJPlayerCard*)pActPlayer->getPlayerCard();
	pActPlayerCard->checkCanHuPao();

	std::vector<YZMJPlayerCard::tSongGangIdx> vSongGangs = pActPlayerCard->getPengSongIdx();
	uint8_t nInvokeIdx = uint8_t(-1);
	for (auto tSong : vSongGangs)
	{
		if (tSong.nGangCard == nCard)
		{
			nInvokeIdx = tSong.nIdx;
			break;
		}
	}
	if (nInvokeIdx == uint8_t(-1))
	{
		LOGFMTE("onPlayerBuGang, not found peng card(%u) song idx!", nCard);
		return;
	}

	//auto pInvokerPlayer = getMJPlayerByIdx(nInvokeIdx);

	//// do cacualte ;
	//stSettle st;
	//st.eSettleReason = eMJAct_BuGang;
	//uint16_t nLose = 1;
	//if (m_isJingYuanZi && pInvokerPlayer->getCoin() - nLose <= m_nInitCoin - m_nJingYuanZiFen)
	//{
	//	if (pInvokerPlayer->getCoin() + m_nJingYuanZiFen > m_nInitCoin)
	//	{
	//		nLose = pInvokerPlayer->getCoin() + m_nJingYuanZiFen - m_nInitCoin;
	//	}
	//	else
	//	{
	//		nLose = 0;
	//		return;
	//	}
	//}

	//pInvokerPlayer->addOffsetCoin(-1 * (int32_t)nLose);
	//st.addLose(nInvokeIdx, nLose);
	//pActPlayer->addOffsetCoin(nLose);
	//st.addWin(nIdx, nLose);
	//addSettle(st);

	std::vector<uint8_t> vCanNotGang = pActPlayerCard->getCanNotGang();
	bool bCanNotGang = false;
	for (auto c : vCanNotGang)
	{
		if (c == nCard)
		{
			bCanNotGang = true;
			break;
		}
	}
	if (!bCanNotGang)
	{
		// do cacualte ;
		stSettle st;
		st.eSettleReason = eMJAct_BuGang;
		uint16_t nWin = 0;
		for (uint8_t nCheckIdx = 0; nCheckIdx < MAX_SEAT_CNT; ++nCheckIdx)
		{
			if (nIdx == nCheckIdx)
			{
				continue;
			}

			auto pPlayer = getMJPlayerByIdx(nCheckIdx);
			uint16_t nLose = 1;

			if (m_isJingYuanZi && pPlayer->getCoin() - nLose <= m_nInitCoin - m_nJingYuanZiFen)
			{
				if (pPlayer->getCoin() + m_nJingYuanZiFen > m_nInitCoin)
				{
					nLose = pPlayer->getCoin() + m_nJingYuanZiFen - m_nInitCoin;
				}
				else
				{
					nLose = 0;
					continue;
				}
			}

			pPlayer->addOffsetCoin(-1 * (int32_t)nLose);
			st.addLose(nCheckIdx, nLose);

			nWin += nLose;
		}
		pActPlayer->addOffsetCoin(nWin);
		st.addWin(nIdx, nWin);
		addSettle(st);
	}

	if (isInternalShouldClosedAll())
	{
		goToState(eRoomState_GameEnd);
	}
}

void YZMJRoom::onPlayerPeng(uint8_t nIdx, uint8_t nCard, uint8_t nInvokeIdx)
{
	IMJRoom::onPlayerPeng(nIdx, nCard, nInvokeIdx);

	auto pPlayer = getMJPlayerByIdx(nIdx);
	auto pPlayerCard = (YZMJPlayerCard*)pPlayer->getPlayerCard();
	std::vector<uint8_t> vHoldCard;
	pPlayerCard->getHoldCard(vHoldCard);
	auto it = std::find(vHoldCard.begin(), vHoldCard.end(), nCard);
	if (it != vHoldCard.end())
	{
		pPlayerCard->addCanNotGang(nCard);
	}
	pPlayerCard->addSongPengIdx(nCard, nInvokeIdx);

	// 碰搬子算明杠
	if (nCard == m_nBanZiCard)
	{
		stSettle st;
		st.eSettleReason = eMJAct_MingGang;

		auto pLosePlayer = getMJPlayerByIdx(nInvokeIdx);
		uint16_t nLose = 2;

		if (m_isJingYuanZi && pLosePlayer->getCoin() - nLose <= m_nInitCoin - m_nJingYuanZiFen)
		{
			if (pLosePlayer->getCoin() + m_nJingYuanZiFen > m_nInitCoin)
			{
				nLose = pLosePlayer->getCoin() + m_nJingYuanZiFen - m_nInitCoin;
			}
			else
			{
				nLose = 0;
				return;
			}
		}

		pLosePlayer->addOffsetCoin(-1 * (int32_t)nLose);
		st.addLose(nInvokeIdx, nLose);

		pPlayer->addOffsetCoin(nLose);
		st.addWin(nIdx, nLose);
		addSettle(st);

		if (isInternalShouldClosedAll())
		{
			goToState(eRoomState_GameEnd);
		}
	}
}

void YZMJRoom::onPlayerMo(uint8_t nIdx)
{
	IMJRoom::onPlayerMo(nIdx);

	if (m_isPeiZi)
	{
		auto pPlayer = getMJPlayerByIdx(nIdx);
		auto pPlayerCard = (YZMJPlayerCard*)pPlayer->getPlayerCard();
		pPlayerCard->checkCanHuPao();
	}
}

void YZMJRoom::settleInfoToJson(Json::Value& jsRealTime, std::vector<uint32_t> vGangCoinGet, std::vector<uint32_t> vGangCoinOut)
{
	for (auto& ref : m_vSettle)
	{
		Json::Value jsItem, jsRDetail;
		jsItem["actType"] = ref.eSettleReason;

		for (auto& refl : ref.vLoseIdx)
		{
			Json::Value jsPlayer;
			jsPlayer["idx"] = refl.first;
			jsPlayer["offset"] = -1 * refl.second;
			jsRDetail[jsRDetail.size()] = jsPlayer;

			vGangCoinOut[refl.first] += refl.second;
		}

		for (auto& refl : ref.vWinIdxs)
		{
			Json::Value jsPlayer;
			jsPlayer["idx"] = refl.first;
			jsPlayer["offset"] = refl.second;
			jsRDetail[jsRDetail.size()] = jsPlayer;

			vGangCoinGet[refl.first] += refl.second;
		}
		jsItem["detial"] = jsRDetail;
		jsRealTime[jsRealTime.size()] = jsItem;
	}
}

void YZMJRoom::onPlayerHu(std::vector<uint8_t>& vHuIdx, uint8_t nCard, uint8_t nInvokeIdx)
{
	if (vHuIdx.empty())
	{
		LOGFMTE("why hu vec is empty ? room id = %u", getRoomID());
		return;
	}

	auto iterBankWin = std::find(vHuIdx.begin(), vHuIdx.end(), getBankerIdx());
	m_isBankerHu = iterBankWin != vHuIdx.end();

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

	for (auto pCheckPlayer : m_vMJPlayers)
	{
		auto pCheckPlayerCard = (YZMJPlayerCard*)pCheckPlayer->getPlayerCard();

		//三个搬子算暗杠
		if (m_isPeiZi && !m_isBaiBanPeiZi && pCheckPlayerCard->haveThreeBanZi())
		{
			// do caculate ;
			stSettle st;
			st.eSettleReason = eMJAct_AnGang;
			uint16_t nWin = 0;
			for (uint8_t nCheckIdx = 0; nCheckIdx < MAX_SEAT_CNT; ++nCheckIdx)
			{
				if (pCheckPlayer->getIdx() == nCheckIdx)
				{
					continue;
				}

				auto pPlayer = getMJPlayerByIdx(nCheckIdx);
				uint16_t nLose = 2;

				if (m_isJingYuanZi && pPlayer->getCoin() - nLose <= m_nInitCoin - m_nJingYuanZiFen)
				{
					if (pPlayer->getCoin() + m_nJingYuanZiFen > m_nInitCoin)
					{
						nLose = pPlayer->getCoin() + m_nJingYuanZiFen - m_nInitCoin;
					}
					else
					{
						nLose = 0;
						continue;
					}
				}

				pPlayer->addOffsetCoin(-1 * (int32_t)nLose);
				st.addLose(nCheckIdx, nLose);

				nWin += nLose;
			}
			auto pPlayerWin = getMJPlayerByIdx(pCheckPlayer->getIdx());
			pPlayerWin->addOffsetCoin(nWin);
			st.addWin(pCheckPlayer->getIdx(), nWin);
			addSettle(st);
		}
	}

	Json::Value jsDetail;
	Json::Value jsMsg;

	bool isZiMo = vOrderHu.at(0) == nInvokeIdx;
	jsMsg["isZiMo"] = isZiMo ? 1 : 0;
	jsMsg["huCard"] = nCard;

	std::vector<uint32_t> vLoseCoin, vWinCoin, vGangCoinGet, vGangCoinOut;
	for (auto i = 0; i < MAX_SEAT_CNT; ++i)
	{
		vLoseCoin.push_back(0);
		vWinCoin.push_back(0);
		vGangCoinGet.push_back(0);
		vGangCoinOut.push_back(0);
	}

	Json::Value jsReal;
	settleInfoToJson(jsReal, vGangCoinGet, vGangCoinOut);
	jsMsg["realTimeCal"] = jsReal;

	if (isZiMo)
	{
		onPlayerZiMo(nInvokeIdx, nCard, jsDetail, vWinCoin, vLoseCoin, vGangCoinGet, vGangCoinOut);
		jsMsg["detail"] = jsDetail;
		sendRoomMsg(jsMsg, MSG_ROOM_YZ_PLAYER_HU);
		return;
	}

	auto pLosePlayer = getMJPlayerByIdx(nInvokeIdx);
	if (!pLosePlayer)
	{
		LOGFMTE("room id = %u lose but player idx = %u is nullptr", getRoomID(), nInvokeIdx);
		return;
	}

	//{ dianPaoIdx : 23 , isRobotGang : 0 , nLose : 23, huPlayers : [{ idx : 234 , win : 234 , baoPaiIdx : 2 , huardSoftHua : 23, vhuTypes : [ eFanxing , ] } , .... ] } 
	jsDetail["dianPaoIdx"] = nInvokeIdx;
	jsDetail["isRobotGang"] = pLosePlayer->haveDecareBuGangFalg() ? 1 : 0;
	pLosePlayer->addDianPaoCnt();
	Json::Value jsHuPlayers;

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

		auto pHuPlayerCard = (YZMJPlayerCard*)pHuPlayer->getPlayerCard();

		std::vector<uint16_t> vType;
		int32_t nAllFanCnt = 0;
		pHuPlayerCard->onDoHu(false, nCard, vType, nAllFanCnt);

		Json::Value jsHuTyps;
		for (auto& refHu : vType)
		{
			jsHuTyps[jsHuTyps.size()] = refHu;
		}
		jsHuPlayer["vhuTypes"] = jsHuTyps;

		uint32_t nLoseCoin = nAllFanCnt;
		if (m_isJingYuanZi && pLosePlayer->getCoin() - nAllFanCnt <= m_nInitCoin - m_nJingYuanZiFen)
		{
			if (pLosePlayer->getCoin() + m_nJingYuanZiFen > m_nInitCoin)
			{
				nLoseCoin = pLosePlayer->getCoin() + m_nJingYuanZiFen - m_nInitCoin;
			}
			else
			{
				nLoseCoin = 0;
			}
		}
		pLosePlayer->addOffsetCoin(-1 * (int32_t)nLoseCoin);
		pHuPlayer->addOffsetCoin(nLoseCoin);

		vLoseCoin[nInvokeIdx] += nLoseCoin;
		vWinCoin[nHuIdx] += nLoseCoin;

		jsHuPlayer["win"] = nLoseCoin;
		jsHuPlayers[jsHuPlayers.size()] = jsHuPlayer;
	}
	jsDetail["huPlayers"] = jsHuPlayers;
	jsDetail["zhuangPlayerIdx"] = m_nBankerIdx;
	
	Json::Value jsVLoses;
	for (auto i = 0; i < MAX_SEAT_CNT; ++i)
	{
		int nCoin = vWinCoin[i] - vLoseCoin[i] + vGangCoinGet[i] - vGangCoinOut[i];
		Json::Value jsLose;
		jsLose["loseCoin"] = nCoin;
		jsLose["idx"] = i;
		jsVLoses[jsVLoses.size()] = jsLose;
	}
	jsDetail["LoseIdxs"] = jsVLoses;
	
	jsMsg["detail"] = jsDetail;
	sendRoomMsg(jsMsg, MSG_ROOM_YZ_PLAYER_HU);
	LOGFMTD("room id = %u hu end ", getRoomID());
}

void YZMJRoom::onPlayerZiMo(uint8_t nIdx, uint8_t nCard, Json::Value& jsDetail, std::vector<uint32_t> vWinCoin, std::vector<uint32_t> vLoseCoin,
	std::vector<uint32_t> vGangCoinGet, std::vector<uint32_t> vGangCoinOut)
{
	auto pZiMoPlayer = (YZMJPlayer*)getMJPlayerByIdx(nIdx);
	if (pZiMoPlayer == nullptr)
	{
		LOGFMTE("room id = %u zi mo player is nullptr idx = %u ", getRoomID(), nIdx);
		return;
	}
	pZiMoPlayer->addZiMoCnt();
	pZiMoPlayer->setState(eRoomPeer_AlreadyHu);
	// svr :{ huIdx : 234 , baoPaiIdx : 2 , winCoin : 234,huardSoftHua : 23, isGangKai : 0 ,vhuTypes : [ eFanxing , ], LoseIdxs : [ {idx : 1 , loseCoin : 234 }, .... ]   }
	jsDetail["huIdx"] = nIdx;

	auto pHuPlayerCard = (YZMJPlayerCard*)pZiMoPlayer->getPlayerCard();

	std::vector<uint16_t> vType;
	int32_t nAllFanCnt = 0;
	pHuPlayerCard->onDoHu(true, nCard, vType, nAllFanCnt);

	Json::Value jsHuTyps;
	for (auto& refHu : vType)
	{
		jsHuTyps[jsHuTyps.size()] = refHu;
	}
	jsDetail["vhuTypes"] = jsHuTyps;

	int32_t nTotalWin = 0;
	for (auto pPlayer : m_vMJPlayers)
	{
		if (pPlayer->getIdx() != nIdx) // 另外三家分别计算输的coin，总和就是赢家获得的
		{
			uint32_t nLoseCoin = nAllFanCnt;
			if (m_isJingYuanZi && pPlayer->getCoin() - nAllFanCnt <= m_nInitCoin - m_nJingYuanZiFen)
			{
				if (pPlayer->getCoin() + m_nJingYuanZiFen > m_nInitCoin)
				{
					nLoseCoin = pPlayer->getCoin() + m_nJingYuanZiFen - m_nInitCoin;
				}
				else
				{
					nLoseCoin = 0;
				}
			}
			pPlayer->addOffsetCoin(-1 * (int32_t)nLoseCoin);
			pZiMoPlayer->addOffsetCoin(nLoseCoin);

			vLoseCoin[pPlayer->getIdx()] += nLoseCoin;
			vWinCoin[nIdx] += nLoseCoin;

			nTotalWin += nLoseCoin;
		}
	}
	jsDetail["winCoin"] = nTotalWin;

	jsDetail["isGangKai"] = 0;
	if (pZiMoPlayer->haveGangFalg())
	{
		jsDetail["isGangKai"] = 1;
	}
	jsDetail["invokerGangIdx"] = nIdx;
	jsDetail["zhuangPlayerIdx"] = m_nBankerIdx;

	Json::Value jsVLoses;
	for (auto i = 0; i < MAX_SEAT_CNT; ++i)
	{
		int nCoin = vWinCoin[i] - vLoseCoin[i] + vGangCoinGet[i] - vGangCoinOut[i];
		Json::Value jsLose;
		jsLose["loseCoin"] = nCoin;
		jsLose["idx"] = i;
		jsVLoses[jsVLoses.size()] = jsLose;
	}
	jsDetail["LoseIdxs"] = jsVLoses;

	LOGFMTD("room id = %u hu end ", getRoomID());
}

bool YZMJRoom::onPlayerApplyLeave(uint32_t nPlayerUID)
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

void YZMJRoom::sendPlayersCardInfo(uint32_t nSessionID)
{
	Json::Value jsmsg;
	Json::Value vPeerCards;
	for (auto& pp : m_vMJPlayers)
	{
		if (pp == nullptr /*|| pp->haveState(eRoomPeer_CanAct) == false*/)  // lose also have card 
		{
			continue;
		}

		auto pCard = (YZMJPlayerCard*)pp->getPlayerCard();
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

	LOGFMTD("send player card infos !");
}

std::shared_ptr<IGameRoomRecorder> YZMJRoom::createRoomRecorder()
{
	return std::make_shared<YZMJRoomRecorder>();
}

bool YZMJRoom::isInternalShouldClosedAll()
{
	if (m_isJingYuanZi)
	{
		int nCount = 0;
		for (auto& ref : m_vMJPlayers)
		{
			if (ref && ref->getCoin() + m_nJingYuanZiFen <= m_nInitCoin)
			{
				nCount++;
			}
		}
		if (nCount >= 2)
		{
			return true;
		}
	}

	return false;
}

bool YZMJRoom::isAnyPlayerPengOrHuThisCard(uint8_t nInvokeIdx, uint8_t nCard)
{
	for (auto& ref : m_vMJPlayers)
	{
		if (ref == nullptr || nInvokeIdx == ref->getIdx())
		{
			continue;
		}

		auto pMJCard = ref->getPlayerCard();
		if (pMJCard->canPengWithCard(nCard))
		{
			return true;
		}

		if (pMJCard->canHuWitCard(nCard))
		{
			auto pPlayer = getMJPlayerByIdx(nInvokeIdx);
			if (m_isJingYuanZi)
			{
				if (pPlayer->getCoin() > m_nInitCoin - m_nJingYuanZiFen)
				{
					return true;
				}
			}
			else
			{
				return true;
			}
		}

		if (isCanGoOnMoPai() && pMJCard->canMingGangWithCard(nCard)) // must can gang , will not run here , will return when check peng ;
		{
			return true;
		}
	}

	return false;
}

void YZMJRoom::onAskForPengOrHuThisCard(uint8_t nInvokeIdx, uint8_t nCard, std::vector<uint8_t>& vWaitHuIdx, std::vector<uint8_t>& vWaitPengGangIdx, bool& isNeedWaitEat)
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
		if (isCanGoOnMoPai() && pMJCard->canMingGangWithCard(nCard))
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
			auto pPlayer = getMJPlayerByIdx(nInvokeIdx);
			if (m_isJingYuanZi)
			{
				if (pPlayer->getCoin() > m_nInitCoin - m_nJingYuanZiFen)
				{
					jsActs[jsActs.size()] = eMJAct_Hu;
					vWaitHuIdx.push_back(ref->getIdx());
				}
			}
			else
			{
				jsActs[jsActs.size()] = eMJAct_Hu;
				vWaitHuIdx.push_back(ref->getIdx());
			}
		}

		if (jsActs.size() > 0)
		{
			jsActs[jsActs.size()] = eMJAct_Pass;
		}

		if (jsActs.size() == 0)
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
