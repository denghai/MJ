#pragma once 
#include "MJPlayer.h"
#include "XLMJPlayerCard.h"
class XLMJPlayer
	:public MJPlayer
{
public:
	XLMJPlayer();
	void init(stEnterRoomData* pData)override;
	void onWillStartGame()override;
	void onStartGame()override;
	void onGameDidEnd()override;
	void onGameEnd()override;
	IMJPlayerCard* getPlayerCard()override;
	uint8_t getMaxFanShu();
	uint8_t getMaxFanXing();
	void updateFanXingAndFanShu( uint8_t nFanXing, uint8_t nFanShu );
protected:
	XLMJPlayerCard m_tPlayerCard;
	uint8_t m_nFanXing;
	uint8_t m_nFanShu;
};