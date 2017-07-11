#pragma once
#include "MJRoomStateWaitPlayerAct.h"
#include "CommonDefine.h"
#include "JSMJRoom.h"
#include "JSMJPlayer.h"
#include "JSMJPlayerCard.h"
class JSRoomStateWaitPlayerAct
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

			auto pRoom = (JSMJRoom*)getRoom();
			auto pPlayer = (JSMJPlayer*)pRoom->getMJPlayerBySessionID(nSessionID);
			
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

				auto pMJCard = (JSMJPlayerCard*)pPlayer->getPlayerCard();
				if (actType == eMJAct_AnGang)
				{
					if (pRoom->get2ZhangDi())
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
					else
					{
						if (!pMJCard->canAnGangWithCard(nCard) || nGangGetCard != pRoom->getGangCard1())
						{
							nRet = 3;
						}
						else
						{
							pRoom->setGangCard(nGangGetCard);
						}
					}
				}
				if (actType == eMJAct_BuGang) // eMJAct_BuGang_Declare
				{
					if (pRoom->get2ZhangDi())
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
					else
					{
						if (!pMJCard->canBuGangWithCard(nCard) || nGangGetCard != pRoom->getGangCard1())
						{
							nRet = 3;
						}
						else
						{
							pRoom->setGangCard(nGangGetCard);
						}
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
		return MJRoomStateWaitPlayerAct::onMsg(prealMsg, nMsgType, eSenderPort, nSessionID);
	}
};