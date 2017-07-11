#include "CYMJRoom.h"
#include "CYMJPlayerCard.h"
#include "log4z.h"
#include "IMJPoker.h"
#include "CYMJPlayer.h"
#include "IGameRoomManager.h"
#include "ServerMessageDefine.h"
#include "MJRoomStateWaitReady.h"
#include "MJRoomStateWaitPlayerChu.h"
#include "MJRoomStateWaitPlayerAct.h"
#include "MJRoomStateStartGame.h"
#include "MJRoomStateGameEnd.h"
#include "MJRoomStateDoPlayerAct.h"
#include "CYRoomStateAskForPengOrHu.h"
#include "MJRoomStateAskForRobotGang.h"
#include "CYRoomStateDoPlayerAct.h"
#include "CYRoomStateWaitPlayerAct.h"
#include "CYRoomStateWaitPlayerChu.h"
#include "CYMJPlayerRecorderInfo.h"
#include "MJReplayFrameType.h"

#include <ctime>

bool CYMJRoom::init(IGameRoomManager* pRoomMgr, stBaseRoomConfig* pConfig, uint32_t nSeialNum, uint32_t nRoomID, Json::Value& vJsValue)
{
	IMJRoom::init(pRoomMgr, pConfig, nSeialNum, nRoomID, vJsValue);
	m_isBankerHu = false;
	m_isOnePay = vJsValue["isOnePay"].asUInt() == 0 ? false : true;
	m_isBiMenHu = vJsValue["isBiMenHu"].asUInt() == 0 ? false : true;
	m_isJiaHu = vJsValue["isJiaHu"].asUInt() == 0 ? false : true;
	m_isQingYiSe = vJsValue["isQingYiSe"].asUInt() == 0 ? false : true;
	m_isLouBao = vJsValue["isLouBao"].asUInt() == 0 ? false : true;
	m_tPoker.initAllCard(eMJ_COMMON);
	// create state and add state ;
	IMJRoomState* vState[] = {
		new CMJRoomStateWaitReady(), new CYRoomStateWaitPlayerChu(), new CYRoomStateWaitPlayerAct(), new MJRoomStateStartGame()
		, new MJRoomStateGameEnd(), new CYRoomStateDoPlayerAct(), new CYRoomStateAskForPengOrHu(), new MJRoomStateAskForRobotGang()
	};
	for (uint8_t nIdx = 0; nIdx < sizeof(vState) / sizeof(IMJRoomState*); ++nIdx)
	{
		addRoomState(vState[nIdx]);
	}
	setInitState(vState[0]);

	// init banker
	m_nBankerIdx = -1;
	auto pRoomRecorder = (CYMJRoomRecorder*)getRoomRecorder().get();
	pRoomRecorder->setRoomOpts(m_isOnePay ? 1 : 0);

	return true;
}

void CYMJRoom::willStartGame()
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

	for (auto& pPlayer : m_vMJPlayers)
	{
		if (pPlayer)
		{
			((CYMJPlayer*)pPlayer)->ClearTing();
			((CYMJPlayer*)pPlayer)->ClearBiMen();
			((CYMJPlayer*)pPlayer)->setTingGangType(eMJAct_Max);
			((CYMJPlayer*)pPlayer)->SetGangMo(false);
			((CYMJPlayer*)pPlayer)->SetIsBiMenHu(m_isBiMenHu);
			((CYMJPlayer*)pPlayer)->SetHaveQingYiSe(m_isQingYiSe);
		}
	}
	m_nBaoCard = -1;
	m_isChiAndTing = false;
	m_bHuanBaoCardFlag = false;
}

void CYMJRoom::startGame()
{
	IMJRoom::startGame();
	Json::Value jsMsg;
	IMJRoom::packStartGameMsg(jsMsg);
	sendRoomMsg(jsMsg, MSG_ROOM_START_GAME);

	// replay arg 
	Json::Value jsReplayInfo;
	jsReplayInfo["roomID"] = getRoomID();
	jsReplayInfo["time"] = (uint32_t)time(nullptr);
	jsReplayInfo["isOnePay"] = m_isOnePay ? 1 : 0;
	jsReplayInfo["isBiMenHu"] = m_isBiMenHu ? 1 : 0;
	jsReplayInfo["isJiaHu"] = m_isJiaHu ? 1 : 0;
	jsReplayInfo["isQingYiSe"] = m_isQingYiSe ? 1 : 0;
	jsReplayInfo["isLouBao"] = m_isLouBao ? 1 : 0;
	getGameReplay()->setReplayRoomInfo(jsReplayInfo);
}

void CYMJRoom::getSubRoomInfo(Json::Value& jsSubInfo)
{
	jsSubInfo["isOnePay"] = m_isOnePay ? 1 : 0;
	jsSubInfo["isBiMenHu"] = m_isBiMenHu ? 1 : 0;
	jsSubInfo["isJiaHu"] = m_isJiaHu ? 1 : 0;
	jsSubInfo["isQingYiSe"] = m_isQingYiSe ? 1 : 0;
	jsSubInfo["isLouBao"] = m_isLouBao ? 1 : 0;
}

void CYMJRoom::onGameDidEnd()
{
	IMJRoom::onGameDidEnd();
	if (getDelegate())
	{
		getDelegate()->onDidGameOver(this);
		return;
	}
}

void CYMJRoom::onGameEnd()
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

			auto pPlayerRecorderInfo = std::make_shared<CYMJPlayerRecorderInfo>();
			pPlayerRecorderInfo->init(ref->getUID(), ref->getOffsetCoin());
			ptrSingleRecorder->addPlayerRecorderInfo(pPlayerRecorderInfo);
		}
	}

	jsMsg["isLiuJu"] = isAnyOneHu ? 0 : 1;
	jsMsg["detail"] = jsDetial;

	sendRoomMsg(jsMsg, MSG_ROOM_CY_GAME_OVER);
	// send msg to player ;
	IMJRoom::onGameEnd();
}

IMJPlayer* CYMJRoom::doCreateMJPlayer()
{
	return new CYMJPlayer();
}

IMJPoker* CYMJRoom::getMJPoker()
{
	return &m_tPoker;
}

bool CYMJRoom::isGameOver()
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

void CYMJRoom::onPlayerTing(uint8_t nIdx/*, std::vector<uint8_t> vecTingCards*/)
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

void CYMJRoom::onPlayerPeng(uint8_t nIdx, uint8_t nCard, uint8_t nInvokeIdx)
{
	IMJRoom::onPlayerPeng(nIdx, nCard, nInvokeIdx);
	auto pActPlayer = getMJPlayerByIdx(nIdx);
	auto pActCard = (CYMJPlayerCard*)pActPlayer->getPlayerCard();
	pActCard->addSongPengIdx(nCard, nInvokeIdx);
}

