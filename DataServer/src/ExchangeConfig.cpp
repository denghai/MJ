#include "ExchangeConfig.h"
CExchangeConfig::~CExchangeConfig()
{
	for ( auto pairE : m_allExchangeItems )
	{
		delete pairE.second ;
		pairE.second = nullptr ;
	}
	m_allExchangeItems.clear() ;
}

bool CExchangeConfig::OnPaser(CReaderRow& refReaderRow )
{
	auto pItem = new stExchangeItem ;
	pItem->nConfigID = refReaderRow["id"]->IntValue() ;
	pItem->nDiamondNeed = refReaderRow["diamond"]->IntValue();
	pItem->strDesc = refReaderRow["desc"]->StringValue() ;
	pItem->strIcon = refReaderRow["icon"]->StringValue() ;
	auto already = getExchangeByID(pItem->nConfigID) ;
	printf("%s\n",pItem->strDesc.c_str());
	if ( already )
	{
		printf("already have exchange item config id = %d\n",pItem->nConfigID) ;
		delete pItem ;
		pItem = nullptr ;
		return true ;
	}
	m_allExchangeItems[pItem->nConfigID] = pItem ;
	return true ;
}

CExchangeConfig::stExchangeItem* CExchangeConfig::getExchangeByID(uint16_t nConfigID )
{
	auto iter = m_allExchangeItems.find(nConfigID) ;
	if ( iter != m_allExchangeItems.end() )
	{
		return iter->second ;
	}
	return nullptr ;
} 