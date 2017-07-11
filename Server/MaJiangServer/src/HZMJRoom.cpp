#include "HZMJRoom.h"
#include "MJRoomStateWaitReady.h"
#include "MJRoomStateWaitPlayerChu.h"
#include "MJRoomStateWaitPlayerAct.h"
#include "MJRoomStateStartGame.h"
#include "MJRoomStateGameEnd.h"
#include "MJRoomStateDoPlayerAct.h"
#include "MJRoomStateAskForPengOrHu.h"
#include "HZMJPlayerCard.h"
#include "HZMJPlayer.h"
#include "RoomConfig.h"
#include "IGameRoomManager.h"
#include "ServerMessageDefine.h"
bool HZMJRoom::init(IGameRoomManager* pRoomMgr, stBaseRoomConfig* pConfig, uint32_t nRoomID, Json::Value& vJsValue)
{
	IMJRoom::init(pRoomMgr,pConfig,nRoomID,vJsValue);
	// create room state ;
	IMJRoomState* vState[] = {
		new CMJRoomStateWaitReady(), new MJRoomStateWaitPlayerChu(), new MJRoomStateWaitPlayerAct(), new MJRoomStateStartGame()
		, new MJRoomStateGameEnd(), new MJRoomStateDoPlayerAct(), new MJRoomStateAskForPengOrHu()
	};
	for (uint8_t nIdx = 0; nIdx < sizeof(vState) / sizeof(IMJRoomState*); ++nIdx)
	{
		addRoomState(vState[nIdx]);
	}
	setInitState(vState[0]);
	m_tPoker.initAllCard(eMJ_HZ);
	m_nContinueBankes = 0;
	m_nLeasetLeftCard = 20;
	memset(m_vCaiPiaoFlag,0,sizeof(m_vCaiPiaoFlag));
	return true;
}

bool HZMJRoom::onPlayerApplyLeave(uint32_t nPlayerUID)
{
	auto pPlayer = getMJPlayerByUID(nPlayerUID);
	if (!pPlayer)
	{
		LOGFMTE("you are not in room id = %u , how to leave this room ? uid = %u", getRoomID(), nPlayerUID);
		return false;
	}

	auto curState = getCurRoomState()->getStateID();
	if (eRoomSate_WaitReady == curState || eRoomState_GameEnd == curState )
	{
		// direct leave just stand up ;
		stMsgSvrDoLeaveRoom msgdoLeave;
		msgdoLeave.nCoin = pPlayer->getCoin();
		msgdoLeave.nGameType = getRoomType();
		msgdoLeave.nRoomID = getRoomID();
		msgdoLeave.nUserUID = pPlayer->getUID();
		msgdoLeave.nMaxFangXingType = 0;
		msgdoLeave.nMaxFanShu = 0;
		msgdoLeave.nRoundsPlayed = 1;
		msgdoLeave.nGameOffset = pPlayer->getOffsetCoin();
		getRoomMgr()->sendMsg(&msgdoLeave, sizeof(msgdoLeave), pPlayer->getSessionID());

		return standup(nPlayerUID);

	}
	else
	{
		pPlayer->setState((pPlayer->getState() | eRoomPeer_DelayLeave));
		LOGFMTE("decide player already sync data uid = %u", pPlayer->getUID());
	}
	return true;
}

void HZMJRoom::startGame()
{
	memset(m_vCaiPiaoFlag, 0, sizeof(m_vCaiPiaoFlag));
	IMJRoom::startGame();
}

void HZMJRoom::onGameEnd()
{
	// send game result ;
	IMJRoom::onGameEnd();

	// if have player delay leave just leave ;

}

void HZMJRoom::willStartGame()
{
	IMJRoom::willStartGame();
	memset(m_vCaiPiaoFlag, 0, sizeof(m_vCaiPiaoFlag));
	if (getBankerIdx() == (uint8_t)-1)
	{
		setBankIdx(rand() % getSeatCnt());
	}
}

