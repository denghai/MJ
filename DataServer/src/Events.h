#pragma once
enum eEventType
{
	eEvent_NewDay,
	eEvent_PlayerOnline ,
	eEvent_PlayerOffline,
	eEvent_Max,
};

struct stEventArg
{
	eEventType cEvent ;
	unsigned int nID ;
	void* pData ;
	stEventArg(){ nID = 0 ; pData = 0 ; cEvent = eEvent_Max ;}
};