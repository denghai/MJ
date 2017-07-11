#pragma once
#include "IPlayerComponent.h"
#include <list>
class CPlayer ;
struct stShopItem;
class CPlayerShop
	:public IPlayerComponent
{
public:
	struct stBuyedShopItem
	{
		unsigned short nShopItemID ;
		unsigned int nTimes ;
	};
	typedef std::list<stBuyedShopItem*> LIST_BUYED_SHOP_ITEM ;
public:
	CPlayerShop(CPlayer* pPlayer) ;
	~CPlayerShop() ;
	void OnPlayerDisconnect();
	bool OnMessage(stMsg* pMsg );
	virtual void Reset();
	virtual void Init();
	//unsigned int GetBuyShopItemTimes(unsigned short nShopItemID );
	virtual void TimerSave();
protected:
	void SavePlayerShopToDB();
	void SendShopListToClient();
	void Clear();
	stBuyedShopItem* GetBuyedShpItem(unsigned short nShopItemID );
	float GetDiscountRate();
	void OnGetShopItem(stShopItem* pShopItem ,unsigned int nCount , bool bSelfBuy, CPlayer* pBuyer );
protected:
	LIST_BUYED_SHOP_ITEM m_vAllBuyedShopItems ;
};