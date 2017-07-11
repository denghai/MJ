#pragma once
#include "CommonDefine.h"
#include <map>
#include "RoomGolden.h"
#include "RoomPaiJiu.h"
#include "RoomGoldenNew.h"
class CRoomBaseNew ;
class CGameRooms
{
public:
 	  typedef std::map<int,CRoomBaseNew*> MAP_ROOM;
	  //typedef std::map<int,CRoomBaseNew*> MAP_ROOM_NEW;
public:
	CGameRooms();
	~CGameRooms();
	void AddRoom(CRoomBaseNew* pRoom, eRoomLevel eLevel );
	CRoomBaseNew* GetRoomByID(unsigned int nRoomID, eRoomLevel eLevel );
	int GetRoomCount( eRoomLevel eLevel ) ;
	int GetAllRoomCount();
protected:
	friend class CRoomManager ;
	void RemoveEmptyRoom();
	void RemoveEmptyRoom(MAP_ROOM& vRooms, unsigned short nLeftEmpty ) ;
protected:
	MAP_ROOM m_vRooms[eRoomLevel_Max] ;
	//MAP_ROOM_NEW m_vRoomsNew[eRoomLevel_Max] ;
};

class CPlayer ;
struct stBaseRoomConfig;
class CRoomManager
{
public:
	typedef std::vector<CRoomBaseNew*> VEC_ROOM;
	//typedef std::vector<CRoomBaseNew*> VEC_ROOM_NEW;
	typedef std::map<unsigned int, VEC_ROOM>  MAP_BLIND_ROOMS;
	struct stSpeedRoom
	{
		MAP_BLIND_ROOMS vSeatRooms[eSeatCount_Max];
		void AddRoom(CRoomBaseNew*pRoom);
	};
public:
	CRoomManager();
	~CRoomManager();
	void Init();
	CRoomBaseNew* GetRoom(char cRoomType , char cRoomLevel, unsigned int nRoomID);
	CRoomBaseNew* CreateRoom( unsigned int nRoomID );
	CRoomBaseNew* CreateRoom(stBaseRoomConfig* pConfig );
	CRoomBaseNew* CreateRoom(char cRoomType , char cRoomLevel);
	void SendRoomListToPlayer( CPlayer* pTargetPlayer , unsigned char eType, unsigned char cRoomLevel );
	void AddRoomToType(CRoomBaseNew* pRoomBase);
	CRoomBaseNew* GetProperRoomToJoin(unsigned char cSpeed , unsigned char cSeatType, unsigned int nBlindBet , unsigned int nExptedRoomID = 0 );

	static unsigned int s_RoomID ;
protected:
	CGameRooms m_vGames[eRoom_Max] ;
	stSpeedRoom m_vSpeedRooms[eSpeed_Max];
};