void CYMJRoom::onPlayerMoBao(uint8_t nIdx, uint8_t nBaoCard)
{
	m_nBaoCard = nBaoCard;

	Json::Value msg;
	msg["idx"] = nIdx;
	msg["actType"] = eMJAct_MoBao;
	msg["dice"] = 1 + rand() % 6;
	sendRoomMsg(msg, MSG_ROOM_ACT);

	auto pPlayer = getMJPlayerByIdx(nIdx);
	Json::Value jsMsg;
	jsMsg["baoCard"] = nBaoCard;
	sendMsgToPlayer(jsMsg, MSG_BAO_CARD, pPlayer->getSessionID());

	Json::Value jsFrameArg;
	auto ptrReplay = getGameReplay()->createFrame(eMJFrame_Player_MoBao, (uint32_t)time(nullptr));
	jsFrameArg["idx"] = nIdx;
	jsFrameArg["card"] = nBaoCard;
	ptrReplay->setFrameArg(jsFrameArg);
	getGameReplay()->addFrame(ptrReplay);
}

void CYMJRoom::onPlayerEat(uint8_t nIdx, uint8_t nCard, uint8_t nWithA, uint8_t nWithB, uint8_t nInvokeIdx)
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

void CYMJRoom::calculateGangCoin(Json::Value& jsMsg, std::vector<uint32_t>& vGangCoinGet, std::vector<uint32_t>& vGangCoinOut, bool isOnePay)
{
	//realTimeCal: [{ actType: 23, detial : [{idx: 2, offset : -23 }, ...]  }, ...]
	for (auto pWinPlayer : m_vMJPlayers)
	{
		auto pPlayerCard = (CYMJPlayerCard*)pWinPlayer->getPlayerCard();
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

		std::vector<CYMJPlayerCard::tSongGangIdx> vMingGang = pPlayerCard->getMingGangSongIdx();
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

			//if (isOnePay)
			//{
			//	Json::Value jsLose;
			//	jsLose["idx"] = vMingGang[i].nIdx;
			//	jsLose["offset"] = -2;
			//	jsDetial[jsDetial.size()] = jsLose;

			//	vGangCoinOut[vMingGang[i].nIdx] += 2;

			//	auto pLosePlayer = getMJPlayerByIdx(vMingGang[i].nIdx);
			//	pLosePlayer->addOffsetCoin(-2);
			//}
			//else
			//{
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
			//}

			jsReal["detial"] = jsDetial;
			jsMsg[jsMsg.size()] = jsReal;
		}
	}
}

void CYMJRoom::onPlayerHu(std::vector<uint8_t>& vHuIdx, uint8_t nCard, uint8_t nInvokeIdx)
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
		auto pHuPlayer = (CYMJPlayer*)getMJPlayerByIdx(nInvokeIdx);
		auto pPlayerCard = (CYMJPlayerCard*)pHuPlayer->getPlayerCard();
		bool isLouBao = false;


		//uint8_t nGangType = pHuPlayer->getTingGangType();
		//if (nGangType == eMJAct_Hu && nCard == m_nBaoCard)
		if (nCard == m_nBaoCard && pHuPlayer->GetTing())
		{
			isLouBao = true;
		}
		onPlayerZiMo(isLouBao, nInvokeIdx, nCard, jsDetail, vLoseCoin, vWinCoin, vGangCoinGet, vGangCoinOut);
		jsMsg["detail"] = jsDetail;
		sendRoomMsg(jsMsg, MSG_ROOM_CY_PLAYER_HU);
		return;
	}
	//for (uint8_t i = 1; i <= 3; ++i)
	//{
	//	auto nCheckIdx = nInvokeIdx + i;
	//	nCheckIdx = nCheckIdx % 4;
	//	auto iter = std::find(vHuIdx.begin(), vHuIdx.end(), nCheckIdx);
	//	if (iter != vHuIdx.end())
	//	{
	//		auto pPlayer = (CYMJPlayer*)getMJPlayerByIdx(nCheckIdx);
	//		if (!pPlayer)
	//		{
	//			LOGFMTE("room id = %u, player idx = %u is nullptr, do not know KanBao!!!", getRoomID(), nInvokeIdx);
	//			return;
	//		}
	//		auto pPlayerCard = (CYMJPlayerCard*)pPlayer->getPlayerCard();
	//		if (pPlayerCard->getKanBao() != uint8_t(-1))
	//		{
	//			onPlayerZiMo(true, nCheckIdx, nCard, jsDetail, vLoseCoin, vWinCoin, vGangCoinGet, vGangCoinOut);
	//			jsMsg["detail"] = jsDetail;
	//			sendRoomMsg(jsMsg, MSG_ROOM_CY_PLAYER_HU);
	//			return;
	//		}
	//	}
	//}

	auto pLosePlayer = (CYMJPlayer*)getMJPlayerByIdx(nInvokeIdx);
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
		auto pHuPlayerCard = (CYMJPlayerCard*)pHuPlayer->getPlayerCard();
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

		auto pHuPlayerCard = (CYMJPlayerCard*)pHuPlayer->getPlayerCard();

		std::vector<uint16_t> vType;
		int32_t nAllFanCnt = 0;
		pHuPlayerCard->onDoHu(false, false, m_isQingYiSe, nCard, vType, nAllFanCnt);

		//auto pLoseCard = (CYMJPlayerCard*)pLosePlayer->getPlayerCard();
		//if ((uint8_t)-1 != pLoseCard->getSongGangIdx())
		if (pLosePlayer->haveDecareBuGangFalg()) // robot gang ;
		{
			nAllFanCnt++;
		}
		if (pLosePlayer->haveGangFlagPao())
		{
			vType.push_back(eFanxing_LiuLei);
			nAllFanCnt++;
		}

		if (nHuIdx == m_nBankerIdx)
		{
			nAllFanCnt++; // 庄家1番
		}

		if (((CYMJPlayer*)pHuPlayer)->GetTing())
		{
			nAllFanCnt++; // 听牌1番
		}

		if (m_isBiMenHu && ((CYMJPlayer*)pHuPlayer)->GetBiMen())
		{
			nAllFanCnt++; // 闭门胡1番
		}

		if (m_isJiaHu && pHuPlayerCard->checkJiaHu(nCard, false) > 0)
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
			if (((CYMJPlayer*)pPlayer)->GetBiMen() && pPlayer->getIdx() != nHuIdx)
			{
				nBiMenCnt++;
			}
		}
		if (nBiMenCnt == 3) // 三家闭门：2番
		{
			nAllFanCnt += 2;
		}

		//if (m_isOnePay)
		//{
		//	//一家付
		//	nAllFanCnt += 1; // 点炮的人算2番

		//	if (nBiMenCnt != 3 && ((CYMJPlayer*)pLosePlayer)->GetBiMen())
		//	{
		//		nAllFanCnt++; // 闭门1番
		//	}
		//	if (nInvokeIdx == getBankerIdx())
		//	{
		//		nAllFanCnt++;
		//	}

		//	auto nLoseCoin = 1;
		//	for (int32_t i = 0; i < nAllFanCnt; ++i) nLoseCoin *= 2;

		//	pLosePlayer->addOffsetCoin(-1 * (int32_t)nLoseCoin);
		//	pHuPlayer->addOffsetCoin(nLoseCoin);

		//	jsHuPlayer["win"] = nLoseCoin;

		//	vWinCoin[pHuPlayer->getIdx()] = nLoseCoin;
		//	vLoseCoin[pLosePlayer->getIdx()] += nLoseCoin;
		//}
		//else
		//{
			uint32_t nTotalLose = 0;

			////三家付
			//int32_t nBiMenCnt = 0;
			//for (auto pPlayer : m_vMJPlayers)
			//{
			//	if (((CYMJPlayer*)pPlayer)->GetBiMen() && pPlayer->getIdx() != nHuIdx)
			//	{
			//		nBiMenCnt++;
			//	}
			//}
			//if (nBiMenCnt == 3) // 三家闭门：2番
			//{
			//	nAllFanCnt += 2;
			//}

			for (auto pPlayer : m_vMJPlayers)
			{
				if (pPlayer->getIdx() == nHuIdx)
				{
					continue;
				}

				auto nAllFanCnt_ = nAllFanCnt;
				auto nLoseCoin = 1;

				if (((CYMJPlayer*)pPlayer)->GetBiMen() && nBiMenCnt != 3)
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

					//for (int32_t i = 0; i < nAllFanCnt_; ++i) nLoseCoin *= 2;

					//pPlayer->addOffsetCoin(-1 * (int32_t)nLoseCoin);
					//pHuPlayer->addOffsetCoin(nLoseCoin);
				}
				//else
				//{
					for (int32_t i = 0; i < nAllFanCnt_; ++i) nLoseCoin *= 2;

					//nLoseCoin++;

					if (!m_isOnePay)
					{
						pPlayer->addOffsetCoin(-1 * (int32_t)nLoseCoin);
						pHuPlayer->addOffsetCoin(nLoseCoin);
						vLoseCoin[pPlayer->getIdx()] += nLoseCoin;
					}
				//}

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

			m_isBankerHu = nHuIdx == m_nBankerIdx;
			break;
		//}
		//jsHuPlayers[jsHuPlayers.size()] = jsHuPlayer;
	}

	Json::Value jsTingPlayer, jsBiMenPlayer;
	for (auto pPlayer : m_vMJPlayers)
	{
		if (((CYMJPlayer*)pPlayer)->GetTing())
		{
			jsTingPlayer[jsTingPlayer.size()] = pPlayer->getIdx();
		}
		if (((CYMJPlayer*)pPlayer)->GetBiMen())
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

	jsDetail["tingPlayers"] = jsTingPlayer;
	jsDetail["bimenPlayers"] = jsBiMenPlayer;
	jsDetail["zhuangPlayerIdx"] = m_nBankerIdx;
	jsMsg["detail"] = jsDetail;
	sendRoomMsg(jsMsg, MSG_ROOM_CY_PLAYER_HU);
	LOGFMTD("room id = %u hu end ", getRoomID());
}

