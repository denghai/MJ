#pragma once
#pragma once
#include "MJRoomStateWaitPlayerAct.h"
#include "CommonDefine.h"
#include "SZMJRoom.h"
#include "SZMJPlayerCard.h"
class SZRoomStateWaitPlayerAct
	:public MJRoomStateWaitPlayerAct
{
public:
	void enterState(IMJRoom* pmjRoom, Json::Value& jsTranData)override
	{
		IMJRoomState::enterState(pmjRoom, jsTranData);
		m_isAutoBuHuaOrHuaGang = false;
		m_nHuaCard = -1;
		setStateDuringTime(pmjRoom->isWaitPlayerActForever() ? 100000000 : eTime_WaitPlayerAct);
		if (jsTranData["idx"].isNull() == false && jsTranData["idx"].isUInt())
		{
			m_nIdx = jsTranData["idx"].asUInt();
			auto pPlayer = getRoom()->getMJPlayerByIdx(m_nIdx);
			if (pPlayer == nullptr)
			{
				LOGFMTE("room id = %u , player idx = %u  can not check hua bu ", getRoom()->getRoomID(), m_nIdx);
			}

			auto nNewCard = ((SZMJPlayerCard*)pPlayer->getPlayerCard())->getHuaCardToBuHua();
			if ((uint8_t)-1 != nNewCard)
			{
				m_nHuaCard = nNewCard;
				if (m_nHuaCard)
				{
					m_isAutoBuHuaOrHuaGang = true;
					setStateDuringTime(0.3); // one seconds later auto bu hua or hua gang 
					LOGFMTD("room id = %u , player idx = %u wait hua bu or gang card = %u", getRoom()->getRoomID(), m_nIdx, m_nHuaCard);
					return;
				}
			}

			getRoom()->onWaitPlayerAct(m_nIdx, m_isCanPass);

			// check tuo guan 
			getRoom()->onCheckTrusteeForWaitPlayerAct(m_nIdx, m_isCanPass);
			return;
		}
		assert(0 && "invalid argument");
	}

	void onStateTimeUp()override
	{
		if (!m_isAutoBuHuaOrHuaGang)
		{
			MJRoomStateWaitPlayerAct::onStateTimeUp();
			return;
		}

		if (getRoom()->isCanGoOnMoPai())
		{
			// auto do hua gang , peng 
			Json::Value jsAct;
			jsAct["actType"] = eMJAct_BuHua;
			jsAct["card"] = m_nHuaCard;
			auto p = getRoom()->getMJPlayerByIdx(m_nIdx);
			this->onMsg(jsAct, MSG_PLAYER_ACT, ID_MSG_PORT_CLIENT, p->getSessionID());
		}
		else
		{
			// game over ;
			getRoom()->goToState(eRoomState_GameEnd);
		}
	}

	bool onMsg(Json::Value& prealMsg, uint16_t nMsgType, eMsgPort eSenderPort, uint32_t nSessionID)override
	{
		if (MSG_PLAYER_ACT == nMsgType)
		{
			auto actType = prealMsg["actType"].asUInt();
			if (actType == eMJAct_HuaGang || eMJAct_BuHua == actType)
			{
				auto nCard = prealMsg["card"].asUInt();
				auto pPlayer = getRoom()->getMJPlayerBySessionID(nSessionID);
				if (pPlayer == nullptr)
				{
					LOGFMTE("player is nullptr , nSession id = %u", nSessionID);
					return true;
				}

				Json::Value jsTran;
				jsTran["idx"] = m_nIdx;
				jsTran["act"] = actType;
				jsTran["card"] = nCard;
				jsTran["invokeIdx"] = m_nIdx;
				getRoom()->goToState(eRoomState_DoPlayerAct, &jsTran);
				return true;
			}

			if (m_isAutoBuHuaOrHuaGang)
			{
				LOGFMTD("doing bu hua or hua gang , can not do other things ");
				return true;
			}
		}


		return MJRoomStateWaitPlayerAct::onMsg(prealMsg, nMsgType, eSenderPort, nSessionID);
	}

protected:
	bool m_isAutoBuHuaOrHuaGang;
	uint8_t m_nHuaCard;
};