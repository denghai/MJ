#pragma once 
#include "IConfigFile.h"
#include "NativeTypes.h"
#include <map>
#include "CommonDefine.h"
struct stItemConfig
{
	unsigned short nItemID ;
	eItemType eType ;
#ifndef GAME_SERVER
	std::string strName;
	std::string strDesc ;
	std::string strIcon ;
#endif
	uint64_t nPrizeCoin ;
	int nPrizeDiamoned ;
	unsigned int nValue ; // keep value ;
};

struct stItemGift
	:public stItemConfig
{
	bool bOpenRightNow ;
	uint64_t nCoin ;
	uint64_t nCoinRand ;
	unsigned int nDiamoned;
	unsigned int nDiamonedRand ;
	std::map<unsigned short, unsigned short> vItemsAndCount ;
};

class CItemConfigManager
	:public IConfigFile
{
public:
	typedef std::map<unsigned short,stItemConfig*> MAP_ITEM_CONFIGS ;
public:
	CItemConfigManager();
	~CItemConfigManager();
	virtual bool OnPaser(CReaderRow& refReaderRow );
	stItemConfig* GetItemConfigByItemID(unsigned short nItemID );
protected:
	void ClearAllItems();
protected:
	MAP_ITEM_CONFIGS m_vAllItems ;
};