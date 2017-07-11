#pragma once
#include "CommonData.h"
#include "MessageDefine.h"
struct stMsg ;
class CRoomBaseData
{
public:
	typedef std::vector<unsigned int> VEC_KICKED_SESSIONID ;  // session ids will be clicked ;
public:
	CRoomBaseData();
	virtual ~CRoomBaseData();
	virtual void Init();

	virtual void OnStartGame(stMsg*pmsg = NULL ) ;
	virtual void OnEndGame() ;
	unsigned char AddPeer(stPeerBaseData* peerData,unsigned char nRoomIdx = 0); // must new outside , return final room idx ;
	void RemovePeer(unsigned int nSessionID );
	unsigned char GetEmptySeatCnt();
	unsigned char GetPlayingSeatCnt();
	unsigned char GetMaxSeat();
	stPeerBaseData* GetPeerDataBySessionID(unsigned int nSessionID );
	stPeerBaseData* GetPeerDataByIdx(unsigned char cIdx );
	stPeerBaseData* GetPeerDataByUserUID(unsigned int nUserUID);
	char GetRoomIdxBySessionID(unsigned int nSessionID );
	unsigned int GetSessionIDByIdx(unsigned char nIdx);
	virtual void SetRoomState(unsigned short nRoomState );
	unsigned char OnPlayerKick(unsigned int nSessionID,unsigned char nTargetIdx,bool* bRightKicked );
	unsigned char OnCheckDelayKickPlayers(std::vector<unsigned int>& vKickedPlayer );  // vKickedPlayer contain sessionId in server side , vKickedPlayer contain roomIdx  in client side ;

	virtual bool CheckCanPlayerBeKickedRightNow(unsigned char nTargetIdx ) = 0;
	bool DoBeKickedPlayer(unsigned int nSessionID );
	unsigned short GetKickSessionIDsToBuffer(const char* pBuffer );
	unsigned char GetKickPlayerCnt(){ return m_vSessionIDs.size() ;}
protected:
	void ClearAllPeers();
public:
	stRoomBaseDataOnly* m_pData;
	stPeerBaseData** m_vPeerDatas ;
	VEC_KICKED_SESSIONID m_vSessionIDs ;
};

