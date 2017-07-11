#include "XLMJRoom.h"
#include "log4z.h"
#include "XLMJPlayer.h"
#include "RoomConfig.h"
#include "XLMJPlayerCard.h"
#include "XLRoomStateStartGame.h"
#include "XLRoomStateWaitDecideQue.h"
#include "MJRoomStateWaitPlayerAct.h"
#include "MJRoomStateWaitReady.h"
#include "MJRoomStateWaitPlayerChu.h"
#include "MJRoomStateWaitPlayerAct.h"
#include "MJRoomStateGameEnd.h"
#include "XLRoomStateDoPlayerAct.h"
#include "MJRoomStateAskForPengOrHu.h"
#include "IGameRoomManager.h"
#include "RobotDispatchStrategy.h"
#include "XLRoomStateWaitPlayerAct.h"
#include "XLRoomStateAskPengOrHu.h"
#include "XLRoomStateAskForRobotGang.h"
#include "ServerMessageDefine.h"
#define MAX_BEISHU 32
bool XLMJRoom::init(IGameRoomManager* pRoomMgr, stBaseRoomConfig* pConfig, uint32_t nSiealNum,uint32_t nRoomID, Json::Value& vJsValue)
{
	IMJRoom::init(pRoomMgr, pConfig, nSiealNum, nRoomID, vJsValue);
	m_tPoker.initAllCard(eMJ_BloodRiver);
	// create state and add state ;
	IMJRoomState* vState[] = {
		new CMJRoomStateWaitReady(), new MJRoomStateWaitPlayerChu(), new XLRoomStateWaitPlayerAct(), new XLRoomStateStartGame()
		, new MJRoomStateGameEnd(), new XLRoomStateDoPlayerAct(), new XLRoomStateAskForPengOrHu(), new XLRoomStateAskForRobotGang(),new XLRoomStateWaitDecideQue()
	};
	for (uint8_t nIdx = 0; nIdx < sizeof(vState) / sizeof(IMJRoomState*); ++nIdx)
	{
		addRoomState(vState[nIdx]);
	}
	setInitState(vState[0]);

	// init banker
	m_nBankerIdx = getSeatCnt() - 1;
	return true;
}

bool XLMJRoom::onPlayerApplyLeave(uint32_t nPlayerUID)
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
	if (eRoomSate_WaitReady == curState || eRoomState_GameEnd == curState )
	{
		// direct leave just stand up ;
		auto pXLPlayer = (XLMJPlayer*)pPlayer;
		stMsgSvrDoLeaveRoom msgdoLeave;
		msgdoLeave.nCoin = pPlayer->getCoin();
		msgdoLeave.nGameType = getRoomType();
		msgdoLeave.nRoomID = getRoomID();
		msgdoLeave.nUserUID = pPlayer->getUID();
		msgdoLeave.nGameOffset = pPlayer->getOffsetCoin();
		getRoomMgr()->sendMsg(&msgdoLeave, sizeof(msgdoLeave), pPlayer->getSessionID());
		LOGFMTD("player uid = %u , leave room id = %u",pPlayer->getUID(),getRoomID());

		if (eRoomSate_WaitReady == curState || eRoomState_GameEnd == curState)  // when game over or not start , delte player in room data ;
		{
			// tell robot dispatch player leave 
			auto ret = standup(nPlayerUID);
			return ret;
		}
		else 
		{
			LOGFMTE("decide player already sync data uid = %u room id = %u" , pPlayer->getUID(),getRoomID());
		}
	}
	pPlayer->doTempLeaveRoom();
	onPlayerTrusteedStateChange(pPlayer->getIdx(), true);
	return true;
}

uint8_t XLMJRoom::checkPlayerCanEnter(stEnterRoomData* pEnterRoomPlayer)
{
	auto nRet = IMJRoom::checkPlayerCanEnter(pEnterRoomPlayer);
	if (nRet)
	{
		return nRet;
	}

	auto pPlayer = getMJPlayerByUID(pEnterRoomPlayer->nUserUID);
	if (pPlayer)
	{
		LOGFMTE("you are already in room by let you in again uid = %u", pEnterRoomPlayer->nUserUID);
		//return 8;
	}
	return 0;
}

//void XLMJRoom::onWaitPlayerAct(uint8_t nIdx, bool& isCanPass)
//{
//	auto pPlayer = getMJPlayerByIdx(nIdx);
//	if (!pPlayer->haveState(eRoomPeer_AlreadyHu))
//	{
//		IMJRoom::onWaitPlayerAct(nIdx,isCanPass);
//		return;
//	}
//
//	// can only hu , or bu gang ;  when already hu ;
//	auto pMJCard = pPlayer->getPlayerCard();
//	// send msg to tell player do act 
//	Json::Value jsArrayActs;
//
//	if (isCanGoOnMoPai())
//	{
//		// check bu gang .
//		IMJPlayerCard::VEC_CARD vCards;
//		pMJCard->getHoldCardThatCanBuGang(vCards);
//		for (auto& ref : vCards)
//		{
//			if (ref != pMJCard->getNewestFetchedCard())
//			{
//				LOGFMTD("room id = %u have bu gang card = %u , but not nest get card = %u", getRoomID(), ref, pMJCard->getNewestFetchedCard());
//				continue;
//			}
//			Json::Value jsAct;
//			jsAct["act"] = eMJAct_BuGang;
//			jsAct["cardNum"] = ref;
//			jsArrayActs[jsArrayActs.size()] = jsAct;
//			LOGFMTD("room id =%u player already hu , tell client bu gang card = %u ",getRoomID(),ref);
//		}
//	}
//
//	// check hu .
//	if (pMJCard->isHoldCardCanHu())
//	{
//		Json::Value jsAct;
//		jsAct["act"] = eMJAct_Hu;
//		jsAct["cardNum"] = pMJCard->getNewestFetchedCard();
//		jsArrayActs[jsArrayActs.size()] = jsAct;
//	}
//
//	isCanPass = jsArrayActs.empty() == false;
//
//	// add default alwasy chu , infact need not add , becaust it alwasy in ,but compatable with current client ;
//	Json::Value jsAct;
//	jsAct["act"] = eMJAct_Chu;
//	jsAct["cardNum"] = getAutoChuCardWhenWaitActTimeout(nIdx);
//	jsArrayActs[jsArrayActs.size()] = jsAct;
//
//	Json::Value jsMsg;
//	jsMsg["acts"] = jsArrayActs;
//	sendMsgToPlayer(jsMsg, MSG_PLAYER_WAIT_ACT_AFTER_RECEIVED_CARD, pPlayer->getSessionID());
//
//	LOGFMTD("already hu , tell player idx = %u do act size = %u", nIdx, jsArrayActs.size());
//}

