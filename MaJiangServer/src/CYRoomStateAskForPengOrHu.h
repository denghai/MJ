#pragma once
#include "MJRoomStateAskForPengOrHu.h"
#include "log4z.h"
#include "CYMJRoom.h"
#include "CYMJPlayer.h"
#include "CYMJPlayerCard.h"
#include "CommonDefine.h"
#include "MJCard.h"
class CYRoomStateAskForPengOrHu
	:public MJRoomStateAskForPengOrHu
{
	void responeReqActList(uint32_t nSessionID)
	{
		auto pRoom = (CYMJRoom*)getRoom();
		if (!pRoom->getModeLouBao())
		{
			MJRoomStateAskForPengOrHu::responeReqActList(nSessionID);
			return;
		}

		auto pPlayer = getRoom()->getMJPlayerBySessionID(nSessionID);
		if (!pPlayer)
		{
			LOGFMTE("you are  not in room id = %u , session id = %u , can not send you act list", getRoom()->getRoomID(), nSessionID);
			return;
		}
		Json::Value jsMsg;
		jsMsg["invokerIdx"] = m_nInvokeIdx;
		jsMsg["cardNum"] = m_nCard;

		Json::Value jsActs;

		auto iterPeng = std::find(m_vWaitPengGangIdx.begin(), m_vWaitPengGangIdx.end(), pPlayer->getIdx());
		// check peng 
		if (iterPeng != m_vWaitPengGangIdx.end())
		{
			jsActs[jsActs.size()] = eMJAct_Peng;
		}

		// check ming gang 
		if (getRoom()->isCanGoOnMoPai() && pPlayer->getPlayerCard()->canMingGangWithCard(m_nCard) && ((CYMJPlayer*)pPlayer)->checkTingedCanMingGang(m_nCard))
		{
			jsActs[jsActs.size()] = eMJAct_MingGang;
			// already add in peng ;  vWaitPengGangIdx
		}

		if (pPlayer->getIdx() == (m_nInvokeIdx + 1) % getRoom()->getSeatCnt() && m_isNeedWaitEat)
		{
			jsActs[jsActs.size()] = eMJAct_Chi;
		}

		// check hu ;
		auto iterHu = std::find(m_vWaitHuIdx.begin(), m_vWaitHuIdx.end(), pPlayer->getIdx());
		if (iterHu != m_vWaitHuIdx.end())
		{
			jsActs[jsActs.size()] = eMJAct_Hu;
		}

		if (jsActs.empty())
		{
			LOGFMTE("you are not in any wait list , so cannot resp you act list room id = %u , uid = %u", getRoom()->getRoomID(), pPlayer->getUID());
			return;
		}

		if (jsActs.size() > 0)
		{
			jsActs[jsActs.size()] = eMJAct_Pass;
		}

		jsMsg["acts"] = jsActs;
		getRoom()->sendMsgToPlayer(jsMsg, MSG_PLAYER_WAIT_ACT_ABOUT_OTHER_CARD, nSessionID);
		LOGFMTD("respon act list inform uid = %u act about other card room id = %u card = %u", pPlayer->getUID(), getRoom()->getRoomID(), m_nCard);
	}

	bool onMsg(Json::Value& prealMsg, uint16_t nMsgType, eMsgPort eSenderPort, uint32_t nSessionID)
	{
		auto actType = prealMsg["actType"].asUInt();

		if (MSG_REQ_ACT_LIST == nMsgType)
		{
			responeReqActList(nSessionID);
			return true;
		}

		auto pPlayer = (CYMJPlayer*)getRoom()->getMJPlayerBySessionID(nSessionID);
		auto pPlayerCard = (CYMJPlayerCard*)pPlayer->getPlayerCard();
		auto pRoom = (CYMJRoom*)getRoom();

		if (MSG_PLAYER_ACT == nMsgType && eMJAct_Pass == actType)
		{
			if (m_vWaitPengGangIdx.size() > 0)
			{
				pPlayerCard->setLouPeng(m_nCard);
			}

			if (m_vWaitHuIdx.size() > 0)
			{
				if (pPlayer->getIdx() != m_nInvokeIdx)
				{
					if (pRoom->getModeLouBao())
					{
						if (pPlayerCard->getBaoCard() != m_nCard)
						{
							pPlayerCard->setLouHu(true);
						}
					} 
					else
					{
						pPlayerCard->setLouHu(true);
					}
				}
			}
		}
		if (pRoom->getModeLouBao() && MSG_PLAYER_ACT == nMsgType && eMJAct_Pass != actType && eMJAct_Hu != actType)
		{
			pPlayerCard->setKanBao(uint8_t(-1));
		}

		return MJRoomStateAskForPengOrHu::onMsg(prealMsg, nMsgType, eSenderPort, nSessionID);
	}

	void onStateTimeUp()override
	{
		auto pRoom = (CYMJRoom*)getRoom();
		if (!pRoom->getModeLouBao())
		{
			MJRoomStateAskForPengOrHu::onStateTimeUp();
			return;
		}

		if (getRoom()->isGameOver())
		{
			getRoom()->goToState(eRoomState_GameEnd);
			return;
		}

		if (doAct())  // some body time out ,but have player do act ; 
		{
			return;
		}


		auto player = (CYMJPlayer*)getRoom()->getMJPlayerByIdx(m_nInvokeIdx);
		uint8_t nGangType = player->getTingGangType();
		if (player->GetTing() && (nGangType == eMJAct_MingGang || nGangType == eMJAct_AnGang || 
			nGangType == eMJAct_BuGang || nGangType == eMJAct_Hu))
		{
			player->setTingGangType(eMJAct_Max);

			auto playerCard = (CYMJPlayerCard*)player->getPlayerCard();
			auto nKanBao = playerCard->getKanBao();
			playerCard->setKanBao(uint8_t(-1));

			if (nKanBao != uint8_t(-1))
			{
				playerCard->setBaoCard(nKanBao);
			}
			if (nKanBao == uint8_t(-1) && nGangType == eMJAct_Hu)
			{
				Json::Value jsTran;
				jsTran["idx"] = getRoom()->getNextActPlayerIdx(m_nInvokeIdx);
				jsTran["act"] = eMJAct_Mo;
				getRoom()->goToState(eRoomState_DoPlayerAct, &jsTran);
				return;
			}
			//if (nGangType == eMJAct_Hu)
			//{
			//	
			//	auto nRoomBaoCard = ((CYMJRoom*)getRoom())->getBaoCard();
			//	playerCard->setBaoCard(nRoomBaoCard);
			//}
			
			Json::Value jsTran;
			jsTran["idx"] = m_nInvokeIdx;
			jsTran["act"] = eMJAct_Mo;
			getRoom()->goToState(eRoomState_DoPlayerAct, &jsTran);
			return;
		}

		Json::Value jsTran;
		jsTran["idx"] = getRoom()->getNextActPlayerIdx(m_nInvokeIdx);
		jsTran["act"] = eMJAct_Mo;
		getRoom()->goToState(eRoomState_DoPlayerAct, &jsTran);
	}
};
