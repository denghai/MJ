#include "NiuNiuRoom.h"
#include "NiuNiuRoomPlayer.h"
#include "NiuNiuServer.h"
#include "NiuNiuRoomDistribute4CardState.h"
#include "NiuNiuRoomTryBankerState.h"
#include "NiuNiuRoomRandBankerState.h"
#include "NiuNiuRoomBetState.h"
#include "NiuNiuRoomDistributeFinalCardState.h"
#include "NiuNiuRoomGameResult.h"
#include "NiuNiuMessageDefine.h"
#include "ServerMessageDefine.h"
#include "log4z.h"
#include "AutoBuffer.h"
#include "CardPoker.h"
#include "NiuNiuRoomPlayerCaculateCardState.h"
#include "ServerStringTable.h"
#include "NiuNiuWaitStartGame.h"
#include "NiuNiuGrabBanker.h"
#include <algorithm>
#include <json/json.h>
CNiuNiuRoom::CNiuNiuRoom() :m_tGameResult(100)
{
	m_nRateLevel = 1;
	getPoker()->InitTaxasPoker() ;
}

bool CNiuNiuRoom::onFirstBeCreated(IRoomManager* pRoomMgr,uint32_t nRoomID, const Json::Value& vJsValue )
{
	ISitableRoom::onFirstBeCreated(pRoomMgr,nRoomID,vJsValue) ;
	m_nBaseBet = vJsValue["baseBet"].asUInt();
	auto jsopt = vJsValue["opts"];
	m_nResignBankerCtrl = jsopt["unbankerType"].asUInt() ;
	m_nBankerIdx = -1;
	m_nBetBottomTimes = 1 ;
	m_nBankerCoinLimitForBet = 0 ;
	m_isWillManualLeaveBanker = false ;
	if ( jsopt["rateLevel"].isNull() == false)
	{
		m_nRateLevel = jsopt["rateLevel"].asUInt();
		LOGFMTD("create room rate level = %u , roomID = %u",m_nRateLevel,getRoomID() );
	}
	return true ;
}

void CNiuNiuRoom::prepareState()
{
	//ISitableRoom::prepareState();
	// create room state ;
	IRoomState* vState[] = {
		new CNiuNiuWaitStartGame(),new CNiuNiuRoomGrabBanker(), new CNiuNiuRoomRandBankerState(),
		new CNiuNiuRoomBetState(),new CNiuNiuRoomDistribute4CardState(),new CNiuNiuRoomDistributeFinalCardState(),new CNiuNiuRoomStatePlayerCaculateCardState() ,new CNiuNiuRoomGameResultState()
	};
	for ( uint8_t nIdx = 0 ; nIdx < sizeof(vState) / sizeof(IRoomState*); ++nIdx )
	{
		addRoomState(vState[nIdx]) ;
	}
	setInitState(vState[0]);
}

void CNiuNiuRoom::serializationFromDB(IRoomManager* pRoomMgr,stBaseRoomConfig* pConfig,uint32_t nRoomID , Json::Value& vJsValue )
{
	ISitableRoom::serializationFromDB(pRoomMgr,pConfig,nRoomID,vJsValue);
	m_nBaseBet = ((stNiuNiuRoomConfig*)pConfig)->nBaseBet;
}

void CNiuNiuRoom::willSerializtionToDB(Json::Value& vOutJsValue)
{
	ISitableRoom::willSerializtionToDB(vOutJsValue);
}

void CNiuNiuRoom::roomItemDetailVisitor(Json::Value& vOutJsValue)
{
	ISitableRoom::roomItemDetailVisitor(vOutJsValue) ;
	//vOutJsValue["baseBet"] = getBaseBet() ;
}

ISitableRoomPlayer* CNiuNiuRoom::doCreateSitableRoomPlayer()
{
	return new CNiuNiuRoomPlayer();
}

void CNiuNiuRoom::onPlayerWillStandUp( ISitableRoomPlayer* pPlayer )
{
	if ( pPlayer->isHaveState(eRoomPeer_CanAct) )
	{
		 ISitableRoom::onPlayerWillStandUp(pPlayer) ;
	}
	else
	{
		playerDoStandUp(pPlayer);
	}
}

