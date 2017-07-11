#pragma once
#include "CardPoker.h"
#include <string>
class CPaiJiuPeerCard
{
public:
	void SetPeerCardCompsiteNumber( unsigned char nA , unsigned char nB );
	bool PKPeerCard(CPaiJiuPeerCard* pTargetIdle );
	CCard* GetBiggerCard();
	CCard& GetCardByIdx(unsigned char nIdx ){ return m_Card[nIdx] ; }
	const char* GetName(){ return m_strName.c_str(); }
protected:
	unsigned short GetSingleCardWeight(CCard* pacd );
	void LogInfo();
	unsigned short GetSelfWeight(){ return m_nSelftWeight ; }
protected:
	CCard m_Card[2];
	unsigned short m_nSelftWeight;
	std::string m_strName ;
};