#pragma once
#include "ConfigReader.h"
class IConfigFile
{
public:
	IConfigFile(){nID = 0 ;}
	virtual ~IConfigFile(){;}
	bool LoadFile(const char* pfile );
	virtual bool OnPaser(CReaderRow& refReaderRow ) = 0;
	void SetID( unsigned short nID ){this->nID = nID ;}
	virtual void OnFinishPaseFile(){}
protected:
	unsigned short nID ;
};