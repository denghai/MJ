#include "GoldenRoomPlayer.h"
#include <cassert>
#include "log4z.h"
void CGoldenRoomPlayer::reset(IRoom::stStandPlayer* pPlayer)
{
	ISitableRoomPlayer::reset(pPlayer);
	m_nBetCoin = 0 ;
	m_tPeerCard.reset();
	m_nWinCoin = 0 ;
	setState(eRoomPeer_WaitNextGame) ;
}

void CGoldenRoomPlayer::onGameEnd()
{
	ISitableRoomPlayer::onGameEnd() ;
	if (  getState() != eRoomPeer_Ready )
	{
		LOGFMTE("uid = %d , should come here , ready when other state ? ",getUserUID()) ;
		setState(eRoomPeer_WaitNextGame) ;
	}

	m_nBetCoin = 0 ;
	m_nWinCoin = 0 ;
	m_tPeerCard.reset();
}

void CGoldenRoomPlayer::onGameBegin()
{
	ISitableRoomPlayer::onGameBegin() ;
	setState(eRoomPeer_CanAct) ;
	m_nBetCoin = 0 ;
	m_nWinCoin = 0 ;
	m_tPeerCard.reset();
}

void CGoldenRoomPlayer::onGetCard( uint8_t nIdx , uint8_t nCard )
{
	assert(nIdx < GOLDEN_PEER_CARD && "invalid idx" );
	m_tPeerCard.addCompositCardNum(nCard);	
}

uint8_t CGoldenRoomPlayer::getCardByIdx(uint8_t nIdx )
{
	assert(nIdx < GOLDEN_PEER_CARD && "invalid idx" );
	return m_tPeerCard.getCardByIdx(nIdx);
}


bool CGoldenRoomPlayer::betCoin(uint32_t nBetCoin )
{
	if ( getCoin() < nBetCoin )
	{
		return false ;
	}

	m_nBetCoin += nBetCoin ;
	setCoin(getCoin() - nBetCoin ) ;
	return true ;
}

void CGoldenRoomPlayer::addWinCoin(uint32_t nWinCoin )
{
	m_nWinCoin += nWinCoin ;
}