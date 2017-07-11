#pragma once 
#include "IMJRoomState.h"
#include "log4z.h"
#include "IMJRoom.h"
#include "IMJPlayer.h"
#include <cassert>
#include "XLMJPlayerCard.h" 
class XLRoomStateWaitDecideQue
	:public IMJRoomState
{
public:
	uint32_t getStateID()final{ return eRoomState_WaitDecideQue; }
	void enterState(IMJRoom* pmjRoom, Json::Value& jsTranData)override
	{
		m_isDoingDingQue = false;
		IMJRoomState::enterState(pmjRoom, jsTranData);
		setStateDuringTime(eTime_WaitDecideQue);
		for ( uint8_t nIdx = 0; nIdx < pmjRoom->getSeatCnt(); ++nIdx)
		{
			vWaitIdx.push_back(nIdx);
		}

		Json::Value msg;
		pmjRoom->sendRoomMsg(msg, MSG_ROOM_WAIT_DECIDE_QUE);
	}

	void onStateTimeUp()override
	{
		if (m_isDoingDingQue) // doing ding que time up 
		{
			// go to wait banker do act ;
			Json::Value jsValue;
			jsValue["idx"] = getRoom()->getBankerIdx();
			getRoom()->goToState(eRoomState_WaitPlayerAct, &jsValue);
			return;
		}

		// waiting ding que time up 
		for (auto& nIdx : vWaitIdx)
		{
			auto pPlayer = getRoom()->getMJPlayerByIdx(nIdx);
			if (nullptr == pPlayer)
			{
				LOGFMTE("wait player decide que but player is null idx = %u",nIdx );
				continue;
			}
			auto pCard = (XLMJPlayerCard*)pPlayer->getPlayerCard();
			pCard->setQueType(pCard->getAutoQueType());
		}

		// do tell client all que type 
		m_isDoingDingQue = true;
		setStateDuringTime(eTime_DoDecideQue);
		// send msg ;
		Json::Value msg;
		Json::Value arrayType;
		for (uint8_t nIdx = 0; nIdx < getRoom()->getSeatCnt(); ++nIdx )
		{
			auto pPlayer = getRoom()->getMJPlayerByIdx(nIdx);
			if ( nullptr == pPlayer)
			{
				LOGFMTE("get player decide que but player is null idx = %u", nIdx);
				continue;
			}
			auto pCard = (XLMJPlayerCard*)pPlayer->getPlayerCard();

			Json::Value jsPlayer;
			jsPlayer["idx"] = nIdx;
			jsPlayer["type"] = pCard->getQueType();
			arrayType[arrayType.size()] = jsPlayer;
			LOGFMTD("idx = %u , que Type : %u", nIdx, pCard->getQueType() );
		}

		msg["ret"] = arrayType;
		getRoom()->sendRoomMsg(msg, MSG_ROOM_FINISH_DECIDE_QUE);
	}

	bool onMsg(Json::Value& prealMsg, uint16_t nMsgType, eMsgPort eSenderPort, uint32_t nSessionID)override
	{
		if (MSG_PLAYER_DECIDE_QUE != nMsgType)
		{
			return false;
		}

		auto pp = getRoom()->getMJPlayerBySessionID(nSessionID);
		if (pp == nullptr)
		{
			LOGFMTE("you are not in this room , can not ding que , room id = %u , session id = %u",getRoom()->getRoomID(),nSessionID);
			return true;
		}

		auto iter = std::find(vWaitIdx.begin(),vWaitIdx.end(),pp->getIdx());
		if (iter == vWaitIdx.end())
		{
			LOGFMTE("why you are not in this wait list ? you already ding que , room id = %u , sessionid = %u", getRoom()->getRoomID(), nSessionID);
			return true;
		}
		vWaitIdx.erase(iter);
		auto pCard = (XLMJPlayerCard*)pp->getPlayerCard();
		auto nQueType = prealMsg["type"].asUInt();
		if (nQueType <= eCT_None || nQueType > eCT_Tiao)
		{
			LOGFMTE("idx = %u ,decide invalid que type = %u", pp->getIdx(), nQueType);
			return true;
		}
		pCard->setQueType(nQueType);

		if (vWaitIdx.empty())  // all body decide que , so wait over ;
		{
			onStateTimeUp();
		}
		 
		return true;
	}
	uint8_t getCurIdx()override{ return 0; }
protected:
	std::vector<uint8_t> vWaitIdx;
	bool m_isDoingDingQue;
};



