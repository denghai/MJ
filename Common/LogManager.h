//
//  LogManager.h
//  God
//
//  Created by Xu BILL on 12-10-25.
//
//

#ifndef God_LogManager_h
#define God_LogManager_h
#include <stdio.h>
#include <string>
class CLogMgr
{
public:
    enum eLogState
    {
        eLogState_Noraml,
        eLogState_Warnning,
        eLogState_Error,
		eLogState_System,
        eLogState_Max,
    };
public:
    static CLogMgr* SharedLogMgr();
    CLogMgr();
    ~CLogMgr();
    void SetDisablePrint( bool bEnable );
    void PrintLog( const char* sformate , ... );
    void ErrorLog( const char* sformate , ... );
	void SystemLog(const char* sformate , ...);
    void SetOutputFile( const char* pFilePre );
    void CloseFile();
protected:
    void Print( const char* sFormate , va_list va , eLogState eSate ) ;
	void RefreshFileState();
protected:
    bool bOutPutToFile ;
    bool bEnable ;
	unsigned int nSerialNum; // for output file name ;
	std::string strFilePre ; // for output file name ;
    FILE* pFile ;
};
#endif
