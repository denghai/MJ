#pragma once 
#include "IMJRoomState.h"
#include "IMJRoom.h"
#include "log4z.h"
#include "IMJPlayer.h"
class MJRoomStateDoPlayerAct
	:public IMJRoomState
{
public:
	uint32_t getStateID()final{ return eRoomState_DoPlayerAct; }
	void enterState(IMJRoom* pmjRoom, Json::Value& jsTranData)override
	{
		IMJRoomState::enterState(pmjRoom, jsTranData);
		m_nActIdx = jsTranData["idx"].asUInt();
		m_eActType = jsTranData["act"].asUInt();
		if (jsTranData["card"].isNull() == false)
		{
			m_nCard = jsTranData["card"].asUInt();
		}

		if (jsTranData["invokeIdx"].isNull() == false)
		{
			m_nInvokeIdx = jsTranData["invokeIdx"].asUInt();
		}

		if (jsTranData["eatWithA"].isNull() == false)
		{
			m_vEatWith[0] = jsTranData["eatWithA"].asUInt();
		}

		if (jsTranData["eatWithB"].isNull() == false)
		{
			m_vEatWith[1] = jsTranData["eatWithB"].asUInt();
		}

		m_vHuIdxs.clear();
		if (jsTranData["huIdxs"].isNull() == false && jsTranData["huIdxs"].isArray() )
		{
			auto jsA = jsTranData["huIdxs"];
			for (uint8_t nIdx = 0; nIdx < jsA.size(); ++nIdx)
			{
				m_vHuIdxs.push_back(jsA[nIdx].asUInt());
			}
			LOGFMTD("hu idx size = %u",m_vHuIdxs.size());
		}

		doAct();
		setStateDuringTime(getActTime());
	}

	void onStateTimeUp()
	{
		switch (m_eActType)
		{
		case eMJAct_Chi:
		case eMJAct_Peng:
		{
			Json::Value jsValue;
			jsValue["idx"] = m_nActIdx;
			getRoom()->goToState(eRoomState_WaitPlayerChu, &jsValue);
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
			getRoom()->goToState(eRoomState_WaitPlayerAct, &jsValue);
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

			do
			{
				nIdx = getRoom()->getNextActPlayerIdx(nIdx);
				LOGFMTD("next act player should not in hu list, try next");
			} while (std::find(m_vHuIdxs.begin(), m_vHuIdxs.end(), nIdx) != m_vHuIdxs.end());
			

			// next player mo pai 
			doNextPlayerMoPai(nIdx);
		}
			break;
		case eMJAct_Chu:
			if (getRoom()->isAnyPlayerPengOrHuThisCard(m_nActIdx, m_nCard))
			{
				Json::Value jsValue;
				jsValue["invokeIdx"] = m_nActIdx;
				jsValue["card"] = m_nCard;
				getRoom()->goToState(eRoomState_AskForHuAndPeng, &jsValue);
				return;
			}

			if (getRoom()->isGameOver())
			{
				getRoom()->goToState(eRoomState_GameEnd);
				return;
			}

			{
				auto nIdx = getRoom()->getNextActPlayerIdx(m_nActIdx);

			// next player mo pai 
				doNextPlayerMoPai(nIdx);
			}
			break;
		default:
			break;
		}
	}
protected:
	virtual void doAct()
	{
		switch (m_eActType)
		{
		case eMJAct_Mo:
			getRoom()->onPlayerMo(m_nActIdx);
			break;
		case eMJAct_Peng:
			getRoom()->onPlayerPeng(m_nActIdx,m_nCard,m_nInvokeIdx);
			break;
		case eMJAct_MingGang:
			getRoom()->onPlayerMingGang(m_nActIdx,m_nCard,m_nInvokeIdx);
			break;
		case eMJAct_AnGang:
			getRoom()->onPlayerAnGang(m_nActIdx, m_nCard);
			break;
		case eMJAct_BuGang:
			getRoom()->onPlayerBuGang(m_nActIdx,m_nCard);
			break;
		case eMJAct_Hu:
		{
			if (m_vHuIdxs.empty())
			{
				m_vHuIdxs.push_back(m_nActIdx);
			}
			getRoom()->onPlayerHu(m_vHuIdxs, m_nCard, m_nInvokeIdx);
		}
			break;
		case eMJAct_Chu:
			getRoom()->onPlayerChu(m_nActIdx, m_nCard);
			break;
		case eMJAct_Chi:
			if (m_vEatWith[0] * m_vEatWith[1] == 0)
			{
				LOGFMTE("eat lack of right card");
				break;
			}
			getRoom()->onPlayerEat(m_nActIdx,m_nCard,m_vEatWith[0],m_vEatWith[1],m_nInvokeIdx);
			break;
		default:
			LOGFMTE("unknow act  how to do it %u",m_eActType);
			break;
		}
	}

	virtual float getActTime()
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
			LOGFMTE("unknown act type = %u can not return act time",m_eActType);
			return 0 ;
		}
		return 0;
	}

	void doNextPlayerMoPai(uint8_t nIdx)
	{
		Json::Value jsTrans;
		jsTrans["act"] = eMJAct_Mo;
		jsTrans["idx"] = nIdx;
		enterState(getRoom(),jsTrans);
	}

	uint8_t getCurIdx()override{ return m_nInvokeIdx; }
protected:
	std::vector<uint8_t> m_vHuIdxs;
	uint8_t m_nActIdx;
	uint32_t m_eActType;
	uint8_t m_nCard;
	uint8_t m_nInvokeIdx;
	uint8_t m_vEatWith[2];
};

