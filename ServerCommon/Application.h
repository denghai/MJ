#pragma once
#include "ISeverApp.h"
#include <windows.h>
#include "catch_dump_file.h"
class CApplication
{
public:
	CApplication(IServerApp* pApp );
	void startApp();
	static DWORD WINAPI consoleInput(LPVOID lpParam);
protected:
	IServerApp* getApp();
	void stopRuning();
	bool& isRunning();
	void runAppLoop();
protected:
	IServerApp* m_pApp ;
	bool m_isRunning ;
	CatchDumpFile::CDumpCatch m_exception_handler;
};