#pragma once
#include "NativeTypes.h"
#include <map>
#include <cassert>
#include "json/json.h"
#include "MessageIdentifer.h"
#include "log4z.h"
struct ISettle
{
	eSettleType eType;
	virtual bool isContainPlayer( uint8_t nIdx ) = 0 ;
	virtual bool writePlayerBillInfo(uint8_t nIdx, Json::Value& jsInfo) = 0;  // { type : 234 , offset : -23 , huType : 23 , beiShu : 234 , target : [2,4] }
	virtual bool writeSettleInfo(Json::Value& jsInfo) = 0 ;  // depend on detail ;
};

struct stHuItemInfo
{
	uint32_t nHuType;
	uint32_t nBeiShu;
	uint32_t nWinCoin;
};

struct stSettleDiaoPao
	:public ISettle
{
	stSettleDiaoPao(uint8_t nInvokerIdx, bool isQiangGao, bool isGangShangPao){ this->isGangShangPao = isGangShangPao; isRobotGang = isQiangGao; eType = eSettle_DianPao; this->nInvokerIdx = nInvokerIdx; nLoseCoin = 0; vMapHuAndCoin.clear(); }
	void addHuPlayer(uint8_t nIdx, uint32_t nWinCoin, uint32_t nHuType, uint8_t nBeiShu )
	{
		auto iter = vMapHuAndCoin.find(nIdx);
		if (iter != vMapHuAndCoin.end())
		{
			assert(0&& "can not add twice");
		}
		nLoseCoin += nWinCoin;

		stHuItemInfo stInfo;
		stInfo.nBeiShu = nBeiShu;
		stInfo.nHuType = nHuType;
		stInfo.nWinCoin = nWinCoin;
		vMapHuAndCoin[nIdx] = stInfo;
	}

	bool isContainPlayer(uint8_t nIdx)override
	{
		if (nIdx == nInvokerIdx)
		{
			return true;
		}

		auto iter = vMapHuAndCoin.find(nIdx);
		return iter != vMapHuAndCoin.end();
	}

	bool writePlayerBillInfo(uint8_t nIdx, Json::Value& jsInfo)override
	{
		if (!isContainPlayer(nIdx) )
		{
			return false;
		}

		if (nIdx == nInvokerIdx)
		{
			if (vMapHuAndCoin.empty())
			{
				LOGFMTE("type = %u hu and coin is null ,invoker idx = %u",eType,nInvokerIdx);
				return false;
			}
			//{ type: 234, offset : -23, huType : 23, beiShu : 234, target : [2, 4] }
			jsInfo["type"] = eType;
			jsInfo["offset"] = -1 * (int32_t)nLoseCoin;
			jsInfo["huType"] = vMapHuAndCoin.begin()->second.nHuType;
			jsInfo["beiShu"] = vMapHuAndCoin.begin()->second.nBeiShu;
			jsInfo["isGangShangPao"] = isGangShangPao ? 1 : 0;
			jsInfo["isRobotGang"] = isRobotGang ? 1 : 0;
			Json::Value jsTarget;
			Json::Value vhuType;
			for (auto& iter : vMapHuAndCoin)
			{
				jsTarget[jsTarget.size()] = iter.first;
				vhuType[vhuType.size()] = iter.second.nHuType;
			}

			if (eType == eSettle_DianPao)
			{
				jsInfo["vHuTypes"] = vhuType;
			}
			jsInfo["target"] = jsTarget;
		}
		else
		{
			auto iter = vMapHuAndCoin.find(nIdx);
			if (iter == vMapHuAndCoin.end())
			{
				return false;
			}

			jsInfo["type"] = eType;
			jsInfo["offset"] = iter->second.nWinCoin;
			jsInfo["huType"] = iter->second.nHuType;
			jsInfo["beiShu"] = iter->second.nHuType;
			jsInfo["isGangShangPao"] = isGangShangPao ? 1 : 0;
			jsInfo["isRobotGang"] = isRobotGang ? 1 : 0;
			Json::Value jsTarget;
			jsTarget[0u] = nInvokerIdx;
			jsInfo["target"] = jsTarget;
		}
		return true;
	}

	bool writeSettleInfo(Json::Value& jsInfo)override  // { paoIdx : 234 , isGangPao : 0 , isRobotGang : 0 , huPlayers : [ { idx : 2 , coin : 2345 }, { idx : 2, coin : 234 }, ... ]  }
	{
		jsInfo["paoIdx"] = nInvokerIdx;
		jsInfo["isGangPao"] = isGangShangPao;
		jsInfo["isRobotGang"] = isRobotGang;
		Json::Value jsPlayers;
		for (auto& refPair : vMapHuAndCoin)
		{
			Json::Value js;
			js["idx"] = refPair.first;
			js["coin"] = refPair.second.nWinCoin;
			jsPlayers[jsPlayers.size()] = js;
		}
		jsInfo["huPlayers"] = jsPlayers;
		return true;
	}
private:
	uint8_t nInvokerIdx;
	uint32_t nLoseCoin;
	bool isGangShangPao;
	bool isRobotGang;
	std::map<uint8_t, stHuItemInfo> vMapHuAndCoin;
};

