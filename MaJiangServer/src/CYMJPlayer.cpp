#include "CYMJPlayer.h"
IMJPlayerCard* CYMJPlayer::getPlayerCard()
{
	return &m_tPlayerCard;
}

bool CYMJPlayer::checkTingedCanMingGang(uint8_t nCard)
{
	if (!m_isTing || nCard == 0)
	{
		return true;
	}

	if (!m_tPlayerCard.canMingGangWithCard(nCard))
	{
		return false;
	}

	std::set<uint8_t> vCanHuCards;
	m_tPlayerCard.getCanHuCards(vCanHuCards);

	std::vector<uint8_t> vCards;
	m_tPlayerCard.getHoldCard(vCards);

	bool bRet = false;
	bool bOperate = false;
	while (std::count(vCards.begin(), vCards.end(), nCard) == 3)
	{
		bRet = true;
		m_tPlayerCard.pickoutHoldCard(nCard);
		m_tPlayerCard.pickoutHoldCard(nCard);
		m_tPlayerCard.pickoutHoldCard(nCard);

		m_tPlayerCard.addGangTest(nCard);
		bOperate = true;

		if (m_tPlayerCard.isTingPai() == false)
		{
			bRet = false;
			break;
		}

		std::set<uint8_t> vCanHuCards_;
		m_tPlayerCard.getCanHuCards(vCanHuCards_);

		if (vCanHuCards.size() != vCanHuCards_.size())
		{
			bRet = false;
			break;
		}

		for (auto c : vCanHuCards)
		{
			std::set<uint8_t>::iterator it = vCanHuCards_.find(c);
			if (it == vCanHuCards_.end())
			{
				bRet = false;
				break;
			} 
			else
			{
				vCanHuCards_.erase(it);
			}
		}
		if (vCanHuCards_.empty() == false)
		{
			bRet = false;
			break;
		}
		break;
	}
	if (bOperate)
	{
		m_tPlayerCard.removeGangTest(nCard);
		m_tPlayerCard.addDistributeCard(nCard);
		m_tPlayerCard.addDistributeCard(nCard);
		m_tPlayerCard.addDistributeCard(nCard);
	}

	return bRet;
}

bool CYMJPlayer::checkTingedCanAnGang(uint8_t nCard)
{
	if (!m_isTing || nCard == 0)
	{
		return true;
	}

	if (!m_tPlayerCard.canAnGangWithCard(nCard))
		return false;

	if (m_tPlayerCard.pickoutHoldCard(nCard) == false)
		return false;

	if (m_tPlayerCard.checkMingPiao() == 0)
	{
		m_tPlayerCard.addDistributeCard(nCard);
		return false;
	}

	std::set<uint8_t> vCanHuCards;
	m_tPlayerCard.getCanHuCards(vCanHuCards);

	std::vector<uint8_t> vCards;
	m_tPlayerCard.getHoldCard(vCards);

	bool bRet = false;
	bool bOperate = false;
	while (std::count(vCards.begin(), vCards.end(), nCard) == 3)
	{
		bRet = true;
		m_tPlayerCard.pickoutHoldCard(nCard);
		m_tPlayerCard.pickoutHoldCard(nCard);
		m_tPlayerCard.pickoutHoldCard(nCard);

		m_tPlayerCard.addAnGangTest(nCard);
		bOperate = true;

		if (m_tPlayerCard.isTingPai() == false)
		{
			bRet = false;
			break;
		}

		std::set<uint8_t> vCanHuCards_;
		m_tPlayerCard.getCanHuCards(vCanHuCards_);

		if (vCanHuCards.size() != vCanHuCards_.size())
		{
			if (vCanHuCards.size() < vCanHuCards_.size())
			{
				bRet = false;
				break;
			}
			for (auto c : vCanHuCards_)
			{
				auto it_1 = std::find(vCanHuCards.begin(), vCanHuCards.end(), c);
				if (it_1 == vCanHuCards.end())
				{
					bRet = false;
					break;
				}
			}
		}
		break;
	}
	if (bOperate)
	{
		m_tPlayerCard.removeAnGangTest(nCard);
		m_tPlayerCard.addDistributeCard(nCard);
		m_tPlayerCard.addDistributeCard(nCard);
		m_tPlayerCard.addDistributeCard(nCard);
	}
	m_tPlayerCard.addDistributeCard(nCard);

	return bRet;
}

bool CYMJPlayer::checkTingedCanBuGang(uint8_t nCard)
{
	if (!m_isTing || nCard == 0)
	{
		return true;
	}

	if (m_tPlayerCard.pickoutHoldCard(nCard) == false)
		return false;

	std::set<uint8_t> vCanHuCards;
	m_tPlayerCard.getCanHuCards(vCanHuCards);

	std::vector<uint8_t> vCards;
	m_tPlayerCard.getHoldCard(vCards);

	std::vector<uint8_t> vPengedCards;
	m_tPlayerCard.getPengedCard(vPengedCards);

	bool bRet = false;
	bool bOperate = false;
	while (std::count(vPengedCards.begin(), vPengedCards.end(), nCard) == 1)
	{
		bRet = true;

		m_tPlayerCard.removePengedVector(nCard);

		m_tPlayerCard.addGangTest(nCard);
		bOperate = true;

		if (m_tPlayerCard.isTingPai() == false)
		{
			bRet = false;
			break;
		}

		std::set<uint8_t> vCanHuCards_;
		m_tPlayerCard.getCanHuCards(vCanHuCards_);

		if (vCanHuCards.size() != vCanHuCards_.size())
		{
			bRet = false;
			break;
		}

		for (auto c : vCanHuCards)
		{
			std::set<uint8_t>::iterator it = vCanHuCards_.find(c);
			if (it == vCanHuCards_.end())
			{
				bRet = false;
				break;
			}
			else
			{
				vCanHuCards_.erase(it);
			}
		}
		if (vCanHuCards_.empty() == false)
		{
			bRet = false;
			break;
		}
		break;
	}
	if (bOperate)
	{
		m_tPlayerCard.removeGangTest(nCard);
		m_tPlayerCard.addPengedVector(nCard);
	}
	m_tPlayerCard.addDistributeCard(nCard);

	return bRet;
}

void CYMJPlayer::init(stEnterRoomData* pData)
{
	clearGangFlagPao();
	MJPlayer::init(pData);
}

void CYMJPlayer::onWillStartGame()
{
	clearGangFlagPao();
	MJPlayer::onWillStartGame();
}

void CYMJPlayer::onStartGame()
{
	clearGangFlagPao();
	MJPlayer::onStartGame();
}

void CYMJPlayer::onGameDidEnd()
{
	clearGangFlagPao();
	MJPlayer::onGameDidEnd();
}
