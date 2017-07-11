#include "BPMJRoom.h"
#include "BPMJPlayerCard.h"
#include "log4z.h"
#include "IMJPoker.h"
#include "BPMJPlayer.h"
#include "IGameRoomManager.h"
#include "ServerMessageDefine.h"
#include "MJRoomStateWaitReady.h"
#include "MJRoomStateWaitPlayerChu.h"
#include "MJRoomStateWaitPlayerAct.h"
#include "MJRoomStateStartGame.h"
#include "MJRoomStateGameEnd.h"
#include "MJRoomStateDoPlayerAct.h"
#include "BPRoomStateAskForPengOrHu.h"
#include "MJRoomStateAskForRobotGang.h"
#include "BPRoomStateDoPlayerAct.h"
#include "MJRoomStateWaitPlayerAct.h"
#include "BPRoomStateWaitPlayerChu.h"
#include "BPMJPlayerRecorderInfo.h"
#include "MJReplayFrameType.h"

#include <ctime>

bool BPMJRoom::init(IGameRoomManager* pRoomMgr, stBaseRoomConfig* pConfig, uint32_t nSeialNum, uint32_t nRoomID, Json::Value& vJsValue)
{
	IMJRoom::init(pRoomMgr, pConfig, nSeialNum, nRoomID, vJsValue);
	m_isBankerHu = false;
	m_isOnePay = vJsValue["isOnePay"].asUInt() == 0 ? false : true;
	m_isBiMenHu = vJsValue["isBiMenHu"].asUInt() == 0 ? false : true;
	m_isJiaHu = vJsValue["isJiaHu"].asUInt() == 0 ? false : true;
	m_isHuiPai = vJsValue["isHuiPai"].asUInt() == 0 ? false : true;
	//m_isQiongHu = vJsValue["isQiongHu"].asUInt() == 0 ? false : true;
	m_is7Pair = vJsValue["is7Pair"].asUInt() == 0 ? false : true;
	m_isJiXiaDan = vJsValue["isJiXiaDan"].asUInt() == 0 ? false : true;
	m_isGangLiuLei = vJsValue["isGangLiuLei"].asUInt() == 0 ? false : true;
	m_tPoker.initAllCard(eMJ_COMMON);
	// create state and add state ;
	IMJRoomState* vState[] = {
		new CMJRoomStateWaitReady(), new BPRoomStateWaitPlayerChu(), new MJRoomStateWaitPlayerAct(), new MJRoomStateStartGame()
		, new MJRoomStateGameEnd(), new BPRoomStateDoPlayerAct(), new BPRoomStateAskForPengOrHu(), new MJRoomStateAskForRobotGang()
	};
	for (uint8_t nIdx = 0; nIdx < sizeof(vState) / sizeof(IMJRoomState*); ++nIdx)
	{
		addRoomState(vState[nIdx]);
	}
	setInitState(vState[0]);

	// init banker
	m_nBankerIdx = -1;
	m_nHuiCard = uint8_t(-1);
	m_vSettle.clear();
	auto pRoomRecorder = (BPMJRoomRecorder*)getRoomRecorder().get();
	pRoomRecorder->setRoomOpts(m_isOnePay ? 1 : 0);

	return true;
}

void BPMJRoom::willStartGame()
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
	m_vSettle.clear();

	for (auto& pPlayer : m_vMJPlayers)
	{
		if (pPlayer)
		{
			((BPMJPlayer*)pPlayer)->ClearBiMen();
			((BPMJPlayer*)pPlayer)->SetGangMo(false);
			((BPMJPlayer*)pPlayer)->SetIsBiMenHu(m_isBiMenHu);
			((BPMJPlayer*)pPlayer)->SetIsHuiPai(m_isHuiPai);
			((BPMJPlayer*)pPlayer)->SetIs7Pair(m_is7Pair);
		}
	}
}

