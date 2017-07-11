#include "YZMJPlayer.h"

IMJPlayerCard* YZMJPlayer::getPlayerCard()
{
	return &m_tPlayerCard;
}

void YZMJPlayer::addOffsetCoin(int32_t nOffset)
{
	m_nOffset += nOffset;
	m_nCoin = (uint32_t)((int32_t)m_nCoin + nOffset);
}