uint32_t CNiuNiuRoom::getLeastCoinNeedForCurrentGameRound(ISitableRoomPlayer* pp)
{
	CNiuNiuRoomPlayer* pPlayer = (CNiuNiuRoomPlayer*)pp ;
	return pPlayer->getCoin();
	//if ( getBankerIdx() == pPlayer->getIdx() )
	//{
	//	uint8_t nCnt = getPlayerCntWithState(eRoomPeer_CanAct) - 1 ;
	//	uint32_t nNeedCoin = ( getBaseBet() * m_nBetBottomTimes * getMaxRate() * 25 ) * nCnt ;
	//	LOGFMTD("uid = %d will standup but is banker BankeTimes = %d , need Coin = %d",pPlayer->getUserUID(),m_nBetBottomTimes,nNeedCoin) ;
	//	return nNeedCoin ;
	//}
	//else
	//{
	//	if ( m_nBetBottomTimes != 0 )
	//	{
	//		uint32_t nNeedCoin = getBaseBet() * m_nBetBottomTimes * getMaxRate() *( pPlayer->getBetTimes() > 5 ? pPlayer->getBetTimes() : 5 )  ;
	//		LOGFMTD("uid = %d will standup BankeTimes = %d , need Coin = %d",pPlayer->getUserUID(),m_nBetBottomTimes,nNeedCoin) ;
	//		return nNeedCoin ;
	//	}
	//	else
	//	{
	//		uint8_t nCnt = getPlayerCntWithState(eRoomPeer_CanAct) - 1 ;
	//		uint32_t nWhenBankNeed = ( getBaseBet() * 1 * getMaxRate() * 25 ) * nCnt ;
	//		uint32_t nWhenNotBanker = getBaseBet() * 4 * getMaxRate() * 5 ;
	//		uint32_t nNeedCoin = nWhenNotBanker > nWhenBankNeed ? nWhenNotBanker : nWhenBankNeed ;
	//		LOGFMTD("uid = %d will standup BankeTimes not decide , need Coin = %d",pPlayer->getUserUID(),m_nBetBottomTimes,nNeedCoin) ;
	//		return nNeedCoin ;
	//	}
 //
	//}
}

void CNiuNiuRoom::roomInfoVisitor(Json::Value& vOutJsValue)
{
	vOutJsValue["bankIdx"] = m_nBankerIdx;
	if ( m_nBankerIdx == (uint8_t)-1 )
	{
		vOutJsValue["bankIdx"] = 0;
	}
	vOutJsValue["baseBet"] = getBaseBet();
	vOutJsValue["bankerTimes"] = m_nBetBottomTimes;
	vOutJsValue["unbankerType"] = m_nResignBankerCtrl;
	vOutJsValue["rateLevel"] = m_nRateLevel;
}

void CNiuNiuRoom::sendRoomPlayersInfo(uint32_t nSessionID)
{
	stMsgNNRoomPlayers msgInfo ;
	msgInfo.nPlayerCnt = (uint8_t)getSitDownPlayerCount();
	CAutoBuffer auBuffer(sizeof(msgInfo) + sizeof(stNNRoomInfoPayerItem) * msgInfo.nPlayerCnt);
	auBuffer.addContent(&msgInfo,sizeof(msgInfo));

	uint8_t nSeatCount = (uint8_t)getSeatCount();
	stNNRoomInfoPayerItem item ;
	uint8_t nDisCardCnt = getDistributeCardCnt();
	for ( uint8_t nIdx = 0 ; nIdx < nSeatCount ; ++nIdx )
	{
		CNiuNiuRoomPlayer* psit = (CNiuNiuRoomPlayer*)getPlayerByIdx(nIdx) ;
		if ( psit )
		{
			memset(item.vHoldChard,0,sizeof(item.vHoldChard)) ;
			item.nBetTimes = psit->getBetTimes() ;
			item.nCoin = psit->getCoin() ;
			item.nIdx = psit->getIdx() ;
			item.nStateFlag = psit->getState() ;
			item.nUserUID = psit->getUserUID() ;
			for ( uint8_t nCardIdx = 0 ; nCardIdx < nDisCardCnt ; ++nCardIdx )
			{
				item.vHoldChard[nCardIdx] = psit->getCardByIdx(nCardIdx) ;
			}
			auBuffer.addContent(&item,sizeof(item)) ;
		}
	}

	sendMsgToPlayer((stMsg*)auBuffer.getBufferPtr(),auBuffer.getContentSize(),nSessionID) ;
	LOGFMTD("send room info to session id = %d, player cnt = %d ", nSessionID,msgInfo.nPlayerCnt) ;
}