struct ISettleGang
	:public ISettle
{
	uint8_t nGangIdx;
};

struct stSettleMingGang
	:public ISettleGang
{
	stSettleMingGang(uint8_t nInvokeIdx, uint8_t nGangIdx, uint32_t nCoin)
	{
		this->nGangIdx = nGangIdx;
		eType = eSettle_MingGang; 
		nInvokerIdx = nInvokeIdx; 
		nCoinOffset = nCoin; 
	}

	bool isContainPlayer(uint8_t nIdx)override
	{
		if (nIdx == nInvokerIdx || nIdx == nGangIdx )
		{
			return true;
		}
		return false;
	}

	bool writePlayerBillInfo(uint8_t nIdx, Json::Value& jsInfo)override
	{
		if (isContainPlayer(nIdx) == false )
		{
			return false;
		}

		jsInfo["type"] = eType;
		jsInfo["huType"] = 0;
		jsInfo["beiShu"] = 0;
		Json::Value jsTarget;
		if (nIdx == nGangIdx)
		{
			jsInfo["offset"] = nCoinOffset;
			jsTarget[0u] = nInvokerIdx;
		}
		else
		{
			jsInfo["offset"] = (int32_t)nCoinOffset * -1 ;
			jsTarget[0u] = nGangIdx;
		}
		jsInfo["target"] = jsTarget;
		return true;
	}

	bool writeSettleInfo(Json::Value& jsInfo)override  // { invokerIdx : 234 , gangIdx : 234 , gangWin : 2344 }
	{
		jsInfo["invokerIdx"] = nInvokerIdx;
		jsInfo["gangIdx"] = nGangIdx;
		jsInfo["gangWin"] = nCoinOffset;
		return true;
	}
public:
	uint8_t nInvokerIdx;
	uint32_t nCoinOffset;
};

