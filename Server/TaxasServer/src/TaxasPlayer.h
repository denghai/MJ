#pragma once
#include "ISitableRoomPlayer.h"
#include "TaxasPokerPeerCard.h"
#include "TaxasPeerCardAdapter.h"
class CTaxasPlayer
	:public ISitableRoomPlayer
{
public:
	void willStandUp()override ;
	void onGameEnd() override;
	void onGameBegin()override ;
	void reset(IRoom::stStandPlayer* pPlayer)override;
	uint8_t getPeerCardByIdx(uint8_t nIdx );
	void addPublicCard(uint8_t vPublicCards[TAXAS_PUBLIC_CARD] );
	void addPeerCard(uint8_t nIdx , uint8_t nCardNum );
	bool betCoin( uint32_t nBetCoin );
	eRoomPeerAction getCurActType();
	uint32_t getWinCoinThisGame();
	void addWinCoinThisGame(uint32_t nWinCoin );
	uint32_t getBetCoinThisRound(){ return nBetCoinThisRound ; }
	void setBetCoinThisRound( int32_t nValue ){ nBetCoinThisRound = nValue ;}
	void setCurActType(eRoomPeerAction act){ eCurAct = act ;};
	uint32_t getAllBetCoin(){ return nAllBetCoin ;}
	int32_t getGameOffset()override ;
	IPeerCard* getPeerCard(){ return &m_tPeerCard ; }
	uint32_t getInsuranceLoseCoin(){ return nInsuranceLoseCoin ; }
	void setInsuranceLoseCoin(uint32_t nLoseCoin ) { nInsuranceLoseCoin += nLoseCoin ; }
	void setInsuredAmount(uint32_t nAmount ){ nBuyInsuredAmount = nAmount ; }
	uint32_t getInsuredAmount(){ return nBuyInsuredAmount ;}
	void addWinInsuredCoin(uint32_t nWinCoin);
protected:
	void zeroData();
protected:
	eRoomPeerAction eCurAct ;  
	uint32_t nBetCoinThisRound ;
	uint32_t nAllBetCoin ;  // used for tell win or lose
	uint32_t nWinCoinThisGame ;    // used for tell win or lose
	CTaxasPeerCardAdpater m_tPeerCard ;
	uint32_t nBuyInsuredAmount ;
	uint32_t nInsuranceLoseCoin ;
};