#pragma once
#include "CommonDefine.h"
#include "Timer.h"
#include "NativeTypes.h"
#include <list>
class CRoomPeer ;
class CPlayer;
struct stMsg ;
struct stKicPlayer
{
	unsigned char nIdxInRoom ;
	unsigned int nUserUIDLimit ;
	unsigned int nLimitEndTime ;
	bool bDoKicked ;
	//char cNameBeKicked[MAX_LEN_CHARACTER_NAME] ;
	char cNameKicker[MAX_LEN_CHARACTER_NAME];
	stKicPlayer(){ nUserUIDLimit = 0 ; bDoKicked = false ; nLimitEndTime = 0 ; memset(cNameKicker,0,sizeof(cNameKicker)) ; /*memset(cNameBeKicked,0,sizeof(cNameBeKicked)) ;*/}
};

class CRoomBase
	:public CTimerDelegate
{
public:
	typedef std::list<stKicPlayer> LIST_KICK_PEERS ;
public:
	CRoomBase();
	virtual ~CRoomBase();
	virtual void Init( unsigned int nRoomID , unsigned char nMaxSeat );
	virtual void SendMsgRoomPeers(stMsg*pMsg ,unsigned short nLen ,CRoomPeer* pExcpetPeer = NULL);
	CRoomPeer* GetRoomPeerBySessionID( unsigned int nSessionID );
	virtual void OnPeerLeave( CRoomPeer* peer );
	int GetRoomPeerCount();
	unsigned char GetEmptySeatCount(){ return m_nMaxSeat - GetRoomPeerCount() ; }
	unsigned short GetMaxSeat(){ return m_nMaxSeat ; }
	void SetRoomID(unsigned int nRoomID ){ m_nRoomID = nRoomID ; }
	unsigned int GetRoomID(){ return m_nRoomID ; }
	eRoomType GetRoomType(){ return m_eRoomType ;}
	void SetRoomType(eRoomType nType ){ m_eRoomType = nType ;}
	eRoomState GetRoomState(){ return m_eRoomState ;}

	virtual unsigned char CanJoin(CPlayer* peer );  // 0 means ok , 1 room is full , 2 money is not engough , 3 peer is not allowed ;
	virtual void AddPeer(CRoomPeer* peer ) ;
	virtual bool AddBeInvitedPlayer(CPlayer* peer,unsigned char nRoomIdx ){ return false ;}
	virtual void Update(float fTimeElpas, unsigned int nTimerID );
	virtual bool OnPeerMsg(CRoomPeer* pPeer, stMsg* pmsg );
	virtual bool SendCurRoomInfoToPlayer(CRoomPeer* pPeer ) = 0 ;

	void SetAntesCoin( uint64_t nCont ){ m_nAntesCoin = nCont ; };
	uint64_t GetAntesCoin(){ return m_nAntesCoin ; }
	bool IsDiamonedRoom(){ return m_bIsDiamonedRoom ;}
	void SetIsDiamonedRoom(bool bDaimoned){ m_bIsDiamonedRoom = bDaimoned ;}
	int GetPassword(){ return m_nPassword ; }
	const std::string& GetRoomName(){ return m_strName ;}
	int SetPassword( int nPassword ){ return m_nPassword = nPassword ; }
	void SetRoomName( const char* pName ) { m_strName = pName ;}
	void SetRoomLevel(unsigned char nRoomLevel ){ m_nRoomLevel = nRoomLevel ; }
	unsigned char GetRoomLevel(){ return m_nRoomLevel ;}
	uint64_t GetMaxTakeInCoin(){ return m_nMaxTakeIn ;}
	void SetMaxTakeInCoin( uint64_t nMaxCoin){ m_nMaxTakeIn = nMaxCoin ;}
	void SetWaitOperateTime(unsigned char nWaitOperateTime ){ m_nWaitOperateTime = nWaitOperateTime ;}
	unsigned char GetWaitOperateTime(){ return m_nWaitOperateTime ;}
	bool IsInNotAllowedPeerList(unsigned int nUserUID );
protected:
	void ProcessKickedPeers();
	char GetRoomPeerIdx(CRoomPeer* pPeer );
protected:
	eRoomType m_eRoomType ;
	unsigned char m_nRoomLevel ;
	unsigned int m_nRoomID ;
	CRoomPeer**m_vRoomPeer;
	eRoomState m_eRoomState ;
	uint64_t m_nAntesCoin ;
	uint64_t m_nMaxTakeIn ;
	bool m_bIsDiamonedRoom ;

	// pass word and name
	int m_nPassword ;
	std::string m_strName ;

	LIST_KICK_PEERS m_vAllKickedPeers ;
private:
	unsigned short m_nMaxSeat ;
	unsigned char m_nWaitOperateTime ; // by second ;
};