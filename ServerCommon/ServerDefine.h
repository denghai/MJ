#pragma once
#pragma pack(push)
#pragma pack(1)
#include "ServerCommon.h"

struct stEnterRoomLimitCondition
{
	bool isRegisted ;
	uint32_t nCoinLowLimit ;
	uint32_t nCoinUpLimit ;
};

struct stEnterRoomData
{
	uint32_t nUserUID ;
	uint32_t nUserSessionID ;
	uint8_t nNewPlayerHaloWeight ; // xin shou  guang huan quan zhong 
	uint8_t nPlayerType ; // ePlayerType 
	bool isRegisted ;
	uint32_t nCoin ;
	uint32_t nDiamond;
};




#pragma pack(pop)//