#pragma once 
#include "MJRoomStateAskForPengOrHu.h"
class XLRoomStateAskForPengOrHu
	:public MJRoomStateAskForPengOrHu
{
public:
	void enterState(IMJRoom* pmjRoom, Json::Value& jsTranData)override
	{
		MJRoomStateAskForPengOrHu::enterState(pmjRoom, jsTranData);
		// check if have already hu player ?
		std::vector<uint8_t> vAlreadyHuWaitHu;
		for (auto& nCheckIdx : m_vWaitHuIdx)
		{
			auto pPlayer = getRoom()->getMJPlayerByIdx(nCheckIdx);
			if ( pPlayer && pPlayer->haveState(eRoomPeer_AlreadyHu) && pPlayer->isTrusteed() == false )
			{
				vAlreadyHuWaitHu.push_back(nCheckIdx);
			}
		}

		if (vAlreadyHuWaitHu.empty() == false)
		{
			m_tAlreadyHuActTimer.reset();
			m_tAlreadyHuActTimer.setInterval(1);
			m_tAlreadyHuActTimer.setIsAutoRepeat(false);
			m_tAlreadyHuActTimer.setCallBack([vAlreadyHuWaitHu,this](CTimer* pt , float f )
			{
				for (auto& refIdx : vAlreadyHuWaitHu)
				{
					auto pPlayer = getRoom()->getMJPlayerByIdx(refIdx);
					if (pPlayer == nullptr)
					{
						LOGFMTE("why you are nullptr ? idx = %",refIdx );
						continue;
					}

					LOGFMTD("auto hu dian pao , already hu player room id = %u uid = %u ",getRoom()->getRoomID(),pPlayer->getUID());
					Json::Value jsMsg;
					jsMsg["actType"] = eMJAct_Hu;
					onMsg(jsMsg, MSG_PLAYER_ACT,ID_MSG_PORT_CLIENT,pPlayer->getSessionID());
				}
			}
			);
			m_tAlreadyHuActTimer.start();
			return;
		}
	}
	void leaveState()override 
	{
		MJRoomStateAskForPengOrHu::leaveState();
		m_tAlreadyHuActTimer.reset();
	}
protected:
	CTimer m_tAlreadyHuActTimer;
};