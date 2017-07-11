#pragma once 
#include "IMJRoomState.h"
#include "log4z.h"
#include "IMJRoom.h"
#include "IMJPlayer.h"
#include "IMJPlayerCard.h"
#include <cassert>
#include "MJPlayer.h"
class MJRoomStateWaitPlayerAct
	:public IMJRoomState
{
public:
	uint32_t getStateID()final{ return eRoomState_WaitPlayerAct; }
	void enterState(IMJRoom* pmjRoom, Json::Value& jsTranData)override
	{
		IMJRoomState::enterState(pmjRoom, jsTranData);
		setStateDuringTime( pmjRoom->isWaitPlayerActForever() ? 100000000 : eTime_WaitPlayerAct);
		if ( jsTranData["idx"].isNull() == false && jsTranData["idx"].isUInt() )
		{
			m_nIdx = jsTranData["idx"].asUInt();
			getRoom()->onWaitPlayerAct(m_nIdx, m_isCanPass );

			// check tuo guan 
			getRoom()->onCheckTrusteeForWaitPlayerAct(m_nIdx, m_isCanPass);
			return;
		}
		assert(0 && "invalid argument");
	}

	void onStateTimeUp()override 
	{
		if (m_isCanPass)
		{
			m_isCanPass = false;
			setStateDuringTime(eTime_WaitPlayerAct);
			return;
		}
		auto nCard = getRoom()->getAutoChuCardWhenWaitActTimeout(m_nIdx);
		LOGFMTE("wait time out , auto chu card = %u idx = %u", nCard,m_nIdx);
		Json::Value jsTran;
		jsTran["idx"] = m_nIdx;
		jsTran["act"] = eMJAct_Chu;
		jsTran["card"] = nCard;
		getRoom()->goToState(eRoomState_DoPlayerAct, &jsTran);

		// go to tuo guan zhuang tai 
		auto pPlayer = getRoom()->getMJPlayerByIdx(m_nIdx);
		if (!pPlayer || pPlayer->haveState(eRoomPeer_AlreadyHu) == false)
		{
			getRoom()->onPlayerTrusteedStateChange(m_nIdx, true);
		}
	}

	bool onMsg(Json::Value& prealMsg, uint16_t nMsgType, eMsgPort eSenderPort, uint32_t nSessionID)override
	{
		if (MSG_REQ_ACT_LIST == nMsgType)
		{
			auto pPlayer = getRoom()->getMJPlayerBySessionID(nSessionID);
			if (pPlayer == nullptr)
			{
				LOGFMTE("you are not in room  why req act list" );
				return false;
			}

			if (m_nIdx != pPlayer->getIdx())
			{
				LOGFMTD("you are not cur act player , so omit you message");
				return false;
			}

			if (m_isCanPass)  // means player need wait to do act chose ;
			{
				getRoom()->onWaitPlayerAct(m_nIdx, m_isCanPass);
			}
			return true;
		}

		if (MSG_PLAYER_ACT != nMsgType)
		{
			return false;
		}

		auto actType = prealMsg["actType"].asUInt();
		auto nCard = prealMsg["card"].asUInt();
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

			if (m_nIdx != pPlayer->getIdx())
			{
				nRet = 1;
				break;
			}

			auto pMJCard = pPlayer->getPlayerCard();
			switch (actType)
			{
			case eMJAct_Chu:
			{
				if (!pMJCard->isHaveCard(nCard))
				{
					nRet = 3;
				}
			}
			break;
			case eMJAct_AnGang:
			{
				if (!pMJCard->canAnGangWithCard(nCard))
				{
					nRet = 3;
				}
			}
			break;
			case eMJAct_BuGang:
			case eMJAct_BuGang_Declare:
			{
				if (!pMJCard->canBuGangWithCard(nCard))
				{
					nRet = 3;
				}
			}
			break;
			case eMJAct_Hu:
			{
				if (!pMJCard->isHoldCardCanHu())
				{
					nRet = 3;
				}
				nCard = pMJCard->getNewestFetchedCard();
			}
			break;
			case eMJAct_Pass:
			break;
			default:
				nRet = 2;
				break;
			}
		} while (0);

		if (nRet)
		{
			Json::Value jsRet;
			jsRet["ret"] = nRet;
			getRoom()->sendMsgToPlayer(jsRet,nMsgType,nSessionID);
			return true;
		}

		if (eMJAct_Pass == actType)
		{
			setStateDuringTime(getRoom()->isWaitPlayerActForever() ? 100000000 : eTime_WaitPlayerAct);
			return true;
		}

		// do transfer 
		Json::Value jsTran;
		jsTran["idx"] = m_nIdx;
		jsTran["act"] = actType;
		jsTran["card"] = nCard;
		jsTran["invokeIdx"] = m_nIdx;
		if (eMJAct_BuGang_Declare == actType || eMJAct_BuGang == actType)
		{
			pPlayer->signDecareBuGangFlag();
			if (getRoom()->isAnyPlayerRobotGang(m_nIdx, nCard))
			{
				getRoom()->goToState(eRoomState_AskForRobotGang, &jsTran);
				return true;
			}
		}
		getRoom()->goToState(eRoomState_DoPlayerAct, &jsTran);
		return true;
	}
	uint8_t getCurIdx()override{ return m_nIdx; }
protected:
	uint8_t m_nIdx;
	bool m_isCanPass;
};

