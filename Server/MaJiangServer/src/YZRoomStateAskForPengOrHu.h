#pragma once
#include "MJRoomStateAskForPengOrHu.h"
#include "log4z.h"
#include "YZMJRoom.h"
#include "YZMJPlayer.h"
#include "YZMJPlayerCard.h"
#include "CommonDefine.h"
#include "MJCard.h"
class YZRoomStateAskForPengOrHu
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

		auto pRoom = (YZMJRoom*)getRoom();
		auto pPlayer = (YZMJPlayer*)pRoom->getMJPlayerBySessionID(nSessionID);
		auto pPlayerCard = (YZMJPlayerCard*)pPlayer->getPlayerCard();
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
					pPlayerCard->setLouHu(true, m_nCard);
				}
			}
		}

		return MJRoomStateAskForPengOrHu::onMsg(prealMsg, nMsgType, eSenderPort, nSessionID);
	}
};