void CNiuNiuRoom::setBankerIdx(uint8_t nIdx )
{
	m_nBankerIdx = nIdx ; 	
	auto pp = getPlayerByIdx(m_nBankerIdx) ;
	uint16_t nCnt = getPlayerCntWithState(eRoomPeer_CanAct) - 1 ;
	setBankCoinLimitForBet( pp->getCoin() / nCnt ) ; 
}

uint8_t CNiuNiuRoom::getMaxRate()
{
	return getReateByNiNiuType(CNiuNiuPeerCard::NiuNiuType::Niu_FiveSmall,10);
}

uint8_t CNiuNiuRoom::getDistributeCardCnt()
{
	uint32_t nState = getCurRoomState()->getStateID() ;
	switch (nState)
	{
	case eRoomState_NN_Disribute4Card:
	case eRoomState_NN_FinalCard:
	case eRoomState_NN_CaculateCard:
	case eRoomState_NN_GameResult:
		return 5 ;
	default:
		break;
	}
	return 0 ;
}

uint32_t CNiuNiuRoom::getBaseBet()
{
	return m_nBaseBet ;
}

uint32_t& CNiuNiuRoom::getBankCoinLimitForBet()
{
	return m_nBankerCoinLimitForBet;
}

void CNiuNiuRoom::setBankCoinLimitForBet( uint64_t nCoin )
{
	m_nBankerCoinLimitForBet = nCoin ;
}

uint8_t CNiuNiuRoom::getReateByNiNiuType(uint8_t nType , uint8_t nPoint )
{
	switch (m_nRateLevel)
	{
		case 0:
		{
			if (nPoint <= 7 || nType == CNiuNiuPeerCard::Niu_None)
			{
				return 1;
			}

			if (8 == nPoint || 9 == nPoint)
			{
				return 2;
			}

			return 3;
		}
		break;
		case 1:
		{
			if (nPoint < 7 || nType == CNiuNiuPeerCard::Niu_None)
			{
				return 1;
			}

			return (nPoint - 5);
		}
		break;
		case 2:
		{
			if (nPoint <= 1 || nType == CNiuNiuPeerCard::Niu_None)
			{
				return 1;
			}

			return nPoint;
		}
		break;
		default:
			LOGFMTD("unknown rate level %u",m_nRateLevel);
			return 1;
	}

	LOGFMTD("unknown rate level %u", m_nRateLevel);
	return 1;
}

uint32_t CNiuNiuRoom::getLeastCoinNeedForBeBanker( uint8_t nBankerTimes )
{
	uint8_t n = getPlayerCntWithState(eRoomPeer_CanAct) ;
	n += getPlayerCntWithState(eRoomPeer_WaitNextGame) ;
	return getBaseBet() * nBankerTimes * getMaxRate() * ( n - 1 );
}

void CNiuNiuRoom::onGameWillBegin()
{
	ISitableRoom::onGameWillBegin();
	m_nBankerCoinLimitForBet = 0 ;
	m_nBetBottomTimes = 1 ;
	m_isWillManualLeaveBanker = false ;
	getPoker()->RestAllPoker();
	if ( isHaveBanker() )
	{
		setBankerIdx(m_nBankerIdx);

		if (3 == m_nResignBankerCtrl )
		{
			stMsgNNProducedBanker msgInfom;
			msgInfom.nBankerBetTimes = 1;
			msgInfom.nBankerIdx = getBankerIdx();
			sendRoomMsg(&msgInfom, sizeof(msgInfom));
			LOGFMTD("inform new round banker = %u , room id = %u",getBankerIdx(),getRoomID());
		}
	}
	LOGFMTD("room game begin");
}

