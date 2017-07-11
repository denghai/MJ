//#include <windows.h>
//#include "LogSvrApp.h"
//#include <iostream>
//#include "ThreadMod.h"
//#include "catch_dump_file.h"
//#pragma comment(lib,"JsonDll.lib")
//#include "log4z.h"
////#include "mutex.h"
////#include <my_global.h>
////#include "mysql.h"
////#include "DataBaseThread.h"
////#include "MySqlData.h"
////#include "DBRequest.h"
////#include "RakNetTypes.h"
////#include "RakPeerInterface.h"
////#include "ServerNetwork.h"
////#include "DBPlayerManager.h"
//
//
//CLogSvrApp theApp ;
//class CGetInput
//	:public CThreadT
//{
//	void __run()
//	{
//		char c ;
//		while ( 1 )
//		{
//			c = getchar();
//			if ( c == 'q' || c == 'Q')
//			{
//				theApp.stop();
//				break; 
//			}
//			Sleep(10);
//		}
//	}
//};
//
//LONG WINAPI MyUnhandledExceptionFilter( struct _EXCEPTION_POINTERS* ExceptionInfo )
//{
//	HANDLE lhDumpFile = CreateFile(L"DumpFileForlog.dmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL ,NULL);
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
//void RunFunc ( CLogSvrApp* pApp )
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
//CatchDumpFile::CDumpCatch g_exception_handler;
//int main()
//{
//	//SetConsoleCtrlHandler(ConsoleHandler, TRUE); 
//	_CrtSetReportMode(_CRT_ASSERT, 0);
//	CGetInput input ;
//	input.Start();
//	CLogMgr::SharedLogMgr()->SetOutputFile("Log");
//	theApp.init();
//#ifdef NDEBUG
//	RunFunc(&theApp) ;
//#else
//	theApp.run();
//#endif
//	theApp.onExit();
//	Sleep(3000) ; // wait other thread finish work ;
//	return 0 ; 
//}

#include "LogSvrApp.h"
#include "Application.h"
int main()
{
	CLogSvrApp theApp ;
	CApplication theAplication(&theApp);
	theAplication.startApp();
	return 0 ;
}