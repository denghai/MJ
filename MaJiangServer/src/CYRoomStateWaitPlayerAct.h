#pragma once
#include "MJRoomStateWaitPlayerAct.h"
#include "CommonDefine.h"
#include "CYMJRoom.h"
#include "CYMJPlayer.h"
#include "CYMJPlayerCard.h"
class CYRoomStateWaitPlayerAct
	:public MJRoomStateWaitPlayerAct
{
public:
	void enterState(IMJRoom* pmjRoom, Json::Value& jsTranData)override
	{
		MJRoomStateWaitPlayerAct::enterState(pmjRoom, jsTranData);
		m_isTing = false;
	}

	bool onMsg(Json::Value& prealMsg, uint16_t nMsgType, eMsgPort eSenderPort, uint32_t nSessionID)override
	{
		auto pRoom = (CYMJRoom*)getRoom();
		if (!pRoom->getModeLouBao())
		{
			return MJRoomStateWaitPlayerAct::onMsg(prealMsg, nMsgType, eSenderPort, nSessionID);
		}

		if (MSG_PLAYER_ACT == nMsgType)
		{
			auto actType = prealMsg["actType"].asUInt();
			auto nCard = prealMsg["card"].asUInt();
			auto pPlayer = (CYMJPlayer*)getRoom()->getMJPlayerBySessionID(nSessionID);
			if (pPlayer == nullptr)
			{
				LOGFMTE("player is nullptr , nSession id = %u", nSessionID);
				return true;
			}

			if (actType == eMJAct_Ting)
			{
				m_isTing = true;
				return true;
			}

			auto pPlayerCard = (CYMJPlayerCard*)pPlayer->getPlayerCard();
			if (actType == eMJAct_Chu && pPlayerCard->isHaveCard(nCard) && m_isTing)
			{
				//Json::Value jsRet;
				//jsRet["ret"] = 3;
				//LOGFMTE("why chu this card")
				//getRoom()->sendMsgToPlayer(jsRet, nMsgType, nSessionID);
				//return true;

				pPlayer->TingPai();

				/*Json::Value msg;
				msg["idx"] = pPlayer->getIdx();
				msg["actType"] = eMJAct_Ting;

				getRoom()->sendRoomMsg(msg, MSG_ROOM_ACT);*/

				((CYMJRoom*)getRoom())->onPlayerTing(pPlayer->getIdx());

				m_isTing = false;
			}
			
			if (actType == eMJAct_Pass)
			{
				// 如果这个人听了，可胡、杠牌时，过牌则自动再打前一张牌
				if (pPlayer->GetTing() && m_nIdx == pPlayer->getIdx())
				{
					auto playerCard = (CYMJPlayerCard*)pPlayer->getPlayerCard();
					auto nKanBao = playerCard->getKanBao();
					auto eTingGangType = pPlayer->getTingGangType();
					if (eMJAct_AnGang == eTingGangType || eMJAct_BuGang == eTingGangType || eMJAct_Hu == eTingGangType)
					{
						pPlayer->setTingGangType(eMJAct_Max);
						auto nRoomBaoCard = ((CYMJRoom*)getRoom())->getBaoCard();

						//if (eTingGangType == eMJAct_Hu)
						//{
						//	playerCard->setBaoCard(nRoomBaoCard);
						//}
						//else
						//{
						//	playerCard->pickoutHoldCard(nRoomBaoCard);
						//}
						
						if (nKanBao != uint8_t(-1))
						{
							playerCard->setBaoCard(nKanBao);
						}
						if (eTingGangType != eMJAct_Hu)
						{
							playerCard->pickoutHoldCard(nRoomBaoCard);
						}

						pPlayerCard->setKanBao(uint8_t(-1));

						Json::Value jsTran;
						jsTran["idx"] = m_nIdx;
						jsTran["act"] = eMJAct_Mo;
						getRoom()->goToState(eRoomState_DoPlayerAct, &jsTran);
						return true;
					}
					else
					{
						pPlayerCard->setKanBao(uint8_t(-1));

						Json::Value jsValue;
						jsValue["idx"] = pPlayer->getIdx();
						jsValue["card"] = pPlayerCard->getNewestFetchedCard();
						getRoom()->goToState(eRoomState_WaitPlayerChu, &jsValue);
						return true;
					}
				}
			}
			
			auto nKanBao = pPlayerCard->getKanBao();
			pPlayerCard->setKanBao(uint8_t(-1));

			if (eMJAct_Hu == actType && pPlayerCard->isHoldCardCanHu() && nKanBao != uint8_t(-1))
			{
				// do transfer 
				Json::Value jsTran;
				jsTran["idx"] = m_nIdx;
				jsTran["act"] = actType;
				jsTran["card"] = nKanBao;
				jsTran["invokeIdx"] = m_nIdx;
				getRoom()->goToState(eRoomState_DoPlayerAct, &jsTran);
				return true;
			}
		}

		return MJRoomStateWaitPlayerAct::onMsg(prealMsg, nMsgType, eSenderPort, nSessionID);
	}

	void onStateTimeUp()override
	{
		auto pRoom = (CYMJRoom*)getRoom();
		if (!pRoom->getModeLouBao())
		{
			MJRoomStateWaitPlayerAct::onStateTimeUp();
			return;
		}

		if (m_isCanPass)
		{
			m_isCanPass = false;
			setStateDuringTime(eTime_WaitPlayerAct);
			return;
		}
		//////////
		auto pPlayer = (CYMJPlayer*)getRoom()->getMJPlayerByIdx(m_nIdx);
		auto eTingGangType = pPlayer->getTingGangType();

		if (pPlayer->GetTing() && (eMJAct_AnGang == eTingGangType || eMJAct_BuGang == eTingGangType 
			|| eMJAct_Hu == eTingGangType))
		{
			pPlayer->setTingGangType(eMJAct_Max);

			auto playerCard = (CYMJPlayerCard*)pPlayer->getPlayerCard();
			auto nRoomBaoCard = ((CYMJRoom*)getRoom())->getBaoCard();

			//if (eTingGangType == eMJAct_Hu)
			//{
			//	playerCard->setBaoCard(nRoomBaoCard);
			//}
			//else
			//{
			//	playerCard->pickoutHoldCard(nRoomBaoCard);
			//}
			auto nKanBao = playerCard->getKanBao();
			if (nKanBao != uint8_t(-1))
			{
				playerCard->setBaoCard(nKanBao);
			}
			if (eTingGangType != eMJAct_Hu)
			{
				playerCard->pickoutHoldCard(nRoomBaoCard);
			} 

			Json::Value jsTran;
			jsTran["idx"] = m_nIdx;
			jsTran["act"] = eMJAct_Mo;
			getRoom()->goToState(eRoomState_DoPlayerAct, &jsTran);
		} 
		else
		{
			auto nCard = getRoom()->getAutoChuCardWhenWaitActTimeout(m_nIdx);
			LOGFMTE("wait time out , auto chu card = %u idx = %u", nCard, m_nIdx);
			Json::Value jsTran;
			jsTran["idx"] = m_nIdx;
			jsTran["act"] = eMJAct_Chu;
			jsTran["card"] = nCard;
			getRoom()->goToState(eRoomState_DoPlayerAct, &jsTran);
		}

		// go to tuo guan zhuang tai 
		if (!pPlayer || pPlayer->haveState(eRoomPeer_AlreadyHu) == false)
		{
			getRoom()->onPlayerTrusteedStateChange(m_nIdx, true);
		}
	}

private:
	bool m_isTing;
};