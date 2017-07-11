#pragma once
#include "IGlobalModule.h"
#include <algorithm>
#include <queue>
class CGameRoomCenter
	:public IGlobalModule
{
public:
	struct stRoomItem
	{
		uint32_t nRoomID ;
		uint32_t nSerialNumber ;
		uint32_t nCreator ;
		uint32_t nBelongsToClubUID ;
		uint32_t nChatRoomID ;
		stRoomItem(){ nRoomID = 0 ; nSerialNumber = 0 ; nCreator = 0 ; nBelongsToClubUID = 0 ; nChatRoomID = 0; }
	};

	struct stRoomOwnerInfo
	{
		uint32_t nOwnerUID ;
		std::vector<uint32_t> vRoomIDs ;
	};
	typedef std::map<uint32_t,stRoomItem*> MAP_ROOM_ITEM ;
	typedef std::map<uint32_t,stRoomOwnerInfo*> MAP_ROOM_OWNERS ;
	typedef std::queue<uint32_t> LIST_CHAT_ROOM_ID ;
public:
	~CGameRoomCenter();
	void init( IServerApp* svrApp )override ;
	void onConnectedSvr()override ;
	bool onMsg(stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID)override ;
	bool onMsg(Json::Value& prealMsg ,uint16_t nMsgType, eMsgPort eSenderPort , uint32_t nSessionID)override ;
	bool onAsyncRequest(uint16_t nRequestType , const Json::Value& jsReqContent, Json::Value& jsResult )override ;
	void addRoomItem(stRoomItem* pItem , bool isNewAdd );
	void deleteRoomItem( uint32_t nRoomID );
	stRoomItem* getRoomItemByRoomID(uint32_t nRoomID );
	static uint8_t getRoomType(uint32_t nRoomID);
	uint32_t generateRoomID(eRoomType eType, uint32_t& nSerailNum );
	uint16_t getPlayerOwnRoomCnt(uint32_t nPlayerUID,uint8_t nRoomType );
	uint16_t getClubOwnRoomCnt(uint32_t nClubID );
	uint16_t getClubOwnRooms(std::vector<uint32_t>& vRoomIDs , uint32_t nClubID );
	uint32_t getReuseChatRoomID();
	uint16_t getPlayerOwenRooms( std::vector<uint32_t>& vRoomIDs , uint32_t nPlayerUID , uint8_t nRoomType );
protected:
	bool addRoomItemToOwner(MAP_ROOM_OWNERS& vOwners ,uint32_t nOwnerUID ,uint32_t nRoomID );
	bool deleteRoomItemFromOwner(MAP_ROOM_OWNERS& vOwners ,uint32_t nOwnerUID ,uint32_t nRoomID );
	void readRoomItemsInfo();
	void reqChatRoomIDs();
	void checkChatRoomIDReserve();
	void updateRoomItemChatRoomID();
protected:
	MAP_ROOM_ITEM m_vRoomIDKey ;
	MAP_ROOM_OWNERS m_vClubsOwner ;
	MAP_ROOM_OWNERS m_vPlayerOwners ;

	std::vector<uint32_t> m_vWillUseRoomIDs ;
	uint32_t m_nCurSerailNum ;
	bool m_isFinishedReading ;
	bool m_isFinishReadingChatRoomID ;

	LIST_CHAT_ROOM_ID m_vReserveChatRoomIDs ;
};