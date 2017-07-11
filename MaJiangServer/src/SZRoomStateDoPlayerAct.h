#pragma once
#include "MJRoomStateDoPlayerAct.h"
#include "log4z.h"
#include "SZMJRoom.h"
#include "CommonDefine.h"
#include "MJCard.h"
class SZRoomStateDoPlayerAct
	:public MJRoomStateDoPlayerAct
{
public:
	void doAct()override
	{
		switch (m_eActType)
		{
		case eMJAct_BuHua:
		{
			auto pRoom = (SZMJRoom*)getRoom();
			pRoom->onPlayerBuHua(m_nActIdx, m_nCard);
		}
		break;
		default:
			MJRoomStateDoPlayerAct::doAct();
			break;
		}
	}

	void onStateTimeUp()override
	{
		switch (m_eActType)
		{
		case eMJAct_BuHua:
		{
			Json::Value jsValue;
			jsValue["idx"] = m_nActIdx;
			getRoom()->goToState(eRoomState_WaitPlayerAct, &jsValue);
		}
		break;
		default:
			MJRoomStateDoPlayerAct::onStateTimeUp();
			break;
		}

	}

	float getActTime()override
	{
		switch (m_eActType)
		{
		case eMJAct_Mo:
			return eTime_DoPlayerMoPai;
		case eMJAct_Peng:
		case eMJAct_Chi:
			return eTime_DoPlayerAct_Peng;
		case eMJAct_MingGang:
		case eMJAct_BuGang:
		case eMJAct_AnGang:
		case eMJAct_BuHua:
		case eMJAct_HuaGang:
			return eTime_DoPlayerAct_Gang;
		case eMJAct_Hu:
			return eTime_DoPlayerAct_Hu;
			break;
		case eMJAct_Chu:
			return eTime_DoPlayerActChuPai;
		default:
			LOGFMTE("unknown act type = %u can not return act time", m_eActType);
			return 0;
		}
		return 0;
	}
};