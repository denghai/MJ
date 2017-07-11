#pragma once
#include "MJPlayer.h"
#include "BPMJPlayerCard.h"
class BPMJPlayer
	:public MJPlayer
{
public:
	IMJPlayerCard* getPlayerCard()override;
	bool GetBiMen() { return m_isBiMen; }
	void KaiMen() { m_isBiMen = false; }
	void ClearBiMen() { m_isBiMen = true; }
	bool GetGangMo() { return m_bGangMo; }
	void SetGangMo(bool bGangMo) { m_bGangMo = bGangMo; }
	void SetIsBiMenHu(bool isBiMenHu) { m_tPlayerCard.setIsBiMenHu(isBiMenHu); }
	void SetIsHuiPai(bool isHuiPai) { m_tPlayerCard.setIsHuiPai(isHuiPai); }
	void SetIs7Pair(bool is7Pair) { m_tPlayerCard.setIs7Pair(is7Pair); }
protected:
	BPMJPlayerCard m_tPlayerCard;
	bool m_isBiMen;
	bool m_isHuiPai;
	bool m_is7Pair;
	bool m_bGangMo;
};