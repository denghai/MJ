//#include <windows.h>
//#include "GoldenServer.h"
//#include <iostream>
//#include <WinDef.h>
//#include "catch_dump_file.h"
//#pragma comment(lib,"JsonDll.lib")
//#include "log4z.h"
//
//DWORD WINAPI ThreadProc(LPVOID lpParam)
//{
//	bool bRunning = true;
//	char pBuffer[255] ;
//	while(bRunning)
//	{
//		memset(pBuffer,0,sizeof(pBuffer)) ;
//		scanf_s("%s",pBuffer,sizeof(pBuffer)) ;
//		if ( strcmp(pBuffer,"exit") == 0 || strcmp(pBuffer,"Q") == 0 )
//		{
//			bRunning = false ;
//			IServerApp* pAp = (IServerApp*)lpParam ;
//			pAp->stop();
//			printf("Closing!!!\n");
//		}
//		else
//		{
//			printf("Input exit or Q , to close the app, current = %s \n", pBuffer) ;
//		}
//	}
//	return 0;
//}
//
//void CreateThred( IServerApp* pApp )
//{
//	DWORD threadID;
//	HANDLE hThread;
//	hThread = CreateThread(NULL,0,ThreadProc,pApp,0,&threadID); // 创建线程
//}
//
//
//LONG WINAPI MyUnhandledExceptionFilter( struct _EXCEPTION_POINTERS* ExceptionInfo )
//{
//	HANDLE lhDumpFile = CreateFile(L"DumpFileForNiuNiuServer.dmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL ,NULL);
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
//void RunFunc ( IServerApp* pApp )
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
//
//
//CatchDumpFile::CDumpCatch g_exception_handler;
//int main()
//{
//	_CrtSetReportMode(_CRT_ASSERT, 0);
//	CGoldenServerApp* theApp = CGoldenServerApp::getInstance() ;
//	CLogMgr::SharedLogMgr()->SetOutputFile("Golden");
//	bool bok = theApp->init() ;
//	if ( !bok )
//	{
//		printf("init data svr error , start up error\n");
//		char c ;
//		scanf_s("%c",&c);
//		return 0 ;
//	}
//	CreateThred(theApp);
//#ifdef NDEBUG
//	RunFunc(theApp);
//#endif // _DEBUG
//#ifdef _DEBUG
//	theApp->run();
//#endif // _DEBUG
//	return 0 ;
//}

#include "GoldenServer.h"
#include "Application.h"
int main()
{
	CApplication theAplication(CGoldenServerApp::getInstance());
	theAplication.startApp();
	return 0 ;
}