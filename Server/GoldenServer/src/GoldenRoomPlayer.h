#pragma once
#include "CommonDefine.h"
#include "ISitableRoomPlayer.h"
#include "GoldenPeerCardAdapter.h"
#include "CommonData.h"
class CGoldenRoomPlayer
	:public ISitableRoomPlayer
{
public:
	void reset(IRoom::stStandPlayer* pPlayer) override ;
	void onGameEnd()override ;
	void onGameBegin()override ;
	void onGetCard( uint8_t nIdx , uint8_t nCard );
	uint8_t getCardByIdx(uint8_t nIdx );
	bool betCoin(uint32_t nBetCoin );
	uint32_t getBetCoin(){ return m_nBetCoin ;}
	int32_t getGameOffset() override { return m_nWinCoin - m_nBetCoin ;}
	void addWinCoin(uint32_t nWinCoin );
	IPeerCard* getPeerCard()override{ return &m_tPeerCard; }
protected:
	uint32_t m_nWinCoin ;
	uint32_t m_nBetCoin ;
	CGoldenPeerCardAdpater m_tPeerCard ;
};