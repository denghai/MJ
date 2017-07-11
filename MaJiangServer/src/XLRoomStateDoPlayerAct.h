#pragma once
#include "MJRoomStateDoPlayerAct.h"
#include "XLMJRoom.h"
class XLRoomStateDoPlayerAct
	:public MJRoomStateDoPlayerAct
{
public:
	void onStateTimeUp()override
	{
		std::vector<uint8_t> vWaitSupplyIdxs;

		uint32_t nRoomStateAfterSupplyCoin = 0;
		Json::Value jsNextStateArg;

		auto pXLRoom = (XLMJRoom*)getRoom();
		switch (m_eActType)
		{
		case eMJAct_MingGang:
		{
			if (pXLRoom->getWaitSupplyCoinPlayerIdxs(vWaitSupplyIdxs))
			{
				jsNextStateArg["idx"] = m_nActIdx;
				nRoomStateAfterSupplyCoin = eRoomState_WaitPlayerChu;
			}
		}
		break;
		case eMJAct_BuGang:
		case eMJAct_BuGang_Declare:
		case eMJAct_AnGang:
		{
			if (pXLRoom->getWaitSupplyCoinPlayerIdxs(vWaitSupplyIdxs))
			{
				jsNextStateArg["idx"] = m_nActIdx;
				nRoomStateAfterSupplyCoin = eRoomState_WaitPlayerAct;
			}
		}
		break;
		case eMJAct_Hu:
		{
			if (getRoom()->isGameOver())
			{
				getRoom()->goToState(eRoomState_GameEnd);
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

			if (m_vHuIdxs.size() == getRoom()->getSeatCnt())
			{
				LOGFMTE("very very big big error , why all player are hu ? room id = %u",getRoom()->getRoomID() );
				// avoid next step forever loop ;
				auto nFirst = m_vHuIdxs.front();
				m_vHuIdxs.clear();
				m_vHuIdxs.push_back(nFirst);
			}

			do
			{
				nIdx = getRoom()->getNextActPlayerIdx(nIdx);
				LOGFMTD("next act player should not in hu list, try next");
			} while (std::find(m_vHuIdxs.begin(), m_vHuIdxs.end(), nIdx) != m_vHuIdxs.end());


			// next player mo pai 
			if (pXLRoom->getWaitSupplyCoinPlayerIdxs(vWaitSupplyIdxs))
			{
				jsNextStateArg["act"] = eMJAct_Mo;
				jsNextStateArg["idx"] = nIdx;
				nRoomStateAfterSupplyCoin = eRoomState_DoPlayerAct;
			}
		}
		break;
		default:
			break;
		}

		if (vWaitSupplyIdxs.empty()) // supper default process 
		{
			MJRoomStateDoPlayerAct::onStateTimeUp();
		}
		else  // go to wait supply state 
		{
			// go to state ;
			Json::Value jsTrans;
			jsTrans["nextState"] = nRoomStateAfterSupplyCoin;
			jsTrans["transData"] = jsNextStateArg;
			pXLRoom->goToState(eRoomState_WaitSupplyCoin, &jsTrans);
		}
	}

};