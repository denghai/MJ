#pragma once 
#include "IPlayerComponent.h"
#include "PeerCard.h"
#include "PlayerBaseData.h"
#include "CommonDefine.h"
#include "MessageDefine.h"
class CRoomBase ;
class CRoomPeer
	:public IPlayerComponent
{
public:
	CRoomPeer(CPlayer* pPlayer );
	~CRoomPeer();
	virtual bool OnMessage(stMsg* pMsg ) ;
	virtual void OnPlayerDisconnect();
	virtual void Reset();
	virtual void Init(){ Reset();}
	virtual void OnWinCoin(int64_t nWinCoin );
	virtual void TryToLeaveRoom();
	CPlayerBaseData* GetPlayerBaseData();
	bool IsInState( eRoomPeerState eSate ){return (m_eState&eSate) == eSate ;}
	void SetState(eRoomPeerState eSate );
	eRoomPeerState GetStateValue(){ return m_eState ;}
	//void OnGetCard( unsigned char nCardA, unsigned char nCardB , unsigned char nCardC );
	void LeaveRoom();
	//void OnWaitTimeOut();// please make a message fake ;
	//bool IsActive(); // not fail ,not give, not null 
	unsigned int GetSessionID();
	uint64_t GetCoin();
	void SetTakeInCoin(uint64_t nCoin, bool bDiamoned = false);
	//unsigned int AddBetCoin( unsigned int naddBetCoin );  // return indeeed added betCoin ;
	//void GetBrifBaseInfo(stRoomPeerBrifInfo& vInfoOut );
	unsigned char GetPeerIdxInRoom(){ return m_nPeerIdx ; }
	void SetRoom(CRoomBase* pRoom ){ m_pRoom = pRoom ;}
	CRoomBase* GetRoom(){ return m_pRoom ; }
	virtual void OnOtherWillLogined(){ TryToLeaveRoom() ;}
	void SetIdxInRoom(unsigned char nIdx ){ m_nPeerIdx = nIdx ; }
	virtual void OnGameEnd();
	void SetIsInDiamonedRoom(bool bInDiamond){ m_bIsInDiamonedRoom = bInDiamond ;  }
	bool GetIsInDiamonedRoom(){ return m_bIsInDiamonedRoom ;}
	friend class CRoomBase ;
protected:
	//CPeerCard m_PeerCard ;
	//unsigned int m_nBetCoin;
	CRoomBase* m_pRoom ;
	unsigned char m_nPeerIdx ;// index used in room ;
	bool m_bIsInDiamonedRoom ;
	uint64_t m_nLastTakeIn ;
private:
	eRoomPeerState m_eState ;
};