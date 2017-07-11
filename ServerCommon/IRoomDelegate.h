#pragma once
#include "NativeTypes.h"
#include "MessageIdentifer.h"
#include "CardPoker.h"
#include <list>
#include <map>
#include "CommonDefine.h"
#include "ServerDefine.h"
#include <vector>
#include "IRoom.h"
struct stBaseRoomConfig;
struct stMsgCrossServerRequest ;
struct stMsgCrossServerRequestRet ;
struct stMsg ;
class ISitableRoomPlayer ;
class IRoomDelegate
{
public:
	struct stRoomRankItem
	{
		uint32_t nUserUID ;
		int32_t nGameOffset ;
		int32_t nOtherOffset ;
		int16_t nRankIdx ;
		bool bIsDiryt ;
		stRoomRankItem(){ nRankIdx = -1 ; }
	};
	typedef std::map<uint32_t,stRoomRankItem*> MAP_UID_ROOM_RANK_ITEM ;
	typedef std::list<stRoomRankItem*> LIST_ROOM_RANK_ITEM ;
public:
	IRoomDelegate(){ m_tLastUpdateRankIdx = 0 ; m_bDirySorted = true ; }
	virtual ~IRoomDelegate();
	void onUpdatePlayerGameResult( IRoom* pRoom, uint32_t nUserUID , int32_t nOffsetGame, int32_t nOtherOffset = 0 );
	virtual uint8_t canPlayerEnterRoom( IRoom* pRoom,stEnterRoomData* pEnterRoomPlayer );  // return 0 means ok ;
	virtual bool isRoomShouldClose( IRoom* pRoom);
	virtual bool isOmitNewPlayerHalo(IRoom* pRoom );
	virtual void onRankPlayerChanged( uint32_t nUID , uint16_t nPreIdx , uint16_t nCurIdx );
	virtual bool isPlayerLoseReachMax( IRoom* pRoom, uint32_t nUserUID );
	virtual bool onPlayerWillDoLeaveRoom(IRoom* pRoom , IRoom::stStandPlayer* pPlayer );
	virtual bool onDelayPlayerWillLeaveRoom(IRoom* pRoom , ISitableRoomPlayer* pPlayer );
	virtual bool onOneRoundEnd(IRoom* pRoom){ return true; };
protected:
	void removeAllRankItemPlayer();
	void sortRoomRankItem();
	stRoomRankItem* getRankItemByUID(uint32_t nUID );
private:
	void checkUpdateRankIdx();
protected:
	MAP_UID_ROOM_RANK_ITEM m_vRoomRankHistroy ;
	LIST_ROOM_RANK_ITEM m_vSortedRankItems ;
private:
	bool m_bDirySorted ;
	time_t m_tLastUpdateRankIdx ; 
};