void BPMJRoom::startGame()
{
	IMJRoom::startGame();
	Json::Value jsMsg;
	
	if (m_isHuiPai)
	{
		m_nHuiCard = getMJPoker()->distributeOneCard();

		for (auto& pPlayer : m_vMJPlayers)
		{
			auto pPlayerCard = (BPMJPlayerCard*)pPlayer->getPlayerCard();
			pPlayerCard->setHuiCard(m_nHuiCard);
		}
		jsMsg["nHuiCard"] = m_nHuiCard;
	}
	
	IMJRoom::packStartGameMsg(jsMsg);
	sendRoomMsg(jsMsg, MSG_ROOM_START_GAME);

	// replay arg 
	Json::Value jsReplayInfo;
	jsReplayInfo["roomID"] = getRoomID();
	jsReplayInfo["time"] = (uint32_t)time(nullptr);
	jsReplayInfo["isOnePay"] = m_isOnePay ? 1 : 0;
	jsReplayInfo["isBiMenHu"] = m_isBiMenHu ? 1 : 0;
	jsReplayInfo["isJiaHu"] = m_isJiaHu ? 1 : 0;
	jsReplayInfo["isHuiPai"] = m_isHuiPai ? 1 : 0;
	//jsReplayInfo["isQiongHu"] = m_isQiongHu ? 1 : 0;
	jsReplayInfo["is7Pair"] = m_is7Pair ? 1 : 0;
	jsReplayInfo["isJiXiaDan"] = m_isJiXiaDan ? 1 : 0;
	jsReplayInfo["isGangLiuLei"] = m_isGangLiuLei ? 1 : 0;
	jsReplayInfo["huiCard"] = m_nHuiCard;

	getGameReplay()->setReplayRoomInfo(jsReplayInfo);
}

void BPMJRoom::getSubRoomInfo(Json::Value& jsSubInfo)
{
	jsSubInfo["isOnePay"] = m_isOnePay ? 1 : 0;
	jsSubInfo["isBiMenHu"] = m_isBiMenHu ? 1 : 0;
	jsSubInfo["isJiaHu"] = m_isJiaHu ? 1 : 0;
	jsSubInfo["isHuiPai"] = m_isHuiPai ? 1 : 0;
	//jsSubInfo["isQiongHu"] = m_isQiongHu ? 1 : 0;
	jsSubInfo["is7Pair"] = m_is7Pair ? 1 : 0;
	jsSubInfo["isJiXiaDan"] = m_isJiXiaDan ? 1 : 0;
	jsSubInfo["isGangLiuLei"] = m_isGangLiuLei ? 1 : 0;
	jsSubInfo["huiCard"] = m_nHuiCard;
}

void BPMJRoom::onGameDidEnd()
{
	IMJRoom::onGameDidEnd();
	if (getDelegate())
	{
		getDelegate()->onDidGameOver(this);
		return;
	}
}

void BPMJRoom::onGameEnd()
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
		std::vector<uint32_t> vGangCoinGet, vGangCoinOut;
		for (auto i = 0; i < MAX_SEAT_CNT; ++i)
		{
			vGangCoinGet.push_back(0);
			vGangCoinOut.push_back(0);
		}

		Json::Value jsReal;
		calculateGangCoin(jsReal, vGangCoinGet, vGangCoinOut, m_isOnePay);
		//Json::Value jsReal;
		//settleInfoToJson(jsReal);
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

			auto pPlayerRecorderInfo = std::make_shared<BPMJPlayerRecorderInfo>();
			pPlayerRecorderInfo->init(ref->getUID(), ref->getOffsetCoin());
			ptrSingleRecorder->addPlayerRecorderInfo(pPlayerRecorderInfo);
		}
	}

	jsMsg["isLiuJu"] = isAnyOneHu ? 0 : 1;
	jsMsg["detail"] = jsDetial;

	sendRoomMsg(jsMsg, MSG_ROOM_BP_GAME_OVER);
	// send msg to player ;
	IMJRoom::onGameEnd();
}

IMJPlayer* BPMJRoom::doCreateMJPlayer()
{
	return new BPMJPlayer();
}

IMJPoker* BPMJRoom::getMJPoker()
{
	return &m_tPoker;
}

bool BPMJRoom::isGameOver()
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

void BPMJRoom::onPlayerTing(uint8_t nIdx/*, std::vector<uint8_t> vecTingCards*/)
{
	Json::Value msg;
	msg["idx"] = nIdx;
	msg["actType"] = eMJAct_Ting;
	
	sendRoomMsg(msg, MSG_ROOM_ACT);

	//Json::Value jsTingCard;
	//for (size_t i = 0; i < vecTingCards.size(); ++i)
	//{
	//	jsTingCard[i] = vecTingCards.at(i);
	//}

	//auto pPlayer = getMJPlayerByIdx(nIdx);
	//Json::Value jsMsg;
	//jsMsg["tingCard"] = jsTingCard;
	//sendMsgToPlayer(jsMsg, MSG_TING_CARD, pPlayer->getSessionID());

	Json::Value jsFrameArg;
	auto ptrReplay = getGameReplay()->createFrame(eMJFrame_Player_Ting, (uint32_t)time(nullptr));
	jsFrameArg["idx"] = nIdx;
	ptrReplay->setFrameArg(jsFrameArg);
	getGameReplay()->addFrame(ptrReplay);
}

