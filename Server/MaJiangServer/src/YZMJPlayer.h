#pragma once
#include "MJPlayer.h"
#include "YZMJPlayerCard.h"
class YZMJPlayer
	:public MJPlayer
{
public:
	IMJPlayerCard* getPlayerCard()override;

	void addOffsetCoin(int32_t nOffset) override;

protected:
	YZMJPlayerCard m_tPlayerCard;
};