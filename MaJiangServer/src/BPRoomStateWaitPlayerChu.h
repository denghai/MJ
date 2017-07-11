#pragma once 
#include "IMJRoomState.h"
#include "log4z.h"
#include "MJRoomStateWaitPlayerChu.h"
#include "IMJPlayer.h"
#include "IMJPlayerCard.h"
#include <cassert>
#include "MJCard.h"
#include "BPMJPlayer.h"
class BPRoomStateWaitPlayerChu
	:public MJRoomStateWaitPlayerChu
{
public:
	void enterState(IMJRoom* pmjRoom, Json::Value& jsTranData)
	{
		IMJRoomState::enterState(pmjRoom, jsTranData);

		setStateDuringTime(pmjRoom->isWaitPlayerActForever() ? 100000000 : eTime_WaitPlayerAct);
		if (jsTranData["idx"].isNull() == false && jsTranData["idx"].isUInt())
		{
			m_nIdx = jsTranData["idx"].asUInt();
			return;
		}
		assert(0 && "invalid argument");
	}

	bool onMsg(Json::Value& prealMsg, uint16_t nMsgType, eMsgPort eSenderPort, uint32_t nSessionID)
	{
		if (MSG_PLAYER_ACT != nMsgType)
		{
			return false;
		}

		auto pRoom = (BPMJRoom*)getRoom();

		auto actType = prealMsg["actType"].asUInt();
		auto nCard = prealMsg["card"].asUInt();
		auto pPlayer = getRoom()->getMJPlayerBySessionID(nSessionID);
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

			if (eMJAct_Chu != actType)
			{
				nRet = 2;
				break;
			}

			auto pMJCard = pPlayer->getPlayerCard();
			if (!pMJCard->isHaveCard(nCard))
			{
				nRet = 3;
				break;
			}

			auto nHuiCard = pRoom->getHuiCard();
			if (nCard == nHuiCard)
			{
				nRet = 4;
				LOGFMTE("can not chu hui card = %u", nCard);
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

		// do transfer 
		Json::Value jsTran;
		jsTran["idx"] = m_nIdx;
		jsTran["act"] = actType;
		jsTran["card"] = nCard;
		jsTran["invokeIdx"] = m_nIdx;
		getRoom()->goToState(eRoomState_DoPlayerAct, &jsTran);
		return true;
	}
protected:
	uint8_t m_nCard;
};
