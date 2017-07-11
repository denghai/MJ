#pragma once 
#include "MJRoomStateWaitPlayerAct.h"
#include "YZMJPlayerCard.h"

class YZRoomStateWaitPlayerAct
	:public MJRoomStateWaitPlayerAct
{
public:
	bool onMsg(Json::Value& prealMsg, uint16_t nMsgType, eMsgPort eSenderPort, uint32_t nSessionID)override
	{
		auto actType = prealMsg["actType"].asUInt();
		auto pPlayer = getRoom()->getMJPlayerBySessionID(nSessionID);
		auto pPlayerCard = (YZMJPlayerCard*)pPlayer->getPlayerCard();

		if (MSG_PLAYER_ACT == nMsgType && actType == eMJAct_Pass)
		{
			//检查碰的是否有补杠
			std::vector<uint8_t> vHoldCards;
			pPlayerCard->getHoldCard(vHoldCards);

			std::vector<uint8_t> vPengedCards;
			bool bEmpty = pPlayerCard->getPengedCard(vPengedCards);
			if (!bEmpty)
			{
				for (auto nCard : vPengedCards)
				{
					auto it = std::find(vHoldCards.begin(), vHoldCards.end(), nCard);
					if (it != vHoldCards.end())
					{
						pPlayerCard->addCanNotGang(nCard);
					}
				}
			}
		}

		return MJRoomStateWaitPlayerAct::onMsg(prealMsg, nMsgType, eSenderPort, nSessionID);
	}
};

