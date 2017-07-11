//
//  LogManager.cpp
//  God
//
//  Created by Xu BILL on 12-10-25.
//
//
#pragma warning(disable:4996)

#include "log4z.h"
#include <stdarg.h>
#include <time.h>
#ifndef SERVER
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
#define POKER_CLIENT 1
#endif
#endif // !SERVER

#ifdef POKER_CLIENT
#include <string>
#else

#include <string.h>
#include <Windows.h>
#endif
#define MAX_LOG_FILE_SIZE_BYTES (1024*1024*10)   // 10 M PER FILE 
CLogMgr* CLogMgr::SharedLogMgr()
{
    static CLogMgr g_sLogMgr ;
    return &g_sLogMgr ;
}

CLogMgr::CLogMgr()
{
    pFile = NULL ;
    bOutPutToFile = false ;
    bEnable = true ;
	nSerialNum = 0 ;
#ifndef POKER_CLIENT
	char szPath[MAX_PATH] = { 0 };
	::GetModuleFileNameA(NULL, szPath, MAX_PATH);
	std::string strMoudleName = szPath ;
	auto nStarPos = strMoudleName.find_last_of("\\/");
	auto nEndPos = strMoudleName.find_last_of('.');
	strMoudleName = strMoudleName.substr(nStarPos + 1 , nEndPos - nStarPos - 1 ) ;
	strFilePre = strMoudleName;
#endif
}

CLogMgr::~CLogMgr()
{
    if ( pFile )
    {
        fclose(pFile) ;
    }
}

void CLogMgr::SetDisablePrint(bool bEnable)
{
    this->bEnable = bEnable ;
}

void CLogMgr::PrintLog(const char *sformate, ...)
{
//#ifdef NDEBUG
//	return ;
//#endif
    va_list va ;
    va_start(va,sformate);
    Print(sformate, va,eLogState_Noraml);
    va_end(va) ;
}

void CLogMgr::ErrorLog(const char *sformate, ...)
{
	va_list va ;
	va_start(va,sformate);
	Print(sformate, va,eLogState_Error);
	va_end(va) ;
}

void CLogMgr::SystemLog(const char* sformate , ...)
{
	va_list va ;
	va_start(va,sformate);
	Print(sformate, va,eLogState_System);
	va_end(va) ;
}

void CLogMgr::SetOutputFile(const char *pFilenamePre)
{
//#ifndef _DEBUG
	// auto get exe name in ctr function ;
	//strFilePre = pFilenamePre ;
	bOutPutToFile = true ;
	RefreshFileState();
//#endif
}

void CLogMgr::Print(const char *sFormate, va_list va , eLogState eSate )
{
    if ( bEnable == false )
        return ;
    
    static char pBuffer[1024*3] = { 0 } ;
    memset(pBuffer,0,sizeof(pBuffer));
	// time ;
	time_t t;
	time(&t);
	char* pstr = ctime(&t) ;
	*(pstr + strlen(pstr)) = 0 ;
	*(pstr + strlen(pstr)-1) = 0 ;
	//
#ifndef POKER_CLIENT
    if ( eSate == eLogState_Error )
    {
        sprintf_s(pBuffer,1024*3, "Error: [%s] %s\n",pstr,sFormate);
    }
    else if ( eSate == eLogState_Noraml )
    {
        sprintf_s(pBuffer,1024*3, "Log: [%s] %s \n",pstr,sFormate);
    }
    else if ( eSate == eLogState_Warnning )
    {
        sprintf_s(pBuffer,1024*3, "Warnning: [%s] %s \n",pstr,sFormate);
    }
	else if ( eSate == eLogState_System )
	{
		sprintf_s(pBuffer,1024*3, "System: [%s] %s \n",pstr,sFormate);
	}
#endif
#if defined(_WIN64) || defined( _WIN32)
	switch ( eSate )
	{
	case eLogState_Error:
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY |FOREGROUND_RED);
		}
		break;
	case eLogState_Noraml:
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);
		}
		break;
	case eLogState_System:
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY |FOREGROUND_GREEN);
		}
		break;
	case eLogState_Warnning:
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY |FOREGROUND_RED|FOREGROUND_GREEN);
		}
		break;
	default:
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY |FOREGROUND_RED);
		}
		break;
	}
#endif
	
    if ( bOutPutToFile && pFile/* && (eSate == eLogState_Error || eLogState_System == eSate )*/)
    {
        vfprintf(pFile, pBuffer, va) ;
		fflush(pFile);
		RefreshFileState();
    }
   // else
    {
        vprintf(pBuffer, va) ;
    }
}

void CLogMgr::CloseFile()
{
    if ( bOutPutToFile && pFile )
    {
        fclose(pFile) ;
        pFile = NULL ;
        bOutPutToFile = false ;
    }
}

void CLogMgr::RefreshFileState()
{
	if ( bOutPutToFile == false )
	{
		return ;
	}

	if ( pFile && ftell(pFile) < MAX_LOG_FILE_SIZE_BYTES )
	{
		return  ;
	}

	if ( pFile )
	{
		fclose(pFile);
		pFile = NULL ;
	}

	// create new file ;
	char pFileName[1024] = { 0 } ;
	if ( nSerialNum == 0 )
	{
		time_t tCur ;
		time(&tCur);
		tm t ;
		t = *localtime(&tCur);
#ifndef POKER_CLIENT
		sprintf_s(pFileName,sizeof(pFileName),"./log/%s%d_%02d_%02d_%02dh%02dm%02ds",strFilePre.c_str(),1900+t.tm_year,t.tm_mon+1,t.tm_mday,t.tm_hour,t.tm_min,t.tm_sec);
		strFilePre = pFileName ;
		memset(pFileName,0,sizeof(pFileName));
#endif
	}
#ifndef POKER_CLIENT
	sprintf_s(pFileName,sizeof(pFileName),"%s_%d.txt",strFilePre.c_str(),nSerialNum++);
#endif
	pFile = fopen(pFileName, "w");
	bOutPutToFile = pFile != nullptr ;
	return ;
}
