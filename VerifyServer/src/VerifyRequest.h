#pragma once
#include <list>
#include "NativeTypes.h"
//#include "RakNetTypes.h"
#define MAX_VERIFY_STR_LEN 1024*8
#define MAX_MI_UID_LEN 30
enum eVerifiy_Result
{
	eVerify_Apple_Error,
	eVerify_Apple_Success,
	eVerify_DB_Error,
	eVerify_DB_Success,
	eVerify_Success,
	eVerify_Max,
};
struct stVerifyRequest
{
	unsigned int nFromPlayerUserUID ;
	unsigned int nBuyedForPlayerUserUID ;  
	char pBufferVerifyID[MAX_VERIFY_STR_LEN] ;  // base64 from cliend , or tranfaction_id from apple server ;
	unsigned char nChannel ; // 0 apple store ; 1  wechat , 2 xiao mi;
	eVerifiy_Result eResult ;  // eVerifiy_Result
	unsigned short nShopItemID ;  // for mutilp need to verify ;
	uint32_t nSessionID ;
	uint32_t nMiUserUID;
	uint32_t nTotalFee; // wei xin RenZhen ;
	void* pUserData ;
};

typedef std::list<stVerifyRequest*> LIST_VERIFY_REQUEST ;

// order 
struct stShopItemOrderRequest
{
	char cShopDesc[50] ;
	char cOutTradeNo[32] ; // [shopItemID]E[playerUID]E[utc time] 
	uint32_t nPrize ; // fen wei dan wei ;
	char cTerminalIp[17] ;
	uint8_t nChannel ;
	char cPrepayId[64] ;
	uint8_t nRet ; // 0 success , 1 argument error ;

	uint32_t nSessionID ;
	unsigned int nFromPlayerUserUID ;
	void* pUserData ;
};

typedef std::list<stShopItemOrderRequest*> LIST_ORDER_REQUEST ;