void CNiuNiuRoom::onGameDidEnd()
{
	stMsgSaveLog msgSaveLog ;
	msgSaveLog.nLogType = eLog_NiuNiuGameResult ;
	msgSaveLog.nTargetID = getRoomID() ;
	msgSaveLog.nJsonExtnerLen = 0 ;
	memset(msgSaveLog.vArg,0,sizeof(msgSaveLog.vArg));
	msgSaveLog.vArg[0] = 0;//getPlayerByIdx(m_nBankerIdx)->getUserUID();
	msgSaveLog.vArg[1] = m_nBetBottomTimes ;
	msgSaveLog.vArg[2] = getBaseBet() * m_nBetBottomTimes ;
	m_arrPlayers.clear() ;

	/*m_nBankerIdx = 0 ;
	m_nBankerCoinLimitForBet = 0 ;
	m_nBetBottomTimes = 0 ;*/

	uint8_t nSeatCnt = (uint8_t)getSeatCount() ;
	for ( uint8_t nIdx = 0; nIdx < nSeatCnt; ++nIdx )
	{
		ISitableRoomPlayer* pSitDown = getPlayerByIdx(nIdx) ;
		if ( pSitDown == nullptr )
		{
			continue;
		}

		// write to log 
		if ( pSitDown->isHaveState(eRoomPeer_CanAct) )
		{
			CNiuNiuRoomPlayer* pNiuPlayer = (CNiuNiuRoomPlayer*)pSitDown ;
			Json::Value refPlayer ;

			refPlayer["uid"] = pNiuPlayer->getUserUID() ;
			refPlayer["idx"] = pNiuPlayer->getIdx();
			refPlayer["betTimes"] = pNiuPlayer->getBetTimes() <= 0 ? 5 : pNiuPlayer->getBetTimes();
			refPlayer["card0"] = pNiuPlayer->getCardByIdx(0);
			refPlayer["card1"] = pNiuPlayer->getCardByIdx(1);
			refPlayer["card2"] = pNiuPlayer->getCardByIdx(2);
			refPlayer["card3"] = pNiuPlayer->getCardByIdx(3);
			refPlayer["card4"] = pNiuPlayer->getCardByIdx(4);
			refPlayer["offset"] = 0 ;//pNiuPlayer->getCoinOffsetThisGame() ;
			refPlayer["coin"] = (int32_t)pNiuPlayer->getCoin() ;
			m_arrPlayers[pNiuPlayer->getIdx()] = refPlayer ;
		}

		//pSitDown->removeState(eRoomPeer_CanAct);
	}

	Json::StyledWriter write ;
	std::string str = write.write(m_arrPlayers);
	CAutoBuffer auBuffer (sizeof(msgSaveLog) + str.size());
	msgSaveLog.nJsonExtnerLen = str.size() ;
	auBuffer.addContent((char*)&msgSaveLog,sizeof(msgSaveLog)) ;
	auBuffer.addContent(str.c_str(),str.size());
	sendMsgToPlayer((stMsg*)auBuffer.getBufferPtr(),auBuffer.getContentSize(),getRoomID()) ;

	ISitableRoom::onGameDidEnd();
	if (getDelegate())
	{
		getDelegate()->onOneRoundEnd(this);
	}
	LOGFMTD("room game End");
}

void CNiuNiuRoom::prepareCards()
{
	// parepare cards for all player ;
	uint8_t nSeatCnt = (uint8_t)getSeatCount() ;
	for ( uint8_t nIdx = 0 ; nIdx < nSeatCnt ; ++nIdx )
	{
		CNiuNiuRoomPlayer* pRoomPlayer = (CNiuNiuRoomPlayer*)getPlayerByIdx(nIdx) ;
		if ( pRoomPlayer && pRoomPlayer->isHaveState(eRoomPeer_CanAct))
		{
			uint8_t nCardCount = NIUNIU_HOLD_CARD_COUNT ;
			uint8_t nCardIdx = 0 ;
			while ( nCardIdx < nCardCount )
			{
				pRoomPlayer->onGetCard(nCardIdx,getPoker()->GetCardWithCompositeNum()) ;
				++nCardIdx ;
			}
		}
	}
}

uint32_t CNiuNiuRoom::coinNeededToSitDown()
{
	return getBaseBet() * getMaxRate() + getDeskFee() ;
}

