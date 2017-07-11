//#include <windows.h>
//#include "NiuNiuServer.h"
//#include <iostream>
//#include <WinDef.h>
//#include "catch_dump_file.h"
//#pragma comment(lib,"JsonDll.lib")
//#include "log4z.h"
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
////#include "NiuNiuPeerCard.h"
////#include "AsyncRequestQuene.h"
////#include <algorithm>
//CatchDumpFile::CDumpCatch g_exception_handler;
//int main()
//{
//	_CrtSetReportMode(_CRT_ASSERT, 0);
//	//CNiuNiuPeerCard tC ;
//	//tC.reset();
//	//CCard t ;
//	//t.SetCard(CCard::eCard_Club,8);
//	//tC.addCompositCardNum(t.GetCardCompositeNum()) ;
//
//	//t.SetCard(CCard::eCard_Club,1);
//	//tC.addCompositCardNum(t.GetCardCompositeNum()) ;
//
//	//t.SetCard(CCard::eCard_Club,8);
//	//tC.addCompositCardNum(t.GetCardCompositeNum()) ;
//
//	//t.SetCard(CCard::eCard_Club,2);
//	//tC.addCompositCardNum(t.GetCardCompositeNum()) ;
//
//	//t.SetCard(CCard::eCard_Club,13);
//	//tC.addCompositCardNum(t.GetCardCompositeNum()) ;
//
//	//uint8_t n = t.GetType();
//	//return 0 ;
//
//	CNiuNiuServerApp* theApp = CNiuNiuServerApp::getInstance() ;
//	bool bok = theApp->init() ;    
//
//	// test begin
//	//std::string str = "delete from taxpokerdb.circletopic where topicID = 0 limit 10" ; ;
//	//auto tt = str.find("delete",1);
//	//std::string strSql = "ADHAGHSDLHG";
//	//std::transform(strSql.begin(), strSql.end(),strSql.begin(), ::tolower);
//	//printf("s : %s",strSql.c_str());
//	//CTimer tTestTimer ;
//	//tTestTimer.setInterval(2);
//	//tTestTimer.setIsAutoRepeat(true) ;
//	//uint8_t nTimes = 0 ;
//	//tTestTimer.setCallBack([&nTimes,theApp,&tTestTimer](CTimer* p , float f)
//	//{
//	//	if ( nTimes > 0 )
//	//	{
//	//		printf("skiped\n");
//	//		tTestTimer.canncel() ;
//	//		return ;
//	//	}
//
//	//	Json::Value jsReqValue ;
//	//	//jsReqValue["sql"] = "insert into taxpokerdb.circletopic(topicID , authorUID,publishTime,content) values(0,23,2345234,'hesllsdgasdg');" ;
//	//	jsReqValue["sql"] = "select * from taxpokerdb.circletopic where topicID = 0 " ;
//	//	//jsReqValue["sql"] = "delete from taxpokerdb.circletopic where topicID = 0 limit 10" ;
//	//	//std::string str = "delete from taxpokerdb.circletopic where topicID = 0 limit 10" ; ;
//	//	//auto tt = str.find_first_of("delete");
//	//	theApp->getAsynReqQueue()->pushAsyncRequest(ID_MSG_PORT_DB,eAsync_DB_Select,jsReqValue,[](uint16_t nReqType ,const Json::Value& retContent,Json::Value& jsUserData){
//	//		Json::StyledWriter jsW ;
//	//		auto str = jsW.write(retContent) ;
//	//		printf("ret: %s \n",str.c_str()) ;
//	//		if ( retContent["data"].isNull() )
//	//		{
//	//			printf("ret is null\n") ;
//	//		}
//	//		else
//	//		{
//	//			printf("data have content \n");
//	//		}
//	//	}); 
//
//	//	++nTimes ;
//
//	//}
//	//) ;
//	//tTestTimer.start() ;
//	//// test end
//	if ( !bok )
//	{
//		printf("init data svr error , start up error\n");
//		char c ;
//		scanf_s("%c",&c);
//		return 0 ;
//	}
//	CLogMgr::SharedLogMgr()->SetOutputFile("NiuNiu");
//	CreateThred(theApp);
//#ifdef NDEBUG
//	RunFunc(theApp);
//#endif // _DEBUG
//#ifdef _DEBUG
//	theApp->run();
//#endif // _DEBUG
//	return 0 ;
//}

#include "NiuNiuServer.h"
#include "Application.h"
int main()
{
	CApplication theAplication(CNiuNiuServerApp::getInstance());
	theAplication.startApp();
	return 0 ;
}