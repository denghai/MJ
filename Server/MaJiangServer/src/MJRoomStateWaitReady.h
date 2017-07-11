#pragma once 
#include "IMJRoomState.h"
#include "IMJRoom.h"
#include "log4z.h"
#include "IMJPlayer.h"
class CMJRoomStateWaitReady
	:public IMJRoomState
{
public:
	uint32_t getStateID()final{ return eRoomSate_WaitReady; }

	//void enterState(IMJRoom* pmjRoom, Json::Value& jsTranData)override
	CMJRoomStateWaitReady()
	{
		//IMJRoomState::enterState(pmjRoom, jsTranData);
		m_isChanged = false;
	}

	void update(float fDeta)override
	{
		IMJRoomState::update(fDeta);
		if (getRoom()->canStartGame())
		{
			auto roomType = getRoom()->getRoomType();
			if (!m_isChanged && (roomType == eRoom_MJ_CaoYang || roomType == eRoom_MJ_HongHe || roomType == eRoom_MJ_JianShui || roomType == eRoom_MJ_BeiPiao || roomType == eRoom_MJ_YangZhou))
			{
				getRoom()->changeIdx();
				setStateDuringTime(eTime_ChangePlayerIdx);
				m_isChanged = true;
				return;
			}
			getRoom()->goToState(eRoomState_StartGame);
		}
	}

	bool onMsg(Json::Value& prealMsg, uint16_t nMsgType, eMsgPort eSenderPort, uint32_t nSessionID)override
	{
		if ( MSG_PLAYER_SET_READY == nMsgType)
		{
			auto pPlayer = getRoom()->getMJPlayerBySessionID(nSessionID);
			if (pPlayer == nullptr || (pPlayer->haveState(eRoomPeer_WaitNextGame) == false ))
			{
				LOGFMTE("you are not in this room how to set ready ? session id = %u", nSessionID );
				return true;
			}
			getRoom()->onPlayerSetReady(pPlayer->getIdx());
			if (getRoom()->canStartGame())
			{
				auto roomType = getRoom()->getRoomType();
				if (!m_isChanged && (roomType == eRoom_MJ_CaoYang || roomType == eRoom_MJ_HongHe || roomType == eRoom_MJ_JianShui || roomType == eRoom_MJ_BeiPiao || roomType == eRoom_MJ_YangZhou))
				{
					getRoom()->changeIdx();
					setStateDuringTime(eTime_ChangePlayerIdx);
					m_isChanged = true;
					return true;
				}
				getRoom()->goToState(eRoomState_StartGame);
			}
			return true;
		}
		return false;
	}

protected:
	bool m_isChanged;
};