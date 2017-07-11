#pragma once
#include "MJRoomStateWaitPlayerAct.h"
#include "CommonDefine.h"
#include "HHMJRoom.h"
#include "HHMJPlayer.h"
#include "HHMJPlayerCard.h"
class HHRoomStateWaitPlayerAct
	:public MJRoomStateWaitPlayerAct
{
public:
	bool onMsg(Json::Value& prealMsg, uint16_t nMsgType, eMsgPort eSenderPort, uint32_t nSessionID)override
	{
		if (MSG_PLAYER_ACT == nMsgType)
		{
			auto actType = prealMsg["actType"].asUInt();
			auto nCard = prealMsg["card"].asUInt();
			auto nGangGetCard = prealMsg["gangGetCard"].asUInt();

			auto pRoom = (HHMJRoom*)getRoom();
			auto pPlayer = (HHMJPlayer*)pRoom->getMJPlayerBySessionID(nSessionID);
			
			uint8_t nRet = 0;
			do
			{
				if (pPlayer == nullptr)
				{
					LOGFMTE("you are not in this room how to set ready ? session id = %u", nSessionID);
					nRet = 4;
					break;
				}

				if (m_nIdx != pPlayer->getIdx())
				{
					nRet = 1;
					break;
				}

				auto pMJCard = (HHMJPlayerCard*)pPlayer->getPlayerCard();
				if (actType == eMJAct_AnGang)
				{
					if (!pMJCard->canAnGangWithCard(nCard) || (nGangGetCard != pRoom->getGangCard1() && nGangGetCard != pRoom->getGangCard2()))
					{
						nRet = 3;
					}
					else
					{
						pRoom->setGangCard(nGangGetCard);
					}
				}
				if (actType == eMJAct_BuGang) // eMJAct_BuGang_Declare
				{
					if (!pMJCard->canBuGangWithCard(nCard) || (nGangGetCard != pRoom->getGangCard1() && nGangGetCard != pRoom->getGangCard2()))
					{
						nRet = 3;
					}
					else
					{
						pRoom->setGangCard(nGangGetCard);
					}
				}
				if (eMJAct_Pass == actType)
				{
					if (pPlayer->getTangZhiHuPass())
					{
						Json::Value jsTran;
						jsTran["idx"] = getRoom()->getNextActPlayerIdx(m_nIdx);
						jsTran["act"] = eMJAct_Mo;
						getRoom()->goToState(eRoomState_DoPlayerAct, &jsTran);

						pPlayer->setTangZhiHuPass(false);
						return true;
					}
				}
			} while (0);

			if (nRet)
			{
				Json::Value jsRet;
				jsRet["ret"] = nRet;
				getRoom()->sendMsgToPlayer(jsRet, nMsgType, nSessionID);
				return true;
			}
		}
		
		if (MSG_REQ_ACT_LIST == nMsgType)
		{
			auto pPlayer = getRoom()->getMJPlayerBySessionID(nSessionID);
			if (pPlayer == nullptr)
			{
				LOGFMTE("you are not in room  why req act list");
				return false;
			}

			if (m_nIdx != pPlayer->getIdx())
			{
				LOGFMTD("you are not cur act player , so omit you message");
				return false;
			}

			if (m_isCanPass)  // means player need wait to do act chose ;
			{
				getRoom()->onWaitPlayerAct(m_nIdx, m_isCanPass);
			}
			return true;
		}

		if (MSG_PLAYER_ACT != nMsgType)
		{
			return false;
		}

		auto actType = prealMsg["actType"].asUInt();
		auto nCard = prealMsg["card"].asUInt();
		auto pPlayer = (HHMJPlayer*)getRoom()->getMJPlayerBySessionID(nSessionID);
		uint8_t nRet = 0;
		do
		{
			if (pPlayer == nullptr)
			{
				LOGFMTE("you are not in this room how to set ready ? session id = %u", nSessionID);
				nRet = 4;
				break;
			}

			if (m_nIdx != pPlayer->getIdx())
			{
				nRet = 1;
				break;
			}

			auto pMJCard = (HHMJPlayerCard*)pPlayer->getPlayerCard();
			switch (actType)
			{
			case eMJAct_Chu:
			{
				if (!pMJCard->isHaveCard(nCard))
				{
					nRet = 3;
				}
			}
			break;
			case eMJAct_AnGang:
			{
				if (!pMJCard->canAnGangWithCard(nCard))
				{
					nRet = 3;
				}
			}
			break;
			case eMJAct_BuGang:
			case eMJAct_BuGang_Declare:
			{
				if (!pMJCard->canBuGangWithCard(nCard))
				{
					nRet = 3;
				}
			}
			break;
			case eMJAct_Hu:
			{
				if (!pMJCard->isHoldCardCanHu())
				{
					if (pPlayer->getTangZhiHuPass())
					{
						if (!pMJCard->isTangZhiHu13() && !pMJCard->isTangZhiHu14())
						{
							nRet = 3;
						}
					}
					else
					{
						nRet = 3;
					}
				}
				nCard = pMJCard->getNewestFetchedCard();
			}
			break;
			case eMJAct_Pass:
				break;
			default:
				nRet = 2;
				break;
			}
		} while (0);

		if (nRet)
		{
			Json::Value jsRet;
			jsRet["ret"] = nRet;
			getRoom()->sendMsgToPlayer(jsRet, nMsgType, nSessionID);
			return true;
		}

		if (eMJAct_Pass == actType)
		{
			setStateDuringTime(getRoom()->isWaitPlayerActForever() ? 100000000 : eTime_WaitPlayerAct);
			return true;
		}

		// do transfer 
		Json::Value jsTran;
		jsTran["idx"] = m_nIdx;
		jsTran["act"] = actType;
		jsTran["card"] = nCard;
		jsTran["invokeIdx"] = m_nIdx;
		if (eMJAct_BuGang_Declare == actType || eMJAct_BuGang == actType)
		{
			pPlayer->signDecareBuGangFlag();
			if (getRoom()->isAnyPlayerRobotGang(m_nIdx, nCard))
			{
				getRoom()->goToState(eRoomState_AskForRobotGang, &jsTran);
				return true;
			}
		}
		getRoom()->goToState(eRoomState_DoPlayerAct, &jsTran);
		return true;
	}
};