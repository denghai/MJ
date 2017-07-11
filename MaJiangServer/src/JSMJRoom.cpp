#include "JSMJRoom.h"
#include "JSMJPlayerCard.h"
#include "log4z.h"
#include "IMJPoker.h"
#include "JSMJPlayer.h"
#include "IGameRoomManager.h"
#include "ServerMessageDefine.h"
#include "MJRoomStateWaitReady.h"
#include "MJRoomStateWaitPlayerChu.h"
#include "MJRoomStateWaitPlayerAct.h"
#include "MJRoomStateStartGame.h"
#include "MJRoomStateGameEnd.h"
#include "MJRoomStateDoPlayerAct.h"
#include "JSRoomStateAskForPengOrHu.h"
#include "MJRoomStateAskForRobotGang.h"
#include "MJRoomStateWaitPlayerChu.h"
#include "JSRoomStateWaitPlayerAct.h"
#include "JSRoomStateWaitPlayerChu.h"
#include "JSRoomStateDoPlayerAct.h"
#include "JSMJPlayerRecorderInfo.h"
#include "MJReplayFrameType.h"

#include <ctime>

bool JSMJRoom::init(IGameRoomManager* pRoomMgr, stBaseRoomConfig* pConfig, uint32_t nSeialNum, uint32_t nRoomID, Json::Value& vJsValue)
{
	IMJRoom::init(pRoomMgr, pConfig, nSeialNum, nRoomID, vJsValue);
	m_isBankerHu = false;
	
	m_nTangZhiType = vJsValue["nTangZhiType"].asUInt(); // 0:没有塘子；1:塘子一番；2:塘子平胡
	m_bZhuangFan = vJsValue["isZhuangFan"].asUInt() == 0 ? false : true; // 1:庄家翻倍；0:不翻倍
	m_haveYiTiaoLong = true;//vJsValue["haveYiTiaoLong"].asUInt() == 0 ? false : true; //1:一条龙；0:没有一条龙番
	m_haveLuoDiLong = vJsValue["haveLuoDiLong"].asUInt() == 0 ? false : true; //1:落地龙；0:没有
	m_haveShouDaiLong = vJsValue["haveShouDaiLong"].asUInt() == 0 ? false : true; //1:手逮龙；0:没有
	m_bShouDaiLong = vJsValue["isShouDaiLong"].asUInt() == 0 ? false : true; // 1:手逮龙加番；0:不加番
	m_nFan = vJsValue["nFan"].asUInt(); //0:3番；1:4番；2:不封顶
	m_b2ZhangDi = vJsValue["is2ZhangDi"].asUInt() == 0 ? false : true; //0:翻一张；1:翻2张
	m_bTangZi7Pair = vJsValue["isTangZi7Pair"].asUInt() == 0 ? false : true; //0:没有塘子7星；1:有

	m_tPoker.initAllCard(eMJ_COMMON);
	// create state and add state ;
	IMJRoomState* vState[] = {
		new CMJRoomStateWaitReady(), new JSRoomStateWaitPlayerChu(), new JSRoomStateWaitPlayerAct(), new MJRoomStateStartGame()
		, new MJRoomStateGameEnd(), new JSRoomStateDoPlayerAct(), new JSRoomStateAskForPengOrHu(), new MJRoomStateAskForRobotGang()
	};
	for (uint8_t nIdx = 0; nIdx < sizeof(vState) / sizeof(IMJRoomState*); ++nIdx)
	{
		addRoomState(vState[nIdx]);
	}
	setInitState(vState[0]);

	// init banker
	m_nBankerIdx = -1;
	auto pRoomRecorder = (JSMJRoomRecorder*)getRoomRecorder().get();
	//pRoomRecorder->setRoomOpts(m_isOnePay ? 1 : 0);

	return true;
}

void JSMJRoom::willStartGame()
{
	IMJRoom::willStartGame();

	if ((uint8_t)-1 == m_nBankerIdx)
	{
		m_nBankerIdx = 0;
	}

	m_isBankerHu = false;

	m_nGangCard1 = uint8_t(-1);
	m_nGangCard2 = uint8_t(-1);
	m_nGangCard = uint8_t(-1);
	m_nGangCount = 0;
}

void JSMJRoom::startGame()
{
	IMJRoom::startGame();

	//翻开1张杠牌
	m_nGangCard1 = getMJPoker()->distributeOneCard();

	Json::Value jsMsg;
	IMJRoom::packStartGameMsg(jsMsg);

	jsMsg["gangCard1"] = m_nGangCard1;

	if (m_b2ZhangDi)
	{
		m_nGangCard2 = getMJPoker()->distributeOneCard();
		jsMsg["gangCard2"] = m_nGangCard2;
	}

	sendRoomMsg(jsMsg, MSG_ROOM_JS_START_GAME);

	// replay arg 
	Json::Value jsReplayInfo;
	jsReplayInfo["roomID"] = getRoomID();
	jsReplayInfo["time"] = (uint32_t)time(nullptr);
	jsReplayInfo["nTangZhiType"] = m_nTangZhiType;
	jsReplayInfo["isZhuangFan"] = m_bZhuangFan ? 1 : 0;
	jsReplayInfo["haveYiTiaoLong"] = m_haveYiTiaoLong ? 1 : 0;
	jsReplayInfo["haveLuoDiLong"] = m_haveLuoDiLong ? 1 : 0;
	jsReplayInfo["haveShouDaiLong"] = m_haveShouDaiLong ? 1 : 0;
	jsReplayInfo["isShouDaiLong"] = m_bShouDaiLong ? 1 : 0;
	jsReplayInfo["is2ZhangDi"] = m_b2ZhangDi ? 1 : 0;
	jsReplayInfo["isTangZi7Pair"] = m_bTangZi7Pair ? 1 : 0;
	jsReplayInfo["nFan"] = m_nFan;
	jsReplayInfo["gangCard1"] = m_nGangCard1;
	if (m_b2ZhangDi)
	{
		jsReplayInfo["gangCard2"] = m_nGangCard2;
	}
	getGameReplay()->setReplayRoomInfo(jsReplayInfo);
}

