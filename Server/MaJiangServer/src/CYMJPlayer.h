#pragma once
#include "MJPlayer.h"
#include "CYMJPlayerCard.h"
class CYMJPlayer
	:public MJPlayer
{
public:
	IMJPlayerCard* getPlayerCard()override;
	bool GetTing() { return m_isTing; }
	void TingPai() { m_isTing = true; }
	void ClearTing() { m_isTing = false; }
	bool GetBiMen() { return m_isBiMen; }
	void KaiMen() { m_isBiMen = false; }
	void ClearBiMen() { m_isBiMen = true; }
	bool checkTingedCanMingGang(uint8_t nCard);
	bool checkTingedCanAnGang(uint8_t nCard);
	bool checkTingedCanBuGang(uint8_t nCard);
	void setTingGangType(uint8_t nType) { m_nTingGangType = nType; }
	uint8_t getTingGangType() { return m_nTingGangType; }
	bool GetGangMo() { return m_bGangMo; }
	void SetGangMo(bool bGangMo) { m_bGangMo = bGangMo; }
	void SetIsBiMenHu(bool isBiMenHu) { m_tPlayerCard.setIsBiMenHu(isBiMenHu); }
	void SetHaveQingYiSe(bool haveQingYiSe) { m_tPlayerCard.setHaveQingYiSe(haveQingYiSe); }
	void signGangFlagPao() { m_isHaveGangPao = true; }
	void clearGangFlagPao() { m_isHaveGangPao = false; }
	bool haveGangFlagPao() { return m_isHaveGangPao; }

	void init(stEnterRoomData* pData)override;
	void onWillStartGame()override;
	void onStartGame()override;
	void onGameDidEnd()override;
protected:
	CYMJPlayerCard m_tPlayerCard;
	bool m_isTing;
	bool m_isBiMen;
	uint8_t m_nTingGangType;
	bool m_bGangMo;
	bool m_isHaveGangPao;
};