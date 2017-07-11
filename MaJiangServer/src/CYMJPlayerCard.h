#pragma once
#include "MJPlayerCard.h"
#include "json/json.h"
class CYMJPlayerCard
	:public MJPlayerCard
{
public:
	struct tSongGangIdx {
		uint8_t eAct;
		uint8_t nGangCard;
		uint8_t nIdx;
	};
public:
	bool canEatCard(uint8_t nCard, uint8_t& nWithA, uint8_t& nWithB);
	bool canPengWithCard(uint8_t nCard);
	void reset() override;
	bool getCardInfo(Json::Value& jsPeerCards, bool isTing, uint8_t nBaoCard);
	bool onDoHu(bool isZiMo, bool isHaiDiLoaYue, bool haveQingYiSe, uint8_t nCard, std::vector<uint16_t>& vHuTypes, int32_t& nFanCnt);
	bool onBaoPaiDoHu(bool isHaiDiLoaYue, bool haveQingYiSe, uint8_t nCard, std::vector<uint16_t>& vHuTypes, int32_t& nFanCnt);
	uint8_t getSongGangIdx();
	void setSongGangIdx(uint8_t nIdx);
	bool canHuWitCard(uint8_t nCard)override;
	bool isHoldCardCanHu();
	bool pickoutHoldCard(uint8_t nCard);

	void setBaoCard(uint8_t nBaoCard) { m_nBaoCard = nBaoCard; }
	uint8_t getBaoCard() { return m_nBaoCard; }
	void setKanBao(uint8_t nKanBao) { m_nKanBaoCard = nKanBao; }
	uint8_t getKanBao() { return m_nKanBaoCard; }

	void setLouPeng(uint8_t nCard) { m_vLouPeng.push_back(nCard); }
	void clearLouPeng() { m_vLouPeng.clear(); }
	void setLouHu(bool isLouHu) { m_isLouHu = isLouHu; }
	bool checkHu(bool isHold, uint8_t nCard);

	void addSongGangIdx(uint8_t nCard, uint8_t nIdx, uint8_t eAct);
	std::vector<tSongGangIdx> getMingGangSongIdx() { return m_vMingGangSongIdx; }
	void addSongPengIdx(uint8_t nCard, uint8_t nIdx);
	std::vector<tSongGangIdx> getPengSongIdx() { return m_vPengSongIdx; }

	void addGangTest(uint8_t nCard);
	void removeGangTest(uint8_t nCard);
	void addAnGangTest(uint8_t nCard);
	void removeAnGangTest(uint8_t nCard);
	void addPengedVector(uint8_t nCard);
	void removePengedVector(uint8_t nCard);

	bool onPutMingGang(uint8_t nCard);
	bool onPutAnGang(uint8_t nCard);
	bool onPutBuGang(uint8_t nCard);

	uint8_t checkMingPiao();

	void setIsBiMenHu(bool isBiMenHu) { m_isBiMenHu = isBiMenHu; }
	void setHaveQingYiSe(bool haveQingYiSe) { m_haveQingYiSe = haveQingYiSe; }

	uint8_t checkJiaHu(uint8_t nCard, bool isZiMo);

	uint8_t isMingPiao(uint8_t nHuCard);

	bool getCanHuCards_1(std::set<uint8_t>& vCanHuCards);
	bool canHuWitCard_1(uint8_t nCard);

	bool isTingPai();
	bool getCanHuCards(std::set<uint8_t>& vCanHuCards);

	uint8_t getJiang() { return m_nJIang; }

	bool check19(VEC_CARD vCards);

protected:
	bool checkQingYiSe();
	bool checkDuiDuiHu();
	bool checkBaoDuiDuiHu();

	bool checkKeZhi(bool isHold, uint8_t nCard);
	bool takeKeZhi(VEC_CARD vCards);
	
protected:
	uint8_t m_nSongZhiGangIdx;

	uint8_t m_nBaoCard;
	uint8_t m_nKanBaoCard; // ÊÇ·ñ¿´±¦ÅÆ£¨ºú£©

	std::vector<uint8_t> m_vLouPeng;
	bool m_isLouHu;
	std::vector<tSongGangIdx> m_vMingGangSongIdx;
	std::vector<tSongGangIdx> m_vPengSongIdx;

	bool m_isBiMenHu;
	bool m_haveQingYiSe;
};