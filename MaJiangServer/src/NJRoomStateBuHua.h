#pragma once
#include "IMJRoomState.h"
#include "CommonDefine.h"
#include "NJMJRoom.h"
#include "NJMJPlayerCard.h"
#include "IMJPlayer.h"
class NJRoomStateBuHua
	:public IMJRoomState
{
public:
	uint32_t getStateID()final{ return eRoomState_NJ_Auto_Buhua; }
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
		auto pRoom = (NJMJRoom*)getRoom();
		bool bIsDoBuHua = false;
		for (uint8_t nIdx = 0; nIdx < getRoom()->getSeatCnt(); ++nIdx)
		{
			auto pPlayer = pRoom->getMJPlayerByIdx(nIdx);
			auto nNewCard = ((NJMJPlayerCard*)pPlayer->getPlayerCard())->getHuaCardToBuHua();
			if ((uint8_t)-1 == nNewCard)
			{
				continue;
			}

			bIsDoBuHua = true;
			if (pRoom->canPlayerCardHuaGang(nIdx, nNewCard))
			{
				pRoom->onPlayerHuaGang(nIdx, nNewCard );
			}
			else
			{
				pRoom->onPlayerBuHua(nIdx, nNewCard );
			}
		}
		return bIsDoBuHua;
	}
};