void XLMJRoom::onPlayerMingGang(uint8_t nIdx, uint8_t nCard, uint8_t nInvokeIdx)
{
	IMJRoom::onPlayerMingGang(nIdx,nCard,nInvokeIdx);
	// do settle 
	auto nSettleCoin = getBaseBet() * 2 ;
	auto pInvoker = getMJPlayerByIdx(nInvokeIdx);
	auto pGanger = getMJPlayerByIdx(nIdx);
	if (pInvoker->getCoin() <= (int32_t)nSettleCoin)
	{
		nSettleCoin = pInvoker->getCoin();
	}
	pInvoker->addOffsetCoin(-1 * (int32_t)nSettleCoin);
	pGanger->addOffsetCoin(nSettleCoin);
	auto pSettle = new stSettleMingGang(nInvokeIdx, nIdx, nSettleCoin);
	addSettle(pSettle);
	LOGFMTD("room id = %u , idx = %u win coin = %u  from idx = %u MingGang",getRoomID(),nIdx,nSettleCoin,nInvokeIdx);
}

void XLMJRoom::onPlayerAnGang(uint8_t nIdx, uint8_t nCard)
{
	IMJRoom::onPlayerAnGang(nIdx, nCard);
	// do settle 
	auto nSettleCoin = getBaseBet() * 2;
	auto pGanger = getMJPlayerByIdx(nIdx);
	auto pSettle = new stSettleAnGang(nIdx);
	for (auto& pPlayer : m_vMJPlayers)
	{
		if (nullptr == pPlayer || pPlayer->getIdx() == nIdx || canKouPlayerCoin(pPlayer->getIdx()) == false  )
		{
			continue;
		}

		auto nSeCoin = nSettleCoin;
		if ((int32_t)nSeCoin > pPlayer->getCoin())
		{
			nSeCoin = pPlayer->getCoin();
		}
		pPlayer->addOffsetCoin(-1 * (int32_t)nSeCoin);
		pGanger->addOffsetCoin(nSeCoin);
		pSettle->addLosePlayer(pPlayer->getIdx(), nSeCoin);
	}
	addSettle(pSettle);
	LOGFMTD("room id = %u , idx = %u win coin = %u  final = %u anGang", getRoomID(), nIdx, pSettle->getWinCoin(), pGanger->getCoin());
}

void XLMJRoom::onPlayerBuGang(uint8_t nIdx, uint8_t nCard)
{
	IMJRoom::onPlayerBuGang(nIdx, nCard);
	// do settle 
	auto nSettleCoin = getBaseBet();
	auto pGanger = getMJPlayerByIdx(nIdx);
	auto pSettle = new stSettleBuGang(nIdx);
	for (auto& pPlayer : m_vMJPlayers)
	{
		if (nullptr == pPlayer || pPlayer->getIdx() == nIdx || canKouPlayerCoin(pPlayer->getIdx()) == false)
		{
			continue;
		}

		auto nSeCoin = nSettleCoin;
		if ((int32_t)nSeCoin > pPlayer->getCoin())
		{
			nSeCoin = pPlayer->getCoin();
		}
		pPlayer->addOffsetCoin(-1 * (int32_t)nSeCoin);
		pGanger->addOffsetCoin(nSeCoin);
		pSettle->addLosePlayer(pPlayer->getIdx(), nSeCoin);
	}
	addSettle(pSettle);
	LOGFMTD("room id = %u , idx = %u win coin = %u  final = %u BuGang", getRoomID(), nIdx, pSettle->getWinCoin(), pGanger->getCoin());
}

