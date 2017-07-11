//#include <windows.h>
//#include "GameServerApp.h"
//#include <iostream>
//#include <WinDef.h>
//#include "log4z.h"
//#include "catch_dump_file.h"
//#pragma comment(lib,"JsonDll.lib")
//
//DWORD WINAPI ThreadProc(LPVOID lpParam)
//{
//	bool bRunning = true;
//	char pBuffer[255] ;
//	while(bRunning)
//	{
//		memset(pBuffer,0,sizeof(pBuffer)) ;
//		scanf("%s",pBuffer) ;
//		if ( strcmp(pBuffer,"exit") == 0 || strcmp(pBuffer,"Q") == 0 )
//		{
//			bRunning = false ;
//			CGameServerApp* pAp = (CGameServerApp*)lpParam ;
//			pAp->stop();
//			printf("Closing!!!\n");
//		}
//		else
//		{
//			printf("Input exit or Q , to close the app \n") ;
//		}
//	}
//	return 0;
//}
//
//void CreateThred( CGameServerApp* pApp )
//{
//	DWORD threadID;
//	HANDLE hThread;
//	hThread = CreateThread(NULL,0,ThreadProc,pApp,0,&threadID); // 创建线程
//}
//
//
//LONG WINAPI MyUnhandledExceptionFilter( struct _EXCEPTION_POINTERS* ExceptionInfo )
//{
//	HANDLE lhDumpFile = CreateFile(L"DumpFileForDataServer.dmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL ,NULL);
//
//	MINIDUMP_EXCEPTION_INFORMATION loExceptionInfo;
//	loExceptionInfo.ExceptionPointers = ExceptionInfo;
//	loExceptionInfo.ThreadId = GetCurrentThreadId();
//	loExceptionInfo.ClientPointers = TRUE;
//	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),lhDumpFile, MiniDumpNormal, &loExceptionInfo, NULL, NULL);
//
//	CloseHandle(lhDumpFile);
//
//	return EXCEPTION_EXECUTE_HANDLER;
//}
//
//
//void RunFunc ( CGameServerApp* pApp )
//{
//	// exception 
//	__try
//	{
//		pApp->run() ;
//	}
//	__except(CatchDumpFile::CDumpCatch::UnhandledExceptionFilterEx(GetExceptionInformation()))
//	{
//		LOGFMTI("try to recover from exception") ;
//		pApp->run() ;
//	}
//}
//#include <time.h>
////#include "PlayerGameData.h"
////#include <cassert>
//CatchDumpFile::CDumpCatch g_exception_handler;
//int main()
//{
//	_CrtSetReportMode(_CRT_ASSERT, 0);
//	//uint8_t nCnt = 10 ;
//	//while ( nCnt-- )
//	//{
//	//	uint32_t nRID = CPlayerGameData::generateRoomID(eRoom_NiuNiu) ;
//	//	if ( CPlayerGameData::getRoomType(nRID) != eRoom_NiuNiu )
//	//	{
//	//		assert(0&&"error");
//	//	}
//	//	LOGFMTD(" room id = %u" , nRID) ;
//	//}
//
//	//nCnt = 10 ;
//	//while ( nCnt-- )
//	//{
//	//	uint32_t nRID = CPlayerGameData::generateRoomID(eRoom_Golden) ;
//	//	if ( CPlayerGameData::getRoomType(nRID) != eRoom_Golden )
//	//	{
//	//		assert(0&&"error");
//	//	}
//	//	LOGFMTD(" room id = %u" , nRID) ;
//	//}
//
//	//nCnt = 10 ;
//	//while ( nCnt-- )
//	//{
//	//	uint32_t nRID = CPlayerGameData::generateRoomID(eRoom_TexasPoker) ;
//	//	if ( CPlayerGameData::getRoomType(nRID) != eRoom_TexasPoker )
//	//	{
//	//		assert(0&&"error");
//	//	}
//	//	LOGFMTD(" room id = %u" , nRID) ;
//	//}
//	//-----
//	//char p[20] = {0} ;
//	//sprintf_s(p,"12E44E54") ;
//	//std::string str (p,20);
//	//std::string strNo = str.substr(0,str.find_first_of('E')) ;
//	//int a = atoi(strNo.c_str());
//	//printf("%d",a);
//	//-----
//	CGameServerApp theApp ;
//	bool bok = theApp.init() ;
//	CLogMgr::SharedLogMgr()->SetOutputFile("DataSvr");
//
//	//CTimer* pTimer1 = new CTimer ;
//	//pTimer1->setInterval( 1 );
//	//pTimer1->setIsAutoRepeat(true) ;
//	//pTimer1->setCallBack([](CTimer* pTimer , float fD){ LOGFMTD("invoker timer 1") ;} ) ;
//
//	//CTimer tRimer ;
//	//tRimer.setInterval(2) ;
//	//tRimer.setIsAutoRepeat(true) ;
//	//tRimer.setCallBack([&pTimer1](CTimer* pTimer , float fD){ uint8_t nRand = rand() % 100 ;LOGFMTD("invoker timer 2, %u",nRand ) ;  if (pTimer1)pTimer1->canncel(); delete pTimer1; pTimer1 = nullptr ;} ) ;
//	//tRimer.start() ;
//	//pTimer1->start() ;
//	if ( !bok )
//	{
//		printf("init data svr error , start up error\n");
//		char c ;
//		scanf("%c",&c);
//		return 0 ;
//	}
//	CreateThred(CGameServerApp::SharedGameServerApp());
//	RunFunc(CGameServerApp::SharedGameServerApp());
//	return 0 ;
//}

#include "GameServerApp.h"
#include "Application.h"
int main()
{
	CGameServerApp theApp ;
	CApplication theAplication(&theApp);
	theAplication.startApp();
	return 0 ;
}