void HZMJRoom::onGameDidEnd()
{
	uint8_t nHuIdx = -1;
	for (auto& pPlayer : m_vMJPlayers)
	{
		if (pPlayer && pPlayer->haveState(eRoomPeer_AlreadyHu))
		{
			nHuIdx = pPlayer->getIdx();
		}
	}

	if (getBankerIdx() == nHuIdx)
	{
		++m_nContinueBankes;
	}

	if (nHuIdx != (uint8_t)-1)
	{
		setBankIdx(nHuIdx);
		LOGFMTD("player idx = %u be the new banker room id = %u",nHuIdx,getRoomID());
	}
	IMJRoom::onGameDidEnd();
}

void HZMJRoom::onWaitPlayerAct(uint8_t nIdx, bool& isCanPass)
{
	if ( isHavePiao() == false || isPlayerPiao(nIdx) )  // no piao or self piao 
	{
		m_nLeasetLeftCard = 22;
		IMJRoom::onWaitPlayerAct(nIdx, isCanPass);
		m_nLeasetLeftCard = 20;
		return;
	}

	LOGFMTD("some one piao , but not cur idx = %u , so can not do free, room id = %u",nIdx,getRoomID());
	// bei piao xianzhi le
	auto pPlayer = getMJPlayerByIdx(nIdx);
	if (!pPlayer)
	{
		LOGFMTE("player idx = %u is null can not tell it wait act", nIdx);
		return;
	}
	auto pMJCard = (HZMJPlayerCard*)pPlayer->getPlayerCard();
	// send msg to tell player do act 
	Json::Value jsArrayActs;

	// check hu .
	bool isCanHu = pMJCard->isHoldCardCanHu();
	if ( isCanHu )
	{
		Json::Value jsAct;
		jsAct["act"] = eMJAct_Hu;
		jsAct["cardNum"] = pMJCard->getNewestFetchedCard();
		jsArrayActs[jsArrayActs.size()] = jsAct;
	}

	if ( getMJPoker()->getLeftCardCount() >21 && isCanHu && pMJCard->isBaoTou() )
	{
		IMJPlayerCard::VEC_CARD vCards;
		// check an gang .
		vCards.clear();
		pMJCard->getHoldCardThatCanAnGang(vCards);
		for (auto& ref : vCards)
		{
			Json::Value jsAct;
			jsAct["act"] = eMJAct_AnGang;
			jsAct["cardNum"] = ref;
			jsArrayActs[jsArrayActs.size()] = jsAct;
		}
	}

	isCanPass = jsArrayActs.empty() == false;

	// add default alwasy chu , infact need not add , becaust it alwasy in ,but compatable with current client ;
	Json::Value jsAct;
	jsAct["act"] = eMJAct_Chu;
	jsAct["cardNum"] = getAutoChuCardWhenWaitActTimeout(nIdx);
	jsArrayActs[jsArrayActs.size()] = jsAct;

	Json::Value jsMsg;
	jsMsg["acts"] = jsArrayActs;
	sendMsgToPlayer(jsMsg, MSG_PLAYER_WAIT_ACT_AFTER_RECEIVED_CARD, pPlayer->getSessionID());

	LOGFMTD("tell player idx = %u do act size = %u", nIdx, jsArrayActs.size());
}