void XLMJRoom::onPlayerHu(std::vector<uint8_t>& vHuIdx, uint8_t nCard, uint8_t nInvokeIdx)
{
	if (vHuIdx.empty())
	{
		LOGFMTE("why hu , but not hu players ? room id = %u",getRoomID());
		return;
	}

	if (vHuIdx.size() == 1 && vHuIdx.front() == nInvokeIdx)
	{
		onPlayerZiMo(nInvokeIdx,nCard);
		return;
	}

	auto pDianPaoPlayer = getMJPlayerByIdx(nInvokeIdx);
	bool isRobotGang = pDianPaoPlayer->haveDecareBuGangFalg();
	bool isGangShangPao = pDianPaoPlayer->haveGangFalg();
	auto pMJCard = pDianPaoPlayer->getPlayerCard();
	if (isRobotGang)
	{
		pMJCard->onGangCardBeRobot(nCard);
	}

	if (pDianPaoPlayer == nullptr)
	{  
		LOGFMTE("room id = %u why dian pao player is nullptr ? ", getRoomID() );
		return;
	}

	auto pSettle = new stSettleDiaoPao(nInvokeIdx, isRobotGang, isGangShangPao);
	LOGFMTD("room id = %u add dian pao settle uid = %u , hu size = %u",getRoomID(),pDianPaoPlayer->getUID(),vHuIdx.size());
	for (auto& nidx : vHuIdx)
	{
		auto pHuPlayer = (XLMJPlayer*)getMJPlayerByIdx(nidx);
		if (pHuPlayer == nullptr)
		{
			LOGFMTE("why this hu player is null ? idx = %u",nidx);
			continue;
		}

		auto pHuCard = (XLMJPlayerCard*)pHuPlayer->getPlayerCard();

		uint32_t nHuType = 0; uint8_t nBeiShu = 0 , nGenCnt = 0 ;
		if (pHuCard->onDoHu(false,nCard, nHuType, nBeiShu, nGenCnt) == false)
		{
			LOGFMTE("room id = %u hu card return false when hu do hu , idx = %u ,card = %u",getRoomID(),nidx,nCard);
			continue;
		}
		pHuPlayer->setState(eRoomPeer_AlreadyHu);

		// do caculate coin ;
		if (isRobotGang || isGangShangPao)  // qiang gang he gang shang pao dou jia yi fan ;
		{
			nBeiShu *= 2;  //  yi fan , means * 2 ;
		}
	
		for (uint8_t nGenF = 0; nGenF < nGenCnt; ++nGenF)
		{
			nBeiShu *= 2; // add gen cnt fan;
		}

		if (nBeiShu > MAX_BEISHU)
		{
			nBeiShu = MAX_BEISHU;
		}

		uint32_t nSettleCoin = getBaseBet() * nBeiShu;
		if ((int32_t)nSettleCoin > pDianPaoPlayer->getCoin())
		{
			nSettleCoin = pDianPaoPlayer->getCoin();
		}

		pHuPlayer->addOffsetCoin(nSettleCoin);
		pDianPaoPlayer->addOffsetCoin(-1 * (int32_t)nSettleCoin);

		pHuPlayer->updateFanXingAndFanShu(nHuType,nBeiShu);

		// add to settle info ;
		pSettle->addHuPlayer(nidx, nSettleCoin, nHuType, nBeiShu);

		// send hu act msg to client ;
		Json::Value msg;
		msg["idx"] = pHuPlayer->getIdx();
		msg["actType"] = eMJAct_Hu;
		msg["card"] = nCard;
		msg["huType"] = nHuType;
		msg["fanShu"] = nBeiShu;
		sendRoomMsg(msg, MSG_ROOM_ACT);

		LOGFMTD("room id = %u uid = %u hu dianpao uid = %u  beishu = %u , coin = %u huType = %u gen = %u", getRoomID(), pHuPlayer->getUID(), pDianPaoPlayer->getUID(), nBeiShu, nSettleCoin, nHuType, nGenCnt);
		// if no coin will not go settle 
		if (pDianPaoPlayer->getCoin() <= 0)
		{
			LOGFMTI("DianPao Player have not coin , will not go in settle coin room id = %u , dianPao idx = %u",getRoomID(),nInvokeIdx);
			break;
		}
	}
	addSettle(pSettle);
}

void XLMJRoom::onPlayerZiMo(uint8_t nPlayerIdx, uint8_t nCard)
{
	auto pZiMoPlayer = (XLMJPlayer*)getMJPlayerByIdx(nPlayerIdx);
	auto pHuCard = (XLMJPlayerCard*)pZiMoPlayer->getPlayerCard();
	bool isGangShangHua = pZiMoPlayer->haveGangFalg();

	// do hu ;
	uint32_t nHuType = 0; uint8_t nBeiShu = 0, nGenCnt = 0;
	if (pHuCard->onDoHu(true, nCard, nHuType, nBeiShu, nGenCnt) == false)
	{
		LOGFMTE("hu card return false when hu do hu , idx = %u ,card = %u", nPlayerIdx, nCard);
		return;
	}
	pZiMoPlayer->setState(eRoomPeer_AlreadyHu);
	if (isGangShangHua)
	{
		nBeiShu *= 2; // jia yi fan 
	}

	for (uint8_t nIdx = 0; nIdx < nGenCnt; ++nIdx)
	{
		nBeiShu *= 2; // jia yi fan , mei yi ge gen 
	}

	nBeiShu *= 2; // jia yi fan , yin wei zi mo ;
	
	if (nBeiShu > MAX_BEISHU)
	{
		nBeiShu = MAX_BEISHU;
	}

	pZiMoPlayer->updateFanXingAndFanShu(nHuType, nBeiShu);
	
	auto pSettle = new stSettleZiMo(nPlayerIdx, nHuType, nBeiShu, isGangShangHua);
	auto nNeedCoinPerPlayer = getBaseBet() * nBeiShu ; 
	LOGFMTD("room id = %u add zi mo settle  uid = %u  bei shu = %u ",getRoomID(),pZiMoPlayer->getUID(),nBeiShu);
	// do caculate coin 
	for (auto& pLoser : m_vMJPlayers)
	{
		if (pLoser == nullptr || pLoser->getIdx() == nPlayerIdx || canKouPlayerCoin(pLoser->getIdx()) == false)
		{
			continue;
		}

		auto nSettleCoin = nNeedCoinPerPlayer;
		if ((int32_t)nSettleCoin > pLoser->getCoin())
		{
			nSettleCoin = pLoser->getCoin();
		}

		pZiMoPlayer->addOffsetCoin(nSettleCoin);   
		pLoser->addOffsetCoin(-1 * (int32_t)nSettleCoin);

		pSettle->addLosePlayer(pLoser->getIdx(), nSettleCoin);
		LOGFMTD("room id = %u , uid = %u zi mo , win uid = %u , offset = %u" ,getRoomID(),pZiMoPlayer->getUID(),pLoser->getUID(),nSettleCoin);
	}

	// send hu act msg to client ;
	Json::Value msg;
	msg["idx"] = pZiMoPlayer->getIdx();
	msg["actType"] = eMJAct_Hu;
	msg["card"] = nCard;
	msg["huType"] = nHuType;
	msg["fanShu"] = nBeiShu;
	sendRoomMsg(msg, MSG_ROOM_ACT);

	addSettle(pSettle);
}

