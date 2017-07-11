#pragma once 
#include "IMJRoomState.h"
#include "log4z.h"
#include "MJRoomStateWaitPlayerChu.h"
#include "IMJPlayer.h"
#include "IMJPlayerCard.h"
#include <cassert>
#include "MJCard.h"
#include "CYMJPlayer.h"
class CYRoomStateWaitPlayerChu
	:public MJRoomStateWaitPlayerChu
{
public:
	void enterState(IMJRoom* pmjRoom, Json::Value& jsTranData)
	{
		IMJRoomState::enterState(pmjRoom, jsTranData);

		auto pRoom = (CYMJRoom*)getRoom();
		if (!pRoom->getModeLouBao())
		{
			setStateDuringTime(pmjRoom->isWaitPlayerActForever() ? 100000000 : eTime_WaitPlayerAct);
			if (jsTranData["idx"].isNull() == false && jsTranData["idx"].isUInt())
			{
				m_nIdx = jsTranData["idx"].asUInt();
				return;
			}
			assert(0 && "invalid argument");
			return;
		}

		if (jsTranData["idx"].isNull() == false && jsTranData["idx"].isUInt())
		{
			m_nIdx = jsTranData["idx"].asUInt();
		}
		else
		{
			assert(0 && "invalid argument");
			return;
		}

		auto player = (CYMJPlayer*)pmjRoom->getMJPlayerByIdx(m_nIdx);
		auto eTime_Wait = 100000000;
		if (player->GetTing())
		{
			eTime_Wait = eTime_TingedAutoChu;

			if (jsTranData["card"].isNull() == false && jsTranData["card"].isUInt())
			{
				m_nCard = jsTranData["card"].asUInt();
			}
			else
			{
				assert(0 && "invalid argument");
				return;
			}
			setStateDuringTime(pmjRoom->isWaitPlayerActForever() ? eTime_Wait * 0.5 : eTime_WaitPlayerAct);
		}
	}

	void onStateTimeUp()
	{
		auto pRoom = (CYMJRoom*)getRoom();
		if (!pRoom->getModeLouBao())
		{
			MJRoomStateWaitPlayerChu::onStateTimeUp();
			return;
		}

		assert(m_nCard != uint8_t(-1) && "tinged auto mo chu, card can not null!");

		Json::Value jsTran;
		jsTran["idx"] = m_nIdx;
		jsTran["act"] = eMJAct_Chu;
		jsTran["card"] = m_nCard;
		getRoom()->goToState(eRoomState_DoPlayerAct, &jsTran);
	}

	bool onMsg(Json::Value& prealMsg, uint16_t nMsgType, eMsgPort eSenderPort, uint32_t nSessionID)
	{
		if (MSG_PLAYER_ACT != nMsgType)
		{
			return false;
		}

		auto pRoom = (CYMJRoom*)getRoom();

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

			//auto nType = card_Type(nCard);
			//bool isHua = (eCT_Jian == nType || eCT_Hua == nType);
			//if (isHua)
			//{
			//	nRet = 4;
			//	LOGFMTE("can not chu hua = %u", nCard);
			//	break;
			//}

			if (pRoom->getModeLouBao() && ((CYMJPlayer*)pPlayer)->GetTing())
			{
				// 听牌之后自动出牌，不再接收玩家发来的出牌
				nRet = 4;
				LOGFMTE("already tinged, can not chu hua = %u", nCard);
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