void CNiuNiuRoom::caculateGameResult()
{
	// caculate result ;
	CNiuNiuRoomPlayer* pBanker = (CNiuNiuRoomPlayer*)getPlayerByIdx(getBankerIdx()) ;
	assert(pBanker && "why banker is null ?");
	LOGFMTD("banker coin = %u",pBanker->getCoin()) ;
	
	bool isBankerHaveNiu = pBanker->isHaveNiu() ;
	bool isLoseToAll = true ;
	// send result msg ;
	stMsgNNGameResult msgResult ;
	msgResult.nPlayerCnt = getSortedPlayerCnt() ;

	//CAutoBuffer auBuffer(sizeof(msgResult) + msgResult.nPlayerCnt * sizeof(stNNGameResultItem));
	m_tGameResult.clearBuffer();
	m_tGameResult.addContent(&msgResult, sizeof(msgResult));

	int32_t nBankerOffset = 0 ;
	// caclulate banker win ;
	uint8_t nIdx = 0 ;
	CNiuNiuPeerCard* perr = (CNiuNiuPeerCard*)pBanker->getPeerCard();
	for (  ;nIdx < msgResult.nPlayerCnt ; ++nIdx )
	{
		CNiuNiuRoomPlayer* pNNP = (CNiuNiuRoomPlayer*)getSortedPlayerByIdx(nIdx) ;
		assert(pNNP && "why have null player in sorted player list" );
		if ( pNNP == pBanker )
		{
			break;
		}

		uint32_t nLoseCoin = min(5,pNNP->getBetTimes()) * getBaseBet() * m_nBetBottomTimes * getReateByNiNiuType(perr->getType(),perr->getPoint()) ;
		if ( nLoseCoin > pNNP->getCoin() )
		{
			nLoseCoin = pNNP->getCoin() ;
			LOGFMTE("you do not have coin why you bet so many coin , uid = %d",pNNP->getUserUID());
		}

		nBankerOffset += nLoseCoin ;
		pNNP->setCoin(pNNP->getCoin() - nLoseCoin ) ;
		pBanker->setCoin(pBanker->getCoin() + nLoseCoin) ;

		stNNGameResultItem item ;
		item.nFinalCoin = pNNP->getCoin() ;
		item.nOffsetCoin = -1* nLoseCoin ;
		item.nPlayerIdx = pNNP->getIdx() ;
		m_tGameResult.addContent(&item, sizeof(item));
		LOGFMTD("result item : idx = %u, uid = %u , offset = %d",item.nPlayerIdx,pNNP->getUserUID(),item.nOffsetCoin);
		pNNP->setGameOffset(item.nOffsetCoin);

		isLoseToAll = false ;
	}

	// caculate banker lose 
	++nIdx ;
	for (  ; nIdx < msgResult.nPlayerCnt ; ++nIdx )
	{
		CNiuNiuRoomPlayer* pNNP = (CNiuNiuRoomPlayer*)getSortedPlayerByIdx(nIdx) ;
		if ( pNNP == pBanker )
		{
			break;
		}

		CNiuNiuPeerCard* pc = (CNiuNiuPeerCard*)pNNP->getPeerCard();
		uint32_t nBankerLoseCoin = min(5,pNNP->getBetTimes()) * getBaseBet() * m_nBetBottomTimes * getReateByNiNiuType(pc->getType(),pc->getPoint())  ;
		if ( nBankerLoseCoin > pBanker->getCoin() )
		{
			nBankerLoseCoin = pBanker->getCoin() ;
		}

		nBankerOffset -= nBankerLoseCoin ;
		pBanker->setCoin(pBanker->getCoin() - nBankerLoseCoin ) ;
		float fTaxFee = (float)nBankerLoseCoin * getChouShuiRate();
		addTotoalProfit((uint32_t)fTaxFee);
		float nWithoutTaxWin = nBankerLoseCoin - fTaxFee ;

		pNNP->setCoin(pNNP->getCoin() + (int32_t)nWithoutTaxWin ) ;
		LOGFMTD("room id = %u , uid = %u without tax win = %0.3f",getRoomID(),pNNP->getUserUID(),nWithoutTaxWin) ;

		stNNGameResultItem item ;
		item.nFinalCoin = pNNP->getCoin() ;
		item.nOffsetCoin = (int32_t)nWithoutTaxWin ;
		item.nPlayerIdx = pNNP->getIdx() ;
		m_tGameResult.addContent(&item, sizeof(item));
		LOGFMTD("result item : idx = %u, uid = %u , offset = %d",item.nPlayerIdx,pNNP->getUserUID(),item.nOffsetCoin);
		pNNP->setGameOffset(item.nOffsetCoin);
	}

	if ( nBankerOffset > (int32_t)0 )
	{
		float fTaxFee = (float)nBankerOffset * getChouShuiRate();
		addTotoalProfit((uint32_t)fTaxFee);
		nBankerOffset = nBankerOffset - (int32_t)fTaxFee ;
		LOGFMTD("room id = %u , banker uid = %u without tax win = %d",getRoomID(),pBanker->getUserUID(),nBankerOffset) ;
	}

	stNNGameResultItem item ;
	item.nFinalCoin = pBanker->getCoin() ;
	item.nOffsetCoin = nBankerOffset ;
	item.nPlayerIdx = pBanker->getIdx() ;
	m_tGameResult.addContent(&item, sizeof(item));
	pBanker->setGameOffset(item.nOffsetCoin);
	LOGFMTD("result item : idx = %u, uid = %u , offset = %d",item.nPlayerIdx,pBanker->getUserUID(),item.nOffsetCoin);
	LOGFMTD("result player idx = %d , finalCoin = %d, offset coin = %d",item.nPlayerIdx,item.nFinalCoin,item.nOffsetCoin) ;

	sendRoomMsg((stMsg*)m_tGameResult.getBufferPtr(), m_tGameResult.getContentSize());

	// check banker for coin 
	if ( pBanker->getCoin() < getLeastCoinNeedForBeBanker(1) )
	{
		m_nBankerIdx = -1 ;
		LOGFMTI("coin is not enough , so resign banker uid = %u",pBanker->getUserUID()) ;
	}
	// decide banker ;
	 // // 0 no niu leave banker , 1 lose to all  leave banker ;
	if ((m_nResignBankerCtrl == 3 ) || (m_nResignBankerCtrl == 0 && isBankerHaveNiu == false) || (m_nResignBankerCtrl == 1 && isLoseToAll) || (2 == m_nResignBankerCtrl && m_isWillManualLeaveBanker))
	{
		if (m_nResignBankerCtrl == 3)
		{
			// find next banker 
			uint8_t nIdx = pBanker->getIdx() + 1 ;
			uint8_t nNewBankIdx = -1;
			for (; nIdx < (getSeatCount() * 2); ++nIdx)
			{
				uint8_t nNewIdx = nIdx % getSeatCount();
				auto pPlayer = getPlayerByIdx(nNewIdx);
				if (pPlayer && pPlayer->getCoin() >= getLeastCoinNeedForBeBanker(1))
				{
					nNewBankIdx = pPlayer->getIdx();
					break;
				}
			}

			if ((uint8_t)-1 == nNewBankIdx)
			{
				LOGFMTE("no proper banker room id = %u, go on robot banker",getRoomID() );
				m_nBankerIdx = -1;
				m_isWillManualLeaveBanker = false;
			}
			else
			{
				m_nBankerIdx = nNewBankIdx;
			}
		}
		else
		{
			m_nBankerIdx = -1;
			m_isWillManualLeaveBanker = false;
		}

		LOGFMTD("resign banker ctrl = %u , isBankerHaveNiu = %u , isLoseToAll = %u",m_nResignBankerCtrl,isBankerHaveNiu,isLoseToAll) ;
	}
}

