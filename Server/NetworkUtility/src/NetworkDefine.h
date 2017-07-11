#pragma once
//#define _MSG_BUF_LEN 4648
#define _MSG_BUF_LEN (4648 * 20)
typedef unsigned int CONNECT_ID ;
#define INVALID_CONNECT_ID (unsigned int)-1

#define  TIME_CHECK_FIRST_MSG 9
// MUST big than TIME_CHECK_FIRST_MSG
#define  TIME_HEAT_BET (TIME_CHECK_FIRST_MSG + 4)

struct Packet
{
	CONNECT_ID _connectID ;
	bool _brocast ;
	unsigned char _packetType ;
#define _PACKET_TYPE_MSG 1 
#define _PACKET_TYPE_CONNECTED 2 
#define _PACKET_TYPE_DISCONNECT 3
#define _PACKET_TYPE_CONNECT_FAILED 4
	unsigned short _len;
	char		   _orgdata[_MSG_BUF_LEN];
};

struct ConnectInfo
{
	unsigned char strAddress[16];
	unsigned short nPort ;
};