void JSMJRoom::getSubRoomInfo(Json::Value& jsSubInfo)
{
	//MSG_MJ_ROOM_INFO subInfo
	//jsSubInfo["isOnePay"] = isOnePay() ? 1 : 0;
	jsSubInfo["gangCard1"] = m_nGangCard1;
	if (m_b2ZhangDi)
	{
		jsSubInfo["gangCard2"] = m_nGangCard2;
	}
	jsSubInfo["GangCount"] = m_nGangCount;

	jsSubInfo["nTangZhiType"] = m_nTangZhiType;
	jsSubInfo["isZhuangFan"] = m_bZhuangFan ? 1 : 0;
	jsSubInfo["haveYiTiaoLong"] = m_haveYiTiaoLong ? 1 : 0;
	jsSubInfo["haveLuoDiLong"] = m_haveLuoDiLong ? 1 : 0;
	jsSubInfo["haveShouDaiLong"] = m_haveShouDaiLong ? 1 : 0;
	jsSubInfo["isShouDaiLong"] = m_bShouDaiLong ? 1 : 0;
	jsSubInfo["is2ZhangDi"] = m_b2ZhangDi ? 1 : 0;
	jsSubInfo["isTangZi7Pair"] = m_bTangZi7Pair ? 1 : 0;
	jsSubInfo["nFan"] = m_nFan;
}

void JSMJRoom::onGameDidEnd()
{
	IMJRoom::onGameDidEnd();
	if (getDelegate())
	{
		getDelegate()->onDidGameOver(this);
		return;
	}
}

void JSMJRoom::onGameEnd()
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
		calculateGangCoin(uint8_t(-1), uint8_t(-1), jsReal, vGangCoinGet, vGangCoinOut);
		//Json::Value jsReal;
		//settleInfoToJson(jsReal);
		jsMsg["realTimeCal"] = jsReal;
	}
	jsMsg["nNextBankIdx"] = m_nBankerIdx;

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

			auto pPlayerRecorderInfo = std::make_shared<JSMJPlayerRecorderInfo>();
			pPlayerRecorderInfo->init(ref->getUID(), ref->getOffsetCoin());
			ptrSingleRecorder->addPlayerRecorderInfo(pPlayerRecorderInfo);
		}
	}

	jsMsg["isLiuJu"] = isAnyOneHu ? 0 : 1;
	jsMsg["detail"] = jsDetial;

	sendRoomMsg(jsMsg, MSG_ROOM_JS_GAME_OVER);
	// send msg to player ;
	IMJRoom::onGameEnd();
}

IMJPlayer* JSMJRoom::doCreateMJPlayer()
{
	return new JSMJPlayer();
}

IMJPoker* JSMJRoom::getMJPoker()
{
	return &m_tPoker;
}