void XLMJRoom::sendPlayersCardInfo(uint32_t nSessionID)
{
	if (getCurRoomState()->getStateID() == eRoomSate_WaitReady || eRoomState_GameEnd == getCurRoomState()->getStateID())
	{
		LOGFMTD("current room not start game , so need not send runtime info msg");
		return;
	}
	Json::Value jsmsg;
	Json::Value vPeerCards;
	for (auto& pp : m_vMJPlayers)
	{
		if (pp == nullptr /*|| pp->haveState(eRoomPeer_CanAct) == false*/)  // lose also have card 
		{
			continue;
		}

		auto pCard = (XLMJPlayerCard*)pp->getPlayerCard();
		// svr : { bankerIdx : 2, leftCardCnt : 32 ,playersCard: [ { idx : 2,queType: 2, anPai : [2,3,4,34], mingPai : [ 23,67,32] , huPai : [1,34], chuPai: [2,34,4] },{ anPai : [2,3,4,34], mingPai : [ 23,67,32] , huPai : [1,34] }, .... ] }
		// { idx : 2,queType: 2, anPai : [2,3,4,34], mingPai : [ 23,67,32] , huPai : [1,34], chuPai: [2,34,4] }
		Json::Value jsCardInfo;
		jsCardInfo["idx"] = pp->getIdx();
		jsCardInfo["queType"] = pCard->getQueType();

		IMJPlayerCard::VEC_CARD vAnPai, vMingPai, vChuPai, vHuPai, vAnGangedCard, temp;
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

		// get hued 
		pCard->getHuedCard(vHuPai);
		auto toJs = [](IMJPlayerCard::VEC_CARD& vCards, Json::Value& js)
		{
			for (auto& c : vCards)
			{
				js[js.size()] = c;
			}
		};

		Json::Value jsMingPai, jsAnPai, jsChuPai, jsHupai, jsAngangedPai;
		toJs(vMingPai, jsMingPai); toJs(vAnPai, jsAnPai); toJs(vChuPai, jsChuPai); toJs(vHuPai, jsHupai); toJs(vAnGangedCard, jsAngangedPai);
		jsCardInfo["mingPai"] = jsMingPai; jsCardInfo["anPai"] = jsAnPai; jsCardInfo["chuPai"] = jsChuPai; jsCardInfo["huPai"] = jsHupai;
		jsCardInfo["anGangPai"] = jsAngangedPai;
		//vPeerCards[vPeerCards.size()] = jsCardInfo;
		sendMsgToPlayer(jsCardInfo, MSG_ROOM_PLAYER_CARD_INFO, nSessionID);
		LOGFMTD("send player card infos !");
	}
}

bool XLMJRoom::isAnyPlayerPengOrHuThisCard(uint8_t nInvokeIdx, uint8_t nCard)
{
	for (auto& ref : m_vMJPlayers)
	{
		if (ref == nullptr || nInvokeIdx == ref->getIdx() )
		{
			continue;
		}

		auto pMJCard = ref->getPlayerCard();
		if ((ref->haveState(eRoomPeer_AlreadyHu) == false) && pMJCard->canPengWithCard(nCard))
		{
			return true;
		}
		
		if (pMJCard->canHuWitCard(nCard))
		{
			return true;
		}
	}

	return false;
}

bool XLMJRoom::isAnyPlayerRobotGang(uint8_t nInvokeIdx, uint8_t nCard)
{
	for (auto& ref : m_vMJPlayers)
	{
		if (ref == nullptr || nInvokeIdx == ref->getIdx() )
		{
			continue;
		}

		auto pMJCard = ref->getPlayerCard();
		if (pMJCard->canHuWitCard(nCard))
		{
			return true;
		}
	}

	return false;
}