void CYMJRoom::onPlayerZiMo(bool isLouBao, uint8_t nIdx, uint8_t nCard, Json::Value& jsDetail, std::vector<uint32_t> vLoseCoin, std::vector<uint32_t> vWinCoin,
	std::vector<uint32_t> vGangCoinGet, std::vector<uint32_t> vGangCoinOut)
{
	auto pZiMoPlayer = (CYMJPlayer*)getMJPlayerByIdx(nIdx);
	if (pZiMoPlayer == nullptr)
	{
		LOGFMTE("room id = %u zi mo player is nullptr idx = %u ", getRoomID(), nIdx);
		return;
	}
	pZiMoPlayer->addZiMoCnt();
	pZiMoPlayer->setState(eRoomPeer_AlreadyHu);
	// svr :{ huIdx : 234 , baoPaiIdx : 2 , winCoin : 234,huardSoftHua : 23, isGangKai : 0 ,vhuTypes : [ eFanxing , ], LoseIdxs : [ {idx : 1 , loseCoin : 234 }, .... ]   }
	jsDetail["huIdx"] = nIdx;

	auto pHuPlayerCard = (CYMJPlayerCard*)pZiMoPlayer->getPlayerCard();
	std::vector<uint16_t> vType;
	int32_t nAllFanCnt = 0;
	bool bZiMo = true;
	if (isLouBao)
	{
		std::set<uint8_t> vHuCards;
		pHuPlayerCard->getCanHuCards(vHuCards);

		auto it_bao = std::find(vHuCards.begin(), vHuCards.end(), nCard);
		if (it_bao != vHuCards.end())
		{
			pHuPlayerCard->onDoHu(false, getMJPoker()->getLeftCardCount() < getSeatCnt(), m_isQingYiSe, nCard, vType, nAllFanCnt);
		}
		else
		{
			pHuPlayerCard->onBaoPaiDoHu(getMJPoker()->getLeftCardCount() < getSeatCnt(), m_isQingYiSe, nCard, vType, nAllFanCnt);
		}
	}
	else
	{
		pHuPlayerCard->onDoHu(bZiMo, getMJPoker()->getLeftCardCount() < getSeatCnt(), m_isQingYiSe, nCard, vType, nAllFanCnt);
	}

	if (m_nBaoCard == nCard && pZiMoPlayer->GetTing())
	{
		vType.push_back(eFanxing_LouBao);
		nAllFanCnt++;

		if (m_isJiaHu)
		{
			bool isKan = pHuPlayerCard->getKanBao() == nCard;
			if (!isKan)
			{
				pHuPlayerCard->pickoutHoldCard(nCard);
			}

			std::set<uint8_t> vHuCards;
			pHuPlayerCard->getCanHuCards(vHuCards);

			for (auto c : vHuCards)
			{
				uint8_t nType = pHuPlayerCard->checkJiaHu(c, false);
				if (nType > 0)
				{
					//bool bJ = true;
					//if (nType == 1 && (nCard == c - 1 || nCard == c - 2))
					//{
					//	bJ = false;
					//}
					//else if (nType == 2 && (nCard == c - 1 || nCard == c + 1))
					//{
					//	bJ = false;
					//}
					//else if (nType == 3 && (nCard == c + 1 || nCard == c + 2))
					//{
					//	bJ = false;
					//}
					//if (bJ)
					//{
						vType.push_back(eFanxing_JiaHu);
						nAllFanCnt++; // 夹胡1番
						break;
					//}
				}
			}

			if (!isKan)
			{
				pHuPlayerCard->addDistributeCard(nCard);
			}
		}
	}
	else
	{
		if (m_isJiaHu && pHuPlayerCard->checkJiaHu(nCard, bZiMo) > 0)
		{
			vType.push_back(eFanxing_JiaHu);
			nAllFanCnt++; // 夹胡1番
		}
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

	if (((CYMJPlayer*)pZiMoPlayer)->GetTing())
	{
		nAllFanCnt++; // 听牌1番
	}

	if (m_isBiMenHu && ((CYMJPlayer*)pZiMoPlayer)->GetBiMen())
	{
		nAllFanCnt++; // 闭门胡1番
	}

	//三家付
	int32_t nBiMenCnt = 0;
	for (auto pPlayer : m_vMJPlayers)
	{
		if (((CYMJPlayer*)pPlayer)->GetBiMen() && pPlayer->getIdx() != nIdx)
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
			if (((CYMJPlayer*)pPlayer)->GetBiMen() && nBiMenCnt != 3)
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

	Json::Value jsTingPlayer, jsBiMenPlayer;
	for (auto pPlayer : m_vMJPlayers)
	{
		if (((CYMJPlayer*)pPlayer)->GetTing())
		{
			jsTingPlayer[jsTingPlayer.size()] = pPlayer->getIdx();
		}
		if (((CYMJPlayer*)pPlayer)->GetBiMen())
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

	jsDetail["tingPlayers"] = jsTingPlayer;
	jsDetail["bimenPlayers"] = jsBiMenPlayer;
	jsDetail["zhuangPlayerIdx"] = m_nBankerIdx;

	LOGFMTD("room id = %u hu end ", getRoomID());
}

bool CYMJRoom::onPlayerApplyLeave(uint32_t nPlayerUID)
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

void CYMJRoom::sendPlayersCardInfo(uint32_t nSessionID)
{
	Json::Value jsmsg;
	Json::Value vPeerCards;
	for (auto& pp : m_vMJPlayers)
	{
		if (pp == nullptr /*|| pp->haveState(eRoomPeer_CanAct) == false*/)  // lose also have card 
		{
			continue;
		}

		auto pCard = (CYMJPlayerCard*)pp->getPlayerCard();
		Json::Value jsCardInfo;
		jsCardInfo["idx"] = pp->getIdx();
		jsCardInfo["newMoCard"] = 0;
		if (getCurRoomState()->getStateID() == eRoomState_WaitPlayerAct && getCurRoomState()->getCurIdx() == pp->getIdx())
		{
			jsCardInfo["newMoCard"] = pp->getPlayerCard()->getNewestFetchedCard();
		}

		pCard->getCardInfo(jsCardInfo, ((CYMJPlayer*)pp)->GetTing(), m_nBaoCard);
		sendMsgToPlayer(jsCardInfo, MSG_ROOM_PLAYER_CARD_INFO, nSessionID);
	}

	LOGFMTD("send player card infos !");
}

void CYMJRoom::onPlayerMo(uint8_t nIdx)
{
	if (!m_isLouBao)
	{
		IMJRoom::onPlayerMo(nIdx);
		return;
	}

	auto pPlayer = (CYMJPlayer*)getMJPlayerByIdx(nIdx);
	if (!pPlayer)
	{
		LOGFMTE("why this player is null idx = %u , can not mo", nIdx);
		return;
	}
	LOGFMTD("++++++++++++++++++++++++++++++++++++ onPlayerMo");
	auto nNewCard = getMJPoker()->distributeOneCard();
	if (nNewCard == 0)
	{
		Assert(0, "invlid card");
	}
	pPlayer->getPlayerCard()->onMoCard(nNewCard);

	if (pPlayer->GetGangMo())
	{
		pPlayer->SetGangMo(false);
	}
	else
	{
		pPlayer->clearGangFlag();
		pPlayer->clearDecareBuGangFlag();
	}

	// send msg ;
	Json::Value msg;
	msg["idx"] = nIdx;
	msg["actType"] = eMJAct_Mo;
	msg["card"] = nNewCard;
	sendRoomMsg(msg, MSG_ROOM_ACT);


	Json::Value jsFrameArg;
	auto ptrReplay = getGameReplay()->createFrame(eMJFrame_Mo, (uint32_t)time(nullptr));
	jsFrameArg["idx"] = nIdx;
	jsFrameArg["card"] = nNewCard;
	ptrReplay->setFrameArg(jsFrameArg);
	getGameReplay()->addFrame(ptrReplay);
}

void CYMJRoom::onPlayerMingGang(uint8_t nIdx, uint8_t nCard, uint8_t nInvokeIdx)
{
	auto pPlayer = (CYMJPlayer*)getMJPlayerByIdx(nIdx);
	auto pInvoker = getMJPlayerByIdx(nInvokeIdx);
	if (!pPlayer || !pInvoker)
	{
		LOGFMTE("why this player is null idx = %u , can not ming gang", nIdx);
		return;
	}
	auto playerCard = (CYMJPlayerCard*)pPlayer->getPlayerCard();

	if (!m_isLouBao)
	{
		IMJRoom::onPlayerMingGang(nIdx, nCard, nInvokeIdx);

		playerCard->setSongGangIdx(nInvokeIdx);
		playerCard->addSongGangIdx(nCard, nInvokeIdx, eMJAct_MingGang);
		return;
	}

	pPlayer->signGangFlagPao();
	pPlayer->signGangFlag();
	pPlayer->addMingGangCnt();

	auto eTingGangFlag = pPlayer->getTingGangType();
	if (eTingGangFlag == eMJAct_AnGang)
	{
		pPlayer->setTingGangType(eMJAct_Max);
	}

	uint8_t nGangGetCard = uint8_t(-1);

	if (pPlayer->GetTing() && (playerCard->getBaoCard() == uint8_t(-1) || playerCard->getBaoCard() != m_nBaoCard 
		|| m_nBaoCard == uint8_t(-1) || m_bHuanBaoCardFlag))
	{
		//听了，还没有搂宝
		playerCard->onPutMingGang(nCard);

		pInvoker->getPlayerCard()->onCardBeGangPengEat(nCard);

		// send msg 
		Json::Value msg;
		msg["idx"] = nIdx;
		msg["actType"] = eMJAct_MingGang;
		msg["card"] = nCard;
		//msg["gangCard"] = nGangGetCard;
		sendRoomMsg(msg, MSG_ROOM_ACT);

		playerCard->setSongGangIdx(nInvokeIdx);
		playerCard->addSongGangIdx(nCard, nInvokeIdx, eMJAct_MingGang);

		if (m_nBaoCard == uint8_t(-1) || m_bHuanBaoCardFlag)
		{
			m_nBaoCard = getValidBaoCard();
			onPlayerMoBao(nIdx, m_nBaoCard);

			//这里是给投塞子后摸牌判断打的标记
			pPlayer->setTingGangType(eMJAct_MingGang);
			pPlayer->SetGangMo(true);

			Json::Value jsTrans;
			jsTrans["act"] = eMJAct_MoBaoDice;
			jsTrans["idx"] = nIdx;
			goToState(eRoomState_DoPlayerAct, &jsTrans);
			return;
		}
		else
		{
			Json::Value jsMsg;
			jsMsg["baoCard"] = m_nBaoCard;
			sendMsgToPlayer(jsMsg, MSG_BAO_CARD, pPlayer->getSessionID());
		}
		//检查宝牌是否能胡（只有换宝牌才能杠）
		if (playerCard->canHuWitCard(m_nBaoCard))
		{
			playerCard->setKanBao(m_nBaoCard);

			pPlayer->setTingGangType(eMJAct_MingGang);

			Json::Value jsValue;
			jsValue["invokeIdx"] = nIdx;
			jsValue["card"] = m_nBaoCard;
			goToState(eRoomState_AskForHuAndPeng, &jsValue);
			return;
		}
		playerCard->setBaoCard(m_nBaoCard);

		pPlayer->SetGangMo(true);

		Json::Value jsTran;
		jsTran["idx"] = nIdx;
		jsTran["act"] = eMJAct_Mo;
		goToState(eRoomState_DoPlayerAct, &jsTran);
	}
	else if (eTingGangFlag == eMJAct_Max && pPlayer->GetTing())
	{
		playerCard->onPutMingGang(nCard);

		pInvoker->getPlayerCard()->onCardBeGangPengEat(nCard);

		// send msg 
		Json::Value msg;
		msg["idx"] = nIdx;
		msg["actType"] = eMJAct_MingGang;
		msg["card"] = nCard;
		//msg["gangCard"] = nGangGetCard;
		sendRoomMsg(msg, MSG_ROOM_ACT);

		playerCard->setSongGangIdx(nInvokeIdx);
		playerCard->addSongGangIdx(nCard, nInvokeIdx, eMJAct_MingGang);

		pPlayer->SetGangMo(true);

		Json::Value jsTran;
		jsTran["idx"] = nIdx;
		jsTran["act"] = eMJAct_Mo;
		goToState(eRoomState_DoPlayerAct, &jsTran);
	}
	else
	{
		LOGFMTD("++++++++++++++++++++++++++++++++++++ onPlayerMingGang");
		nGangGetCard = getMJPoker()->distributeOneCard();
		if (playerCard->onMingGang(nCard, nGangGetCard) == false)
		{
			LOGFMTE("nidx = %u ming gang card = %u error,", nIdx, nCard);
		}

		pInvoker->getPlayerCard()->onCardBeGangPengEat(nCard);

		// send msg 
		Json::Value msg;
		msg["idx"] = nIdx;
		msg["actType"] = eMJAct_MingGang;
		msg["card"] = nCard;
		msg["gangCard"] = nGangGetCard;
		sendRoomMsg(msg, MSG_ROOM_ACT);

		playerCard->setSongGangIdx(nInvokeIdx);
		playerCard->addSongGangIdx(nCard, nInvokeIdx, eMJAct_MingGang);
	}

	Json::Value jsFrameArg;
	auto ptrReplay = getGameReplay()->createFrame(eMJFrame_MingGang, (uint32_t)time(nullptr));
	jsFrameArg["idx"] = nIdx;
	jsFrameArg["gang"] = nCard;
	if (nGangGetCard != uint8_t(-1))
	{
		jsFrameArg["newCard"] = nGangGetCard;
	}
	jsFrameArg["invokerIdx"] = nInvokeIdx;
	ptrReplay->setFrameArg(jsFrameArg);
	getGameReplay()->addFrame(ptrReplay);
}

void CYMJRoom::onPlayerAnGang(uint8_t nIdx, uint8_t nCard)
{
	if (!m_isLouBao)
	{
		IMJRoom::onPlayerAnGang(nIdx, nCard);
		return;
	}

	auto pPlayer = (CYMJPlayer*)getMJPlayerByIdx(nIdx);
	if (!pPlayer)
	{
		LOGFMTE("why this player is null idx = %u , can not an gang", nIdx);
		return;
	}
	auto playerCard = (CYMJPlayerCard*)pPlayer->getPlayerCard();

	pPlayer->signGangFlagPao();
	pPlayer->signGangFlag();
	pPlayer->addAnGangCnt();

	auto eTingGangFlag = pPlayer->getTingGangType();
	if (eTingGangFlag == eMJAct_AnGang)
	{
		pPlayer->setTingGangType(eMJAct_Max);
	}

	uint8_t nGangGetCard = uint8_t(-1);

	//这里需要知道是不是搂宝杠
	if (pPlayer->GetTing())
	{
		playerCard->onPutAnGang(nCard);

		// send msg ;
		Json::Value msg;
		msg["idx"] = nIdx;
		msg["actType"] = eMJAct_AnGang;
		msg["card"] = nCard;
		//msg["gangCard"] = nGangGetCard;
		sendRoomMsg(msg, MSG_ROOM_ACT);

		if (eTingGangFlag != eMJAct_AnGang && (playerCard->getBaoCard() == uint8_t(-1) ||
			playerCard->getBaoCard() != m_nBaoCard || m_nBaoCard == uint8_t(-1) || m_bHuanBaoCardFlag))
		{
			if (m_nBaoCard == uint8_t(-1) || m_bHuanBaoCardFlag)
			{
				m_nBaoCard = getValidBaoCard();
				onPlayerMoBao(nIdx, m_nBaoCard);

				pPlayer->setTingGangType(eMJAct_AnGang);
				pPlayer->SetGangMo(true);

				Json::Value jsTrans;
				jsTrans["act"] = eMJAct_MoBaoDice;
				jsTrans["idx"] = nIdx;
				goToState(eRoomState_DoPlayerAct, &jsTrans);
				return;
			}
			else
			{
				Json::Value jsMsg;
				jsMsg["baoCard"] = m_nBaoCard;
				sendMsgToPlayer(jsMsg, MSG_BAO_CARD, pPlayer->getSessionID());
			}
			//检查宝牌是否能胡
			if (playerCard->canHuWitCard(m_nBaoCard) && m_nBaoCard != nCard)
			{
				playerCard->setKanBao(m_nBaoCard);

				pPlayer->setTingGangType(eMJAct_AnGang);

				Json::Value jsValue;
				jsValue["invokeIdx"] = nIdx;
				jsValue["card"] = m_nBaoCard;
				goToState(eRoomState_AskForHuAndPeng, &jsValue);
				return;
			}
			playerCard->setBaoCard(m_nBaoCard);
		}

		pPlayer->SetGangMo(true);

		Json::Value jsTran;
		jsTran["idx"] = nIdx;
		jsTran["act"] = eMJAct_Mo;
		goToState(eRoomState_DoPlayerAct, &jsTran);
	}
	else
	{
		LOGFMTD("++++++++++++++++++++++++++++++++++++ onPlayerAnGang");
		nGangGetCard = getMJPoker()->distributeOneCard();

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
		sendRoomMsg(msg, MSG_ROOM_ACT);
	}

	Json::Value jsFrameArg;
	auto ptrReplay = getGameReplay()->createFrame(eMJFrame_AnGang, (uint32_t)time(nullptr));
	jsFrameArg["idx"] = nIdx;
	jsFrameArg["gang"] = nCard;
	if (nGangGetCard != uint8_t(-1))
	{
		jsFrameArg["newCard"] = nGangGetCard;
	}
	ptrReplay->setFrameArg(jsFrameArg);
	getGameReplay()->addFrame(ptrReplay);
}

void CYMJRoom::onPlayerBuGang(uint8_t nIdx, uint8_t nCard)
{
	auto pPlayer = (CYMJPlayer*)getMJPlayerByIdx(nIdx);
	if (!pPlayer)
	{
		LOGFMTE("why this player is null idx = %u , can not bu gang", nIdx);
		return;
	}
	auto playerCard = (CYMJPlayerCard*)pPlayer->getPlayerCard();

	if (!m_isLouBao)
	{
		IMJRoom::onPlayerBuGang(nIdx, nCard);

		std::vector<CYMJPlayerCard::tSongGangIdx> vSongIdx = playerCard->getPengSongIdx();
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
		return;
	}

	pPlayer->signGangFlagPao();
	pPlayer->signGangFlag();
	pPlayer->clearDecareBuGangFlag();

	auto eTingGangFlag = pPlayer->getTingGangType();
	if (eTingGangFlag == eMJAct_BuGang)
	{
		pPlayer->setTingGangType(eMJAct_Max);
	}

	uint8_t nGangCard = uint8_t(-1);

	//这里需要知道是不是搂宝杠
	if (pPlayer->GetTing())
	{
		playerCard->onPutBuGang(nCard);

		pPlayer->addMingGangCnt();
		// send msg 
		Json::Value msg;
		msg["idx"] = nIdx;
		msg["actType"] = eMJAct_BuGang_Done;
		msg["card"] = nCard;
		//msg["gangCard"] = nGangCard;
		sendRoomMsg(msg, MSG_ROOM_ACT);

		std::vector<CYMJPlayerCard::tSongGangIdx> vSongIdx = playerCard->getPengSongIdx();
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

		if (eTingGangFlag != eMJAct_BuGang && (playerCard->getBaoCard() == uint8_t(-1) ||
			playerCard->getBaoCard() != m_nBaoCard || m_nBaoCard == uint8_t(-1) || m_bHuanBaoCardFlag))
		{
			//
			if (m_nBaoCard == uint8_t(-1) || m_bHuanBaoCardFlag)
			{
				m_nBaoCard = getValidBaoCard();
				onPlayerMoBao(nIdx, m_nBaoCard);

				pPlayer->setTingGangType(eMJAct_BuGang);
				pPlayer->SetGangMo(true);

				Json::Value jsTrans;
				jsTrans["act"] = eMJAct_MoBaoDice;
				jsTrans["idx"] = nIdx;
				goToState(eRoomState_DoPlayerAct, &jsTrans);
				return;
			}
			else
			{
				Json::Value jsMsg;
				jsMsg["baoCard"] = m_nBaoCard;
				sendMsgToPlayer(jsMsg, MSG_BAO_CARD, pPlayer->getSessionID());
			}
			//检查宝牌是否能胡
			if (playerCard->canHuWitCard(m_nBaoCard) && m_nBaoCard != nCard)
			{
				playerCard->setKanBao(m_nBaoCard);

				pPlayer->setTingGangType(eMJAct_BuGang);

				Json::Value jsValue;
				jsValue["invokeIdx"] = nIdx;
				jsValue["card"] = m_nBaoCard;
				goToState(eRoomState_AskForHuAndPeng, &jsValue);
				return;
			}
			playerCard->setBaoCard(m_nBaoCard);
		}

		pPlayer->SetGangMo(true);

		Json::Value jsTran;
		jsTran["idx"] = nIdx;
		jsTran["act"] = eMJAct_Mo;
		goToState(eRoomState_DoPlayerAct, &jsTran);
	}
	else
	{
		LOGFMTD("++++++++++++++++++++++++++++++++++++ onPlayerBuGang");
		nGangCard = getMJPoker()->distributeOneCard();

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
		sendRoomMsg(msg, MSG_ROOM_ACT);

		std::vector<CYMJPlayerCard::tSongGangIdx> vSongIdx = playerCard->getPengSongIdx();
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


	Json::Value jsFrameArg;
	auto ptrReplay = getGameReplay()->createFrame(eMJFrame_BuGang, (uint32_t)time(nullptr));
	jsFrameArg["idx"] = nIdx;
	jsFrameArg["gang"] = nCard;
	if (nGangCard != uint8_t(-1))
	{
		jsFrameArg["newCard"] = nGangCard;
	}
	ptrReplay->setFrameArg(jsFrameArg);
	getGameReplay()->addFrame(ptrReplay);
}

void CYMJRoom::onPlayerChu(uint8_t nIdx, uint8_t nCard)
{
	IMJRoom::onPlayerChu(nIdx, nCard);
	auto pActPlayer = getMJPlayerByIdx(nIdx);
	auto pActCard = (CYMJPlayerCard*)pActPlayer->getPlayerCard();
	pActCard->setSongGangIdx(-1); // reset song gang ;

	//清除杠标记（杠开）
	pActPlayer->clearGangFlag();
}

void CYMJRoom::onWaitPlayerAct(uint8_t nIdx, bool& isCanPass)
{
	auto pPlayer = (CYMJPlayer*)getMJPlayerByIdx(nIdx);
	if (!pPlayer)
	{
		LOGFMTE("player idx = %u is null can not tell it wait act", nIdx);
		return;
	}

	auto pMJCard = (CYMJPlayerCard*)pPlayer->getPlayerCard();
	
	// send msg to tell player do act 
	Json::Value jsArrayActs;
	Json::Value jsFrameActs;

	std::map<uint8_t, std::set<uint8_t>> tingCards;

	auto eTingGangFlag = pPlayer->getTingGangType();

	bool bTingedCanGang = false;
	if (pPlayer->GetTing())
	{
		if (eTingGangFlag == eMJAct_AnGang || eTingGangFlag == eMJAct_MingGang || eTingGangFlag == eMJAct_BuGang)
		{
			bTingedCanGang = true;
		}
	}

	bool bTingCheck = pPlayer->GetTing() && (pPlayer->checkTingedCanMingGang(pMJCard->getNewestFetchedCard())
		|| pPlayer->checkTingedCanAnGang(pMJCard->getNewestFetchedCard())
		|| pPlayer->checkTingedCanBuGang(pMJCard->getNewestFetchedCard())
		|| bTingedCanGang);

	if (isCanGoOnMoPai() && (pPlayer->GetTing() == false || bTingCheck))
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

		if (pPlayer->GetTing() == false && m_isLouBao)
		{
			checkTing(pMJCard, tingCards);

			if (tingCards.size() > 0)
			{
				Json::Value jsAct;
				jsAct["act"] = eMJAct_Ting;
				jsAct["cardNum"] = pMJCard->getNewestFetchedCard();
				jsArrayActs[jsArrayActs.size()] = jsAct;
				jsFrameActs[jsFrameActs.size()] = eMJAct_Ting;
			}
		}
	}

	// check hu . 如果是吃的只能听不能胡
	if (pMJCard->isHoldCardCanHu() && !m_isChiAndTing)
	{
		Json::Value jsAct;
		jsAct["act"] = eMJAct_Hu;
		jsAct["cardNum"] = pMJCard->getNewestFetchedCard();
		jsArrayActs[jsArrayActs.size()] = jsAct;
		jsFrameActs[jsFrameActs.size()] = eMJAct_Hu;
	}
	m_isChiAndTing = false;

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

	if (tingCards.size() > 0)
	{
		Json::Value jsCards;
		for (auto vCanHuCard : tingCards)
		{
			Json::Value jsTingCard;
			Json::Value jsHuCard;
			for (auto nHuCard : vCanHuCard.second)
			{
				jsHuCard[jsHuCard.size()] = nHuCard;
			}
			jsTingCard["tingCard"] = vCanHuCard.first;
			jsTingCard["huCard"] = jsHuCard;

			jsCards[jsCards.size()] = jsTingCard;
		}

		auto pPlayer = getMJPlayerByIdx(nIdx);
		Json::Value jsMsg;
		jsMsg["cards"] = jsCards;
		sendMsgToPlayer(jsMsg, MSG_TING_CARD, pPlayer->getSessionID());
	}

	//LOGFMTD("tell player idx = %u do act size = %u", nIdx, jsArrayActs.size());
}

void CYMJRoom::checkTing(CYMJPlayerCard* pMJCard, std::map<uint8_t, std::set<uint8_t>>& tingCards)
{
	tingCards.clear();

	//检查听牌，如果可以听，告诉玩家打掉的牌
	std::vector<uint8_t> holdCards, allCards;
	pMJCard->getHoldCard(holdCards);

	allCards.insert(allCards.end(), holdCards.begin(), holdCards.end());
	pMJCard->getPengedCard(allCards);
	pMJCard->getAnGangedCard(allCards);
	pMJCard->getMingGangedCard(allCards);
	pMJCard->getEatedCard(allCards);

	if (!pMJCard->check19(allCards))
	{
		return;
	}

	std::vector<uint8_t> checkCards;

	for (size_t i = 0; i < holdCards.size(); ++i)
	{
		uint8_t nCard = holdCards.at(i);

		if (checkCards.end() != std::find(begin(checkCards), end(checkCards), nCard))
		{
			continue;
		}
		pMJCard->pickoutHoldCard(nCard);
		auto it = std::find(allCards.begin(), allCards.end(), nCard);
		if (it != allCards.end())
		{
			allCards.erase(it);
		}

		if (pMJCard->isTingPai() && pMJCard->check19(allCards))
		{
			std::set<uint8_t> vCanHuCards;
			pMJCard->getCanHuCards(vCanHuCards);

			for (auto nCard_ : vCanHuCards)
			{
				if (pMJCard->canHuWitCard(nCard_))
				{
					tingCards[nCard] = vCanHuCards;
					break;
				}
			}
		}
		pMJCard->addDistributeCard(nCard);
		allCards.push_back(nCard);
		checkCards.push_back(nCard);
	}
}

bool CYMJRoom::isAnyPlayerPengOrHuThisCard(uint8_t nInvokeIdx, uint8_t nCard)
{
	bool isKanBao = false;
	for (auto& ref : m_vMJPlayers)
	{
		auto pMJCard = (CYMJPlayerCard*)ref->getPlayerCard();
		if (pMJCard->getKanBao() != uint8_t(-1))
		{
			isKanBao = true;
			break;
		}
	}

	for (auto& ref : m_vMJPlayers)
	{
		if (ref == nullptr || nInvokeIdx == ref->getIdx())
		{
			continue;
		}
		((CYMJPlayer*)ref)->clearGangFlagPao();
	}

	for (auto& ref : m_vMJPlayers)
	{
		if (ref == nullptr || nInvokeIdx == ref->getIdx())
		{
			continue;
		}

		auto pMJCard = (CYMJPlayerCard*)ref->getPlayerCard();
		if (pMJCard->canHuWitCard(nCard) && (pMJCard->getBaoCard() != nCard ||
			(pMJCard->getBaoCard() == nCard && pMJCard->getKanBao() == pMJCard->getBaoCard())))
		{
			return true;
		}

		if ((((CYMJPlayer*)ref)->GetTing() == false) && pMJCard->canPengWithCard(nCard) && pMJCard->checkMingPiao() > 0)
		{
			if (nCard == m_nBaoCard)
			{
				if (!isKanBao)
				{
					return true;
				}
			}
			else
			{
				return true;
			}
		}

		if (ref->getIdx() == (nInvokeIdx + 1) % getSeatCnt() && !((CYMJPlayer*)ref)->GetTing() && pMJCard->checkMingPiao() == 1)
		{
			uint8_t a = 0, b = 0;
			if ((((CYMJPlayer*)ref)->GetTing() == false) && pMJCard->canEatCard(nCard, a, b))
			{
				if (nCard == m_nBaoCard)
				{
					if (!isKanBao)
					{
						return true;
					}
				}
				else
				{
					return true;
				}
			}
		}

		if (pMJCard->canMingGangWithCard(nCard) && ((CYMJPlayer*)ref)->checkTingedCanMingGang(nCard) && pMJCard->checkMingPiao() > 0)
		{
			return true;
		}
	}

	return false;
}

void CYMJRoom::onAskForPengOrHuThisCard(uint8_t nInvokeIdx, uint8_t nCard, std::vector<uint8_t>& vWaitHuIdx, std::vector<uint8_t>& vWaitPengGangIdx, bool& isNeedWaitEat)
{
	Json::Value jsFrameArg;

	bool isKanBao = false;
	uint8_t nKanBaoIdx = uint8_t(-1);
	for (auto& ref : m_vMJPlayers)
	{
		auto pMJCard = (CYMJPlayerCard*)ref->getPlayerCard();
		if (pMJCard->getKanBao() != uint8_t(-1))
		{
			isKanBao = true;
			nKanBaoIdx = ref->getIdx();
			break;
		}
	}

	for (auto& ref : m_vMJPlayers)
	{
		if (ref == nullptr)
		{
			continue;
		}

		auto pMJCard = (CYMJPlayerCard*)ref->getPlayerCard();
		if (nInvokeIdx == ref->getIdx() && pMJCard->getKanBao() == uint8_t(-1))
		{
			continue;
		}

		Json::Value jsMsg;
		jsMsg["invokerIdx"] = nInvokeIdx;
		jsMsg["cardNum"] = nCard;

		Json::Value jsActs;

		// check peng 
		if (pMJCard->canPengWithCard(nCard) && !((CYMJPlayer*)ref)->GetTing() && pMJCard->checkMingPiao() > 0)
		{
			if (nCard == m_nBaoCard)
			{
				if (!isKanBao)
				{
					jsActs[jsActs.size()] = eMJAct_Peng;
					vWaitPengGangIdx.push_back(ref->getIdx());
				}
			}
			else
			{
				jsActs[jsActs.size()] = eMJAct_Peng;
				vWaitPengGangIdx.push_back(ref->getIdx());
			}
		}

		// check ming gang 
		if (isCanGoOnMoPai() && pMJCard->canMingGangWithCard(nCard) && 
			((CYMJPlayer*)ref)->checkTingedCanMingGang(nCard) && pMJCard->checkMingPiao() > 0)
		{
			jsActs[jsActs.size()] = eMJAct_MingGang;

			std::vector<uint8_t>::iterator it = std::find(vWaitPengGangIdx.begin(), vWaitPengGangIdx.end(), ref->getIdx());
			if (it == vWaitPengGangIdx.end())
			{
				vWaitPengGangIdx.push_back(ref->getIdx());
			}
			// already add in peng ;  vWaitPengGangIdx
		}

		if (ref->getIdx() == (nInvokeIdx + 1) % getSeatCnt() && !((CYMJPlayer*)ref)->GetTing() && pMJCard->checkMingPiao() == 1)
		{
			if (nCard == m_nBaoCard)
			{
				if (!isKanBao)
				{
					uint8_t a = 0, b = 0;
					isNeedWaitEat = false;
					if (pMJCard->canEatCard(nCard, a, b))
					{
						isNeedWaitEat = true;
						jsActs[jsActs.size()] = eMJAct_Chi;
					}
				}
			}
			else
			{
				uint8_t a = 0, b = 0;
				isNeedWaitEat = false;
				if (pMJCard->canEatCard(nCard, a, b))
				{
					isNeedWaitEat = true;
					jsActs[jsActs.size()] = eMJAct_Chi;
				}
			}
		}

		// check hu ;
		if (pMJCard->canHuWitCard(nCard) && ((pMJCard->getBaoCard() != nCard && !isKanBao) ||
			(isKanBao && nKanBaoIdx == nInvokeIdx && ref->getIdx() == nKanBaoIdx) ||
			(pMJCard->getBaoCard() == nCard && pMJCard->getKanBao() == pMJCard->getBaoCard())))
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

std::shared_ptr<IGameRoomRecorder> CYMJRoom::createRoomRecorder()
{
	return std::make_shared<CYMJRoomRecorder>();
}

uint8_t CYMJRoom::getValidBaoCard()
{
	//do
	//{
	LOGFMTD("++++++++++++++++++++++++++++++++++++ getValidBaoCard");
		auto nBaoCard = getMJPoker()->distributeOneCard();

		//bool isValid = isBaoCardValid(nBaoCard);
		//if (isValid)
		//{
			return nBaoCard;
	//	}
	//	//else
	//	//{
	//	//	getRoom()->getMJPoker()->pushCardToBack(nBaoCard);
	//	//}
	//} while (true);
}

bool CYMJRoom::isBaoCardValid(uint8_t nBaoCard)
{
	auto nCount_ = 0;
	for (auto i = 0; i < MAX_SEAT_CNT; ++i)
	{
		auto player_ = getMJPlayerByIdx(i);
		auto playerCard_ = (CYMJPlayerCard*)player_->getPlayerCard();
		std::vector<uint8_t> vTemp;
		playerCard_->getChuedCard(vTemp);
		nCount_ += std::count(vTemp.begin(), vTemp.end(), nBaoCard);
		vTemp.clear();
		playerCard_->getEatedCard(vTemp);
		nCount_ += std::count(vTemp.begin(), vTemp.end(), nBaoCard);
		vTemp.clear();
		playerCard_->getPengedCard(vTemp);
		nCount_ += std::count(vTemp.begin(), vTemp.end(), nBaoCard) * 3;
		vTemp.clear();
		playerCard_->getAnGangedCard(vTemp);
		nCount_ += std::count(vTemp.begin(), vTemp.end(), nBaoCard) * 4;
		vTemp.clear();
		playerCard_->getMingGangedCard(vTemp);
		nCount_ += std::count(vTemp.begin(), vTemp.end(), nBaoCard) * 4;
	}

	if (nCount_ < 3)
		return true;

	return false;
}
