#pragma once
#include "IMJRoomState.h"
#include "CommonDefine.h"
#include "SZMJRoom.h"
#include "SZMJPlayerCard.h"
#include "IMJPlayer.h"
class SZRoomStateBuHua
	:public IMJRoomState
{
public:
	uint32_t getStateID()final { return eRoomState_NJ_Auto_Buhua; }
	void enterState(IMJRoom* pmjRoom, Json::Value& jsTranData)override
	{
		IMJRoomState::enterState(pmjRoom, jsTranData);
		setStateDuringTime(0);
	}

	void onStateTimeUp()override
	{
		if (!checkBuHuaAct())
		{
			Json::Value jsValue;
			jsValue["idx"] = getRoom()->getBankerIdx();
			getRoom()->goToState(eRoomState_WaitPlayerAct, &jsValue);
		}
		else
		{
			setStateDuringTime(0.2); // bu hua dong hua time ;
		}
	}

	bool checkBuHuaAct()
	{
		auto pRoom = (SZMJRoom*)getRoom();
		bool bIsDoBuHua = false;
		for (uint8_t nIdx = 0; nIdx < getRoom()->getSeatCnt(); ++nIdx)
		{
			auto pPlayer = pRoom->getMJPlayerByIdx(nIdx);
			auto nNewCard = ((SZMJPlayerCard*)pPlayer->getPlayerCard())->getHuaCardToBuHua();
			if ((uint8_t)-1 == nNewCard)
			{
				continue;
			}

			bIsDoBuHua = true;
			pRoom->onPlayerBuHua(nIdx, nNewCard);
		}
		return bIsDoBuHua;
	}
};