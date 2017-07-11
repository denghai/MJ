#include "NiuNiuRoomPlayer.h"
#include <string>
#include "log4z.h"
#include "ServerMessageDefine.h"
void CNiuNiuRoomPlayer::reset(IRoom::stStandPlayer* pPlayer)
{
	ISitableRoomPlayer::reset(pPlayer) ;
	m_nTryBankerTimes = 0 ;
	m_nBetTimes = 0 ;
	m_tPeerCard.reset();
	m_nGameOffset = 0 ;
}

void CNiuNiuRoomPlayer::onGameEnd()
{
	ISitableRoomPlayer::onGameEnd();
	m_nTryBankerTimes = 0 ;
	m_nBetTimes = 0 ;
	m_nGameOffset = 0 ;
	m_tPeerCard.reset();
}

void CNiuNiuRoomPlayer::onGameBegin()
{
	ISitableRoomPlayer::onGameBegin() ;
	m_nTryBankerTimes = 0 ;
	m_nBetTimes = 0 ;
	m_nGameOffset = 0 ;
	m_tPeerCard.reset();
	setState(eRoomPeer_CanAct) ;
}

void CNiuNiuRoomPlayer::doSitdown(uint8_t nIdx )
{
	setIdx(nIdx) ;
}

void CNiuNiuRoomPlayer::willStandUp()
{
	ISitableRoomPlayer::willStandUp() ;
}

void CNiuNiuRoomPlayer::onGetCard( uint8_t nIdx , uint8_t nCard )
{
	if ( nIdx >= NIUNIU_HOLD_CARD_COUNT )
	{
		LOGFMTE("uid = %d ,on get card idx invalid idx = %d",getUserUID(),nIdx);
		return ;
	}
	getPeerCard()->addCompositCardNum(nCard);
}

uint8_t CNiuNiuRoomPlayer::getCardByIdx(uint8_t nIdx )
{
	if ( nIdx >= NIUNIU_HOLD_CARD_COUNT )
	{
		LOGFMTE("uid = %d , get card idx invalid idx = %d",getUserUID(),nIdx);
		return 0 ;
	}
	return m_tPeerCard.getCardByIdx(nIdx) ;
}

uint8_t CNiuNiuRoomPlayer::getTryBankerTimes()
{
	return m_nTryBankerTimes ;
}

void CNiuNiuRoomPlayer::setTryBankerTimes(uint8_t nTimes )
{
	m_nTryBankerTimes = nTimes ;
}

uint8_t CNiuNiuRoomPlayer::getBetTimes()
{
	return m_nBetTimes ;
}

void CNiuNiuRoomPlayer::setBetTimes(uint8_t nTimes)
{
	m_nBetTimes = nTimes ;
}

bool CNiuNiuRoomPlayer::isHaveNiu()
{
	if ( m_tPeerCard.getType() != CNiuNiuPeerCard::Niu_None )
	{
		return true ;
	}
	return false ;
}