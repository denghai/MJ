#define CRTDBG_MAP_ALLOC
#include "GateServer.h"
#include <windows.h>
#include <stdlib.h>

#include <WinDef.h>
#include <Dbghelp.h>
#include "log4z.h"
#pragma comment( lib, "DbgHelp" )
#pragma comment(lib,"JsonDll.lib")
//#include <vld.h>

DWORD WINAPI ThreadProc(LPVOID lpParam)
{
	bool bRunning = true;
	char pBuffer[255] ;
	while(bRunning)
	{
		memset(pBuffer,0,sizeof(pBuffer)) ;
		scanf_s("%s",pBuffer) ;
		if ( strcmp(pBuffer,"exit") == 0 || strcmp(pBuffer,"Q") == 0 )
		{
			bRunning = false ;
			CGateServer* pAp = (CGateServer*)lpParam ;
			pAp->stop();
			printf("Closing!!!\n");
		}
		else
		{
			printf("Input exit or Q , to close the app \n") ;
		}
	}
	return 0;
}

void CreateThred( CGateServer* pApp )
{
	DWORD threadID;
	HANDLE hThread;
	hThread = CreateThread(NULL,0,ThreadProc,pApp,0,&threadID); // 创建线程
}

LONG WINAPI MyUnhandledExceptionFilter( struct _EXCEPTION_POINTERS* ExceptionInfo )
{
	HANDLE lhDumpFile = CreateFile(L"DumpFileForHtml5GateSvr.dmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL ,NULL);

	MINIDUMP_EXCEPTION_INFORMATION loExceptionInfo;
	loExceptionInfo.ExceptionPointers = ExceptionInfo;
	loExceptionInfo.ThreadId = GetCurrentThreadId();
	loExceptionInfo.ClientPointers = TRUE;
	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),lhDumpFile, MiniDumpNormal, &loExceptionInfo, NULL, NULL);

	CloseHandle(lhDumpFile);

	return EXCEPTION_EXECUTE_HANDLER;
}

void RunFunc ( CGateServer* pApp )
{
	// exception 
	__try
	{
		pApp->run() ;
	}
	__except(MyUnhandledExceptionFilter(GetExceptionInformation()))
	{
	}
}

int main()
{
	//zsummer::log4z::ILog4zManager::GetInstance()->Config("server.cfg");
	//zsummer::log4z::ILog4zManager::GetInstance()->Start();
	CGateServer  theApp ;
	//CLogMgr::SharedLogMgr()->SetOutputFile("H5GateSvr");
	bool bok = theApp.init() ;
	if (!bok )
	{
		printf("init svr error \n") ;
		char c ;
		scanf_s("%c",&c);
		return 0 ;
	}
	CreateThred(&theApp);
#ifdef NDEBUG
	RunFunc(&theApp);
#endif // _DEBUG
#ifdef _DEBUG
	theApp.run() ;
#endif // _DEBUG
}