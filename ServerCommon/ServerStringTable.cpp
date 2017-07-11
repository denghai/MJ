#include "ServerStringTable.h"
bool CServerStringTable::OnPaser(CReaderRow& refReaderRow )
{
	m_allString.vMapIDString[refReaderRow["id"]->IntValue()] = refReaderRow["stringValue"]->StringValue();
	//printf("%s\n",refReaderRow["stringValue"]->StringValue());
	return true ;
}

const char* CServerStringTable::getStringByID(uint16_t nStringID , uint8_t nControy )
{
	auto au = m_allString.vMapIDString.find(nStringID) ;
	if ( au != m_allString.vMapIDString.end() )
	{
		return au->second.c_str();
	}
	return nullptr ;
}