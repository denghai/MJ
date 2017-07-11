#pragma once
#include "MJPlayer.h"
#include "SZMJPlayerCard.h"
class SZMJPlayer
	:public MJPlayer
{
public:
	IMJPlayerCard* getPlayerCard()override;
protected:
	SZMJPlayerCard m_tPlayerCard;
};