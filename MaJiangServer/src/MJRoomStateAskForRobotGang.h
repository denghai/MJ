#pragma once 
#include "IMJRoomState.h"
#include "log4z.h"
#include "IMJRoom.h"
#include "IMJPlayer.h"
#include "IMJPlayerCard.h"
#include <cassert>
class MJRoomStateAskForRobotGang
	:public IMJRoomState
{
public:
	uint32_t getStateID()final{ return eRoomState_AskForRobotGang; }
	void enterState(IMJRoom* pmjRoom, Json::Value& jsTranData)override
	{
		IMJRoomState::enterState(pmjRoom, jsTranData);
		setStateDuringTime(pmjRoom->isWaitPlayerActForever() ? 100000000 : eTime_WaitPlayerAct);
		m_nInvokeIdx = jsTranData["invokeIdx"].asUInt();
		m_nCard = jsTranData["card"].asUInt();
		m_vWaitIdx.clear();
		getRoom()->onAskForRobotGang(m_nInvokeIdx, m_nCard, m_vWaitIdx);
		assert(m_vWaitIdx.empty() == false && "invalid argument");
		m_vDoHuIdx.clear();
		getRoom()->onCheckTrusteeForHuOtherPlayerCard(m_vWaitIdx, m_nCard);
	}

	void onStateTimeUp()override
	{
		Json::Value jsTran;
		jsTran["idx"] = m_nInvokeIdx;
		jsTran["act"] = eMJAct_BuGang;
		jsTran["card"] = m_nCard;
		getRoom()->goToState(eRoomState_DoPlayerAct, &jsTran);
	}

	void responeReqActList(uint32_t nSessionID)
	{
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

		auto iterPeng = std::find(m_vWaitIdx.begin(), m_vWaitIdx.end(), pPlayer->getIdx());
		// check peng 
		if (iterPeng != m_vWaitIdx.end())
		{
			jsActs[jsActs.size()] = eMJAct_Hu;
		}

		if (jsActs.empty())
		{
			LOGFMTE(" robot gang you are not in any wait list , so cannot resp you act list room id = %u , uid = %u", getRoom()->getRoomID(), pPlayer->getUID());
			return;
		}

		if (jsActs.size() > 0)
		{
			jsActs[jsActs.size()] = eMJAct_Pass;
		}

		jsMsg["acts"] = jsActs;
		getRoom()->sendMsgToPlayer(jsMsg, MSG_PLAYER_WAIT_ACT_ABOUT_OTHER_CARD, nSessionID);
		LOGFMTD(" resp act list inform uid = %u act about other card room id = %u card = %u", pPlayer->getUID(), getRoom()->getRoomID(), m_nCard);
	}

	bool onMsg(Json::Value& prealMsg, uint16_t nMsgType, eMsgPort eSenderPort, uint32_t nSessionID)override
	{
		if (MSG_PLAYER_ACT != nMsgType && MSG_REQ_ACT_LIST != nMsgType)
		{
			return false;
		}

		if (MSG_REQ_ACT_LIST == nMsgType)
		{
			responeReqActList(nSessionID);
			return true;
		}

		auto actType = prealMsg["actType"].asUInt();
		//auto nCard = prealMsg["card"].asUInt();
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

			auto iter = std::find(m_vWaitIdx.begin(),m_vWaitIdx.end(),pPlayer->getIdx());

			if ( iter == m_vWaitIdx.end() )
			{
				nRet = 1;
				break;
			}

			if ( eMJAct_Pass == actType)
			{
				break;
			}

			if (eMJAct_Hu != actType )
			{
				nRet = 2;
				break;
			}

			auto pMJCard = pPlayer->getPlayerCard();
			if (!pMJCard->canHuWitCard(m_nCard))
			{
				nRet = 2;
				LOGFMTE("why you can not hu ? svr bug ");
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

		auto iter = std::find(m_vWaitIdx.begin(), m_vWaitIdx.end(), pPlayer->getIdx());
		m_vWaitIdx.erase(iter);
		if ( eMJAct_Pass != actType)
		{
			m_vDoHuIdx.push_back(pPlayer->getIdx());
		}

		if ( m_vWaitIdx.empty() == false  )
		{
			return true ;
		}

		if (m_vDoHuIdx.empty())
		{
			LOGFMTD("every one give up robot gang");

			Json::Value jsTran;
			jsTran["idx"] = m_nInvokeIdx;
			jsTran["act"] = eMJAct_BuGang;
			jsTran["card"] = m_nCard;
			getRoom()->goToState(eRoomState_DoPlayerAct, &jsTran);
		}
		else
		{
			LOGFMTD("some body do robot gang");
			// do transfer 
			Json::Value jsTran;
			jsTran["idx"] = m_vDoHuIdx.front();
			jsTran["act"] = eMJAct_Hu;
			jsTran["card"] = m_nCard;
			jsTran["invokeIdx"] = m_nInvokeIdx;

			Json::Value jsHuIdx;
			for (auto& ref : m_vDoHuIdx)
			{
				jsHuIdx[jsHuIdx.size()] = ref;
			}
			jsTran["huIdxs"] = jsHuIdx;
			getRoom()->goToState(eRoomState_DoPlayerAct, &jsTran);
		}

		return true;
	}
	uint8_t getCurIdx()override{ return m_nInvokeIdx; }
protected:
	uint8_t m_nInvokeIdx;
	uint8_t m_nCard;
	std::vector<uint8_t> m_vWaitIdx;
	std::vector<uint8_t> m_vDoHuIdx;
};





