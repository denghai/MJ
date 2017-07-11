#include "InformConfig.h"
CInformConfig::CInformConfig()
{
	Clear();
}

CInformConfig::~CInformConfig()
{
	Clear() ;
}

bool CInformConfig::OnPaser(CReaderRow& refReaderRow )
{
	stInformConfig* pInform = new stInformConfig ;
	pInform->nId = refReaderRow["ID"]->IntValue() ;
	pInform->strTitle = refReaderRow["Title"]->StringValue() ;
	pInform->strContent= refReaderRow["Content"]->StringValue() ;
	m_vAllInforms.push_back(pInform) ;
 
	m_nMaxInformID =  m_nMaxInformID > pInform->nId ? m_nMaxInformID : pInform->nId; 
	return true ;
}

void CInformConfig::Clear()
{
	VEC_INFORMS::iterator iter = m_vAllInforms.begin() ;
	for ( ; iter != m_vAllInforms.end(); ++iter) 
	{
		delete *iter ;
		*iter = NULL ;
	}
	m_vAllInforms.clear() ;
	m_nMaxInformID = 0 ;
}