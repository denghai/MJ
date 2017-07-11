#pragma once
#include "IConfigFile.h"
#include <map>
struct stBoxConfig
{
	unsigned short nBoxID ;
	unsigned int nLastTime ; // by second 
	unsigned int nRewardCoin ;
	unsigned int nRewardDiamond ; 
	unsigned short nNextBoxID ;
};

class CBoxConfigMgr
	:public IConfigFile
{
public:
	typedef std::map<unsigned short , stBoxConfig*> MAP_BOX_CONFIG ;
public:
	CBoxConfigMgr(){ Clear();}
	~CBoxConfigMgr(){ Clear();}
	bool OnPaser(CReaderRow& refReaderRow ) ;
	stBoxConfig* GetBoxConfigByBoxID(unsigned short nBoxID );
protected:
	void Clear();
protected:
	MAP_BOX_CONFIG m_vAllBoxConfig ;
};