void XLMJRoom::onAskForPengOrHuThisCard(uint8_t nInvokeIdx, uint8_t nCard, std::vector<uint8_t>& vOutWaitHuIdx, std::vector<uint8_t>& vOutWaitPengGangIdx, bool& isNeedWaitEat)
{
	isNeedWaitEat = false;
	for (auto& ref : m_vMJPlayers)
	{
		if (ref == nullptr || nInvokeIdx == ref->getIdx() )
		{
			continue;
		}

		Json::Value jsMsg;
		jsMsg["invokerIdx"] = nInvokeIdx;
		jsMsg["cardNum"] = nCard;

		Json::Value jsActs;
		auto pMJCard = ref->getPlayerCard();

		// check hu ;
		if (pMJCard->canHuWitCard(nCard))
		{
			jsActs[jsActs.size()] = eMJAct_Hu;
			vOutWaitHuIdx.push_back(ref->getIdx());
		}

		if (ref->haveState(eRoomPeer_AlreadyHu) == false)  // already hu player can not peng ,gang 
		{
			// check peng 
			if (pMJCard->canPengWithCard(nCard))
			{
				jsActs[jsActs.size()] = eMJAct_Peng;
				vOutWaitPengGangIdx.push_back(ref->getIdx());
			}

			// check ming gang 
			if (isCanGoOnMoPai() && pMJCard->canMingGangWithCard(nCard))
			{
				jsActs[jsActs.size()] = eMJAct_MingGang;
				// already add in peng ;  vWaitPengGangIdx
			}
		}

		if (jsActs.size() > 0)
		{
			jsActs[jsActs.size()] = eMJAct_Pass;
		}

		jsMsg["acts"] = jsActs;
		sendMsgToPlayer(jsMsg, MSG_PLAYER_WAIT_ACT_ABOUT_OTHER_CARD, ref->getSessionID());
		LOGFMTD("inform uid = %u act about other card room id = %u card = %u", ref->getUID(), getRoomID(), nCard);
	}
}

void XLMJRoom::onAskForRobotGang(uint8_t nInvokeIdx, uint8_t nCard, std::vector<uint8_t>& vOutCandinates)
{
	// send decalre gang msg ;
	Json::Value msg;
	msg["idx"] = nInvokeIdx;
	msg["actType"] = eMJAct_BuGang_Pre;
	msg["card"] = nCard;
	sendRoomMsg(msg, MSG_ROOM_ACT);

	// inform target player do this things 
	for (auto& ref : m_vMJPlayers)
	{
		if (ref == nullptr || nInvokeIdx == ref->getIdx() )
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
			vOutCandinates.push_back(ref->getIdx());
		}

		if (jsActs.size() > 0)
		{
			jsActs[jsActs.size()] = eMJAct_Pass;
		}

		jsMsg["acts"] = jsActs;
		sendMsgToPlayer(jsMsg, MSG_PLAYER_WAIT_ACT_ABOUT_OTHER_CARD, ref->getSessionID());
		LOGFMTD("inform uid = %u robot gang card = %u room id = %u ", ref->getUID(), nCard, getRoomID());
	}
}

IMJPlayer* XLMJRoom::doCreateMJPlayer()
{
	return new XLMJPlayer();
}

IMJPoker* XLMJRoom::getMJPoker()
{
	return &m_tPoker;
}

uint8_t XLMJRoom::getNextActPlayerIdx(uint8_t nCurActIdx)
{
	for (uint8_t nIdx = nCurActIdx + 1; nIdx < getSeatCnt() * 2; ++nIdx)
	{
		auto nActIdx = nIdx % getSeatCnt();
		auto p = getMJPlayerByIdx(nActIdx);
		if (p )
		{
			return nActIdx;
		}
	}

	LOGFMTE("why can not find a can do act player ? ");
	return 0;
}

void XLMJRoom::addSettle(ISettle* pSettle)
{
	m_vSettleInfos.push_back(pSettle);
	// send msg to client ;
	uint16_t nMsgID = 0;
	switch (pSettle->eType )
	{
	case eSettle_DianPao:
	{
		nMsgID = MSG_ROOM_SETTLE_DIAN_PAO;
	}
	break;
	case eSettle_MingGang:
	{
		nMsgID = MSG_ROOM_SETTLE_MING_GANG;
	}
	break;
	case eSettle_AnGang:
	{
		nMsgID = MSG_ROOM_SETTLE_AN_GANG;
	}
	break;
	case eSettle_BuGang:
	{
		nMsgID = MSG_ROOM_SETTLE_BU_GANG;
	}
	break;
	case eSettle_ZiMo:
	{
		nMsgID = MSG_ROOM_SETTLE_ZI_MO;
	}
	break;
	default:
	{
		LOGFMTD("this type of settle will not real_time seetle type = %u",pSettle->eType );
	}
	return;
	}

	Json::Value jsInfo;
	pSettle->writeSettleInfo(jsInfo);
	sendRoomMsg(jsInfo, nMsgID);
	LOGFMTD("send real time settle msg id = %u , room id = %u",nMsgID,getRoomID());
}

void XLMJRoom::removeSettle(ISettle* pSettle)
{
	auto iter = m_vSettleInfos.begin();
	for (; iter != m_vSettleInfos.end(); ++iter)
	{
		if ((*iter) == pSettle)
		{
			delete pSettle;
			m_vSettleInfos.erase(iter);
			return;
		}
	}
	LOGFMTE("can not find settle to remove");
}

void XLMJRoom::clearAllSettle()
{
	for (auto& ref : m_vSettleInfos)
	{
		delete ref;
		ref = nullptr;
	}
	m_vSettleInfos.clear();
}

void XLMJRoom::willStartGame()
{
	IMJRoom::willStartGame();
	clearAllSettle();
	// rand banker , ervery round ;
	//m_nBankerIdx = rand() % getSeatCnt();
	++m_nBankerIdx;
	m_nBankerIdx = m_nBankerIdx % getSeatCnt();
}

void XLMJRoom::startGame()
{
	IMJRoom::startGame();
	Json::Value jsMsg;
	packStartGameMsg(jsMsg);
	sendRoomMsg(jsMsg, MSG_ROOM_START_GAME);
}

