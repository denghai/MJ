#pragma once
#include "IGlobalModule.h"
#include <list>
class CEncryptNumber
	:public IGlobalModule
{
public:
	struct stEncryptRequest
	{
		uint8_t nType ;
		uint32_t nCoin ;
		uint32_t nNeedCnt ;
		uint16_t nRMB ;
		uint8_t nCoinType ;
		uint16_t nChannelID ;
	};

public:
	CEncryptNumber(){ m_vAllRequest.clear(); m_pCurRequest = nullptr ;}
	bool onMsg(stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID)override ;
	uint64_t generateNumber();
	static bool isNumberValid(uint64_t nNumber );
protected:
	void doGenerateNumber(stEncryptRequest* pReq );
protected:
	std::list<stEncryptRequest*> m_vAllRequest ;
	stEncryptRequest* m_pCurRequest ;
};