bool CNiuNiuRoom::getPlayersHaveGrabBankerPrivilege(std::vector<uint8_t>& vAllPrivilegeIdxs)
{
	uint32_t nBankerNeedCoin = getLeastCoinNeedForBeBanker(1) ;
	uint8_t nSeatCnt = (uint8_t)getSeatCount() ;
	for ( uint8_t nIdx = 0 ; nIdx < nSeatCnt ; ++nIdx )
	{
		CNiuNiuRoomPlayer* pRoomPlayer = (CNiuNiuRoomPlayer*)getPlayerByIdx(nIdx) ;
		if ( pRoomPlayer && ( pRoomPlayer->isHaveState(eRoomPeer_CanAct) || pRoomPlayer->isHaveState(eRoomPeer_WaitNextGame)))
		{
			 if ( pRoomPlayer->getCoin() > nBankerNeedCoin )
			 {
				 vAllPrivilegeIdxs.push_back(pRoomPlayer->getIdx()) ;
			 }
		}
	}
	return vAllPrivilegeIdxs.empty() == false ;
}

bool CNiuNiuRoom::getPlayersWillBetPlayer(std::vector<uint8_t>& vWillBetIdx)
{
	auto seactCnt = getSeatCount() ;
	for ( uint8_t nIdx = 0 ; nIdx < seactCnt ; ++nIdx )
	{
		if ( nIdx == getBankerIdx() )
		{
			continue; 
		}

		if ( getPlayerByIdx(nIdx) == nullptr )
		{
			continue; 
		}

		if ( getPlayerByIdx(nIdx)->isHaveState(eRoomPeer_CanAct) == false )
		{
			continue;
		}
		vWillBetIdx.push_back(nIdx) ;
	}
	return vWillBetIdx.empty() == false ;
}

