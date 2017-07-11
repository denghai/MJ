
#pragma once
#include "MJPlayer.h"
#include "JSMJPlayerCard.h"
class JSMJPlayer
	:public MJPlayer
{
public:
	IMJPlayerCard* getPlayerCard()override;

protected:
	JSMJPlayerCard m_tPlayerCard;
};