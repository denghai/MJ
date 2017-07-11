#pragma once
#include "MJRoomStateAskForPengOrHu.h"
#include "log4z.h"
#include "JSMJRoom.h"
#include "JSMJPlayer.h"
#include "JSMJPlayerCard.h"
#include "CommonDefine.h"
#include "MJCard.h"
class JSRoomStateAskForPengOrHu
	:public MJRoomStateAskForPengOrHu
{
	bool onMsg(Json::Value& prealMsg, uint16_t nMsgType, eMsgPort eSenderPort, uint32_t nSessionID)
	{
		auto actType = prealMsg["actType"].asUInt();

		if (MSG_REQ_ACT_LIST == nMsgType)
		{
			responeReqActList(nSessionID);
			return true;
		}

		auto pRoom = (JSMJRoom*)getRoom();
		auto pPlayer = (JSMJPlayer*)pRoom->getMJPlayerBySessionID(nSessionID);
		auto pPlayerCard = (JSMJPlayerCard*)pPlayer->getPlayerCard();
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
					pPlayerCard->setLouHu(true);
				}
			}
		}
		if (MSG_PLAYER_ACT == nMsgType && eMJAct_MingGang == actType)
		{
			auto actType = prealMsg["actType"].asUInt();
			auto nCard = prealMsg["card"].asUInt();
			auto nGangGetCard = prealMsg["gangGetCard"].asUInt();

			bool bCanMingGang = pPlayerCard->canMingGangWithCard(m_nCard);
			if (pRoom->get2ZhangDi())
			{
				if (!bCanMingGang || (nGangGetCard != pRoom->getGangCard1() && nGangGetCard != pRoom->getGangCard2()))
				{
					Json::Value jsRet;
					jsRet["ret"] = 2;
					getRoom()->sendMsgToPlayer(jsRet, nMsgType, nSessionID);

					LOGFMTE("why you can not ming gang ? svr bug ");

					return true;
				}
			} 
			else
			{
				if (!bCanMingGang || nGangGetCard != pRoom->getGangCard1())
				{
					Json::Value jsRet;
					jsRet["ret"] = 2;
					getRoom()->sendMsgToPlayer(jsRet, nMsgType, nSessionID);

					LOGFMTE("why you can not ming gang ? svr bug ");

					return true;
				}
			}
			
			
			if (bCanMingGang)
			{
				pRoom->setGangCard(nGangGetCard);
			}
		}

		return MJRoomStateAskForPengOrHu::onMsg(prealMsg, nMsgType, eSenderPort, nSessionID);
	}
};
