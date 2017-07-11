#pragma once
#include "NativeTypes.h"
#include "IConfigFile.h"
#include <list>
#include <map>
#include "CommonDefine.h"
struct stBaseRoomConfig
{
	uint16_t nConfigID ;
	uint8_t nGameType ;
	uint32_t nRentFeePerDay;
	uint32_t nDeskFee;
	float fDividFeeRate;  // chou shui bi li
	
	//std::vector<int32_t> vRewardID ;
	//uint32_t nCoinTopLimit ;
	//uint32_t nCoinLowLimit ;
	//bool bIsNeedRegistered ;
	//bool bIsOmitNewPlayerHalo ;
	//uint32_t nMaxLose ;
#ifdef SERVER
	uint8_t nNeedRobotLevel ;
#endif
};

struct stSitableRoomConfig
	:public stBaseRoomConfig
{
	//uint32_t nMiniTakeInCoin;
	unsigned short nMaxSeat ;
	uint32_t nBaseTakeIn ;
};


struct stTaxasRoomConfig
	:public stSitableRoomConfig
{
	//uint32_t nMaxTakeInCoin ;
	uint32_t nBigBlind ;
};

struct stNiuNiuRoomConfig
	:public stSitableRoomConfig
{
	uint32_t nBaseBet ;
};

struct stGoldenRoomConfig
	:public stNiuNiuRoomConfig
{
//	unsigned int nMiniBet ; // di zhu 
//	unsigned short nTitleNeedToEnter ;
//	unsigned char nChangeCardRound; 
//	bool bCanDoublePK ;
//#ifndef SERVER
//	unsigned int vCoinLevels[GOLDEN_ROOM_COIN_LEVEL_CNT];
//#endif
};

typedef std::vector<stTaxasRoomConfig*> VEC_BLIND_ROOM_CONFIG ;

class CRoomConfigMgr 
	:public IConfigFile
{
public:
	typedef std::list<stBaseRoomConfig*> LIST_ROOM_CONFIG ;
	typedef LIST_ROOM_CONFIG::iterator LIST_ITER ;
public:
	CRoomConfigMgr(){ Clear();}
	~CRoomConfigMgr(){Clear();}
	bool OnPaser(CReaderRow& refReaderRow );
	LIST_ITER GetBeginIter(){ return m_vAllConfig.begin(); }
	LIST_ITER GetEndIter(){ return m_vAllConfig.end();}
	virtual void OnFinishPaseFile();
	int GetConfigCnt();
	stBaseRoomConfig* GetConfigByConfigID( uint16_t nConfigID );
#ifndef SERVER
    int GetConfigCntByRoomType(int roomType);
    stBaseRoomConfig* GetConfigByConfigID( uint16_t idx , int roomType);
#endif
protected:
	void Clear() ;
protected:
	LIST_ROOM_CONFIG m_vAllConfig ;
};