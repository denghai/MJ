#pragma once
#include "NativeTypes.h"
#include <vector>
#include <map>
#include "CardPoker.h"
#include "TaxasPokerPeerCard.h"
#include <memory> 
class CInsuranceCheck
{
public:
	struct stCheckPeer
	{
		uint8_t nIdx ;
		std::vector<uint8_t> vOuts ;
		std::shared_ptr<CTaxasPokerPeerCard> m_pPeerCard ;
		stCheckPeer(){ m_pPeerCard = std::shared_ptr<CTaxasPokerPeerCard>(new CTaxasPokerPeerCard());}
		~stCheckPeer(){ }
	};
public:
	CInsuranceCheck();
	void reset();
	void addCheckPeer(uint16_t nIdx , uint16_t nPeerCard0, uint16_t nPeerCard1 );
	void addPublicCard(uint32_t nPublicCard );
	void addExincludeCard(uint32_t nCards );
	uint8_t getOuts( uint16_t& nInsuranceBuyerIdx, std::vector<uint8_t>& allOuts );
	uint8_t getOutsForPlayer( uint16_t nIdx ,std::vector<uint8_t>* allOuts );
	uint32_t getInsuredProfit( uint32_t nInsuredAmount );
	uint32_t getAmountNeedForPofit( uint32_t nProfit );
protected:
	void caculateResult();
protected:
	CPoker m_tCheckPoker ;
	uint16_t m_nBuyerIdx ;
	std::map<uint16_t,stCheckPeer> m_vAllPeers ;
	bool m_isDataDirty ;
	std::vector<uint8_t> m_vExincludeCardNums ;
};