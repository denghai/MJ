#include "NJMJPlayer.h"
void NJMJPlayer::onWillStartGame()
{
	MJPlayer::onWillStartGame();
	clearHuaGangFlag();
	clearBuHuaFlag();
}

IMJPlayerCard* NJMJPlayer::getPlayerCard()
{
	return &m_tPlayerCard;
}

void NJMJPlayer::signHuaGangFlag()
{
	m_bHaveHuaGangFlag = true;
}

void NJMJPlayer::clearHuaGangFlag()
{
	m_bHaveHuaGangFlag = false;
}

void NJMJPlayer::signBuHuaFlag()
{
	m_bHaveBuHuaFlag = true;
}

void NJMJPlayer::clearBuHuaFlag()
{
	m_bHaveBuHuaFlag = false;
}