void BPMJRoom::onPlayerPeng(uint8_t nIdx, uint8_t nCard, uint8_t nInvokeIdx)
{
	IMJRoom::onPlayerPeng(nIdx, nCard, nInvokeIdx);
	auto pActPlayer = getMJPlayerByIdx(nIdx);
	auto pActCard = (BPMJPlayerCard*)pActPlayer->getPlayerCard();
	pActCard->addSongPengIdx(nCard, nInvokeIdx);
}

void BPMJRoom::onPlayerEat(uint8_t nIdx, uint8_t nCard, uint8_t nWithA, uint8_t nWithB, uint8_t nInvokeIdx)
{
	IMJRoom::onPlayerEat(nIdx, nCard, nWithA, nWithB, nInvokeIdx);

	Json::Value jsFrameArg;
	auto ptrReplay = getGameReplay()->createFrame(eMJFrame_Eat, (uint32_t)time(nullptr));
	jsFrameArg["idx"] = nIdx;
	jsFrameArg["withA"] = nWithA;
	jsFrameArg["withB"] = nWithB;
	jsFrameArg["card"] = nCard;
	jsFrameArg["invokerIdx"] = nInvokeIdx;
	ptrReplay->setFrameArg(jsFrameArg);
	getGameReplay()->addFrame(ptrReplay);
}

void BPMJRoom::calculateGangCoin(Json::Value& jsMsg, std::vector<uint32_t>& vGangCoinGet, std::vector<uint32_t>& vGangCoinOut, bool isOnePay)
{
	//realTimeCal: [{ actType: 23, detial : [{idx: 2, offset : -23 }, ...]  }, ...]
	for (auto pWinPlayer : m_vMJPlayers)
	{
		auto pPlayerCard = (BPMJPlayerCard*)pWinPlayer->getPlayerCard();
		auto nWinIdx = pWinPlayer->getIdx();
		
		std::vector<uint8_t> vAnGang;
		pPlayerCard->getAnGangedCard(vAnGang);
		for (auto i=0; i<vAnGang.size(); ++i)
		{
			Json::Value jsReal;
			jsReal["actType"] = eMJAct_AnGang;
			Json::Value jsDetial;

			Json::Value jsWin;
			jsWin["idx"] = nWinIdx;
			jsWin["offset"] = (MAX_SEAT_CNT - 1) * 4;
			jsDetial[jsDetial.size()] = jsWin;

			vGangCoinGet[nWinIdx] += (MAX_SEAT_CNT - 1) * 4;
			pWinPlayer->addOffsetCoin((MAX_SEAT_CNT - 1) * 4);

			for (auto idx = 0; idx < MAX_SEAT_CNT; ++idx)
			{
				if (idx != nWinIdx)
				{
					Json::Value jsLose;
					jsLose["idx"] = idx;
					jsLose["offset"] = -4;
					jsDetial[jsDetial.size()] = jsLose;

					vGangCoinOut[idx] += 4;

					auto pLosePlayer = getMJPlayerByIdx(idx);
					pLosePlayer->addOffsetCoin(-4);
				}
			}
			jsReal["detial"] = jsDetial;
			jsMsg[jsMsg.size()] = jsReal;
		}

		std::vector<BPMJPlayerCard::tSongGangIdx> vMingGang = pPlayerCard->getMingGangSongIdx();
		for (auto i = 0; i < vMingGang.size(); ++i)
		{
			Json::Value jsReal;
			jsReal["actType"] = vMingGang[i].eAct;
			Json::Value jsDetial;

			Json::Value jsWin;
			jsWin["idx"] = nWinIdx;
			auto nWinCoin = (MAX_SEAT_CNT - 1) * 2;
			jsWin["offset"] = nWinCoin;
			jsDetial[jsDetial.size()] = jsWin;

			vGangCoinGet[nWinIdx] += nWinCoin;
			pWinPlayer->addOffsetCoin(nWinCoin);

			for (auto idx = 0; idx < MAX_SEAT_CNT; ++idx)
			{
				if (idx != nWinIdx)
				{
					auto nLoseCoin = 2;
					Json::Value jsLose;
					jsLose["idx"] = idx;
					jsLose["offset"] = -nLoseCoin;
					jsDetial[jsDetial.size()] = jsLose;

					vGangCoinOut[idx] += nLoseCoin;

					auto pLosePlayer = getMJPlayerByIdx(idx);
					pLosePlayer->addOffsetCoin(-nLoseCoin);
				}
			}

			jsReal["detial"] = jsDetial;
			jsMsg[jsMsg.size()] = jsReal;
		}
	}

	for (auto st : m_vSettle)
	{
		Json::Value jsReal;
		jsReal["actType"] = st.eSettleReason;
		Json::Value jsDetial;

		for (auto winIdx : st.vWinIdxs)
		{
			Json::Value jsWin;
			jsWin["idx"] = winIdx.first;
			jsWin["offset"] = winIdx.second;
			jsDetial[jsDetial.size()] = jsWin;
			vGangCoinGet[winIdx.first] += winIdx.second;
			auto pWinPlayer = getMJPlayerByIdx(winIdx.first);
			pWinPlayer->addOffsetCoin(winIdx.second);
		}
		for (auto loseIdx : st.vLoseIdx)
		{
			Json::Value jsLose;
			jsLose["idx"] = loseIdx.first;
			jsLose["offset"] = -loseIdx.second;
			jsDetial[jsDetial.size()] = jsLose;
			vGangCoinOut[loseIdx.first] += loseIdx.second;
			auto pLosePlayer = getMJPlayerByIdx(loseIdx.first);
			pLosePlayer->addOffsetCoin(-(int32_t)loseIdx.second);
		}

		jsReal["detial"] = jsDetial;
		jsMsg[jsMsg.size()] = jsReal;
	}
}

