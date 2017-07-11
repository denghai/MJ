#pragma once
#include "ServerNetwork.h"
struct stGateClient
{
public:
	stGateClient();
	void Reset( unsigned int nSessionID , CONNECT_ID nNetWorkID,const char* IpInfo );
	void SetNewWorkID( CONNECT_ID& nNetWorkID );
public:
	unsigned int nSessionId ;
	CONNECT_ID nNetWorkID ;
	time_t tTimeForRemove;
	std::string strIPAddress ;
};