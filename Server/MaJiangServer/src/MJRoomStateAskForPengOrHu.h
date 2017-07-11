#pragma once 
#include "IMJRoomState.h"
#include "log4z.h"
#include "IMJRoom.h"
#include "IMJPlayer.h"
#include "IMJPlayerCard.h"
#include <cassert>
class MJRoomStateAskForPengOrHu
	:public IMJRoomState
{
public:
	uint32_t getStateID()final{ return eRoomState_AskForHuAndPeng; }
	void enterState(IMJRoom* pmjRoom, Json::Value& jsTranData)override
	{
		m_vWaitHuIdx.clear();
		m_vWaitPengGangIdx.clear();
		m_isNeedWaitEat = false;
		m_vEatWithInfo.clear();
		m_vDoHuIdx.clear();
		m_vDoPengGangIdx.clear();
		m_ePengGangAct = 0;

		IMJRoomState::enterState(pmjRoom, jsTranData);
		setStateDuringTime(pmjRoom->isWaitPlayerActForever() ? 100000000 : eTime_WaitPlayerAct);
		m_nInvokeIdx = jsTranData["invokeIdx"].asUInt();
		m_nCard = jsTranData["card"].asUInt();
		getRoom()->onAskForPengOrHuThisCard(m_nInvokeIdx, m_nCard, m_vWaitHuIdx, m_vWaitPengGangIdx, m_isNeedWaitEat);
		//assert((m_vWaitHuIdx.empty() == false && m_vWaitPengGangIdx.empty() == false && m_isNeedWaitEat == false) && "invalid argument");

		// wait truastee;
		std::vector<uint8_t> vWaitTruste;
		vWaitTruste = m_vWaitHuIdx;
		vWaitTruste.insert(vWaitTruste.begin(),m_vWaitPengGangIdx.begin(),m_vWaitPengGangIdx.end());
		if (m_isNeedWaitEat)
		{
			auto neatidx = (m_nInvokeIdx + 1) % getRoom()->getSeatCnt();
			vWaitTruste.push_back(neatidx);
		}
		getRoom()->onCheckTrusteeForHuOtherPlayerCard(vWaitTruste, m_nCard);
	}

	void onStateTimeUp()override
	{
		if (getRoom()->isGameOver())
		{
			getRoom()->goToState(eRoomState_GameEnd);
			return;
		}

		if ( doAct() )  // some body time out ,but have player do act ; 
		{
			return;
		}

		Json::Value jsTran;
		jsTran["idx"] = getRoom()->getNextActPlayerIdx(m_nInvokeIdx);
		jsTran["act"] = eMJAct_Mo;
		getRoom()->goToState(eRoomState_DoPlayerAct, &jsTran);
	}

	void responeReqActList( uint32_t nSessionID )
	{
		auto pPlayer = getRoom()->getMJPlayerBySessionID(nSessionID);
		if (!pPlayer)
		{
			LOGFMTE("you are  not in room id = %u , session id = %u , can not send you act list",getRoom()->getRoomID(),nSessionID);
			return;
		}
		Json::Value jsMsg;
		jsMsg["invokerIdx"] = m_nInvokeIdx;
		jsMsg["cardNum"] = m_nCard;

		Json::Value jsActs;

		auto iterPeng = std::find(m_vWaitPengGangIdx.begin(), m_vWaitPengGangIdx.end(), pPlayer->getIdx());
		// check peng 
		if ( iterPeng != m_vWaitPengGangIdx.end() )
		{
			jsActs[jsActs.size()] = eMJAct_Peng;
		}
		
		// check ming gang 
		if (getRoom()->isCanGoOnMoPai() && pPlayer->getPlayerCard()->canMingGangWithCard(m_nCard))
		{
			jsActs[jsActs.size()] = eMJAct_MingGang;
			// already add in peng ;  vWaitPengGangIdx
		}

		if (pPlayer->getIdx() == (m_nInvokeIdx + 1) % getRoom()->getSeatCnt() && m_isNeedWaitEat )
		{
			jsActs[jsActs.size()] = eMJAct_Chi;
		}

		// check hu ;
		auto iterHu = std::find(m_vWaitHuIdx.begin(), m_vWaitHuIdx.end(), pPlayer->getIdx());
		if ( iterHu != m_vWaitHuIdx.end() )
		{
			jsActs[jsActs.size()] = eMJAct_Hu;
		}

		if (jsActs.empty())
		{
			LOGFMTE("you are not in any wait list , so cannot resp you act list room id = %u , uid = %u",getRoom()->getRoomID(),pPlayer->getUID() );
			return;
		}

		if (jsActs.size() > 0)
		{
			jsActs[jsActs.size()] = eMJAct_Pass;
		}

		jsMsg["acts"] = jsActs;
		getRoom()->sendMsgToPlayer(jsMsg, MSG_PLAYER_WAIT_ACT_ABOUT_OTHER_CARD, nSessionID );
		LOGFMTD("respon act list inform uid = %u act about other card room id = %u card = %u", pPlayer->getUID(), getRoom()->getRoomID(), m_nCard);
	}

	bool onMsg(Json::Value& prealMsg, uint16_t nMsgType, eMsgPort eSenderPort, uint32_t nSessionID)override
	{
		if (MSG_PLAYER_ACT != nMsgType && MSG_REQ_ACT_LIST != nMsgType)
		{
			return false;
		}

		if (MSG_REQ_ACT_LIST == nMsgType)
		{
			responeReqActList(nSessionID);
			return true;
		}

		auto actType = prealMsg["actType"].asUInt();
		//auto nCard = prealMsg["card"].asUInt();
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

			if (eMJAct_Chi == actType)
			{
				if (m_isNeedWaitEat == false)
				{
					LOGFMTE("not wait eat act ");
					nRet = 1;
					break;
				}

				if (pPlayer->getIdx() != (m_nInvokeIdx + 1) % getRoom()->getSeatCnt() )
				{
					LOGFMTE("eat only just previous player's card");
					nRet = 1;
					break;
				}
			}
			else
			{
				auto iter = std::find(m_vWaitHuIdx.begin(), m_vWaitHuIdx.end(), pPlayer->getIdx());
				auto iterPeng = std::find(m_vWaitPengGangIdx.begin(), m_vWaitPengGangIdx.end(), pPlayer->getIdx());

				if (iter == m_vWaitHuIdx.end() && iterPeng == m_vWaitPengGangIdx.end() && !m_isNeedWaitEat)
				{
					nRet = 1;
					break;
				}
			}


			if (eMJAct_Pass == actType)
			{
				if ( m_isNeedWaitEat && pPlayer->getIdx() == (m_nInvokeIdx + 1) % getRoom()->getSeatCnt())
				{
					LOGFMTD("give up eat act");
					m_isNeedWaitEat = false;
				}
				break;
			}

			auto pMJCard = pPlayer->getPlayerCard();
			switch (actType)
			{
			case eMJAct_Hu:
			{
				if (!pMJCard->canHuWitCard(m_nCard))
				{
					nRet = 2;
					LOGFMTE("why you can not hu ? svr bug ");
					break;
				}
				m_vDoHuIdx.push_back(pPlayer->getIdx());
			}
			break;
			case eMJAct_Peng:
			{
				if (!pMJCard->canPengWithCard(m_nCard))
				{
					nRet = 2;
					LOGFMTE("why you can not peng ? svr bug ");
					break;
				}
				m_vDoPengGangIdx.push_back(pPlayer->getIdx());
				m_ePengGangAct = eMJAct_Peng;
			}
			break;
			case eMJAct_MingGang:
			{
				if (!pMJCard->canMingGangWithCard(m_nCard))
				{
					nRet = 2;
					LOGFMTE("why you can not ming gang ? svr bug ");
					break;
				}
				m_vDoPengGangIdx.push_back(pPlayer->getIdx());
				m_ePengGangAct = eMJAct_MingGang;
			}
			break;
			case eMJAct_Chi:
			{
				if (prealMsg["eatWith"].isNull() || prealMsg["eatWith"].isArray() == false || prealMsg["eatWith"].size() != 2)
				{
					LOGFMTE("eat arg error");
					nRet = 3;
					break;
				}
				Json::Value jsE;
				jsE = prealMsg["eatWith"];
				m_vEatWithInfo.clear();
				m_vEatWithInfo.push_back(jsE[0u].asUInt());
				m_vEatWithInfo.push_back(jsE[1u].asUInt());
				if ( !pMJCard->canEatCard(m_nCard, m_vEatWithInfo[0], m_vEatWithInfo[1]))
				{
					LOGFMTE("why you can not eat ? svr bug ");
					nRet = 2;
					m_vEatWithInfo.clear();
					break;
				}
			}
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
			getRoom()->sendMsgToPlayer(jsRet, nMsgType, nSessionID);
			return true;
		}

		auto iter = std::find(m_vWaitHuIdx.begin(), m_vWaitHuIdx.end(), pPlayer->getIdx());
		if (iter != m_vWaitHuIdx.end())
		{
			m_vWaitHuIdx.erase(iter);
		}

		auto iterPeng = std::find(m_vWaitPengGangIdx.begin(), m_vWaitPengGangIdx.end(), pPlayer->getIdx());
		if (iterPeng != m_vWaitPengGangIdx.end())
		{
			m_vWaitPengGangIdx.erase(iterPeng);
		}
		
		if (m_vWaitHuIdx.empty() == false)  // wait hu ;
		{
			return true;
		}

		if (m_vDoHuIdx.empty() && m_vWaitPengGangIdx.empty() == false)  // wait peng gang ;
		{
			return true;
		}

		if (m_vDoHuIdx.empty() && m_vDoPengGangIdx.empty() && m_isNeedWaitEat && m_vEatWithInfo.empty())  // wait eat
		{
			return true;
		}

		if (m_vDoHuIdx.empty() && m_vDoPengGangIdx.empty() && m_isNeedWaitEat == false )
		{
			onStateTimeUp();  // the same as wait time out , all candinate give up ;
			return true;
		}
		 
		doAct();
		return true;
	}

	uint8_t getCurIdx()override{ return m_nInvokeIdx; }

	bool doAct()
	{
		if (m_vDoHuIdx.empty() == false)
		{
			LOGFMTD("some body do hu ");
			// do transfer 
			Json::Value jsTran;
			jsTran["idx"] = m_vDoHuIdx.front();
			jsTran["act"] = eMJAct_Hu;
			jsTran["card"] = m_nCard;
			jsTran["invokeIdx"] = m_nInvokeIdx;

			Json::Value jsHuIdx;
			for (auto& ref : m_vDoHuIdx)
			{
				jsHuIdx[jsHuIdx.size()] = ref;
			}
			jsTran["huIdxs"] = jsHuIdx;
			getRoom()->goToState(eRoomState_DoPlayerAct, &jsTran);
			return true;
		}
		else if (!m_vDoPengGangIdx.empty())
		{
			// gang or peng ;
			Json::Value jsTran;
			jsTran["idx"] = m_vDoPengGangIdx.front();
			jsTran["act"] = m_ePengGangAct;
			jsTran["card"] = m_nCard;
			jsTran["invokeIdx"] = m_nInvokeIdx;
			getRoom()->goToState(eRoomState_DoPlayerAct, &jsTran);
			return true;
		}

		// must do eat
		if (m_vEatWithInfo.empty() == false)
		{
			Json::Value jsTran;
			jsTran["idx"] = (m_nInvokeIdx + 1) % getRoom()->getSeatCnt();
			jsTran["act"] = eMJAct_Chi;
			jsTran["card"] = m_nCard;
			jsTran["invokeIdx"] = m_nInvokeIdx;
			jsTran["eatWithA"] = m_vEatWithInfo[0];
			jsTran["eatWithB"] = m_vEatWithInfo[1];
			getRoom()->goToState(eRoomState_DoPlayerAct, &jsTran);
			return true;
		}
		return false;
	}
protected:
	uint8_t m_nInvokeIdx;
	uint8_t m_nCard;

	uint16_t m_ePengGangAct;

	std::vector<uint8_t> m_vWaitHuIdx;
	std::vector<uint8_t> m_vDoHuIdx;

	std::vector<uint8_t> m_vWaitPengGangIdx;
	std::vector<uint8_t> m_vDoPengGangIdx;

	std::vector<uint8_t> m_vEatWithInfo;
	bool m_isNeedWaitEat;
};