void BPMJRoom::onPlayerHu(std::vector<uint8_t>& vHuIdx, uint8_t nCard, uint8_t nInvokeIdx)
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

	std::vector<uint32_t> vLoseCoin, vWinCoin, vGangCoinGet, vGangCoinOut;
	for (auto i = 0; i < MAX_SEAT_CNT; ++i)
	{
		vLoseCoin.push_back(0);
		vWinCoin.push_back(0);
		vGangCoinGet.push_back(0);
		vGangCoinOut.push_back(0);
	}

	Json::Value jsReal;
	calculateGangCoin(jsReal, vGangCoinGet, vGangCoinOut, m_isOnePay);
	jsMsg["realTimeCal"] = jsReal;

	if (isZiMo)
	{
		auto pHuPlayer = (BPMJPlayer*)getMJPlayerByIdx(nInvokeIdx);
		auto pPlayerCard = (BPMJPlayerCard*)pHuPlayer->getPlayerCard();
		onPlayerZiMo(nInvokeIdx, nCard, jsDetail, vLoseCoin, vWinCoin, vGangCoinGet, vGangCoinOut);
		jsMsg["detail"] = jsDetail;
		sendRoomMsg(jsMsg, MSG_ROOM_BP_PLAYER_HU);
		return;
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
	//飘胡截胡、明飘优先级高
	uint8_t nMingPiaoIdx = uint8_t(-1);
	for (auto& nHuIdx : vOrderHu)
	{
		auto pHuPlayer = getMJPlayerByIdx(nHuIdx);
		auto pHuPlayerCard = (BPMJPlayerCard*)pHuPlayer->getPlayerCard();
		auto nFan = pHuPlayerCard->isMingPiao(nCard);
		if (nFan == eFanxing_MingPiao)
		{
			nMingPiaoIdx = nHuIdx;
			break;
		}
		else if (nFan == eFanxing_DuiDuiHu)
		{
			if (nMingPiaoIdx == uint8_t(-1))
			{
				nMingPiaoIdx = nHuIdx;
			}
		}
	}
	if (nMingPiaoIdx != uint8_t(-1))
	{
		vOrderHu.clear();
		vOrderHu.push_back(nMingPiaoIdx);
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

		auto pHuPlayerCard = (BPMJPlayerCard*)pHuPlayer->getPlayerCard();

		std::vector<uint16_t> vType;
		int32_t nAllFanCnt = 0;
		pHuPlayerCard->onDoHu(false, false, nCard, vType, nAllFanCnt);

		if (pLosePlayer->haveDecareBuGangFalg()) // robot gang ;
		{
			nAllFanCnt++;
		}
		if (pLosePlayer->haveGangFalg())
		{
			vType.push_back(eFanxing_LiuLei);
			nAllFanCnt++;
		}

		if (nHuIdx == m_nBankerIdx)
		{
			nAllFanCnt++; // 庄家1番
		}

		if (m_isBiMenHu && ((BPMJPlayer*)pHuPlayer)->GetBiMen())
		{
			nAllFanCnt++; // 闭门胡1番
		}

		if (!m_isHuiPai && m_isJiaHu && pHuPlayerCard->checkJiaHu(nCard, false) > 0)
		{
			vType.push_back(eFanxing_JiaHu);
			nAllFanCnt++; // 夹胡1番
		}

		Json::Value jsHuTyps;
		for (auto& refHu : vType)
		{
			jsHuTyps[jsHuTyps.size()] = refHu;
		}
		jsHuPlayer["vhuTypes"] = jsHuTyps;

		//三家付
		int32_t nBiMenCnt = 0;
		for (auto pPlayer : m_vMJPlayers)
		{
			if (((BPMJPlayer*)pPlayer)->GetBiMen() && pPlayer->getIdx() != nHuIdx)
			{
				nBiMenCnt++;
			}
		}
		if (nBiMenCnt == 3) // 三家闭门：2番
		{
			nAllFanCnt += 2;
		}

		uint32_t nTotalLose = 0;

		for (auto pPlayer : m_vMJPlayers)
		{
			if (pPlayer->getIdx() == nHuIdx)
			{
				continue;
			}

			auto nAllFanCnt_ = nAllFanCnt;
			auto nLoseCoin = 1;

			if (((BPMJPlayer*)pPlayer)->GetBiMen() && nBiMenCnt != 3)
			{
				nAllFanCnt_++; // 闭门1番
			}
			if (pPlayer->getIdx() == getBankerIdx())
			{
				nAllFanCnt_++;
			}

			if (nInvokeIdx == pPlayer->getIdx())
			{
				nAllFanCnt_++; // 点炮的人算1番，其他人只算基础分
			}
			for (int32_t i = 0; i < nAllFanCnt_; ++i) nLoseCoin *= 2;

			if (!m_isOnePay)
			{
				pPlayer->addOffsetCoin(-1 * (int32_t)nLoseCoin);
				pHuPlayer->addOffsetCoin(nLoseCoin);
				vLoseCoin[pPlayer->getIdx()] += nLoseCoin;
			}

			nTotalLose += nLoseCoin;
		}
		if (m_isOnePay)
		{
			pLosePlayer->addOffsetCoin(-1 * (int32_t)nTotalLose);
			pHuPlayer->addOffsetCoin(nTotalLose);
			vLoseCoin[pLosePlayer->getIdx()] += nTotalLose;
		}
		vWinCoin[pHuPlayer->getIdx()] = nTotalLose;
		jsHuPlayer["win"] = nTotalLose;

		//三家付没有一炮多响
		jsHuPlayers[jsHuPlayers.size()] = jsHuPlayer;
		break;
	}

	Json::Value jsBiMenPlayer;
	for (auto pPlayer : m_vMJPlayers)
	{
		if (((BPMJPlayer*)pPlayer)->GetBiMen())
		{
			jsBiMenPlayer[jsBiMenPlayer.size()] = pPlayer->getIdx();
		}
	}

	jsDetail["huPlayers"] = jsHuPlayers;

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

	jsDetail["bimenPlayers"] = jsBiMenPlayer;
	jsDetail["zhuangPlayerIdx"] = m_nBankerIdx;
	jsMsg["detail"] = jsDetail;
	sendRoomMsg(jsMsg, MSG_ROOM_BP_PLAYER_HU);
	LOGFMTD("room id = %u hu end ", getRoomID());
}

