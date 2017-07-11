#pragma once
#include "MJRoomStateDoPlayerAct.h"
#include "log4z.h"
#include "HHMJRoom.h"
#include "HHMJPlayer.h"
#include "HHMJPlayerCard.h"
#include "CommonDefine.h"
#include "MJCard.h"
class HHRoomStateDoPlayerAct
	:public MJRoomStateDoPlayerAct
{
public:
	void doAct()override
	{
		auto pRoom = (HHMJRoom*)getRoom();
		auto player = (HHMJPlayer*)pRoom->getMJPlayerByIdx(m_nActIdx);
		auto pActCard = (HHMJPlayerCard*)player->getPlayerCard();

		if (m_eActType == eMJAct_Hu && (pActCard->isTangZhiHu13() || pActCard->isTangZhiHu14()))
		{
			pRoom->onPlayerTangZhiHu(m_nActIdx);
			return;
		}

		MJRoomStateDoPlayerAct::doAct();

		//if (m_eActType == eMJAct_Chu)
		//{
		//	//player->clearGangFlag();
		//	//pActCard->clearGangTimes();

		//	if (pRoom->getContinueFight())
		//	{
		//		pRoom->setLandLordCard(m_nActIdx, m_nCard);
		//		//斗地主
		//		if (m_nActIdx == ((pRoom->getBankerIdx() + 3) % 4))
		//		{
		//			pRoom->fightLandLord();
		//		}
		//	}
		//}

		if (m_eActType == eMJAct_Mo || m_eActType == eMJAct_Chi || m_eActType == eMJAct_Peng || m_eActType == eMJAct_MingGang)
		{
			pActCard->clearLouPeng();
			pActCard->setLouHu(false);
		}

		if (m_eActType == eMJAct_Chi || m_eActType == eMJAct_Peng || m_eActType == eMJAct_MingGang || 
			m_eActType == eMJAct_AnGang || m_eActType == eMJAct_BuGang)
		{
			pRoom->clearContinueFight();
			pActCard->setChiPengGangFlag();
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
		default:
			LOGFMTE("unknown act type = %u can not return act time", m_eActType);
			return 0;
		}
		return 0;
	}
	void onStateTimeUp()
	{
		auto pRoom = (HHMJRoom*)getRoom();
		switch (m_eActType)
		{
		case eMJAct_Chi:
		case eMJAct_Peng:
		{
			if (pRoom->canGang(m_nActIdx))
			{
				Json::Value jsValue;
				jsValue["idx"] = m_nActIdx;
				pRoom->goToState(eRoomState_WaitPlayerAct, &jsValue);
				return;
			}
			Json::Value jsValue;
			jsValue["idx"] = m_nActIdx;
			pRoom->goToState(eRoomState_WaitPlayerChu, &jsValue);
		}
		break;
		case eMJAct_Mo:
		case eMJAct_BuGang:
		case eMJAct_BuGang_Declare:
		case eMJAct_AnGang:
		case eMJAct_MingGang:
		{
			Json::Value jsValue;
			jsValue["idx"] = m_nActIdx;
			pRoom->goToState(eRoomState_WaitPlayerAct, &jsValue);
		}
		break;
		case eMJAct_Hu:
		{
			if (pRoom->isGameOver())
			{
				pRoom->goToState(eRoomState_GameEnd);
				return;
			}

			uint8_t nIdx = 0;
			for (auto& ref : m_vHuIdxs)
			{
				if (ref > nIdx)
				{
					nIdx = ref;
				}
			}

			do
			{
				nIdx = pRoom->getNextActPlayerIdx(nIdx);
				LOGFMTD("next act player should not in hu list, try next");
			} while (std::find(m_vHuIdxs.begin(), m_vHuIdxs.end(), nIdx) != m_vHuIdxs.end());


			// next player mo pai 
			doNextPlayerMoPai(nIdx);
		}
		break;
		case eMJAct_Chu:
		{
			auto pPlayer = (HHMJPlayer*)getRoom()->getMJPlayerByIdx(m_nActIdx);
			auto pPlayerCard = (HHMJPlayerCard*)pPlayer->getPlayerCard();
			if ((pPlayerCard->isTangZhiHu13() || pPlayerCard->isTangZhiHu14()) && pPlayer->getTangZhiHuPass() == false)
			{
				pPlayer->setTangZhiHuPass(true);

				Json::Value jsValue;
				jsValue["idx"] = m_nActIdx;
				pRoom->goToState(eRoomState_WaitPlayerAct, &jsValue);
				return;
			}
			pPlayer->setTangZhiHuPass(false);

			if (pRoom->isAnyPlayerPengOrHuThisCard(m_nActIdx, m_nCard))
			{
				Json::Value jsValue;
				jsValue["invokeIdx"] = m_nActIdx;
				jsValue["card"] = m_nCard;
				pRoom->goToState(eRoomState_AskForHuAndPeng, &jsValue);
				return;
			}
			else
			{
				if (pRoom->getContinueFight())
				{
					pRoom->setLandLordCard(m_nActIdx, m_nCard);
					//斗地主
					if (m_nActIdx == ((pRoom->getBankerIdx() + 3) % 4))
					{
						pRoom->fightLandLord();
					}
				}
			}

			if (pRoom->isGameOver())
			{
				pRoom->goToState(eRoomState_GameEnd);
				return;
			}

			{
				auto nIdx = pRoom->getNextActPlayerIdx(m_nActIdx);

				// next player mo pai 
				doNextPlayerMoPai(nIdx);
			}
			break;
		}
		default:
			break;
		}
	}
};
