#include "MissionConfig.h"
bool CMissionConfigMgr::OnPaser(CReaderRow& refReaderRow )
{
	stMissionConfig* pconfig = new stMissionConfig ;
	pconfig->eType = (eMissionType)refReaderRow["MissionType"]->IntValue() ;
	pconfig->nMissionID = refReaderRow["ID"]->IntValue() ;
	if ( GetMissionConfigByID(pconfig->nMissionID))
	{
		delete pconfig ;
		pconfig = NULL ;
		return false ;
	}
	pconfig->nMissionProcess = refReaderRow["MissionProcess"]->IntValue();
	pconfig->nRewardCoin = refReaderRow["RewardCoin"]->IntValue() ;
	pconfig->nMissionValue = refReaderRow["MissionValue"]->IntValue() ;
	std::vector<int> vInts ;
	refReaderRow["RewardItems"]->VecInt(vInts) ;
	if ( vInts.size() != 0 && vInts.size() % 2 == 0 )
	{
		for ( unsigned int i = 0 ; i + 1 < vInts.size() ; i += 2 )
		{
			pconfig->vItemsAndCount[vInts[i]] = vInts[i+1] ;
		}
	}
	else
	{
		pconfig->vItemsAndCount.clear() ;
	}
#ifndef GAME_SERVER
	pconfig->stdDesc = refReaderRow["Desc"]->StringValue() ;
	pconfig->strIcon = refReaderRow["Icon"]->StringValue() ;
	pconfig->strName = refReaderRow["Name"]->StringValue() ;
    printf("\n%s----",pconfig->stdDesc.c_str());
#endif
	m_vAllMission[pconfig->nMissionID] = pconfig ;
	return true ;
}

stMissionConfig* CMissionConfigMgr::GetMissionConfigByID(unsigned short nMissionID )
{
	MAP_MISSION::iterator iter = m_vAllMission.find(nMissionID) ;
	if ( iter != m_vAllMission.end() )
	{
		return iter->second ;
	}
	return NULL ;
}

void CMissionConfigMgr::Clear()
{
	MAP_MISSION::iterator iter = m_vAllMission.begin() ;
	for ( ; iter != m_vAllMission.end(); ++iter )
	{
		delete iter->second ;
		iter->second = NULL ;
	}
	m_vAllMission.clear() ;
}