#include "TitleLevelConfig.h"

bool CTitleLevelConfig::OnPaser(CReaderRow& refReaderRow )
{
	stTitleConfigItem* pItem = new stTitleConfigItem ;
	pItem->nDiamondNeedMax = refReaderRow["DiamonedNeedMax"]->IntValue();
	pItem->nDiamondNeedMin = refReaderRow["DiamonedNeedMin"]->IntValue();
	pItem->nTitleLevel = refReaderRow["TitleLevel"]->IntValue();
#ifndef GAME_SERVER
	pItem->strTitleName = refReaderRow["TitleName"]->StringValue();
#endif
	m_vAllConfigs.push_back(pItem);
	return true ;
}

unsigned short CTitleLevelConfig::GetTitleLevel(unsigned int nDiamond)
{
	stTitleConfigItem* pitem = GetConfigItem(nDiamond) ;
	if ( pitem )
	{
		return pitem->nTitleLevel ;
	}
	return 0 ;
}

void CTitleLevelConfig::Clear()
{
	VEC_TITLE_CONFIGS::iterator iter = m_vAllConfigs.begin() ;
	for ( ; iter != m_vAllConfigs.end(); ++iter )
	{
		delete *iter ;
		*iter = NULL ;
	}
	m_vAllConfigs.clear() ;
}

#ifndef GAME_SERVER
const char* CTitleLevelConfig::GetTitleName(unsigned int nDiamond)
{
	stTitleConfigItem* pitem = GetConfigItem(nDiamond) ;
	if ( pitem )
	{
		return pitem->strTitleName.c_str() ;
	}
	return NULL ;
}
#endif

CTitleLevelConfig::stTitleConfigItem* CTitleLevelConfig::GetConfigItem(unsigned int nDiamond )
{
	VEC_TITLE_CONFIGS::iterator iter = m_vAllConfigs.begin() ;
	for ( ; iter != m_vAllConfigs.end(); ++iter )
	{
		stTitleConfigItem* pItem = *iter ;
		if ( pItem->nDiamondNeedMin <= nDiamond && nDiamond < pItem->nDiamondNeedMax )
		{
			return pItem ;
		}
	}
	return NULL ;
}