struct stSettleAnGang
	:public ISettleGang
{
	stSettleAnGang(uint8_t GangIdx){ eType = eSettle_AnGang; this->nGangIdx = GangIdx; nWinCoin = 0; vMapPlayeIdxAndCoin.clear(); }
	void addLosePlayer(uint8_t nIdx, uint32_t nLoseCoin)
	{
		auto iter = vMapPlayeIdxAndCoin.find(nIdx);
		if (iter != vMapPlayeIdxAndCoin.end())
		{
			assert(0 && "can not add twice");
		}
		nWinCoin += nLoseCoin;
		vMapPlayeIdxAndCoin[nIdx] = nLoseCoin;
	}

	uint32_t getWinCoin()
	{
		return nWinCoin;
	}

	bool isContainPlayer(uint8_t nIdx)override
	{
		if (nIdx == nGangIdx)
		{
			return true;
		}

		auto iter = vMapPlayeIdxAndCoin.find(nIdx);
		return iter != vMapPlayeIdxAndCoin.end();
	}

	bool writePlayerBillInfo(uint8_t nIdx, Json::Value& jsInfo)override
	{
		if (isContainPlayer(nIdx) == false)
		{
			return false;
		}

		jsInfo["type"] = eType;
		jsInfo["huType"] = 0;
		jsInfo["beiShu"] = 0;
		Json::Value jsTarget;
		if (nIdx == nGangIdx)
		{
			jsInfo["offset"] = getWinCoin();
			for (auto& ref : vMapPlayeIdxAndCoin)
			{
				jsTarget[jsTarget.size()] = ref.first;
			}
		}
		else
		{
			auto iter = vMapPlayeIdxAndCoin.find(nIdx);
			if (iter == vMapPlayeIdxAndCoin.end())
			{
				LOGFMTE("idx = %u be win but have recorder ? ",nIdx);
				return false;
			}
			jsInfo["offset"] = (int32_t)iter->second * -1;
			jsTarget[0u] = nGangIdx;
		}
		jsInfo["target"] = jsTarget;
		return true;
	}

	bool writeSettleInfo(Json::Value& jsInfo)override // { gangIdx : 234 , losers : [ {idx : 23 , lose : 234 } , .....  ] } 
	{
		jsInfo["gangIdx"] = nGangIdx;
		Json::Value jsLosers;
		for (auto& ref : vMapPlayeIdxAndCoin)
		{
			Json::Value js;
			js["idx"] = ref.first;
			js["lose"] = ref.second;
			jsLosers[jsLosers.size()] = js;
		}
		jsInfo["losers"] = jsLosers;
		return true;
	}

public:
	uint32_t nWinCoin;
	std::map<uint8_t, uint32_t> vMapPlayeIdxAndCoin;
};

struct stSettleBuGang
	:public stSettleAnGang
{
	stSettleBuGang(uint8_t gangIdx) :stSettleAnGang(gangIdx){ eType = eSettle_BuGang; }
};

struct stSettleZiMo
	:public stSettleAnGang
{
	stSettleZiMo(uint8_t huIdx, uint32_t huType, uint8_t beiShu, bool isGangHua) :stSettleAnGang(huIdx){ eType = eSettle_ZiMo; nHuType = huType, nBeiShu = beiShu; isGangShangHua = isGangHua; }
	bool writePlayerBillInfo(uint8_t nIdx, Json::Value& jsInfo)override
	{
		if (isContainPlayer(nIdx) == false)
		{
			return false;
		}
		stSettleAnGang::writePlayerBillInfo(nIdx,jsInfo);
		jsInfo["huType"] = nHuType;
		jsInfo["beiShu"] = nBeiShu;
		jsInfo["isGangShangHua"] = isGangShangHua ? 1 : 0;
		return true;
	}

	bool writeSettleInfo(Json::Value& jsInfo)override // { ziMoIdx : 234 , losers : [ {idx : 23 , lose : 234 } , .....  ] } 
	{
		stSettleAnGang::writeSettleInfo(jsInfo);
		jsInfo["ziMoIdx"] = nGangIdx;
		return true;
	}
public:
	uint32_t nHuType;
	uint8_t nBeiShu;
	bool isGangShangHua;
};

struct stSettleHuaZhu
	:public stSettleDiaoPao
{
	stSettleHuaZhu(uint8_t nHuaZhuIdx) :stSettleDiaoPao(nHuaZhuIdx, 0, 0) { eType = eSettle_HuaZhu; }
	bool writeSettleInfo(Json::Value& jsInfo)override
	{
		return false;
	}
};

struct stSettleDaJiao
	:public stSettleHuaZhu
{
	stSettleDaJiao(uint8_t nDaJiaoIdx) : stSettleHuaZhu(nDaJiaoIdx){ eType = eSettle_DaJiao;  }
	bool writeSettleInfo(Json::Value& jsInfo)override
	{
		return false;
	}
};
	

