#pragma once
#include "IConfigFile.h"
#include "CommonDefine.h"
#include <string>
#include <map>
#include "Singleton.h"
class CServerStringTable
	:public IConfigFile
	,public CSingleton<CServerStringTable>
{
public:
	struct StringCollection 
	{
		std::map<uint16_t,std::string> vMapIDString ;
	};
public:
	CServerStringTable(){}
	~CServerStringTable() {;}
	bool OnPaser(CReaderRow& refReaderRow )override ;
	const char* getStringByID(uint16_t nStringID , uint8_t nControy = 0);
protected:
	StringCollection m_allString ;
};