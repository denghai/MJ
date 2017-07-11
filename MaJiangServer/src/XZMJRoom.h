#pragma once 
#include "XLMJRoom.h"
class XZMJRoom
	:public XLMJRoom
{
public:
	bool isGameOver()override;
	bool isAnyPlayerPengOrHuThisCard(uint8_t nInvokeIdx, uint8_t nCard)override;
	bool isAnyPlayerRobotGang(uint8_t nInvokeIdx, uint8_t nCard)override;
	void onAskForPengOrHuThisCard(uint8_t nInvokeIdx, uint8_t nCard, std::vector<uint8_t>& vOutWaitHuIdx, std::vector<uint8_t>& vOutWaitPengGangIdx, bool& isNeedWaitEat)override;
	void onAskForRobotGang(uint8_t nInvokeIdx, uint8_t nCard, std::vector<uint8_t>& vOutCandinates)override;
	uint8_t getNextActPlayerIdx(uint8_t nCurActIdx);
	uint8_t getRoomType()override{ return eMJ_BloodTheEnd; }
protected:
	bool canKouPlayerCoin(uint8_t nPlayerIdx) override;
};