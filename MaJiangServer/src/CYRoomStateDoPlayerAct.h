#pragma once
#include "MJRoomStateDoPlayerAct.h"
#include "log4z.h"
#include "CYMJRoom.h"
#include "CYMJPlayer.h"
#include "CYMJPlayerCard.h"
#include "CommonDefine.h"
#include "MJCard.h"
class CYRoomStateDoPlayerAct
	:public MJRoomStateDoPlayerAct
{
public:
	void doAct()override
	{
		m_isGoToEndState = false;

		auto pRoom = (CYMJRoom*)getRoom();

		auto player = (CYMJPlayer*)pRoom->getMJPlayerByIdx(m_nActIdx);
		auto pActCard = (CYMJPlayerCard*)player->getPlayerCard();

		if (m_eActType == eMJAct_Mo || m_eActType == eMJAct_Chi || m_eActType == eMJAct_Peng || m_eActType == eMJAct_MingGang)
		{
			pActCard->clearLouPeng();
			pActCard->setLouHu(false);
		}
		if (m_eActType == eMJAct_Chi || m_eActType == eMJAct_Peng || m_eActType == eMJAct_MingGang)
		{
			player->KaiMen();
		}

		if (!pRoom->getModeLouBao())
		{
			MJRoomStateDoPlayerAct::doAct();
			return;
		}

		//MJRoomStateDoPlayerAct::doAct();
		switch (m_eActType)
		{
		case eMJAct_Mo:
		{
			//如果是杠宝牌后再摸牌
			auto eTingGangType = player->getTingGangType();
			if (player->GetTing() && (eTingGangType == eMJAct_MingGang || eTingGangType == eMJAct_AnGang || eTingGangType == eMJAct_BuGang))
			{
				player->setTingGangType(eMJAct_Max);

				if (!getRoom()->isCanGoOnMoPai())
				{
					m_isGoToEndState = true;
					//getRoom()->goToState(eRoomState_GameEnd);
					return;
				}
				else
				{
					getRoom()->onPlayerMo(m_nActIdx);
				}
				break;
			}
			auto nRoomBaoCard = ((CYMJRoom*)getRoom())->getBaoCard();
			//必须先看宝牌-》再摸牌（判断后摸的牌是否胡）
			if (player->GetTing())
			{
				bool bHuanBaoFlag = ((CYMJRoom*)getRoom())->getHuanBaoFlag();

				if (pActCard->getBaoCard() != nRoomBaoCard || pActCard->getBaoCard() == uint8_t(-1) 
					|| nRoomBaoCard == uint8_t(-1) || bHuanBaoFlag)
				{
					if (nRoomBaoCard == uint8_t(-1) || bHuanBaoFlag)
					{
						if (bHuanBaoFlag)
						{
							((CYMJRoom*)getRoom())->setHuanBaoFlag(false);
						}

						nRoomBaoCard = ((CYMJRoom*)getRoom())->getValidBaoCard();
						((CYMJRoom*)getRoom())->onPlayerMoBao(m_nActIdx, nRoomBaoCard);

						Json::Value jsTrans;
						jsTrans["act"] = eMJAct_MoBaoDice;
						jsTrans["idx"] = m_nActIdx;
						enterState(getRoom(), jsTrans);
						return;
					}
					else
					{
						Json::Value jsMsg;
						jsMsg["baoCard"] = nRoomBaoCard;
						getRoom()->sendMsgToPlayer(jsMsg, MSG_BAO_CARD, player->getSessionID());
					}
					////检查宝牌是否能胡（只有换宝牌才能杠）
					//if (pActCard->canHuWitCard(nRoomBaoCard))
					//{
					//	player->setTingGangType(eMJAct_Hu);

					//	pActCard->setKanBao(nRoomBaoCard);

					//	Json::Value jsValue;
					//	jsValue["invokeIdx"] = m_nActIdx;
					//	jsValue["card"] = nRoomBaoCard;
					//	getRoom()->goToState(eRoomState_AskForHuAndPeng, &jsValue);
					//	return;
					//}
					if (checkBaoGangHu())
						return;
				}
				pActCard->setBaoCard(nRoomBaoCard);
			}
			//getRoom()->onPlayerMo(m_nActIdx);
			if (!getRoom()->isCanGoOnMoPai())
			{
				m_isGoToEndState = true;
				//getRoom()->goToState(eRoomState_GameEnd);
				return;
			}
			else
			{
				getRoom()->onPlayerMo(m_nActIdx);
			}
			break;
		}
		case eMJAct_Peng:
			getRoom()->onPlayerPeng(m_nActIdx, m_nCard, m_nInvokeIdx);
			break;
		case eMJAct_MingGang:
			getRoom()->onPlayerMingGang(m_nActIdx, m_nCard, m_nInvokeIdx);
			break;
		case eMJAct_AnGang:
			getRoom()->onPlayerAnGang(m_nActIdx, m_nCard);
			break;
		case eMJAct_BuGang:
			getRoom()->onPlayerBuGang(m_nActIdx, m_nCard);
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
			getRoom()->onPlayerEat(m_nActIdx, m_nCard, m_vEatWith[0], m_vEatWith[1], m_nInvokeIdx);
			break;
		case eMJAct_MoBaoDice:
			break;
		default:
			LOGFMTE("unknow act  how to do it %u", m_eActType);
			break;
		}
	}

	void onStateTimeUp()override
	{
		if (m_isGoToEndState)
		{
			getRoom()->goToState(eRoomState_GameEnd);
			return;
		}

		auto pRoom = (CYMJRoom*)getRoom();
		if (!pRoom->getModeLouBao())
		{
			MJRoomStateDoPlayerAct::onStateTimeUp();
			return;
		}

		auto player = (CYMJPlayer*)pRoom->getMJPlayerByIdx(m_nActIdx);
		auto playerCard = (CYMJPlayerCard*)player->getPlayerCard();

		auto nIdx = pRoom->getNextActPlayerIdx(m_nActIdx);
		auto nextPlayer = (CYMJPlayer*)pRoom->getMJPlayerByIdx(nIdx);
		auto nextPlayerCard = (CYMJPlayerCard*)nextPlayer->getPlayerCard();

		auto nRoomBaoCard = pRoom->getBaoCard();

		if (m_eActType == eMJAct_Chu)
		{
			if (nRoomBaoCard != uint8_t(-1) && pRoom->isBaoCardValid(nRoomBaoCard) == false && pRoom->isCanGoOnMoPai())
			{
				pRoom->setHuanBaoFlag(true);
			}
		}
		if (m_eActType == eMJAct_MoBaoDice)
		{
			if (checkBaoGangHu())
				return;

			Json::Value jsTran;
			jsTran["idx"] = m_nActIdx;
			jsTran["act"] = eMJAct_Mo;
			pRoom->goToState(eRoomState_DoPlayerAct, &jsTran);
		}

		switch (m_eActType)
		{
		case eMJAct_Chi:
		case eMJAct_Peng:
		{
			std::map<uint8_t, std::set<uint8_t>> tingCards;
			pRoom->checkTing(playerCard, tingCards);
			if (tingCards.size() > 0)
			{
				pRoom->setIsChiAndTing();

				Json::Value jsValue;
				jsValue["idx"] = m_nActIdx;
				pRoom->goToState(eRoomState_WaitPlayerAct, &jsValue);
			}
			else
			{
				Json::Value jsValue;
				jsValue["idx"] = m_nActIdx;
				pRoom->goToState(eRoomState_WaitPlayerChu, &jsValue);
			}
			break;
		}
		case eMJAct_Mo:
		{
			if (player->GetTing() && !playerCard->isHoldCardCanHu() &&
				playerCard->getNewestFetchedCard() != playerCard->getBaoCard() && 
				player->checkTingedCanAnGang(playerCard->getNewestFetchedCard()) == false && 
				player->checkTingedCanBuGang(playerCard->getNewestFetchedCard()) == false)
			{
				Json::Value jsValue;
				jsValue["idx"] = m_nActIdx;
				jsValue["card"] = playerCard->getNewestFetchedCard();
				pRoom->goToState(eRoomState_WaitPlayerChu, &jsValue);
			}
			else
			{
				Json::Value jsValue;
				jsValue["idx"] = m_nActIdx;
				pRoom->goToState(eRoomState_WaitPlayerAct, &jsValue);
			}
			break;
		}
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

	bool checkBaoGangHu()
	{
		auto player = (CYMJPlayer*)getRoom()->getMJPlayerByIdx(m_nActIdx);
		auto playerCard = (CYMJPlayerCard*)player->getPlayerCard();

		auto nRoomBaoCard = ((CYMJRoom*)getRoom())->getBaoCard();

		//检查宝牌是否能胡
		bool bCanHu = playerCard->canHuWitCard(nRoomBaoCard);

		std::vector<uint8_t> vGangCards;
		playerCard->getAnGangedCard(vGangCards);
		playerCard->getMingGangedCard(vGangCards);
		std::vector<uint8_t>::iterator it = std::find(vGangCards.begin(), vGangCards.end(), nRoomBaoCard);
		if (it != vGangCards.end())
		{
			bCanHu = false;
		}

		playerCard->addDistributeCard(nRoomBaoCard);
		bool bCanAnGang = player->checkTingedCanAnGang(nRoomBaoCard);
		bool bCanBuGang = player->checkTingedCanBuGang(nRoomBaoCard);//playerCard->canBuGangWithCard(nRoomBaoCard);
		playerCard->pickoutHoldCard(nRoomBaoCard);

		if (bCanHu || bCanAnGang || bCanBuGang)
		{
			playerCard->addDistributeCard(nRoomBaoCard);
			if (bCanAnGang)
			{
				player->setTingGangType(eMJAct_AnGang);

				playerCard->setKanBao(nRoomBaoCard);

				Json::Value jsValue;
				jsValue["idx"] = m_nActIdx;
				getRoom()->goToState(eRoomState_WaitPlayerAct, &jsValue);
			}
			else if (bCanBuGang)
			{
				player->setTingGangType(eMJAct_BuGang);

				playerCard->setKanBao(nRoomBaoCard);

				Json::Value jsValue;
				jsValue["idx"] = m_nActIdx;
				getRoom()->goToState(eRoomState_WaitPlayerAct, &jsValue);
			}
			else
			{
				playerCard->pickoutHoldCard(nRoomBaoCard);

				player->setTingGangType(eMJAct_Hu);

				playerCard->setKanBao(nRoomBaoCard);

				Json::Value jsValue;
				jsValue["invokeIdx"] = m_nActIdx;
				jsValue["card"] = nRoomBaoCard;
				getRoom()->goToState(eRoomState_AskForHuAndPeng, &jsValue);
			}
			return true;
		}
		playerCard->setBaoCard(nRoomBaoCard);
		return false;
	}

private:
	bool m_isGoToEndState;
};