bool JSMJRoom::isGameOver()
{
	if (IMJRoom::isGameOver())
	{
		return true;
	}

	if (m_nTangZhiType != 0)
	{
		//检查塘子是否胡牌
		for (auto pPlayer : m_vMJPlayers)
		{
			auto pPlayerCard = (JSMJPlayerCard*)pPlayer->getPlayerCard();

			if (pPlayerCard->isTangZhiHu(m_bTangZi7Pair))
			{
				onPlayerTangZhiHu(pPlayer->getIdx());
				pPlayer->setState(eRoomPeer_AlreadyHu);
				return true;
			}
		}
	}

	//检查剩余牌数是否流局
	auto nLeftCardNum = getMJPoker()->getLeftCardCount();
	auto nGangCount = m_nGangCount % 6;
	if (m_b2ZhangDi)
	{
		//杠三次留10墩（19张），杠四次留7墩（14张），杠五次留8墩（15张），杠六次留10墩（20张）
		auto nLeftCount = 20 - 2;
		if (nGangCount == 1)
		{
			nLeftCount = 13 - 2;
		}
		else if (nGangCount == 2)
		{
			nLeftCount = 16 - 2;
		}
		else if (nGangCount == 3)
		{
			nLeftCount = 19 - 2;
		}
		else if (nGangCount == 4)
		{
			nLeftCount = 14 - 2;
		}
		else if (nGangCount == 5)
		{
			nLeftCount = 15 - 2;
		}
		if (nLeftCount >= nLeftCardNum)
		{
			return true;
		}
	} 
	else
	{
		auto nLeftCount = 20 - 1;
		if (nGangCount == 1)
		{
			nLeftCount = 13 - 1;
		}
		else if (nGangCount == 2)
		{
			nLeftCount = 16 - 1;
		}
		else if (nGangCount == 3)
		{
			nLeftCount = 19 - 1;
		}
		else if (nGangCount == 4)
		{
			nLeftCount = 14 - 1;
		}
		else if (nGangCount == 5)
		{
			nLeftCount = 15 - 1;
		}
		if (nLeftCount >= nLeftCardNum)
		{
			return true;
		}
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

void JSMJRoom::onPlayerPeng(uint8_t nIdx, uint8_t nCard, uint8_t nInvokeIdx)
{
	IMJRoom::onPlayerPeng(nIdx, nCard, nInvokeIdx);
	auto pActPlayer = getMJPlayerByIdx(nIdx);
	auto pActCard = (JSMJPlayerCard*)pActPlayer->getPlayerCard();
	pActCard->addSongPengIdx(nCard, nInvokeIdx);
}

void JSMJRoom::removeInvalidGang(uint8_t nHuIdx, uint8_t nInvokeIdx, std::vector<uint8_t>& vAnGang, std::vector<JSMJPlayerCard::tSongGangIdx>& vMingGang)
{
	auto funRemoveGang = [this](uint8_t nIdx, std::vector<uint8_t>& vAnGang, std::vector<JSMJPlayerCard::tSongGangIdx>& vMingGang)
	{
		auto pPlayerCard = (JSMJPlayerCard*)getMJPlayerByIdx(nIdx)->getPlayerCard();
		auto vGangTimes = pPlayerCard->getGangTimes();
		for (auto songIdx : vGangTimes)
		{
			auto nGangCard = songIdx.nGangCard;
			bool bFound = false;

			for (auto it = vMingGang.begin(); it != vMingGang.end(); ++it)
			{
				if ((*it).nGangCard == nGangCard)
				{
					bFound = true;
					vMingGang.erase(it);
					break;
				}
			}

			auto it_an = std::find(vAnGang.begin(), vAnGang.end(), nGangCard);
			if (it_an != vAnGang.end())
			{
				bFound = true;
				vAnGang.erase(it_an);
			}

			if (!bFound)
			{
				assert(0);
			}
		}
	};

	if (nHuIdx == uint8_t(-1) || nInvokeIdx == uint8_t(-1))
	{
		return;
	}
	
	if (nHuIdx != nInvokeIdx)
	{
		//抢杠胡（不算杠分）
		funRemoveGang(nInvokeIdx, vAnGang, vMingGang);
	}
}

void JSMJRoom::calculateGangCoin(uint8_t nHuIdx, uint8_t nInvokeIdx, Json::Value& jsMsg, std::vector<uint32_t>& vGangCoinGet, std::vector<uint32_t>& vGangCoinOut)
{
	//realTimeCal: [{ actType: 23, detial : [{idx: 2, offset : -23 }, ...]  }, ...]
	for (auto pWinPlayer : m_vMJPlayers)
	{
		auto pPlayerCard = (JSMJPlayerCard*)pWinPlayer->getPlayerCard();
		auto nWinIdx = pWinPlayer->getIdx();
		
		std::vector<uint8_t> vAnGang;
		pPlayerCard->getAnGangedCard(vAnGang);
		std::vector<JSMJPlayerCard::tSongGangIdx> vMingGang = pPlayerCard->getMingGangSongIdx();
		if (nHuIdx == nWinIdx || nInvokeIdx == nWinIdx)
		{
			removeInvalidGang(nHuIdx, nInvokeIdx, vAnGang, vMingGang);
		}

		for (auto i=0; i<vAnGang.size(); ++i)
		{
			Json::Value jsReal;
			jsReal["actType"] = eMJAct_AnGang;
			Json::Value jsDetial;

			Json::Value jsWin;
			jsWin["idx"] = nWinIdx;
			jsWin["offset"] = (MAX_SEAT_CNT - 1) * 2;
			jsDetial[jsDetial.size()] = jsWin;

			vGangCoinGet[nWinIdx] += (MAX_SEAT_CNT - 1) * 2;
			pWinPlayer->addOffsetCoin((MAX_SEAT_CNT - 1) * 2);

			for (auto idx = 0; idx < MAX_SEAT_CNT; ++idx)
			{
				if (idx != nWinIdx)
				{
					Json::Value jsLose;
					jsLose["idx"] = idx;
					jsLose["offset"] = -2;
					jsDetial[jsDetial.size()] = jsLose;

					vGangCoinOut[idx] += 2;
					auto pLosePlayer = getMJPlayerByIdx(idx);
					pLosePlayer->addOffsetCoin(-2);
				}
			}
			jsReal["detial"] = jsDetial;
			jsMsg[jsMsg.size()] = jsReal;
		}

		for (auto i = 0; i < vMingGang.size(); ++i)
		{
			Json::Value jsReal;
			jsReal["actType"] = vMingGang[i].eAct;
			Json::Value jsDetial;

			Json::Value jsWin;
			jsWin["idx"] = nWinIdx;
			auto nWinCoin = MAX_SEAT_CNT - 1;
			jsWin["offset"] = nWinCoin;
			jsDetial[jsDetial.size()] = jsWin;

			vGangCoinGet[nWinIdx] += nWinCoin;
			pWinPlayer->addOffsetCoin(nWinCoin);

			for (auto idx = 0; idx < MAX_SEAT_CNT; ++idx)
			{
				if (idx != nWinIdx)
				{
					Json::Value jsLose;
					jsLose["idx"] = idx;
					jsLose["offset"] = -1;
					jsDetial[jsDetial.size()] = jsLose;

					vGangCoinOut[idx] += 1;
					auto pLosePlayer = getMJPlayerByIdx(idx);
					pLosePlayer->addOffsetCoin(-1);
				}
			}

			jsReal["detial"] = jsDetial;
			jsMsg[jsMsg.size()] = jsReal;
		}
	}
}

void JSMJRoom::onPlayerHu(std::vector<uint8_t>& vHuIdx, uint8_t nCard, uint8_t nInvokeIdx)
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

	std::vector<uint32_t> vLoseCoin, vWinCoin, vGangCoinGet, vGangCoinOut, vFightLandLordGet, vFightLandLordOut;
	for (auto i = 0; i < MAX_SEAT_CNT; ++i)
	{
		vLoseCoin.push_back(0);
		vWinCoin.push_back(0);
		vGangCoinGet.push_back(0);
		vGangCoinOut.push_back(0);
		vFightLandLordGet.push_back(0);
		vFightLandLordOut.push_back(0);
	}

	// adjust caculate order 
	uint8_t nHuPlayerIdx = vHuIdx.at(0);
	if (vHuIdx.size() > 1)
	{
		for (uint8_t offset = 1; offset <= 3; ++offset)
		{
			auto nCheckIdx = (nInvokeIdx + offset) % 4;
			auto iter = std::find(vHuIdx.begin(), vHuIdx.end(), nCheckIdx);
			if (iter != vHuIdx.end())
			{
				nHuPlayerIdx = nCheckIdx;
				break;
			}
		}
	}
	auto pHuPlayer = getMJPlayerByIdx(nHuPlayerIdx);
	pHuPlayer->addHuCnt();

	Json::Value jsReal;
	calculateGangCoin(nHuPlayerIdx, nInvokeIdx, jsReal, vGangCoinGet, vGangCoinOut);
	jsMsg["realTimeCal"] = jsReal;

	if (isZiMo)
	{
		onPlayerZiMo(nInvokeIdx, nCard, jsDetail, vLoseCoin, vWinCoin, vGangCoinGet, vGangCoinOut, vFightLandLordGet, vFightLandLordOut);
		jsMsg["detail"] = jsDetail;
		sendRoomMsg(jsMsg, MSG_ROOM_JS_PLAYER_HU);

		m_nBankerIdx = nInvokeIdx;
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
	Json::Value jsHuPlayer;
	jsHuPlayer["idx"] = nHuPlayerIdx;
	pHuPlayer->setState(eRoomPeer_AlreadyHu);

	std::vector<uint16_t> vType;
	int32_t nAllFanCnt = 0;

	auto pHuPlayerCard = (JSMJPlayerCard*)pHuPlayer->getPlayerCard();
	pHuPlayerCard->onDoHu(false, nCard, m_haveYiTiaoLong, m_haveLuoDiLong, m_haveShouDaiLong, m_bShouDaiLong, vType, nAllFanCnt);

	auto pLosePlayerCard = (JSMJPlayerCard*)pLosePlayer->getPlayerCard();
	auto nGangTimes = pLosePlayerCard->getGangTimes().size();
	if (pLosePlayer->haveDecareBuGangFalg())
	{
		if (nGangTimes >= 2)
		{
			vType.push_back(eFanxing_Qiang2Gang);
		}
		else
		{
			vType.push_back(eFanxing_QiangGang);
		}
		if (nInvokeIdx == m_nBankerIdx)
		{
			nAllFanCnt += 2;
		}
	}
	if (pLosePlayer->haveGangFalg())
	{
		if (nGangTimes >= 2)
		{
			vType.push_back(eFanxing_2GangPao);
		}
		else
		{
			vType.push_back(eFanxing_GangPao);
		}
		if (nInvokeIdx == m_nBankerIdx)
		{
			nAllFanCnt += 2;
		}
	}

	int nCount = pHuPlayerCard->check7Pair(false, nCard);
	bool bAllZiPai = false;
	if (nCount >= 0)
	{
		std::vector<uint8_t> vHoldCards;
		pHuPlayerCard->getHoldCard(vHoldCards);
		if (pHuPlayerCard->isAllZiPai(vHoldCards))
		{
			bAllZiPai = true;
			nAllFanCnt++;
		}
	}
	if (nCount == 3)
	{
		if (bAllZiPai)
		{
			vType.push_back(eFanxing_Zi3LongJia);
			nAllFanCnt++;
		}
		else
		{
			vType.push_back(eFanxing_3LongJia);
			nAllFanCnt += 2;
		}
	}
	else if (nCount == 2)
	{
		if (bAllZiPai)
		{
			vType.push_back(eFanxing_Zi2LongJia);
			nAllFanCnt++;
		}
		else
		{
			vType.push_back(eFanxing_2LongJia);
			nAllFanCnt += 2;
		}
	}
	else if (nCount == 1)
	{
		if (bAllZiPai)
		{
			vType.push_back(eFanxing_ZiLongJia);
			nAllFanCnt++;
		}
		else
		{
			vType.push_back(eFanxing_LongJia);
			nAllFanCnt += 2;
		}
	}
	else if (nCount == 0)
	{
		if (bAllZiPai)
		{
			vType.push_back(eFanxing_Zi7Pair);
		}
		else
		{
			vType.push_back(eFanxing_QiDui);
		}
		nAllFanCnt++;
	}

	Json::Value jsHuTyps;
	for (auto& refHu : vType)
	{
		if (refHu == eFanxing_PingHu && vType.size() > 1)
		{
			continue;
		}
		jsHuTyps[jsHuTyps.size()] = refHu;
	}
	jsHuPlayer["vhuTypes"] = jsHuTyps;

	if (m_bZhuangFan && nHuPlayerIdx == m_nBankerIdx)
	{
		nAllFanCnt++; // 庄家1番
	}

	uint32_t nTotalLose = 0;

	for (auto pPlayer : m_vMJPlayers)
	{
		if (pPlayer->getIdx() == nHuPlayerIdx)
		{
			continue;
		}

		auto nAllFanCnt_ = nAllFanCnt;
		int32_t nLoseCoin = 1;

		if (nInvokeIdx == pPlayer->getIdx())
		{
			nAllFanCnt_++; // 点炮的人算1番，其他人只算基础分
		}

		if (m_bZhuangFan && pPlayer->getIdx() == m_nBankerIdx)
		{
			nAllFanCnt_++; // 庄家1番
		}

		if (m_nFan == 0)
		{
			if (nAllFanCnt_ > 3)
			{
				nAllFanCnt_ = 3;
			}
		}
		else if (m_nFan == 1)
		{
			if (nAllFanCnt_ > 4)
			{
				nAllFanCnt_ = 4;
			}
		}

		for (int32_t i = 0; i < nAllFanCnt_; ++i) nLoseCoin *= 2;

		pPlayer->addOffsetCoin(-1 * (int32_t)nLoseCoin);
		pHuPlayer->addOffsetCoin(nLoseCoin);

		nTotalLose += nLoseCoin;

		vLoseCoin[pPlayer->getIdx()] += nLoseCoin;
	}
	jsHuPlayer["win"] = nTotalLose;

	vWinCoin[pHuPlayer->getIdx()] = nTotalLose;

	jsHuPlayers[jsHuPlayers.size()] = jsHuPlayer;

	jsDetail["huPlayers"] = jsHuPlayers;

	Json::Value jsVLoses;
	for (auto i = 0; i < MAX_SEAT_CNT; ++i)
	{
		int nCoin = vWinCoin[i] - vLoseCoin[i] + vGangCoinGet[i] - vGangCoinOut[i] + vFightLandLordGet[i] - vFightLandLordOut[i];
		Json::Value jsLose;
		jsLose["loseCoin"] = nCoin;
		jsLose["idx"] = i;
		jsVLoses[jsVLoses.size()] = jsLose;
	}
	jsDetail["LoseIdxs"] = jsVLoses;

	jsDetail["zhuangPlayerIdx"] = m_nBankerIdx;
	jsMsg["detail"] = jsDetail;
	sendRoomMsg(jsMsg, MSG_ROOM_JS_PLAYER_HU);
	LOGFMTD("room id = %u hu end ", getRoomID());

	m_nBankerIdx = nHuPlayerIdx;
}

void JSMJRoom::onPlayerZiMo(uint8_t nIdx, uint8_t nCard, Json::Value& jsDetail, std::vector<uint32_t> vLoseCoin, std::vector<uint32_t> vWinCoin,
	std::vector<uint32_t> vGangCoinGet, std::vector<uint32_t> vGangCoinOut, 
	std::vector<uint32_t> vFightLandLordGet, std::vector<uint32_t> vFightLandLordOut)
{
	auto pZiMoPlayer = (JSMJPlayer*)getMJPlayerByIdx(nIdx);
	if (pZiMoPlayer == nullptr)
	{
		LOGFMTE("room id = %u zi mo player is nullptr idx = %u ", getRoomID(), nIdx);
		return;
	}
	pZiMoPlayer->addZiMoCnt();
	pZiMoPlayer->setState(eRoomPeer_AlreadyHu);
	// svr :{ huIdx : 234 , baoPaiIdx : 2 , winCoin : 234,huardSoftHua : 23, isGangKai : 0 ,vhuTypes : [ eFanxing , ], LoseIdxs : [ {idx : 1 , loseCoin : 234 }, .... ]   }
	jsDetail["huIdx"] = nIdx;

	auto pHuPlayerCard = (JSMJPlayerCard*)pZiMoPlayer->getPlayerCard();

	std::vector<uint16_t> vType;
	int32_t nAllFanCnt = 0;
	pHuPlayerCard->onDoHu(true, nCard, m_haveYiTiaoLong, m_haveLuoDiLong, m_haveShouDaiLong, m_bShouDaiLong, vType, nAllFanCnt);

	auto nGangTimes = pHuPlayerCard->getGangTimes().size();
	if (pZiMoPlayer->haveGangFalg())
	{
		if (nGangTimes == 4)
		{
			vType.push_back(eFanxing_4GangKai);
			nAllFanCnt++;
		}
		else if (nGangTimes == 3)
		{
			vType.push_back(eFanxing_3GangKai);
			nAllFanCnt++;
		}
		else if (nGangTimes == 2)
		{
			vType.push_back(eFanxing_2GangKai);
			nAllFanCnt++;
		}
		else
		{
			vType.push_back(eFanxing_GangKai);
			nAllFanCnt++;
		}
	}

	auto nCount = pHuPlayerCard->check7Pair(true, nCard);
	if (nCount == 3)
	{
		vType.push_back(eFanxing_3LongJia);
		nAllFanCnt += 4;
	}
	else if (nCount == 2)
	{
		vType.push_back(eFanxing_2LongJia);
		nAllFanCnt += 3;
	}
	else if (nCount == 1)
	{
		vType.push_back(eFanxing_LongJia);
		nAllFanCnt += 2;
	}
	else if (nCount == 0)
	{
		vType.push_back(eFanxing_QiDui);
		nAllFanCnt++;
	}

	Json::Value jsHuTyps;
	for (auto& refHu : vType)
	{
		if (refHu == eFanxing_PingHu && vType.size() > 1)
		{
			continue;
		}
		jsHuTyps[jsHuTyps.size()] = refHu;
	}
	jsDetail["vhuTypes"] = jsHuTyps;

	nAllFanCnt += 1; // 自摸1番
	
	if (m_bZhuangFan && nIdx == m_nBankerIdx)
	{
		nAllFanCnt++; // 庄家1番
	}

	int32_t nTotalWin = 0;
	for (auto pPlayer : m_vMJPlayers)
	{
		if (pPlayer->getIdx() != nIdx) // 另外三家分别计算输的coin，总和就是赢家获得的
		{
			auto nAllFanCnt_ = nAllFanCnt;

			if (m_bZhuangFan && pPlayer->getIdx() == m_nBankerIdx)
			{
				nAllFanCnt_++;
			}

			if (m_nFan == 0)
			{
				if (nAllFanCnt_ > 3)
				{
					nAllFanCnt_ = 3;
				}
			}
			else if (m_nFan == 1)
			{
				if (nAllFanCnt_ > 4)
				{
					nAllFanCnt_ = 4;
				}
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

	Json::Value jsVLoses;
	for (auto i = 0; i < MAX_SEAT_CNT; ++i)
	{
		int nCoin = vWinCoin[i] - vLoseCoin[i] + vGangCoinGet[i] - vGangCoinOut[i] + vFightLandLordGet[i] - vFightLandLordOut[i];
		//if (vLoseCoin[i] > 0)
		{
			Json::Value jsLose;
			jsLose["loseCoin"] = nCoin;
			jsLose["idx"] = i;
			jsVLoses[jsVLoses.size()] = jsLose;
		}
	}
	jsDetail["LoseIdxs"] = jsVLoses;
	jsDetail["zhuangPlayerIdx"] = m_nBankerIdx;

	LOGFMTD("room id = %u hu end ", getRoomID());
}

void JSMJRoom::onPlayerTangZhiHu(uint8_t nIdx)
{
	m_isBankerHu = (nIdx == getBankerIdx());

	Json::Value jsDetail;
	Json::Value jsMsg;

	jsMsg["isZiMo"] = 2;

	std::vector<uint32_t> vLoseCoin, vWinCoin, vGangCoinGet, vGangCoinOut, vFightLandLordGet, vFightLandLordOut;
	for (auto i = 0; i < MAX_SEAT_CNT; ++i)
	{
		vLoseCoin.push_back(0);
		vWinCoin.push_back(0);
		vGangCoinGet.push_back(0);
		vGangCoinOut.push_back(0);
		vFightLandLordGet.push_back(0);
		vFightLandLordOut.push_back(0);
	}

	Json::Value jsReal;
	calculateGangCoin(uint8_t(-1), uint8_t(-1), jsReal, vGangCoinGet, vGangCoinOut);
	jsMsg["realTimeCal"] = jsReal;

	jsDetail["huIdx"] = nIdx;

	auto pHuPlayer = (JSMJPlayer*)getMJPlayerByIdx(nIdx);
	auto pHuPlayerCard = (JSMJPlayerCard*)pHuPlayer->getPlayerCard();

	int32_t nAllFanCnt = 0;
	Json::Value jsHuTyps;
	if (m_nTangZhiType == 1)
	{
		std::vector<uint16_t> vType;
		pHuPlayerCard->onDoTangZhiHu(vType, nAllFanCnt, m_nTangZhiType == 1, m_bTangZi7Pair);

		for (auto& refHu : vType)
		{
			jsHuTyps[jsHuTyps.size()] = refHu;
		}
	}
	else
	{
		jsHuTyps[jsHuTyps.size()] = eFanxing_PingHu;
	}
	jsDetail["vhuTypes"] = jsHuTyps;
	if (m_bZhuangFan && nIdx == getBankerIdx())
	{
		nAllFanCnt++;
	}

	int32_t nTotalWin = 0;
	for (auto pPlayer : m_vMJPlayers)
	{
		if (pPlayer->getIdx() != nIdx) // 另外三家分别计算输的coin，总和就是赢家获得的
		{
			auto nAllFanCnt_ = nAllFanCnt;

			if (m_bZhuangFan && pPlayer->getIdx() == getBankerIdx())
			{
				nAllFanCnt_++;
			}

			if (m_nFan == 0)
			{
				if (nAllFanCnt_ > 3)
				{
					nAllFanCnt_ = 3;
				}
			}
			else if (m_nFan == 1)
			{
				if (nAllFanCnt_ > 4)
				{
					nAllFanCnt_ = 4;
				}
			}

			int32_t nLose = 1;
			for (int32_t i = 0; i < nAllFanCnt_; ++i) nLose *= 2;

			pPlayer->addOffsetCoin(-1 * (int32_t)nLose);
			pHuPlayer->addOffsetCoin(nLose);

			vLoseCoin[pPlayer->getIdx()] = nLose;

			nTotalWin += nLose;
		}
	}
	vWinCoin[pHuPlayer->getIdx()] = nTotalWin;

	jsDetail["winCoin"] = nTotalWin;

	//jsDetail["isGangKai"] = 0;
	//jsDetail["invokerGangIdx"] = nIdx;

	Json::Value jsVLoses;
	for (auto i = 0; i < MAX_SEAT_CNT; ++i)
	{
		int nCoin = vWinCoin[i] - vLoseCoin[i] + vGangCoinGet[i] - vGangCoinOut[i] + vFightLandLordGet[i] - vFightLandLordOut[i];
		Json::Value jsLose;
		jsLose["loseCoin"] = nCoin;
		jsLose["idx"] = i;
		jsVLoses[jsVLoses.size()] = jsLose;
	}
	jsDetail["LoseIdxs"] = jsVLoses;
	jsDetail["zhuangPlayerIdx"] = m_nBankerIdx;

	jsMsg["detail"] = jsDetail;
	sendRoomMsg(jsMsg, MSG_ROOM_JS_PLAYER_HU);
	LOGFMTD("room id = %u hu end ", getRoomID());

	m_nBankerIdx = nIdx;
}

bool JSMJRoom::onPlayerApplyLeave(uint32_t nPlayerUID)
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

void JSMJRoom::sendPlayersCardInfo(uint32_t nSessionID)
{
	Json::Value jsmsg;
	Json::Value vPeerCards;
	for (auto& pp : m_vMJPlayers)
	{
		if (pp == nullptr /*|| pp->haveState(eRoomPeer_CanAct) == false*/)  // lose also have card 
		{
			continue;
		}

		auto pCard = (JSMJPlayerCard*)pp->getPlayerCard();
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

void JSMJRoom::onPlayerChu(uint8_t nIdx, uint8_t nCard)
{
	IMJRoom::onPlayerChu(nIdx, nCard);
	auto pActPlayer = getMJPlayerByIdx(nIdx);
	auto pActCard = (JSMJPlayerCard*)pActPlayer->getPlayerCard();
	if (pActCard->getChiPengGangFlag() == false)
	{
		pActCard->addTangZhiCard(nCard);
	}

	if (isGameOver())
	{
		goToState(eRoomState_GameEnd);
	}
}

std::shared_ptr<IGameRoomRecorder> JSMJRoom::createRoomRecorder()
{
	return std::make_shared<JSMJRoomRecorder>();
}

void JSMJRoom::onPlayerMingGang(uint8_t nIdx, uint8_t nCard, uint8_t nInvokeIdx)
{
	auto pPlayer = getMJPlayerByIdx(nIdx);
	auto pInvoker = getMJPlayerByIdx(nInvokeIdx);
	if (!pPlayer || !pInvoker)
	{
		LOGFMTE("why this player is null idx = %u , can not ming gang", nIdx);
		return;
	}

	if (m_b2ZhangDi)
	{
		assert((m_nGangCard == m_nGangCard1 || m_nGangCard == m_nGangCard2) && "gang card not exist! why?");
	} 
	else
	{
		assert((m_nGangCard == m_nGangCard1) && "gang card not exist! why?");
	}

	pPlayer->signGangFlag();
	pPlayer->addMingGangCnt();

	//auto nGangGetCard = getMJPoker()->distributeOneCard();
	if (pPlayer->getPlayerCard()->onMingGang(nCard, m_nGangCard) == false)
	{
		LOGFMTE("nidx = %u ming gang card = %u error,", nIdx, nCard);
	}
	pInvoker->getPlayerCard()->onCardBeGangPengEat(nCard);

	// send msg 
	Json::Value msg;
	msg["idx"] = nIdx;
	msg["actType"] = eMJAct_MingGang;
	msg["card"] = nCard;
	msg["gangCard"] = m_nGangCard;
	sendRoomMsg(msg, MSG_ROOM_ACT);

	//
	Json::Value jsFrameArg;
	auto ptrReplay = getGameReplay()->createFrame(eMJFrame_MingGang, (uint32_t)time(nullptr));
	jsFrameArg["idx"] = nIdx;
	jsFrameArg["gang"] = nCard;
	jsFrameArg["newCard"] = m_nGangCard;
	jsFrameArg["invokerIdx"] = nInvokeIdx;
	ptrReplay->setFrameArg(jsFrameArg);
	getGameReplay()->addFrame(ptrReplay);

	auto playerCard = (JSMJPlayerCard*)pPlayer->getPlayerCard();
	playerCard->addSongGangIdx(nCard, nInvokeIdx, eMJAct_MingGang);

	playerCard->addGangTimes(nCard, nInvokeIdx);

	changeGangCard();
}

void JSMJRoom::onPlayerAnGang(uint8_t nIdx, uint8_t nCard)
{
	auto pPlayer = getMJPlayerByIdx(nIdx);
	if (!pPlayer)
	{
		LOGFMTE("why this player is null idx = %u , can not an gang", nIdx);
		return;
	}

	if (m_b2ZhangDi)
	{
		assert((m_nGangCard == m_nGangCard1 || m_nGangCard == m_nGangCard2) && "gang card not exist! why?");
	}
	else
	{
		assert((m_nGangCard == m_nGangCard1) && "gang card not exist! why?");
	}

	pPlayer->signGangFlag();
	pPlayer->addAnGangCnt();
	//auto nGangGetCard = getMJPoker()->distributeOneCard();
	if (pPlayer->getPlayerCard()->onAnGang(nCard, m_nGangCard) == false)
	{
		LOGFMTE("nidx = %u an gang card = %u error,", nIdx, nCard);
	}

	// send msg ;
	Json::Value msg;
	msg["idx"] = nIdx;
	msg["actType"] = eMJAct_AnGang;
	msg["card"] = nCard;
	msg["gangCard"] = m_nGangCard;
	sendRoomMsg(msg, MSG_ROOM_ACT);

	//
	Json::Value jsFrameArg;
	auto ptrReplay = getGameReplay()->createFrame(eMJFrame_AnGang, (uint32_t)time(nullptr));
	jsFrameArg["idx"] = nIdx;
	jsFrameArg["gang"] = nCard;
	jsFrameArg["newCard"] = m_nGangCard;
	ptrReplay->setFrameArg(jsFrameArg);
	getGameReplay()->addFrame(ptrReplay);

	auto playerCard = (JSMJPlayerCard*)pPlayer->getPlayerCard();
	playerCard->addGangTimes(nCard, nIdx);

	changeGangCard();
}

void JSMJRoom::onPlayerBuGang(uint8_t nIdx, uint8_t nCard)
{
	auto pPlayer = getMJPlayerByIdx(nIdx);
	if (!pPlayer)
	{
		LOGFMTE("why this player is null idx = %u , can not bu gang", nIdx);
		return;
	}

	if (m_b2ZhangDi)
	{
		assert((m_nGangCard == m_nGangCard1 || m_nGangCard == m_nGangCard2) && "gang card not exist! why?");
	}
	else
	{
		assert((m_nGangCard == m_nGangCard1) && "gang card not exist! why?");
	}

	pPlayer->signGangFlag();
	pPlayer->clearDecareBuGangFlag();
	//auto nGangCard = getMJPoker()->distributeOneCard();
	if (pPlayer->getPlayerCard()->onBuGang(nCard, m_nGangCard) == false)
	{
		LOGFMTE("nidx = %u bu gang card = %u error,", nIdx, nCard);
	}
	pPlayer->addMingGangCnt();
	// send msg 
	Json::Value msg;
	msg["idx"] = nIdx;
	msg["actType"] = eMJAct_BuGang_Done;
	msg["card"] = nCard;
	msg["gangCard"] = m_nGangCard;
	sendRoomMsg(msg, MSG_ROOM_ACT);

	//
	Json::Value jsFrameArg;
	auto ptrReplay = getGameReplay()->createFrame(eMJFrame_BuGang, (uint32_t)time(nullptr));
	jsFrameArg["idx"] = nIdx;
	jsFrameArg["gang"] = nCard;
	jsFrameArg["newCard"] = m_nGangCard;
	ptrReplay->setFrameArg(jsFrameArg);
	getGameReplay()->addFrame(ptrReplay);

	auto playerCard = (JSMJPlayerCard*)pPlayer->getPlayerCard();
	std::vector<JSMJPlayerCard::tSongGangIdx> vSongIdx = playerCard->getPengSongIdx();
	bool bFind = false;
	for (auto tSong : vSongIdx)
	{
		if (tSong.nGangCard == nCard)
		{
			bFind = true;
			playerCard->addSongGangIdx(nCard, tSong.nIdx, eMJAct_BuGang);
			playerCard->addGangTimes(nCard, tSong.nIdx);
			break;
		}
	}
	if (!bFind) assert(0 && "bu gang, peng player not found!!!");

	changeGangCard();
}

void JSMJRoom::changeGangCard()
{
	if (m_b2ZhangDi)
	{
		assert((m_nGangCard == m_nGangCard1 || m_nGangCard == m_nGangCard2) && "gang card not exist! why?");
	}
	else
	{
		assert((m_nGangCard == m_nGangCard1) && "gang card not exist! why?");
	}
	
	auto nGangCard = getMJPoker()->distributeOneCard();

	if (m_nGangCard == m_nGangCard1)
	{
		m_nGangCard1 = nGangCard;
	}
	else
	{
		if (m_b2ZhangDi)
		{
			m_nGangCard2 = nGangCard;
		}
	}
	m_nGangCard = uint8_t(-1);

	m_nGangCount++;

	// send msg 
	Json::Value msg;
	msg["card"] = nGangCard;
	sendRoomMsg(msg, MSG_ROOM_JS_GANG_GET_CARD);

	Json::Value jsFrameArg;
	auto ptrReplay = getGameReplay()->createFrame(eMJFrame_GetGang, (uint32_t)time(nullptr));
	jsFrameArg["newCard"] = nGangCard;
	ptrReplay->setFrameArg(jsFrameArg);
	getGameReplay()->addFrame(ptrReplay);
}

void JSMJRoom::onPlayerMo(uint8_t nIdx)
{
	IMJRoom::onPlayerMo(nIdx);

	auto pPlayer = getMJPlayerByIdx(nIdx);
	auto pPlayerCard = (JSMJPlayerCard*)pPlayer->getPlayerCard();
	pPlayerCard->clearGangTimes();
}

bool JSMJRoom::canGang(uint8_t nIdx)
{
	auto pPlayer = getMJPlayerByIdx(nIdx);
	if (!pPlayer)
	{
		LOGFMTE("player idx = %u is null can not tell it can gang", nIdx);
		return false;
	}
	auto pMJCard = pPlayer->getPlayerCard();

	bool bRet = false;

	if (isCanGoOnMoPai())
	{
		// check bu gang .
		IMJPlayerCard::VEC_CARD vCards;
		bRet = pMJCard->getHoldCardThatCanBuGang(vCards);

		// check an gang .
		vCards.clear();
		bRet |= pMJCard->getHoldCardThatCanAnGang(vCards);
	}

	return bRet;
}

bool JSMJRoom::isAnyPlayerPengOrHuThisCard(uint8_t nInvokeIdx, uint8_t nCard)
{
	for (auto& ref : m_vMJPlayers)
	{
		if (ref == nullptr || nInvokeIdx == ref->getIdx())
		{
			continue;
		}
		ref->clearGangFlag();
		auto pCard = (JSMJPlayerCard*)ref->getPlayerCard();
		pCard->clearGangTimes();
	}

	for (auto& ref : m_vMJPlayers)
	{
		if (ref == nullptr || nInvokeIdx == ref->getIdx())
		{
			continue;
		}

		auto pMJCard = ref->getPlayerCard();
		if (pMJCard->canHuWitCard(nCard))
		{
			return true;
		}

		if (pMJCard->canPengWithCard(nCard))
		{
			return true;
		}

		if (isCanGoOnMoPai() && pMJCard->canMingGangWithCard(nCard)) // must can gang , will not run here , will return when check peng ;
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