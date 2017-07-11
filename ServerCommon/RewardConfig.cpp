#include "RewardConfig.h"
CRewardConfig::~CRewardConfig()
{
	for ( auto p : m_allReward )
	{
		delete p.second ;
		p.second = nullptr ;
	}
	m_allReward.clear() ;
}

bool CRewardConfig::OnPaser(CReaderRow& refReaderRow )
{
	stReward* p = new stReward ;
	p->eType = (eRewardType)refReaderRow["rewardType"]->IntValue() ;
	p->nCupCnt = refReaderRow["cupCnt"]->IntValue() ;
	p->nDiamond = refReaderRow["diamondCnt"]->IntValue() ;
	p->nRewardID = refReaderRow["id"]->IntValue() ;
	p->strRewardDesc = refReaderRow["desc"]->StringValue();
	p->nCoin = refReaderRow["coin"]->IntValue() ;
	auto pp = getRewardByID(p->nRewardID);
	if ( pp )
	{
		printf("already have reward config id = %d\n",p->nRewardID ) ;
		delete p ;
		p = nullptr ;
	}
	else
	{
		m_allReward[p->nRewardID] = p ;
	}
	return true ;
}

CRewardConfig::stReward* CRewardConfig::getRewardByID(uint16_t nRewardID )
{
	auto p = m_allReward.find(nRewardID) ;
	if ( p != m_allReward.end() )
	{
		return p->second ;
	}
	return nullptr ;
}