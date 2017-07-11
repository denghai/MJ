#include "XLMJPlayer.h"
XLMJPlayer::XLMJPlayer()
{
	m_nFanXing = 0;
	m_nFanShu = 0;
}

void XLMJPlayer::init(stEnterRoomData* pData)
{
	MJPlayer::init(pData);
}

void XLMJPlayer::onWillStartGame()
{
	MJPlayer::onWillStartGame();
	m_nFanXing = 0;
	m_nFanShu = 0 ;
}

void XLMJPlayer::onStartGame()
{
	MJPlayer::onStartGame();
	m_nFanXing = 0;
	m_nFanShu = 0;
}

void XLMJPlayer::onGameDidEnd()
{
	MJPlayer::onGameDidEnd();
	m_nFanXing = 0;
	m_nFanShu = 0;
}

void XLMJPlayer::onGameEnd()
{
	MJPlayer::onGameEnd();
	m_nFanXing = 0;
	m_nFanShu = 0;
}

IMJPlayerCard* XLMJPlayer::getPlayerCard()
{
	return &m_tPlayerCard;
}

uint8_t XLMJPlayer::getMaxFanShu()
{
	return m_nFanShu;
}

uint8_t XLMJPlayer::getMaxFanXing()
{
	return m_nFanXing;
}

void XLMJPlayer::updateFanXingAndFanShu(uint8_t nFanXing, uint8_t nFanShu)
{
	if (nFanXing > m_nFanXing)
	{
		m_nFanXing = nFanXing;
	}

	if (nFanShu > m_nFanShu)
	{
		m_nFanShu = nFanShu;
	}
}