bool CNiuNiuRoom::isHaveBanker()
{
	if ( m_nBankerIdx == (uint8_t)-1 )
	{
		return false ;
	}

	auto pBanker = getPlayerByIdx(m_nBankerIdx) ;
	if ( pBanker == nullptr )
	{
		m_nBankerIdx = -1 ;
		return false ;
	}
	return true ;
}

bool CNiuNiuRoom::canStartGame()
{
	if ( ISitableRoom::canStartGame() == false  )
	{
		return false ;
	}

	std::vector<uint8_t> v ;
	return getPlayersHaveGrabBankerPrivilege(v) ;
} 

void CNiuNiuRoom::sendResultToPlayerWhenDuringResultState(uint32_t nSessionID)
{
	if (getCurRoomState()->getStateID() == CNiuNiuRoomGameResultState::eStateID)
	{
		auto msgID = (stMsg*)m_tGameResult.getBufferPtr();
		sendMsgToPlayer((stMsg*)m_tGameResult.getBufferPtr(), m_tGameResult.getContentSize(), nSessionID);
		LOGFMTI("send result info to session = %u duiring result state msg id = %u, size = %u",nSessionID,msgID->usMsgType,m_tGameResult.getContentSize() );
	}
}

bool CNiuNiuRoom::onMessage( Json::Value& prealMsg ,uint16_t nMsgType, eMsgPort eSenderPort , uint32_t nSessionID  )
{
	if ( ISitableRoom::onMessage(prealMsg,nMsgType,eSenderPort,nSessionID) )
	{
		return true ;
	}

	switch ( nMsgType )
	{
	case MSG_REQ_RESIGN_BANKER:
		{
			auto pPlayer = getSitdownPlayerBySessionID(nSessionID) ;
			uint8_t nRet = 0 ;
			do
			{
				if ( !pPlayer || pPlayer->getIdx() != getBankerIdx() )
				{
					nRet = 1 ;
					break; 
				}

				if ( 2 != m_nResignBankerCtrl )
				{
					nRet = 2 ;
					break ;
				}

				m_isWillManualLeaveBanker = true ;
			}
			while(0);
			Json::Value jsmsgBack ;
			jsmsgBack["ret"] = nRet ;
			sendMsgToPlayer(nSessionID,jsmsgBack,nMsgType) ;
		}
		break ;
	default:
		return false ;
	}
	return true ;
}
bool CNiuNiuRoom::onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID )
{
	if (MSG_PLAYER_STANDUP == prealMsg->usMsgType)
	{
		stMsgPlayerStandUpRet msgBack;
		msgBack.nRet = 0;
		auto player = getSitdownPlayerBySessionID(nPlayerSessionID);
		if (player == nullptr)
		{
			msgBack.nRet = 1;
			sendMsgToPlayer(&msgBack, sizeof(msgBack), nPlayerSessionID);
			return true ;
		}
		
		if ( getBankerIdx() == player->getIdx())
		{
			msgBack.nRet = 3;
			sendMsgToPlayer(&msgBack, sizeof(msgBack), nPlayerSessionID);
			return true;
		}

		auto nState = getCurRoomState()->getStateID();
		if ( eRoomState_WaitOpen !=nState && nState != eRoomState_WaitJoin && (nState != eRoomState_WillClose || eRoomState_Close != nState))
		{
			msgBack.nRet = 4;
			sendMsgToPlayer(&msgBack, sizeof(msgBack), nPlayerSessionID);
			return true;
		}
		onPlayerWillStandUp(player);
		return true;
	}
	else if ( MSG_PLAYER_LEAVE_ROOM == prealMsg->usMsgType )
	{
		stMsgPlayerLeaveRoomRet msg;
		auto nState = getCurRoomState()->getStateID();
		auto p = getSitdownPlayerBySessionID(nPlayerSessionID);
		if ( p && eRoomState_WaitOpen != nState && nState != eRoomState_WaitJoin && (nState != eRoomState_WillClose || eRoomState_Close != nState))
		{
			msg.nRet = 2;
			sendMsgToPlayer(&msg, sizeof(msg), nPlayerSessionID);
			LOGFMTD("current room state will not allow you leave session id  = %u roomID = %u", nPlayerSessionID,getRoomID());
			return true;
		}
	}
	return ISitableRoom::onMessage(prealMsg,eSenderPort,nPlayerSessionID) ;
}