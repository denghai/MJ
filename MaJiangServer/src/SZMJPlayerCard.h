#pragma once
#include "MJPlayerCard.h"
#include "json/json.h"
class SZMJPlayerCard
	:public MJPlayerCard
{
public:
	bool canEatCard(uint8_t nCard, uint8_t& nWithA, uint8_t& withB) override { return false; }
	void reset() override;
	void onBuHua(uint8_t nHuaCard, uint8_t nCard);
	bool getCardInfo(Json::Value& jsPeerCards);
	bool onDoHu(bool isZiMo, bool isHaiDiLoaYue, uint8_t nCard, std::vector<uint16_t>& vHuTypes, uint16_t& nHuHuaCnt, uint16_t& nHardAndSoftHua);
	uint8_t getSongGangIdx();
	void setSongGangIdx( uint8_t nIdx );
	uint8_t getHuaCardToBuHua(); // -1 means no target ;
	bool canHuWitCard(uint8_t nCard)override;
protected:
	uint8_t getHuaCntWithoutHuTypeHuaCnt();
	bool checkDaMenQing();
	bool checkXiaoMenQing();
	bool checkHunYiSe();
	bool checkQingYiSe();
	bool checkDuiDuiHu();
	bool checkQiDui();
	bool checkHaoHuaQiDui();
	bool checkDaDiaoChe();
protected:
	VEC_CARD m_vBuHuaCard;
	uint8_t m_nSongZhiGangIdx;
};