#pragma once
#include "MJRoomStateDoPlayerAct.h"
#include "log4z.h"
#include "YZMJRoom.h"
#include "YZMJPlayer.h"
#include "YZMJPlayerCard.h"
#include "CommonDefine.h"
#include "MJCard.h"
class YZRoomStateDoPlayerAct
	:public MJRoomStateDoPlayerAct
{
public:
	void doAct()override
	{
		auto pRoom = (YZMJRoom*)getRoom();
		auto player = (YZMJPlayer*)pRoom->getMJPlayerByIdx(m_nActIdx);
		auto pActCard = (YZMJPlayerCard*)player->getPlayerCard();

		MJRoomStateDoPlayerAct::doAct();

		//if (m_eActType == eMJAct_Chu)
		//{
		//	player->clearGangFlag();
		//}

		if (m_eActType == eMJAct_Mo || m_eActType == eMJAct_Chi || m_eActType == eMJAct_Peng || m_eActType == eMJAct_MingGang)
		{
			pActCard->clearLouPeng();
			pActCard->setLouHu(false, uint8_t(-1));
		}
	}

	float getActTime()override
	{
		switch (m_eActType)
		{
		case eMJAct_Mo:
			return eTime_DoPlayerMoPai * 0.5;
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
			return eTime_DoPlayerActChuPai * 0.5;
		case eMJAct_Ting:
			return eTime_DoPlayerAct_Ting;
		case eMJAct_MoBaoDice:
			return eTime_MoBaoDice;
		default:
			LOGFMTE("unknown act type = %u can not return act time", m_eActType);
			return 0;
		}
		return 0;
	}
};
