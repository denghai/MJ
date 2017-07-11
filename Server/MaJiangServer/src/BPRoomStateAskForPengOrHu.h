#pragma once
#include "MJRoomStateAskForPengOrHu.h"
#include "log4z.h"
#include "BPMJRoom.h"
#include "BPMJPlayer.h"
#include "BPMJPlayerCard.h"
#include "CommonDefine.h"
#include "MJCard.h"
class BPRoomStateAskForPengOrHu
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

		auto pPlayer = (BPMJPlayer*)getRoom()->getMJPlayerBySessionID(nSessionID);
		auto pPlayerCard = (BPMJPlayerCard*)pPlayer->getPlayerCard();
		auto pRoom = (BPMJRoom*)getRoom();

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
		if (MSG_PLAYER_ACT == nMsgType && eMJAct_Peng == actType && m_nCard == pRoom->getHuiCard())
		{
			Json::Value jsRet;
			jsRet["ret"] = 2;
			getRoom()->sendMsgToPlayer(jsRet, nMsgType, nSessionID);
			return true;
		}
		if (MSG_PLAYER_ACT == nMsgType && eMJAct_Chi == actType)
		{
			int nRet = 0;
			if (prealMsg["eatWith"].isNull() || prealMsg["eatWith"].isArray() == false || prealMsg["eatWith"].size() != 2)
			{
				LOGFMTE("eat arg error");
				nRet = 3;
			}
			else
			{
				Json::Value jsE;
				jsE = prealMsg["eatWith"];
				if (m_nCard == pRoom->getHuiCard() || jsE[0u].asUInt() == pRoom->getHuiCard() || jsE[1u].asUInt() == pRoom->getHuiCard())
				{
					LOGFMTE("why you can not eat ? svr bug ");
					nRet = 2;
				}
			}
			if (nRet > 0)
			{
				Json::Value jsRet;
				jsRet["ret"] = nRet;
				getRoom()->sendMsgToPlayer(jsRet, nMsgType, nSessionID);
				return true;
			}
		}

		return MJRoomStateAskForPengOrHu::onMsg(prealMsg, nMsgType, eSenderPort, nSessionID);
	}
};
