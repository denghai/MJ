#pragma once
#include "IConfigFile.h"
#include <vector>
#include <time.h>
class CInformConfig
	:public IConfigFile
{
public:
	struct stInformConfig
	{
		unsigned int nId ;
		std::string strTitle;
		std::string strContent ;
	};
	typedef std::vector<stInformConfig*> VEC_INFORMS ;
public:
	CInformConfig();
	~CInformConfig();
	bool OnPaser(CReaderRow& refReaderRow );
	void OnFinishPaseFile(){ m_tLoadTime = time(NULL) ;}
	unsigned int GetMaxInformID(){ return m_nMaxInformID ; }
	unsigned int GetInformCount(){ return m_vAllInforms.size() ;}
protected:
	void Clear();
	friend class CBrocaster ;
protected:
	VEC_INFORMS m_vAllInforms ;
	time_t m_tLoadTime ;
	unsigned int m_nMaxInformID ;
};