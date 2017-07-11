//#include <windows.h>
//#include "main.h"
//#include "LoginApp.h"
//#include <WinDef.h>
//#include "catch_dump_file.h"
//#pragma comment(lib,"JsonDll.lib")
//CLoginApp* pTheApp = NULL ;
//#include "log4z.h"
//
////void DisableSetUnhandledExceptionFilter()
////
////{
////
////	void *addr = (void*)GetProcAddress(LoadLibrary(("kernel32.dll")),
////
////		"SetUnhandledExceptionFilter");
////
////	if (addr)
////
////	{
////
////		unsigned char code[16];
////
////		int size = 0;
////
////		code[size++] = 0x33;
////
////		code[size++] = 0xC0;
////
////		code[size++] = 0xC2;
////
////		code[size++] = 0x04;
////
////		code[size++] = 0x00;
////
////
////
////		DWORD dwOldFlag, dwTempFlag;
////
////		VirtualProtect(addr, size, PAGE_READWRITE, &dwOldFlag);
////
////		WriteProcessMemory(GetCurrentProcess(), addr, code, size, NULL);
////
////		VirtualProtect(addr, size, dwOldFlag, &dwTempFlag);
////
////	}
////
////}
//
//BOOL WINAPI ConsoleHandler(DWORD msgType)
//{    
//	if ( pTheApp )
//	{
//		delete pTheApp ;
//	}
//	return TRUE;
//}
//
////LONG WINAPI MyUnhandledExceptionFilter( struct _EXCEPTION_POINTERS* ExceptionInfo )
////{
////	HANDLE lhDumpFile = CreateFile("DumpFileForLoginSvr.dmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL ,NULL);
////
////	MINIDUMP_EXCEPTION_INFORMATION loExceptionInfo;
////	loExceptionInfo.ExceptionPointers = ExceptionInfo;
////	loExceptionInfo.ThreadId = GetCurrentThreadId();
////	loExceptionInfo.ClientPointers = TRUE;
////	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),lhDumpFile, MiniDumpNormal, &loExceptionInfo, NULL, NULL);
////
////	CloseHandle(lhDumpFile);
////
////	return EXCEPTION_EXECUTE_HANDLER;
////}
//
//void RunFunc ( CLoginApp* pApp )
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
//	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG )|_CRTDBG_LEAK_CHECK_DF) ;
//	_CrtSetReportMode(_CRT_ASSERT, 0);
//	SetConsoleCtrlHandler(ConsoleHandler, TRUE); 
//
//	//SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)MyUnhandledExceptionFilter);  
//	//DisableSetUnhandledExceptionFilter();
//	//zsummer::log4z::ILog4zManager::GetInstance()->Config("server.cfg");
//	//zsummer::log4z::ILog4zManager::GetInstance()->Start();
//
//	CLoginApp theApp ;
//	CLogMgr::SharedLogMgr()->SetOutputFile("Login");
//	bool bok = theApp.init();
//	if ( bok == false )
//	{
//		printf("init svr error \n");
//		char c ;
//		scanf("%c",&c) ;
//		return 0 ;
//	}
//#ifdef NDEBUG
//	RunFunc(&theApp);
//#endif // _DEBUG
//#ifdef _DEBUG
//	theApp.run() ;
//#endif // _DEBUG
//	return 0 ;
//}

#include "LoginApp.h"
#include "Application.h"
int main()
{
	CLoginApp theApp ;
	CApplication theAplication(&theApp);
	theAplication.startApp();
	return 0 ;
}