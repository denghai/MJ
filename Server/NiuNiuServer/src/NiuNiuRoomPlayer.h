#pragma once
#include "CommonDefine.h"
#include "ISitableRoomPlayer.h"
#include "NiuNiuPeerCard.h"
#include "CommonData.h"
class CNiuNiuRoomPlayer
	:public ISitableRoomPlayer
{
public:
	void reset(IRoom::stStandPlayer* pPlayer) override ;
	void onGameEnd()override ;
	void onGameBegin()override ;
	void doSitdown(uint8_t nIdx ) override;
	void willStandUp() override;
	void onGetCard( uint8_t nIdx , uint8_t nCard );
	uint8_t getCardByIdx(uint8_t nIdx );
	uint8_t getTryBankerTimes();
	void setTryBankerTimes(uint8_t nTimes );
	uint8_t getBetTimes();
	void setBetTimes(uint8_t nTimes);
	int32_t getGameOffset()override { return m_nGameOffset ; } ;
	IPeerCard* getPeerCard()override{ return &m_tPeerCard ;};
	void setGameOffset( int32_t nGameOffset ){ m_nGameOffset = nGameOffset ; }
	bool isHaveNiu();
protected:
	uint8_t m_nTryBankerTimes ;
	uint8_t m_nBetTimes ;
	int32_t m_nGameOffset ;
	CNiuNiuPeerCard m_tPeerCard ;
};