#include "ItemConfig.h"
CItemConfigManager::CItemConfigManager()
{
	ClearAllItems();
}

CItemConfigManager::~CItemConfigManager()
{
	ClearAllItems();
}

bool CItemConfigManager::OnPaser(CReaderRow& refReaderRow )
{
	eItemType eType = (eItemType)refReaderRow["Type"]->IntValue() ;
	stItemConfig* pItem = NULL ; 
	if ( eItem_Gift == eType )
	{
		pItem = new stItemGift ;
	}
	else
	{
		pItem = new stItemConfig ;
	}

	pItem->nItemID = refReaderRow["ItemID"]->IntValue() ;
	if ( GetItemConfigByItemID(pItem->nItemID) != NULL )
	{
		delete pItem ;
		pItem = NULL ;
		return false ;
	}
#ifndef GAME_SERVER
    pItem->strDesc = refReaderRow["Desc"]->StringValue() ;
	pItem->strIcon = refReaderRow["Icon"]->StringValue() ;
	pItem->strName = refReaderRow["Name"]->StringValue() ;
#endif
	pItem->eType = (eItemType)refReaderRow["Type"]->IntValue() ;
	pItem->nPrizeCoin = refReaderRow["PrizeCoin"]->IntValue();
	pItem->nPrizeDiamoned = refReaderRow["PrizeDiamoned"]->IntValue() ;
	pItem->nValue = refReaderRow["Value"]->IntValue() ;
	m_vAllItems[pItem->nItemID] = pItem ;

	// read gift 
	if ( pItem->eType == eItem_Gift )
	{
		stItemGift* pGift = (stItemGift*)pItem ;
		pGift->bOpenRightNow = refReaderRow["IsOpenRightNow"]->IntValue() ;
		pGift->nCoin = refReaderRow["CoinWithIn"]->IntValue();
		pGift->nCoinRand = refReaderRow["CoinWithInRand"]->IntValue();
		pGift->nDiamoned = refReaderRow["DiamonedWithIn"]->IntValue();
		pGift->nDiamonedRand = refReaderRow["DiamonedRand"]->IntValue();
		std::vector<int> vInt ;
		refReaderRow["ItemWithIn"]->VecInt(vInt) ;
		if ( vInt.empty() == false && vInt.size() % 2 == 0 )
		{
			for ( unsigned int i = 0 ; i + 1 < vInt.size(); i += 2 )
			{
				pGift->vItemsAndCount[vInt[i]] = vInt[i+1] ;
			}
		}
		else
		{
			pGift->vItemsAndCount.clear();
		}
	}
	return true ;
}

stItemConfig* CItemConfigManager::GetItemConfigByItemID(unsigned short nItemID )
{
	MAP_ITEM_CONFIGS::iterator iter = m_vAllItems.find(nItemID) ;
	if ( iter != m_vAllItems.end() )
		return iter->second ;
	return NULL ;
}

void CItemConfigManager::ClearAllItems()
{
	MAP_ITEM_CONFIGS::iterator iter = m_vAllItems.begin() ;
	for ( ; iter != m_vAllItems.end(); ++iter)
	{
		if ( iter->second )
		{
			delete iter->second ;
			iter->second = NULL ;
		}
	}
	m_vAllItems.clear() ;
}