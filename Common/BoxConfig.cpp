#include "BoxConfig.h"
#include "log4z.h"
bool CBoxConfigMgr::OnPaser(CReaderRow& refReaderRow )
{
	stBoxConfig* pConfig = new stBoxConfig ;
	pConfig->nBoxID = refReaderRow["BoxID"]->IntValue() ;
	pConfig->nLastTime = refReaderRow["LastTime"]->IntValue();
	pConfig->nNextBoxID  = refReaderRow["NextBoxID"]->IntValue();
	pConfig->nRewardCoin  = refReaderRow["RewardCoin"]->IntValue();
	pConfig->nRewardDiamond  = refReaderRow["RewardDiamoned"]->IntValue();
	if ( GetBoxConfigByBoxID(pConfig->nBoxID) )
	{
		LOGFMTE("already have boxid = %d , in box config",pConfig->nBoxID) ;
		return false;
	}
	m_vAllBoxConfig[pConfig->nBoxID] = pConfig ;
	return true ;
}

stBoxConfig* CBoxConfigMgr::GetBoxConfigByBoxID(unsigned short nBoxID )
{
	MAP_BOX_CONFIG::iterator iter = m_vAllBoxConfig.find(nBoxID) ;
	if ( iter != m_vAllBoxConfig.end() )
	{
		return iter->second ;
	}
	return NULL ;
}

void CBoxConfigMgr::Clear()
{
	MAP_BOX_CONFIG::iterator iter = m_vAllBoxConfig.begin();
	for ( ; iter != m_vAllBoxConfig.end(); ++iter )
	{
		delete iter->second ;
		iter->second = NULL ;
	}
	m_vAllBoxConfig.clear();
}