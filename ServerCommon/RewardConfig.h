#pragma once
#include "IConfigFile.h"
#include "CommonDefine.h"
#include <string>
#include <map>
#include "Singleton.h"
class CRewardConfig
	:public IConfigFile
	,public CSingleton<CRewardConfig>
{
public:
	enum eRewardType
	{
		eReward_Mannul,
		eReward_Auto,
		eReward_Max,
	};
	struct stReward 
	{
		 uint16_t nRewardID ;
		 std::string strRewardDesc;
		 eRewardType eType ;
		 uint16_t nCupCnt ;
		 uint32_t nDiamond ;
		 uint32_t nCoin ;
	};
	typedef std::map<uint16_t,stReward*> MAP_REWARD ;
public:
	CRewardConfig(){}
	~CRewardConfig();
	bool OnPaser(CReaderRow& refReaderRow )override ;
	stReward* getRewardByID(uint16_t nRewardID );
protected:
	MAP_REWARD m_allReward ;
};