void XLMJRoom::onGameEnd()
{
	// cha hua zhu when liu ju (more then two player not hu , include 2 );
	// check liu ju 
	//uint8_t nNotHuPlayer = 0;
	//for (auto& ref : m_vMJPlayers)
	//{
	//	if (ref && (ref->haveState(eRoomPeer_AlreadyHu) == false) && ( ref->haveState(eRoomPeer_DecideLose) == false ) )
	//	{
	//		++nNotHuPlayer;
	//	}
	//}

	//std::vector<uint8_t> vecHuaZhu;
	//if ( nNotHuPlayer >= 2)
	//{
		// cha hua zhu 
		//doChaHuaZhu(vecHuaZhu);
		// cha da jiao ;  // when da jiao  give back , gang win , all type of gang ;
		//doChaDaJiao(vecHuaZhu);
	//}
	std::vector<uint8_t> vecHuaZhu;
	// cha hua zhu 
	doChaHuaZhu(vecHuaZhu);
	// cha da jiao ;  // when da jiao  give back , gang win , all type of gang ;
	doChaDaJiao(vecHuaZhu);

	// send game over msg ;
	LOGFMTI("GAME OVER : ");
	Json::Value msg;
	Json::Value msgArray;
	for (uint8_t nIdx = 0; nIdx < getSeatCnt(); ++nIdx)
	{
		auto pPlayer = getMJPlayerByIdx(nIdx);
		if (pPlayer == nullptr)
		{
			LOGFMTE("send result but you leaved room player is null idx =%u",nIdx );
			continue;
		}

		Json::Value info;
		info["idx"] = nIdx;
		info["coin"] = pPlayer->getCoin();
		info["huType"] = 0;
		info["offset"] = pPlayer->getOffsetCoin();
		msgArray[(uint32_t)nIdx] = info;
		LOGFMTI("GAME OVER : UID = %u , offset = %d, coin = %u", pPlayer->getUID(), pPlayer->getOffsetCoin(), pPlayer->getCoin());
		sendPlayerDetailBillInfo(nIdx);
	}
	msg["players"] = msgArray;
	sendRoomMsg(msg, MSG_ROOM_GAME_OVER);

	IMJRoom::onGameEnd();
}

void XLMJRoom::sendPlayerDetailBillInfo(uint8_t nIdx)
{
	auto pPlayer = getMJPlayerByIdx(nIdx);
	if (pPlayer == nullptr)
	{
		LOGFMTE("send detail bill info but you leaved room player is null idx =%u", nIdx);
		return;
	}

	Json::Value jsBills;
	for (auto& pSettle : m_vSettleInfos)
	{
		Json::Value jsInfo;
		if ( pSettle->writePlayerBillInfo(nIdx, jsInfo))
		{
			Json::StyledWriter jsW;
			auto str = jsW.write(jsInfo);
			LOGFMTI("bill info idx = %u type = %u : %s ",nIdx,pSettle->eType,str.c_str());
			jsBills[jsBills.size()] = jsInfo;
		}
	}

	Json::Value jsMsg;
	jsMsg["idx"] = nIdx;
	jsMsg["bills"] = jsBills;
	sendMsgToPlayer(jsMsg, MSG_PLAYER_DETAIL_BILL_INFOS, pPlayer->getSessionID());
}

void XLMJRoom::doChaHuaZhu(std::vector<uint8_t>& vHuaZhu)
{
	// pick out hua zhu ;
	std::vector<uint8_t> vNotHuaZhu;
	for (auto& pHuZhu : m_vMJPlayers)
	{
		if (pHuZhu == nullptr )
		{
			continue;
		}

		auto pPlayerCard = (XLMJPlayerCard*)pHuZhu->getPlayerCard();
		if (pPlayerCard->isHuaZhu())
		{
			vHuaZhu.push_back(pHuZhu->getIdx());
		}
		else
		{
			vNotHuaZhu.push_back(pHuZhu->getIdx());
		}
	}

	if (vHuaZhu.empty() || vNotHuaZhu.empty() )
	{
		LOGFMTD("HuaZhu or NotHuaZhu is empty room id = %u", getRoomID());
		return;
	}
	// hua zhu give coin to not hua zhu player ;
	uint8_t nBeiShu = MAX_BEISHU;
	uint32_t nHuaZhuBaseLose = getBaseBet() * nBeiShu;
	for (auto& nHuaZhuIdx : vHuaZhu)
	{
		auto pHuaZhu = getMJPlayerByIdx(nHuaZhuIdx);
		if (pHuaZhu->getCoin() == 0)
		{
			continue;
		}

		LOGFMTD("room id = %u cha hua zhu uid = %u ",getRoomID(),pHuaZhu->getUID());
		auto pSettle = new stSettleHuaZhu(nHuaZhuIdx);
		// give coin to None HuaZhu player ;
		for (auto& notHuZhu : vNotHuaZhu)
		{
			auto pNotHuaZhu = getMJPlayerByIdx(notHuZhu);
			uint32_t nLoseCoin = nHuaZhuBaseLose;
			if ((int32_t)nLoseCoin > pHuaZhu->getCoin())
			{
				nLoseCoin = pHuaZhu->getCoin();
			}

			pHuaZhu->addOffsetCoin((int32_t)nLoseCoin * -1 );
			pNotHuaZhu->addOffsetCoin(nLoseCoin);

			pSettle->addHuPlayer(notHuZhu, nLoseCoin, 0, nBeiShu);
			LOGFMTD("room id = %u , huaZhu uid = %u , not huaZhu = %u , coin = %u",getRoomID(),pHuaZhu->getUID(),pNotHuaZhu->getUID(),nLoseCoin);
			if (pHuaZhu->getCoin() == 0)
			{
				LOGFMTD("huazhu idx = %u lose all room id = %u",nHuaZhuIdx,getRoomID());
				break;
			}
		}
		addSettle(pSettle);
	}
}

