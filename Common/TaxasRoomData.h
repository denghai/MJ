#pragma once
#include "GoldenRoomData.h"
struct stMsg;
class CTaxasPokerPeerCard;

class CTaxasRoomData
	:public CRoomBaseData
{
public:
	typedef std::vector<unsigned char> VEC_INDEX ;
	struct stVicePool
	{
		unsigned char Idx ;
		uint64_t nCoinInPool ;
		VEC_INDEX vPeerIdxInPool ;
		VEC_INDEX vWinnerIdxs ;
	};
	typedef std::vector<stVicePool*> VEC_VICE_POOL ;
public:
	~CTaxasRoomData(){ClearVicePool();}
	virtual void Init();
	void InitDataFromMsg(stMsg*pmsg);  
	virtual void OnStartGame(stMsg*pmsg = NULL) ;
	virtual void OnEndGame() ;
	virtual bool CheckCanPlayerBeKickedRightNow(unsigned char nTargetIdx );
	float GetCurStateTime();
	stTaxasRoomDataSimple* GetSimpleData();
	bool CheckThisRoundEnd();
	unsigned char OnDistributeCard();   /// return cur Bet Round ;
	unsigned char OnEndThisRound();
	char OnWaitNextPlayerAct();  // return cur wait player idx , other value means error ;
	char OnPlayerStandup(unsigned char nIdx );  // return 0 means ok , other error ;
	char OnCaculateRound( VEC_VICE_POOL* vNewProducedVicePool = NULL );   // vNewProducedVicePool use by client , server will not use it ;
	char OnCaculateGameResult();
	bool CheckGameOver();
	unsigned short GetAllPeerDataToBuffer(const char* pBuffer );
	unsigned short GetAllVicePoolToBuffer(const char* pBuffer);
	unsigned short GetAllPeersHoldCardToBuffer(const char* pBuffer );
	unsigned char OnPlayerFollow(unsigned char nIdx );
	unsigned char OnPlayerAdd(unsigned char nIdx,uint64_t nAddCoin );
	unsigned char OnPlayerPass(unsigned char nIdx );
	unsigned char OnPlayerGiveUp( unsigned char nIdx );
	unsigned char OnPlayerAllIn(unsigned char nIdx);
	unsigned char OnPlayerAutoStand(unsigned char nIdx );
	unsigned char OnPlayerAutoTakeIn(unsigned char nIdx ,unsigned int nTakeInCoin );
	unsigned char GetNeedAutoTakeInPeerIdxs(std::vector<unsigned char>& vIdxs);
	uint64_t GetPlayerMustMinAddCoin( unsigned char nPlayerIdx );
	uint64_t GetPlayerCanMaxAddCoin( unsigned char nPlayerIdx );
	uint64_t GetCanMaxBetCoinThisRound();
	unsigned char GetPlayerCntWithState( unsigned int nState );
protected:
	void OnRecievedCardInfos(stMsg*pmsg);
	unsigned char GetFirstPlayerIdxWithState(unsigned char nIdxBegin, unsigned int nState ); // seach from nIdxBegin;
	void ClearVicePool();
	void CaculateVicePoolWinnerIdxs(stVicePool* pPool );
	bool GetTaxasCardForPlayer(CTaxasPokerPeerCard& pCard,stTaxasPeerData* pPeerData);
public:
	VEC_VICE_POOL m_vAllVicePools ;
	CPoker tPoker ;
};