void BPMJRoom::onPlayerZiMo(uint8_t nIdx, uint8_t nCard, Json::Value& jsDetail, std::vector<uint32_t> vLoseCoin, std::vector<uint32_t> vWinCoin,
	std::vector<uint32_t> vGangCoinGet, std::vector<uint32_t> vGangCoinOut)
{
	auto pZiMoPlayer = (BPMJPlayer*)getMJPlayerByIdx(nIdx);
	if (pZiMoPlayer == nullptr)
	{
		LOGFMTE("room id = %u zi mo player is nullptr idx = %u ", getRoomID(), nIdx);
		return;
	}
	pZiMoPlayer->addZiMoCnt();
	pZiMoPlayer->setState(eRoomPeer_AlreadyHu);
	// svr :{ huIdx : 234 , baoPaiIdx : 2 , winCoin : 234,huardSoftHua : 23, isGangKai : 0 ,vhuTypes : [ eFanxing , ], LoseIdxs : [ {idx : 1 , loseCoin : 234 }, .... ]   }
	jsDetail["huIdx"] = nIdx;

	auto pHuPlayerCard = (BPMJPlayerCard*)pZiMoPlayer->getPlayerCard();
	std::vector<uint16_t> vType;
	int32_t nAllFanCnt = 0;
	bool bZiMo = true;
	pHuPlayerCard->onDoHu(bZiMo, getMJPoker()->getLeftCardCount() < getSeatCnt(), nCard, vType, nAllFanCnt);

	if (!m_isHuiPai && m_isJiaHu && pHuPlayerCard->checkJiaHu(nCard, bZiMo) > 0)
	{
		vType.push_back(eFanxing_JiaHu);
		nAllFanCnt++; // 夹胡1番
	}

	nAllFanCnt++; // 自摸1番

	if (pZiMoPlayer->haveGangFalg())
	{
		nAllFanCnt++; // 杠上开：1番
	}
	
	if (nIdx == m_nBankerIdx)
	{
		nAllFanCnt++; // 庄家1番
	}

	if (m_isBiMenHu && ((BPMJPlayer*)pZiMoPlayer)->GetBiMen())
	{
		nAllFanCnt++; // 闭门胡1番
	}

	//三家付
	int32_t nBiMenCnt = 0;
	for (auto pPlayer : m_vMJPlayers)
	{
		if (((BPMJPlayer*)pPlayer)->GetBiMen() && pPlayer->getIdx() != nIdx)
		{
			nBiMenCnt++;
		}
	}
	if (nBiMenCnt == 3) // 三家闭门：2番
	{
		nAllFanCnt += 2;
	}

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
			auto nAllFanCnt_ = nAllFanCnt;
			if (((BPMJPlayer*)pPlayer)->GetBiMen() && nBiMenCnt != 3)
			{
				nAllFanCnt_++; // 闭门1番
			}
			if (pPlayer->getIdx() == getBankerIdx())
			{
				nAllFanCnt_++;
			}

			int32_t nLose = 1;
			for (int32_t i = 0; i < nAllFanCnt_; ++i) nLose *= 2;

			pPlayer->addOffsetCoin(-1 * (int32_t)nLose);
			pZiMoPlayer->addOffsetCoin(nLose);

			vLoseCoin[pPlayer->getIdx()] = nLose;

			nTotalWin += nLose;
		}
	}
	vWinCoin[pZiMoPlayer->getIdx()] = nTotalWin;

	jsDetail["winCoin"] = nTotalWin;

	jsDetail["isGangKai"] = 0;
	if (pZiMoPlayer->haveGangFalg())
	{
		jsDetail["isGangKai"] = 1;
	}

	jsDetail["invokerGangIdx"] = nIdx;

	Json::Value jsBiMenPlayer;
	for (auto pPlayer : m_vMJPlayers)
	{
		if (((BPMJPlayer*)pPlayer)->GetBiMen())
		{
			jsBiMenPlayer[jsBiMenPlayer.size()] = pPlayer->getIdx();
		}
	}

	Json::Value jsVLoses;
	for (auto i = 0; i < MAX_SEAT_CNT; ++i)
	{
		int nCoin = vWinCoin[i] - vLoseCoin[i] + vGangCoinGet[i] - vGangCoinOut[i];
		//if (vLoseCoin[i] > 0)
		{
			Json::Value jsLose;
			jsLose["loseCoin"] = nCoin;
			jsLose["idx"] = i;
			jsVLoses[jsVLoses.size()] = jsLose;
		}
	}
	jsDetail["LoseIdxs"] = jsVLoses;

	jsDetail["bimenPlayers"] = jsBiMenPlayer;
	jsDetail["zhuangPlayerIdx"] = m_nBankerIdx;

	LOGFMTD("room id = %u hu end ", getRoomID());
}

