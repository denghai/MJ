#pragma once
#include "IConfigFile.h"
#include "CommonDefine.h"
#include <string>
#include <map>
class CExchangeConfig
	:public IConfigFile
{
public:
	struct stExchangeItem
	{
		uint16_t nConfigID ;
		std::string strDesc ;
		uint32_t nDiamondNeed ;
		std::string strIcon ;
	};

	typedef std::map<uint16_t,stExchangeItem*> MAP_EXCHANGE ;
public:
	CExchangeConfig(){}
	~CExchangeConfig();
	bool OnPaser(CReaderRow& refReaderRow )override ;
	stExchangeItem* getExchangeByID(uint16_t nConfigID );
	uint16_t getExchangeItemCnt(){ return m_allExchangeItems.size(); }
	MAP_EXCHANGE::iterator getBeginIter(){ return m_allExchangeItems.begin() ;}
	MAP_EXCHANGE::iterator getEndItem(){ return m_allExchangeItems.end(); }
protected:
	MAP_EXCHANGE m_allExchangeItems ;
};