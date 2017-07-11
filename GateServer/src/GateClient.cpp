#include "GateClient.h"
#include "GateServer.h"
stGateClient::stGateClient()
{
	nSessionId = 0 ;
	nNetWorkID = INVALID_CONNECT_ID ;
	tTimeForRemove = 0 ;
	strIPAddress = "NULL";
}
void stGateClient::Reset(unsigned int nSessionID , CONNECT_ID nNetWorkID,const char* IpInfo )
{
	this->nSessionId = nSessionID ;
	this->nNetWorkID = nNetWorkID ;
	tTimeForRemove = 0 ;
	strIPAddress = "unknown ip";
	if ( IpInfo )
	{
		strIPAddress = IpInfo ;
	}
}


void stGateClient::SetNewWorkID( CONNECT_ID& nNetWorkID)
{
	this->nNetWorkID = nNetWorkID ;
}