bool BPMJRoom::onPlayerApplyLeave(uint32_t nPlayerUID)
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

void BPMJRoom::sendPlayersCardInfo(uint32_t nSessionID)
{
	Json::Value jsmsg;
	Json::Value vPeerCards;
	for (auto& pp : m_vMJPlayers)
	{
		if (pp == nullptr /*|| pp->haveState(eRoomPeer_CanAct) == false*/)  // lose also have card 
		{
			continue;
		}

		auto pCard = (BPMJPlayerCard*)pp->getPlayerCard();
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

void BPMJRoom::checkJiGang(uint8_t nIdx, uint8_t nCard)
{
	auto pPlayer = (BPMJPlayer*)getMJPlayerByIdx(nIdx);
	if (!pPlayer)
	{
		LOGFMTE("why this player is null idx = %u , can not ming gang", nIdx);
		return;
	}
	auto playerCard = (BPMJPlayerCard*)pPlayer->getPlayerCard();

	if (nCard == CMJCard::makeCardNumber(eCT_Tiao, 1))
	{
		auto nGangGetCard = playerCard->getNewestFetchedCard();
		if (card_Type(nGangGetCard) == eCT_Tong && m_isJiXiaDan)
		{
			stSettle st;
			st.eSettleReason = (eMJActType)0;
			uint16_t nLose = card_Value(nGangGetCard);

			for (auto pLosePlayer : m_vMJPlayers)
			{
				if (pLosePlayer->getIdx() != nIdx)
				{
					st.addLose(pLosePlayer->getIdx(), nLose);
				}
			}
			st.addWin(nIdx, nLose * 3);

			m_vSettle.push_back(st);
		}
		else if (card_Type(nGangGetCard) == eCT_Tiao && m_isGangLiuLei)
		{
			stSettle st;
			st.eSettleReason = (eMJActType)1;
			uint16_t nLose = card_Value(nGangGetCard);

			for (auto pWinPlayer : m_vMJPlayers)
			{
				if (pWinPlayer->getIdx() != nIdx)
				{
					st.addWin(pWinPlayer->getIdx(), nLose);
				}
			}
			st.addLose(nIdx, nLose * 3);

			m_vSettle.push_back(st);
		}
	}
}

void BPMJRoom::onPlayerMingGang(uint8_t nIdx, uint8_t nCard, uint8_t nInvokeIdx)
{
	auto pPlayer = (BPMJPlayer*)getMJPlayerByIdx(nIdx);
	auto pInvoker = getMJPlayerByIdx(nInvokeIdx);
	if (!pPlayer || !pInvoker)
	{
		LOGFMTE("why this player is null idx = %u , can not ming gang", nIdx);
		return;
	}
	auto playerCard = (BPMJPlayerCard*)pPlayer->getPlayerCard();

	IMJRoom::onPlayerMingGang(nIdx, nCard, nInvokeIdx);

	checkJiGang(nIdx, nCard);

	playerCard->setSongGangIdx(nInvokeIdx);
	playerCard->addSongGangIdx(nCard, nInvokeIdx, eMJAct_MingGang);
}

void BPMJRoom::onPlayerBuGang(uint8_t nIdx, uint8_t nCard)
{
	auto pPlayer = (BPMJPlayer*)getMJPlayerByIdx(nIdx);
	if (!pPlayer)
	{
		LOGFMTE("why this player is null idx = %u , can not bu gang", nIdx);
		return;
	}
	auto playerCard = (BPMJPlayerCard*)pPlayer->getPlayerCard();

	IMJRoom::onPlayerBuGang(nIdx, nCard);

	checkJiGang(nIdx, nCard);

	std::vector<BPMJPlayerCard::tSongGangIdx> vSongIdx = playerCard->getPengSongIdx();
	uint8_t nInvokeIdx = uint8_t(-1);
	for (auto tSong : vSongIdx)
	{
		if (tSong.nGangCard == nCard)
		{
			nInvokeIdx = tSong.nIdx;
			break;
		}
	}
	if (nInvokeIdx != uint8_t(-1))
	{
		playerCard->addSongGangIdx(nCard, nInvokeIdx, eMJAct_BuGang);
	}
	else
	{
		assert(0 && "bu gang, peng player not found!!!");
	}
}

void BPMJRoom::onPlayerAnGang(uint8_t nIdx, uint8_t nCard)
{
	IMJRoom::onPlayerAnGang(nIdx, nCard);

	checkJiGang(nIdx, nCard);
}

void BPMJRoom::onPlayerChu(uint8_t nIdx, uint8_t nCard)
{
	IMJRoom::onPlayerChu(nIdx, nCard);
	auto pActPlayer = getMJPlayerByIdx(nIdx);
	auto pActCard = (BPMJPlayerCard*)pActPlayer->getPlayerCard();
	pActCard->setSongGangIdx(-1); // reset song gang ;

	//清除杠标记（杠开）,杠炮不能在这里加
	pActPlayer->clearGangFlag();
}

void BPMJRoom::onWaitPlayerAct(uint8_t nIdx, bool& isCanPass)
{
	auto pPlayer = (BPMJPlayer*)getMJPlayerByIdx(nIdx);
	if (!pPlayer)
	{
		LOGFMTE("player idx = %u is null can not tell it wait act", nIdx);
		return;
	}

	auto pMJCard = (BPMJPlayerCard*)pPlayer->getPlayerCard();
	
	// send msg to tell player do act 
	Json::Value jsArrayActs;
	Json::Value jsFrameActs;

	if (isCanGoOnMoPai())
	{
		// check bu gang .
		IMJPlayerCard::VEC_CARD vCards;
		pMJCard->getHoldCardThatCanBuGang(vCards);
		if (pMJCard->checkMingPiao() == 0)
		{
			vCards.clear();
		}
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
		if (pMJCard->checkMingPiao() == 0)
		{
			vCards.clear();
		}
		for (auto& ref : vCards)
		{
			Json::Value jsAct;
			jsAct["act"] = eMJAct_AnGang;
			jsAct["cardNum"] = ref;
			jsArrayActs[jsArrayActs.size()] = jsAct;
			jsFrameActs[jsFrameActs.size()] = eMJAct_AnGang;
		}
	}

	// check hu . 如果是吃的只能听不能胡
	if (pMJCard->isHoldCardCanHu())
	{
		Json::Value jsAct;
		jsAct["act"] = eMJAct_Hu;
		jsAct["cardNum"] = pMJCard->getNewestFetchedCard();
		jsArrayActs[jsArrayActs.size()] = jsAct;
		jsFrameActs[jsFrameActs.size()] = eMJAct_Hu;
	}

	isCanPass = jsArrayActs.empty() == false;

	// add default alwasy chu , infact need not add , becaust it alwasy in ,but compatable with current client ;
	Json::Value jsAct;
	jsAct["act"] = eMJAct_Chu;
	jsAct["cardNum"] = getAutoChuCardWhenWaitActTimeout(nIdx);
	jsArrayActs[jsArrayActs.size()] = jsAct;
	jsFrameActs[jsFrameActs.size()] = eMJAct_Chu;

	Json::Value jsMsg;
	jsMsg["acts"] = jsArrayActs;
	sendMsgToPlayer(jsMsg, MSG_PLAYER_WAIT_ACT_AFTER_RECEIVED_CARD, pPlayer->getSessionID());

	if (isCanPass)  // player do have option do select or need not give frame ;
	{
		Json::Value jsFrameArg;
		auto ptrReplay = getGameReplay()->createFrame(eMJFrame_WaitPlayerAct, (uint32_t)time(nullptr));
		jsFrameArg["idx"] = nIdx;
		jsFrameArg["act"] = jsFrameActs;
		ptrReplay->setFrameArg(jsFrameArg);
		getGameReplay()->addFrame(ptrReplay);
	}

	//LOGFMTD("tell player idx = %u do act size = %u", nIdx, jsArrayActs.size());
}

bool BPMJRoom::isAnyPlayerPengOrHuThisCard(uint8_t nInvokeIdx, uint8_t nCard)
{
	for (auto& ref : m_vMJPlayers)
	{
		if (ref == nullptr || nInvokeIdx == ref->getIdx())
		{
			continue;
		}
		ref->clearGangFlag();
	}

	for (auto& ref : m_vMJPlayers)
	{
		if (ref == nullptr || nInvokeIdx == ref->getIdx())
		{
			continue;
		}

		auto pMJCard = (BPMJPlayerCard*)ref->getPlayerCard();
		if (nCard != m_nHuiCard && pMJCard->canHuWitCard(nCard))
		{
			return true;
		}

		if (nCard != m_nHuiCard && pMJCard->canPengWithCard(nCard) && pMJCard->checkMingPiao() > 0)
		{
			return true;
		}

		if (ref->getIdx() == (nInvokeIdx + 1) % getSeatCnt() && pMJCard->checkMingPiao() == 1)
		{
			uint8_t a = 0, b = 0;
			if (pMJCard->canEatCard(nCard, a, b))
			{
				return true;
			}
		}

		if (pMJCard->canMingGangWithCard(nCard) && pMJCard->checkMingPiao() > 0)
		{
			return true;
		}
	}

	return false;
}

void BPMJRoom::onAskForPengOrHuThisCard(uint8_t nInvokeIdx, uint8_t nCard, std::vector<uint8_t>& vWaitHuIdx, std::vector<uint8_t>& vWaitPengGangIdx, bool& isNeedWaitEat)
{
	Json::Value jsFrameArg;

	for (auto& ref : m_vMJPlayers)
	{
		if (ref == nullptr)
		{
			continue;
		}

		auto pMJCard = (BPMJPlayerCard*)ref->getPlayerCard();
		if (nInvokeIdx == ref->getIdx())
		{
			continue;
		}

		Json::Value jsMsg;
		jsMsg["invokerIdx"] = nInvokeIdx;
		jsMsg["cardNum"] = nCard;

		Json::Value jsActs;

		// check peng 
		if (nCard != m_nHuiCard && pMJCard->canPengWithCard(nCard) && pMJCard->checkMingPiao() > 0)
		{
			jsActs[jsActs.size()] = eMJAct_Peng;
			vWaitPengGangIdx.push_back(ref->getIdx());
		}

		// check ming gang 
		if (isCanGoOnMoPai() && pMJCard->canMingGangWithCard(nCard) && pMJCard->checkMingPiao() > 0)
		{
			jsActs[jsActs.size()] = eMJAct_MingGang;

			std::vector<uint8_t>::iterator it = std::find(vWaitPengGangIdx.begin(), vWaitPengGangIdx.end(), ref->getIdx());
			if (it == vWaitPengGangIdx.end())
			{
				vWaitPengGangIdx.push_back(ref->getIdx());
			}
			// already add in peng ;  vWaitPengGangIdx
		}

		if (ref->getIdx() == (nInvokeIdx + 1) % getSeatCnt() && pMJCard->checkMingPiao() == 1)
		{
			uint8_t a = 0, b = 0;
			isNeedWaitEat = false;
			if (nCard != m_nHuiCard && pMJCard->canEatCard(nCard, a, b))
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

std::shared_ptr<IGameRoomRecorder> BPMJRoom::createRoomRecorder()
{
	return std::make_shared<BPMJRoomRecorder>();
}
