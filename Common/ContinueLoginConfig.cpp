#include "ContinueLoginConfig.h"
#include "log4z.h"
bool CContiuneLoginConfigMgr::OnPaser(CReaderRow& refReaderRow )
{
	stConLoginConfig* pConfig = new stConLoginConfig ;
	pConfig->nDayIdx = refReaderRow["DayIndex"]->IntValue() ;
	pConfig->nDiamoned = refReaderRow["Diamoned"]->IntValue() ;
	pConfig->nGiveCoin = refReaderRow["Coin"]->IntValue() ;
#ifndef SERVER
	pConfig->strVipRewardDesc = refReaderRow["VipRewardDesc"]->StringValue();
#endif
	
	MAP_CON_LOGIN_CONFIGS::iterator iter = m_vAllConfigs.find(pConfig->nDayIdx );
	if ( iter != m_vAllConfigs.end() )
	{
		LOGFMTE("double contiun login day idx = %d",pConfig->nDayIdx) ;
		delete pConfig ;
		return false;
	}

	// paser items ;
	std::vector<int> vecInt ;
	refReaderRow["ItemIDAndCount"]->VecInt(vecInt);
	if ( vecInt.size() % 2 == 0 )
	{
		stConLoginConfig::stItems nItem ;
		for ( unsigned int i = 0 ; i + 1 < vecInt.size() ; i += 2 )
		{
			nItem.nItemID = vecInt[i] ;
			nItem.nCount = vecInt[i + 1 ] ;
			pConfig->vItems.push_back(nItem) ;
		}
	}

	m_vAllConfigs[pConfig->nDayIdx] = pConfig ;
    if ( pConfig->nDayIdx > m_nMaxDayIdx  )
	{
		m_nMaxDayIdx = pConfig->nDayIdx;
	}
	return true ;
}

stConLoginConfig* CContiuneLoginConfigMgr::GetConfigByDayIdx(unsigned short nDayIdx )
{
	if ( nDayIdx > m_nMaxDayIdx  )
	{
		nDayIdx = m_nMaxDayIdx ;
	}
	MAP_CON_LOGIN_CONFIGS::iterator iter = m_vAllConfigs.find((unsigned char)nDayIdx) ;
	if ( iter == m_vAllConfigs.end() )
	{
		return NULL ;
	}
	return iter->second ;
}

void CContiuneLoginConfigMgr::ClearConfigs()
{
	MAP_CON_LOGIN_CONFIGS::iterator iter = m_vAllConfigs.begin() ;
	for ( ; iter != m_vAllConfigs.end(); ++iter )
	{
		if ( iter->second )
		{
			delete iter->second  ;
			iter->second = NULL ;
		}
	}
	m_vAllConfigs.clear() ;
	m_nMaxDayIdx = 0 ;
}