void XLMJRoom::doChaDaJiao(std::vector<uint8_t>& vHuaZhu)
{
	std::vector<uint8_t> vDaJiao, vTingPai;
	for (auto& pCheckPlayer : m_vMJPlayers)
	{
		if (pCheckPlayer == nullptr || pCheckPlayer->haveState(eRoomPeer_AlreadyHu))
		{
			continue;
		}

		// omit hua zhu ;
		auto iter = std::find(vHuaZhu.begin(), vHuaZhu.end(), pCheckPlayer->getIdx());
		if (iter != vHuaZhu.end())
		{
			continue;
		}

		auto pPlayerCard = (XLMJPlayerCard*)pCheckPlayer->getPlayerCard();
		if (pPlayerCard->isTingPai())
		{
			vTingPai.push_back(pCheckPlayer->getIdx());
		}
		else
		{
			vDaJiao.push_back(pCheckPlayer->getIdx());
		}
	}

	if (vDaJiao.empty() || vTingPai.empty())
	{
		LOGFMTD("da jiao or ting is empty room id = %u",getRoomID() );
		return;
	}

	// give back gang win coin 
	for (auto& nDaJiaoIdx : vDaJiao)
	{
		auto pPlayerDaJiao = getMJPlayerByIdx(nDaJiaoIdx);
		if (pPlayerDaJiao->getCoin() == 0)
		{
			LOGFMTD("da jiao idx = %u do not have coin room id = %u", nDaJiaoIdx, getRoomID());
			continue;
		}
		giveBackGangWin(nDaJiaoIdx);
	}

	// give coin to ting pai 
	for (auto& nDaJiaoIdx : vDaJiao)
	{
		auto pPlayerDaJiao = getMJPlayerByIdx(nDaJiaoIdx);
		if (pPlayerDaJiao->getCoin() == 0)
		{
			LOGFMTD("da jiao idx = %u do not have coin room id = %u",nDaJiaoIdx,getRoomID());
			continue;
		}

		LOGFMTD("room id = %u cha da jiao = %u ", getRoomID(),pPlayerDaJiao->getUID());
		auto pSettle = new stSettleDaJiao(nDaJiaoIdx);
		// give coin to None HuaZhu player ;
		for (auto& nTingIdx : vTingPai )
		{
			auto pTingPaiPlayer = getMJPlayerByIdx(nTingIdx);
			auto pTingPlayerCard = (XLMJPlayerCard*)pTingPaiPlayer->getPlayerCard();
			uint32_t nLoseCoin = min(MAX_BEISHU,pTingPlayerCard->getMaxPossibleBeiShu()) * getBaseBet();
			if (nLoseCoin == 0)
			{
				LOGFMTE("why i hu is cha jiao win 0 ? my card is : ");
				pTingPlayerCard->debugCardInfo();
			}
			if ((int32_t)nLoseCoin > pPlayerDaJiao->getCoin())
			{
				nLoseCoin = pPlayerDaJiao->getCoin();
			}

			pPlayerDaJiao->addOffsetCoin((int32_t)nLoseCoin * -1);
			pTingPaiPlayer->addOffsetCoin(nLoseCoin);

			pSettle->addHuPlayer(nTingIdx, nLoseCoin, 0, 0);
			LOGFMTD("room id = %u , da jiao uid = %u lose coin = %u to TingPaiPlayer = %u", getRoomID(), pPlayerDaJiao->getUID(),nLoseCoin,pTingPaiPlayer->getUID());
			if (pPlayerDaJiao->getCoin() == 0)
			{
				LOGFMTD("DaJiao idx = %u lose all room id = %u", nDaJiaoIdx, getRoomID());
				break;
			}
		}
		addSettle(pSettle);
	}
}

void XLMJRoom::giveBackGangWin(uint8_t nIdx)
{
	auto player = getMJPlayerByIdx(nIdx);
	if (player == nullptr || player->getCoin() == 0)
	{
		LOGFMTD("player coin is 0 or player is null can not give back gang win idx = %u, roomID = %u",nIdx,getRoomID());
		return;
	}

	auto funGiveBack = [this, player]( uint8_t nBackIdx, uint32_t nBackCoin)
	{
		if (player->getCoin() == 0)
		{
			return;
		}

		if ((int32_t)nBackCoin > player->getCoin())
		{
			nBackCoin = player->getCoin();
		}

		player->addOffsetCoin(-1 * (int32_t)nBackCoin);

		auto pBackPlayer = getMJPlayerByIdx(nBackIdx);
		if (pBackPlayer == nullptr )
		{
			LOGFMTE("give back coin to you , but you not can act or leave idx = %u, system take the coin = %u ", nBackIdx, nBackCoin);
			//continue;
		}
		else
		{
			pBackPlayer->addOffsetCoin(nBackCoin);
		}
	};

	// find the idx gang ;
	for (auto& pSe : m_vSettleInfos)
	{
		if (player->getCoin() == 0)
		{
			LOGFMTD("player idx = %u no more coin to give back gang win ", nIdx);
			break;
		}

		if (pSe->eType != eSettle_AnGang && pSe->eType != eSettle_BuGang && eSettle_MingGang != pSe->eType )
		{
			continue;
		}

		auto pGS = (ISettleGang*)pSe;
		if (pGS->nGangIdx != nIdx)
		{
			continue;
		}

		if (pSe->eType == eSettle_AnGang || pSe->eType == eSettle_BuGang)
		{
			auto pGang = (stSettleAnGang*)pSe;			
			// do give back ;
			for (auto& ref : pGang->vMapPlayeIdxAndCoin)
			{
				funGiveBack(ref.first,ref.second);
			}
		}
		else
		{
			// check ming gang 
			auto pSettleM = (stSettleMingGang*)pSe;
			// give back coin 
			funGiveBack(pSettleM->nInvokerIdx, pSettleM->nCoinOffset);
		}

		// delete this settle recorder , and go on check next ;
		delete pSe;
		pSe = nullptr;
	}

	// remove player gang pSettle recorder 
	auto iter = std::find_if(m_vSettleInfos.begin(), m_vSettleInfos.end(), [](ISettle* pS){ return pS == nullptr; });
	while (iter != m_vSettleInfos.end())
	{
		m_vSettleInfos.erase(iter);
		iter = std::find_if(m_vSettleInfos.begin(), m_vSettleInfos.end(), [](ISettle* pS){ return pS == nullptr; });
	}
}

