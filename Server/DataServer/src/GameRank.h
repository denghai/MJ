#pragma once
#include "CommonDefine.h"
#include "ServerMessageDefine.h"
#include <vector>
#include "RakNetTypes.h"
struct stEventArg ;
class CPlayer ;
struct stRankPeerInfo
{
	stPlayerDetailData stDetailData ;
	CPlayer* pPlayer ;
	uint64_t nYesterDayWin ;
public:
	stRankPeerInfo(){  pPlayer = NULL ;nYesterDayWin = 0 ; }
	void OnPlayerOffline( CPlayer*pPlayerOffline );
	void OnPlayerOnline(CPlayer*pPlayerOnline );
	void SetPeerInfo(stServerGetRankPeerInfo* pData );
	void GetBrifePlayerData(stPlayerBrifData* poutData );
};

class CGamerRank
{
public:
	typedef std::vector<stRankPeerInfo*> VEC_RANKPEER_INFO ;
public:
	CGamerRank();
	~CGamerRank();
	void Init();
	void RequestNewRank(eRankType eType );
	bool OnMessage(RakNet::Packet* packet );
	void SendRankToPlayer(CPlayer* pPlayer , eRankType eType, unsigned char nFromIdx , unsigned char nCount );
	void SendRankDetailToPlayer(CPlayer* pPlayer , unsigned int nRankPeerUserUID, eRankType eType );
	static bool EventFunc(void* pUserData,stEventArg* pArg);
protected:
	void ClearPeerInfos(VEC_RANKPEER_INFO& vInfos );
	stRankPeerInfo* GetRankPeerInfo(unsigned int nUserUID, eRankType eType );
	void UpdateOnlineState( eRankType eType );
	void OnProcessEvent(stEventArg* pArg);
protected:
	VEC_RANKPEER_INFO m_vRank[eRank_Max];
	time_t m_vRefreshTime[eRank_Max] ;
};