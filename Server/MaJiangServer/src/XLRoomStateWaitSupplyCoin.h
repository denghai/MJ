#pragma once
#include "IMJRoomState.h"
#include "XLMJRoom.h"
#include "ServerMessageDefine.h"
#include "MJPlayer.h"
#include "log4z.h"
class XLRoomStateWaitSupplyCoin
	:public IMJRoomState
{
public:
	uint32_t getStateID()final{ return eRoomState_WaitSupplyCoin; }

	void enterState(IMJRoom* pmjRoom, Json::Value& jsTranData)override
	{
		IMJRoomState::enterState(pmjRoom, jsTranData);
		setStateDuringTime(eTime_WaitSupplyCoin);

		XLMJRoom* pRoom = (XLMJRoom*)pmjRoom;
		pRoom->getWaitSupplyCoinPlayerIdxs(vWaitIdx);

		nTargetState = jsTranData["nextState"].asUInt();
		jsTransArg = jsTranData["transData"];
		pRoom->infoPlayerSupplyCoin(vWaitIdx);
	}

	bool onMsg(Json::Value& prealMsg, uint16_t nMsgType, eMsgPort eSenderPort, uint32_t nSessionID)override
	{
		if (MSG_PLAYER_DECIDE_LOSE == nMsgType)
		{
			auto pp = getRoom()->getMJPlayerBySessionID(nSessionID);
			if (nullptr == pp)
			{
				LOGFMTE("you may not sit down in room id = %u , so can not decide lose session id = %u",getRoom()->getRoomID(),nSessionID);
				return true;
			}

			auto removeIter = std::find(vWaitIdx.begin(), vWaitIdx.end(), pp->getIdx());
			if (removeIter != vWaitIdx.end())
			{
				vWaitIdx.erase(removeIter);
			}
			else
			{
				LOGFMTE("you are not wait supply coin ,so skip you idx = %u room id = %u",pp->getIdx(),getRoom()->getRoomID());
				return true;
			}
			pp->setState(eRoomPeer_DecideLose);

			Json::Value jsmsg;
			jsmsg["playerIdx"] = pp->getIdx();
			jsmsg["result"] = 1;
			getRoom()->sendRoomMsg(jsmsg, MSG_ROOM_PLAYER_SUPPLY_COIN_RESULT);

			if (vWaitIdx.empty())
			{
				onStateTimeUp();
			}
			return true;
		}
		return false;
	}

	bool onMessage(stMsg* prealMsg, eMsgPort eSenderPort, uint32_t nPlayerSessionID)override
	{
		if (MSG_SYNC_IN_GAME_ADD_COIN == prealMsg->usMsgType)
		{
			stMsgSyncInGameCoin* pRet = (stMsgSyncInGameCoin*)prealMsg;
			auto pSitPlayer = getRoom()->getMJPlayerBySessionID(nPlayerSessionID);

			// is sitdown layer 
			if (!pSitPlayer)
			{
				LOGFMTD("you are not sit down player room id = %u, session id = %u", getRoom()->getRoomID(),nPlayerSessionID);
				return true;
			}

			pSitPlayer->onRecievedSupplyCoin(pRet->nAddCoin);
			// is in waiting list ;
			auto pW = std::find(vWaitIdx.begin(), vWaitIdx.end(), pSitPlayer->getIdx());
			if (pW == vWaitIdx.end())
			{
				LOGFMTD("you are not in the wait list ");
				return true;
			}

			// inform room players ;
			if (pSitPlayer->getCoin() >= (int32_t)getRoom()->getCoinNeedToSitDown())
			{
				vWaitIdx.erase(pW);

				Json::Value jsmsg;
				jsmsg["playerIdx"] = pSitPlayer->getIdx();
				jsmsg["result"] = 1;
				getRoom()->sendRoomMsg(jsmsg, MSG_ROOM_PLAYER_SUPPLY_COIN_RESULT);
				LOGFMTD("uid = %u supply coin ok", pSitPlayer->getUID());
			}
			else
			{
				LOGFMTD("uid = %u supply coin is not enough , go on waiting to supply coin reset wait time", pSitPlayer->getUID());
				std::vector<uint8_t> vTmp;
				vTmp.push_back(pSitPlayer->getIdx());
				XLMJRoom* pRoom = (XLMJRoom*)getRoom();
				pRoom->infoPlayerSupplyCoin(vTmp);
				setStateDuringTime(eTime_WaitSupplyCoin);
			}

			if (vWaitIdx.empty())
			{
				onStateTimeUp();
			}

			return true;
		}
		return false;
	}

	void onStateTimeUp()override
	{
		// wait time out auto decide lose 
		for (auto& ref : vWaitIdx)
		{
			auto pSitPlayer = getRoom()->getMJPlayerByIdx(ref);
			// is sitdown layer 
			if (!pSitPlayer)
			{
				LOGFMTE("idx = %u auto decide lose but is null ptr",ref);
				continue;
			}
			pSitPlayer->setState(eRoomPeer_DecideLose);
			LOGFMTD("uid = %u auto decide lose", pSitPlayer->getUID());
		}

		if (getRoom()->isGameOver())
		{
			getRoom()->goToState(eRoomState_GameEnd);
			return;
		}

		// if next act player decide lose then must chage another one 
		if (nTargetState == eRoomState_DoPlayerAct)
		{
			auto eAct = jsTransArg["act"].asUInt(); // = eMJAct_Mo;
			auto nIdx = jsTransArg["idx"].asUInt(); // = nIdx;
			if (eMJAct_Mo == eAct)
			{
				auto pPlayer = getRoom()->getMJPlayerByIdx(nIdx);
				if (!pPlayer)
				{
					LOGFMTE("room id = %u next player is nullptr , why ? idx = %u",getRoom()->getRoomID(),nIdx);
				}
				else
				{
					if (pPlayer->haveState(eRoomPeer_DecideLose))
					{
						jsTransArg["idx"] = getRoom()->getNextActPlayerIdx(nIdx);
						LOGFMTD("room id = %u , idx = %u decide lose ,so next should skip to next idx = %u", getRoom()->getRoomID(), nIdx, jsTransArg["idx"].asUInt());
					}
				}
			}
		}
		getRoom()->goToState(nTargetState, &jsTransArg);
	}
protected:
	std::vector<uint8_t> vWaitIdx;
	uint32_t nTargetState;
	Json::Value jsTransArg;
};