
#pragma once
#include "MJPlayer.h"
#include "HHMJPlayerCard.h"
class HHMJPlayer
	:public MJPlayer
{
public:
	IMJPlayerCard* getPlayerCard()override;
	bool getTangZhiHuPass() { return m_isTangzhiHuPass; }
	void setTangZhiHuPass(bool bTangzhiHuPass) { m_isTangzhiHuPass = bTangzhiHuPass; }

protected:
	HHMJPlayerCard m_tPlayerCard;
	bool m_isTangzhiHuPass;
};