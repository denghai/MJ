#pragma once 
#include "MJPlayer.h"
#include "NJMJPlayerCard.h"
class NJMJPlayer
	:public MJPlayer
{
public:
	void onWillStartGame()override;
	IMJPlayerCard* getPlayerCard()override;
	void signHuaGangFlag();
	void clearHuaGangFlag();
	void signBuHuaFlag();
	void clearBuHuaFlag();
	bool haveBuHuaFlag(){ return m_bHaveBuHuaFlag; }
	bool haveHuaGangFlag(){ return m_bHaveHuaGangFlag; }
protected:
	bool m_bHaveHuaGangFlag;
	bool m_bHaveBuHuaFlag;
	NJMJPlayerCard m_tPlayerCard;
};