bool HZMJRoom::isAnyPlayerPengOrHuThisCard(uint8_t nInvokeIdx, uint8_t nCard)
{
	for (auto& ref : m_vMJPlayers)
	{
		if (ref == nullptr || nInvokeIdx == ref->getIdx())
		{
			continue;
		}

		if (isHavePiao() && isPlayerPiao(ref->getIdx()) == false)
		{
			continue;
		}

		auto pMJCard = ref->getPlayerCard();
		if (pMJCard->canPengWithCard(nCard) /*|| pMJCard->canHuWitCard(nCard)*/) // can not fang chong / pao 
		{
			return true;
		}

		if ( m_tPoker.getLeftCardCount() > 21 && pMJCard->canMingGangWithCard(nCard)) // must can gang 
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

void HZMJRoom::onAskForPengOrHuThisCard(uint8_t nInvokeIdx, uint8_t nCard, std::vector<uint8_t>& vOutWaitHuIdx, std::vector<uint8_t>& vOutWaitPengGangIdx, bool& isNeedWaitEat)
{
	//MSG_PLAYER_WAIT_ACT_ABOUT_OTHER_CARD,  // 有人出了一张牌，等待需要这张牌的玩家 操作，可以 碰，杠，胡
		// svr : { invokerIdx : 2,cardNum : 32 , acts : [type0, type 1 , ..] }  ;
	for (auto& ref : m_vMJPlayers)
	{
		if (ref == nullptr || nInvokeIdx == ref->getIdx())
		{
			continue;
		}

		if (isHavePiao() && isPlayerPiao(ref->getIdx()) == false)
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
			vOutWaitPengGangIdx.push_back(ref->getIdx());
		}

		// check ming gang 
		if (m_tPoker.getLeftCardCount() >= 21 && pMJCard->canMingGangWithCard(nCard))
		{
			jsActs[jsActs.size()] = eMJAct_MingGang;
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

		if (jsActs.size() > 0)
		{
			jsActs[jsActs.size()] = eMJAct_Pass;
		}

		jsMsg["acts"] = jsActs;
		sendMsgToPlayer(jsMsg, MSG_PLAYER_WAIT_ACT_ABOUT_OTHER_CARD, ref->getSessionID());
		LOGFMTD("inform uid = %u act about other card room id = %u card = %u", ref->getUID(), getRoomID(), nCard);
	}
}

bool HZMJRoom::isAnyPlayerRobotGang(uint8_t nInvokeIdx, uint8_t nCard)
{
	return false;
}

bool HZMJRoom::isGameOver()
{
	if (!isCanGoOnMoPai())
	{
		return true;
	}

	// is any one hu 
	for (auto& pPlayer : m_vMJPlayers)
	{
		if (pPlayer && pPlayer->haveState(eRoomPeer_AlreadyHu))
		{
			return true;
		}
	}
	return false;
}

bool HZMJRoom::isCanGoOnMoPai()
{
	return m_tPoker.getLeftCardCount() > m_nLeasetLeftCard;
}

IMJPlayer* HZMJRoom::doCreateMJPlayer()
{
	return new HZMJPlayer();
}

void HZMJRoom::onPlayerChu(uint8_t nIdx, uint8_t nCard)
{
	HZMJPlayer* pPlayer = (HZMJPlayer*)getMJPlayerByIdx(nIdx);
	auto nCaiShen = make_Card_Num(eCT_Jian, 3);
	m_vCaiPiaoFlag[nIdx] = nCaiShen == nCard;
	if (nCaiShen == nCard) // cai piao 
	{
		bool bIsGAngPiao = pPlayer->haveGangFalg() && pPlayer->getPlayerCard()->isHoldCardCanHu();
		pPlayer->increasePiaoTimes(bIsGAngPiao);
	}
	else
	{
		pPlayer->clearPiaoTimes();
	}
	IMJRoom::onPlayerChu(nIdx,nCard);
}

void HZMJRoom::onPlayerHu(std::vector<uint8_t>& vHuIdx, uint8_t nCard, uint8_t nInvokeIdx)
{
	// check only can self mo hu ;
	if (vHuIdx.size() != 1 || vHuIdx.front() != nInvokeIdx)
	{
		LOGFMTE("hzmj can only zi mo , why not this situation");
		return;
	}

	// do check base bet ;
	HZMJPlayer* pPlayer = (HZMJPlayer*)getMJPlayerByIdx(vHuIdx.front());
	if (!pPlayer)
	{
		LOGFMTE("hu player is null , idx = %u",vHuIdx.front());
		return;
	}

	pPlayer->setState(eRoomPeer_AlreadyHu);
	auto pConfig = (stMJRoomConfig*)getRoomConfig();
	
	bool isQiDui = false;
	uint8_t nHaoHuaCnt = 0;
	bool isBaoTiao = false;
	uint8_t nPiaoCnt = 0;
	bool isGangKai = pPlayer->haveGangFalg();

	uint32_t nBasetBet = pConfig->nBaseBet;
	auto pHZCard = (HZMJPlayerCard*)pPlayer->getPlayerCard();
	
	// check qidui 
	isQiDui = pHZCard->canHoldCard7PairHu();
	if ( isQiDui )
	{
		nBasetBet *= 2;
		uint8_t n = pHZCard->get7PairHuHaoHuaCnt();
		nHaoHuaCnt = n;
		while ( n-- > 0 )
		{
			nBasetBet *= 4;
		}
	}

	// check bao tou 
	isBaoTiao = pHZCard->isBaoTou();
	if (isBaoTiao)
	{
		nBasetBet *= 2;
	}

	// check cai piao 
	nPiaoCnt = pPlayer->getPiaoTimes();
	auto nPiCnt = nPiaoCnt;
	while (nPiCnt-- > 0)
	{
		nBasetBet *= 2;
	}

	// check gang kai 
	if ( isGangKai )
	{
		nBasetBet *= 2;
		if (nPiaoCnt > 0)  // piao gang ; 
		{
			nBasetBet *= 8;
		}
	}

	// gang piao 
	if (pPlayer->haveGangPiao())
	{
		nBasetBet *= 8;
	}

	/// do caculate per player ;
	if (getBankerIdx() == vHuIdx.front())
	{
		nBasetBet *= 2;
	}

	for (auto& pLoser : m_vMJPlayers)
	{
		if (pLoser == nullptr || pLoser->getIdx() == pPlayer->getIdx())
		{
			continue;
		}

		uint32_t nCaCoin = pLoser->getIdx() == getBankerIdx() ? (nBasetBet * 2) : nBasetBet;
		if (nCaCoin > (uint32_t)pLoser->getCoin())
		{
			nCaCoin = pLoser->getCoin();
		}
		pPlayer->addOffsetCoin(nCaCoin);
		pLoser->addOffsetCoin(nCaCoin*-1);
	}

	// send msg 
	Json::Value msg;
	msg["idx"] = vHuIdx.front();
	msg["actType"] = eMJAct_Hu;
	msg["card"] = nCard;
	sendRoomMsg(msg, MSG_ROOM_ACT);

	// also game ; send game result here ;
	//MSG_ROOM_HZMJ_RESULT, 
		// svr : { winnerIdx : 2 , is7Pair : 0 , HaoHua : 1 , isBaoTou : 0 , piaoCnt : 1 , isGangKai : 1 , isGangPiao , results: [ {uid : 2345 , offset : -23, final : 23} , ....  ]   } 
	Json::Value jsMsg;
	jsMsg["winnerIdx"] = pPlayer->getIdx();
	jsMsg["is7Pair"] = isQiDui;
	jsMsg["HaoHua"] = nHaoHuaCnt;
	jsMsg["isBaoTou"] = isBaoTiao;
	jsMsg["piaoCnt"] = nPiaoCnt;
	jsMsg["isGangKai"] = isGangKai;
	jsMsg["isGangPiao"] = pPlayer->haveGangPiao();

	Json::Value jsResult;
	for (auto& pp : m_vMJPlayers)
	{
		if ( pp == nullptr )
		{
			continue;
		}

		Json::Value jsP;
		jsP["uid"] = pp->getUID();
		jsP["offset"] = pp->getOffsetCoin();
		jsP["final"] = pp->getCoin();
		jsResult[jsResult.size()] = jsP;
	}

	jsMsg["results"] = jsResult;
	sendRoomMsg(jsMsg, MSG_ROOM_HZMJ_RESULT);
	LOGFMTD("send game result !");
}

bool HZMJRoom::isHavePiao()
{
	for (auto& ref : m_vCaiPiaoFlag)
	{
		if (ref)
		{
			return true;
		}
	}
	return false;
}

bool HZMJRoom::isPlayerPiao(uint8_t nIdx)
{
	if (nIdx >= getSeatCnt())
	{
		return false;
	}

	return m_vCaiPiaoFlag[nIdx];
}