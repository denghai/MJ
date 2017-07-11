#pragma once
#include "PeerCard.h"
#include "CardPoker.h"
#include "RoomBaseData.h"
class CGoldenRoomData
	:public CRoomBaseData
{
public:
	virtual void Init();
	stRoomGoldenDataOnly* GetDataOnly();
	unsigned char GetReadyPeerCnt();
	unsigned char OnUpateActPlayerIdx();
	void OnStartGame();
	void OnEndGame();
	char GameOverCheckAndProcess() ; // return > 0 means game over ret value is winner idx ;
	unsigned char DistributeCard();
	unsigned char OnPlayerReady(unsigned int nSessionID); // 0 success , 1 not your turn ,2 state error 
	unsigned char OnPlayerLook(unsigned int nSessionID);  // 0 success , 1 not your turn ,2 state error 
	unsigned char OnPlayerGiveUp(unsigned int nSessionID );  // 0 success , 1 not your turn ,2 state error 
	unsigned char OnPlayerFollow(unsigned int nSessionID,uint64_t& nFinalFollowCoin); // 0 , means error , other is real finaly follow coin, as princeble needed ;
	unsigned char OnPlayerAdd(unsigned int nSessionId,uint64_t nAddCoin, uint64_t& nFinalBetCoin ); // when this player looked card then nFinal coin adn add coin is not equal ;
	unsigned char OnPlayerPK(unsigned char idx , unsigned char cTargetIdx , bool& bWin ) ;  // 0 success , 1 not your turn ,2 state error 
	unsigned char OnPlayerShowCard(unsigned int nSessionID , unsigned char nCardIdx );
	unsigned char OnPlayerChangeCard(unsigned int nSessionID , unsigned char nCardIdx , unsigned char& cNewCardNum);
	unsigned char OnPlayerPKTimes(unsigned int nSessionID , unsigned char nNewPKTimes );
	bool MovePeerHoldCardToBuffer(char* pBuffer,unsigned short nSize );
	unsigned char GetPeerHoldCardFromBuffer(char* pBuffer,unsigned char nPeerCnt);
	virtual bool CheckCanPlayerBeKickedRightNow(unsigned char nTargetIdx );
protected:
	CPoker tPoker ;
	CPeerCard m_tPeerCard;
	CPeerCard m_tPeerCardTarget ;
};