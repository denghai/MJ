#pragma once 
#include "IConfigFile.h"
#include <map>
enum eShopItemType
{
	eShopItem_Diamoned,
	eShopItem_Coin,
	eShopItem_Assets,
	eShopItem_UseItem,
	eShopItem_Other,
	eShopItem_Max,
};

struct stShopItem
{
#ifndef GAME_SERVER
	std::string strItemName ;
	std::string strIcon ;
	std::string strAppStroeIdentifer ;
#endif
	unsigned int nShopItemID ;
    eShopItemType eType;
	unsigned int nItemID ;  // meanning deponed on eType ; 
	unsigned int nCount ;
	unsigned char nPrizeType ; // 0 RMB ,1 diamoned ,2 coin ;
	unsigned int nOrigPrize ; // prize before 
	unsigned int nPrize ;     // current prize ;
	//unsigned int nCanByTimes ;   // 0 means no limit ;
	unsigned int nBeginTime ;   // 0 means no limit ;
	unsigned int nEndTime ;   // 0 means no limit ;
	unsigned char nFlag ; // 0 default , 1 hot , 2 new , 3 welcome , 4 on sale, 5 time limit ; 
public:
	bool IsTimeLimitOk();
};

class CShopConfigMgr
	:public IConfigFile
{
public:
	typedef std::map<unsigned int ,stShopItem*> MAP_SHOP_ITEMS ;
public:
	CShopConfigMgr(){ Clear();}
	~CShopConfigMgr() { Clear() ;}
	bool OnPaser(CReaderRow& refReaderRow );
	stShopItem* GetShopItem(unsigned int nShopItemID );
    
    unsigned int GetCurrencySize();//货币中心
    unsigned int GetPropsSize();//道具
    unsigned int GetAssetSize();//资产
    
    unsigned int GetAppStoreProductSize();//appstore商品
    unsigned int GetAllProductSize(){return (unsigned int)m_vAllShopItems.size();}
    
    stShopItem* GetCurrencyByIndex(unsigned int index);//货币中心
    stShopItem* GetPropsByIndex(unsigned int index);//道具
    stShopItem* GetAssetByIndex(unsigned int index);//资产
    
    stShopItem* GetCoinProductByIndex(unsigned int index);//appstore商品
    
    stShopItem* GetAppStoreProductByIndex(unsigned int index);//appstore商品
    stShopItem* GetAppStoreProductByProductID(std::string productID);//appstore商品
    
    bool UpdateAppStoreProduct(std::string productID,std::string title,float price);
#ifdef GAME_SERVER
	friend class CPlayerShop ;
#endif
protected:
	void Clear();
protected:
	MAP_SHOP_ITEMS m_vAllShopItems ;
};