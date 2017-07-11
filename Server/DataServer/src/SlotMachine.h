#pragma once 
#include "NativeTypes.h"
#include "TaxasPokerPeerCard.h"
#include "IConfigFile.h"
#include <map>
class CSlotMachine
	:public IConfigFile
{
public:
	struct stSlotConfig
	{
		unsigned char eType ;
		float fRewardRate ; 
		std::map<unsigned char,float> vMapLevelRate ;
		float GetRate(unsigned char cLevel);
	};
public:
	CSlotMachine();
	~CSlotMachine();
	virtual bool OnPaser(CReaderRow& refReaderRow );
	virtual void OnFinishPaseFile();
	CTaxasPokerPeerCard::eCardType RandSlotMachine(unsigned char cLevel,unsigned char* arrayFive, float& fRewardRate);  // return is give both ;
protected:
	CTaxasPokerPeerCard::eCardType GetRandCardType(unsigned cLevel );
	void LogResultInfo();
	void DeBugCard(unsigned char* vCard , unsigned cType );
protected:
	unsigned int m_vResultCnt[CTaxasPokerPeerCard::eCard_Max] ;
	unsigned int nTotalCnt ;
	stSlotConfig m_vAllConfig[CTaxasPokerPeerCard::eCard_Max] ;
	CPoker m_tPoker ;
};