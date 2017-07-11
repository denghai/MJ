#define CRTDBG_MAP_ALLOC
#include <windows.h>
#include <stdlib.h>
#include "CenterServer.h"
#include "log4z.h"
#include <WinDef.h>
#include "catch_dump_file.h"
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
			CCenterServerApp* pAp = (CCenterServerApp*)lpParam ;
			pAp->Stop();
			printf("Closing!!!\n");
		}
		else
		{
			printf("Input exit or Q , to close the app \n") ;
		}
	}
	return 0;
}

void CreateThred( CCenterServerApp* pApp )
{
	DWORD threadID;
	HANDLE hThread;
	hThread = CreateThread(NULL,0,ThreadProc,pApp,0,&threadID); // 创建线程
}

LONG WINAPI MyUnhandledExceptionFilter( struct _EXCEPTION_POINTERS* ExceptionInfo )
{
	HANDLE lhDumpFile = CreateFile(L"DumpFileForCenterSvr.dmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL ,NULL);

	MINIDUMP_EXCEPTION_INFORMATION loExceptionInfo;
	loExceptionInfo.ExceptionPointers = ExceptionInfo;
	loExceptionInfo.ThreadId = GetCurrentThreadId();
	loExceptionInfo.ClientPointers = TRUE;
	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),lhDumpFile, MiniDumpNormal, &loExceptionInfo, NULL, NULL);

	CloseHandle(lhDumpFile);

	return EXCEPTION_EXECUTE_HANDLER;
}

void RunFunc ( CCenterServerApp* pApp )
{
	// exception 
	__try
	{
		pApp->RunLoop() ;
	}
	__except(CatchDumpFile::CDumpCatch::UnhandledExceptionFilterEx(GetExceptionInformation()))
	{
		LOGFMTI("try to recover from exception") ;
		pApp->RunLoop() ;
	}
}

CatchDumpFile::CDumpCatch g_exception_handler;

int main()
{
	//zsummer::log4z::ILog4zManager::GetInstance()->Config("server.cfg");
	zsummer::log4z::ILog4zManager::GetInstance()->Start();
	_CrtSetReportMode(_CRT_ASSERT, 0);
	CCenterServerApp  theApp ;
	//CLogMgr::SharedLogMgr()->SetOutputFile("CenterSvr");
	theApp.Init() ;
	CreateThred(&theApp);
#ifdef NDEBUG
	RunFunc(&theApp);
#endif // _DEBUG
#ifdef _DEBUG
	theApp.RunLoop() ;
#endif // _DEBUG
}