void XLMJRoom::onGameDidEnd()
{
	IMJRoom::onGameDidEnd();
	clearAllSettle();
	if (getDelegate())
	{
		getDelegate()->onDidGameOver(this);
		return;
	}
	// every one leave room  and sync game data ;
	for (auto& pp : m_vMJPlayers)
	{
		if (pp == nullptr ) 
		{
 
			continue;
		}

		auto pXLPlayer = (XLMJPlayer*)pp;
		stMsgSvrDoLeaveRoom msgdoLeave;
		msgdoLeave.nCoin = pp->getCoin();
		msgdoLeave.nGameType = getRoomType();
		msgdoLeave.nRoomID = getRoomID();
		msgdoLeave.nUserUID = pp->getUID();
 
		msgdoLeave.nGameOffset = pp->getOffsetCoin();
		getRoomMgr()->sendMsg(&msgdoLeave, sizeof(msgdoLeave), pp->getSessionID());
		LOGFMTD("game over player uid = %u leave room id = %u",pp->getUID(),getRoomID());
  
	}

	// delete all player object ;
	for (auto& ref : m_vMJPlayers)
	{
		if (ref)
		{
			delete ref;
			ref = nullptr;
		}
	}
}

bool XLMJRoom::isGameOver()
{
	if (IMJRoom::isGameOver())
	{
		return true;
	}

	uint8_t nNotDecideLose = 0;
	for (auto& ref : m_vMJPlayers)
	{
		if (ref )
		{
			++nNotDecideLose;
		}

		if (nNotDecideLose >= 2)
		{
			return false;
		}
	}

	LOGFMTE("most player decide lose = %u , game over ",nNotDecideLose );
	return true;
}

uint32_t XLMJRoom::getBaseBet()
{
	return ((stNiuNiuRoomConfig*)getRoomConfig())->nBaseBet;
}

bool XLMJRoom::getWaitSupplyCoinPlayerIdxs(std::vector<uint8_t>& vOutWaitSupplyIdx)
{
	vOutWaitSupplyIdx.clear();
	auto nLowLimit = getCoinNeedToSitDown();
	for (auto& pPlayer : m_vMJPlayers)
	{
		if (pPlayer == nullptr)
		{
			continue;
		}

		//if (pPlayer->getCoin() < (int32_t)nLowLimit)
		if (pPlayer->getCoin() <= 0 )
		{
			vOutWaitSupplyIdx.push_back(pPlayer->getIdx());
		}
	}

	return vOutWaitSupplyIdx.empty() == false;
}

void XLMJRoom::infoPlayerSupplyCoin(std::vector<uint8_t>& vOutWaitSupplyIdx)
{
	//Json::Value js;
	//Json::Value jsArray;
	//for (auto& ref : vOutWaitSupplyIdx)
	//{
	//	jsArray[jsArray.size()] = ref;
	//}
	//js["players"] = jsArray;
	//sendRoomMsg(js, MSG_ROOM_INFORM_SUPPLY_COIN);
}

uint8_t XLMJRoom::getAutoChuCardWhenWaitActTimeout(uint8_t nIdx)
{
	auto pp = getMJPlayerByIdx(nIdx);
	if (nullptr == pp)
	{
		LOGFMTE("player is null for idx = %u",nIdx);
		return 0;
	}

	auto pCard = (XLMJPlayerCard*)pp->getPlayerCard();
	auto nCard = pCard->getQueTypeCardForChu();
	if (nCard)
	{
		return nCard;
	}

	return IMJRoom::getAutoChuCardWhenWaitActTimeout(nIdx);
}

uint8_t XLMJRoom::getAutoChuCardWhenWaitChuTimeout(uint8_t nIdx)
{
	auto pp = getMJPlayerByIdx(nIdx);
	if (nullptr == pp)
	{
		LOGFMTE("player is null for idx = %u", nIdx);
		return 0;
	}

	auto pCard = (XLMJPlayerCard*)pp->getPlayerCard();
	auto nCard = pCard->getQueTypeCardForChu();
	if (nCard)
	{
		return nCard;
	}

	return IMJRoom::getAutoChuCardWhenWaitChuTimeout(nIdx);
}

bool XLMJRoom::canKouPlayerCoin(uint8_t nPlayerIdx)
{
	auto pPlayer = getMJPlayerByIdx(nPlayerIdx);
	if (nullptr == pPlayer )
	{
		return false;
	}
	return true;
}
