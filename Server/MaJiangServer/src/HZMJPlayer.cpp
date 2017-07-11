#include "HZMJPlayer.h"
void HZMJPlayer::onWillStartGame()
{
	MJPlayer::onWillStartGame();
	clearPiaoTimes();
	m_tPlayerCard.reset();
}

void HZMJPlayer::onStartGame()
{
	MJPlayer::onStartGame();
}

void HZMJPlayer::onGameDidEnd()
{
	MJPlayer::onGameDidEnd();
	clearPiaoTimes();
	m_tPlayerCard.reset();
}

void HZMJPlayer::onGameEnd()
{
	MJPlayer::onGameEnd();
}

IMJPlayerCard* HZMJPlayer::getPlayerCard()
{
	return &m_tPlayerCard;
}

void HZMJPlayer::increasePiaoTimes( bool isGangPiao )
{
	++m_nPiaoTimes;
	if (isGangPiao)
	{
		m_bHaveGangPiao = isGangPiao;
	}
}

void HZMJPlayer::clearPiaoTimes()
{
	m_nPiaoTimes = 0;
	m_bHaveGangPiao = false;
}

uint8_t HZMJPlayer::getPiaoTimes()
{
	return m_nPiaoTimes;
}