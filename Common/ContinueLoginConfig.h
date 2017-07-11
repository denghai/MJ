#pragma once 
#include "IConfigFile.h"
#include <map>
struct stConLoginConfig
{
	struct stItems
	{
		unsigned short nItemID ;
		unsigned short nCount ;
	};
public:
	unsigned char nDayIdx ;
	unsigned int nGiveCoin ;
	unsigned int nDiamoned ;
	std::vector<stItems> vItems ;
#ifndef GAME_SERVER
	std::string strVipRewardDesc ;
#endif
};

class CContiuneLoginConfigMgr
	:public IConfigFile
{
public:
	typedef std::map<unsigned char,stConLoginConfig*> MAP_CON_LOGIN_CONFIGS ;
public:
	CContiuneLoginConfigMgr(){ ClearConfigs();}
	~CContiuneLoginConfigMgr(){ClearConfigs();}
	virtual bool OnPaser(CReaderRow& refReaderRow );
	stConLoginConfig* GetConfigByDayIdx(unsigned short nDayIdx );
protected:
	void ClearConfigs();
protected:
	MAP_CON_LOGIN_CONFIGS m_vAllConfigs ;
	unsigned char m_nMaxDayIdx ;
};