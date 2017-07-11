#pragma once
#include "IConfigFile.h"
#include <vector>
class CTitleLevelConfig
	:public IConfigFile
{
public:
	struct stTitleConfigItem
	{
		unsigned short nTitleLevel ;
		unsigned int nDiamondNeedMin ;
		unsigned int nDiamondNeedMax ;
#ifndef GAME_SERVER
		std::string strTitleName ;
#endif
	};
	typedef std::vector<stTitleConfigItem*> VEC_TITLE_CONFIGS ;
public:
	CTitleLevelConfig(){ Clear();}
	~CTitleLevelConfig(){Clear();}
	virtual bool OnPaser(CReaderRow& refReaderRow );
	unsigned short GetTitleLevel(unsigned int nDiamond);
	void Clear();

#ifndef GAME_SERVER
	const char* GetTitleName(unsigned int nDiamond);
#endif
protected:
	stTitleConfigItem* GetConfigItem(unsigned int nDiamond );
protected:
	VEC_TITLE_CONFIGS m_vAllConfigs ;
};