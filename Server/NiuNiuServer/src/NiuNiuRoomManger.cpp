#include "NiuNiuRoomManger.h"
#include "NiuNiuServer.h"
#include "NiuNiuRoom.h"
#include "NiuNiuMessageDefine.h"
#include "NiuNiuRoomPlayer.h"
#include "log4z.h"
#define ROOM_LIST_ITEM_CNT_PER_PAGE 5 
#include "AutoBuffer.h"
#include "SystemRoom.h"
#include "PrivateRoom.h"
IRoomInterface* CNiuNiuRoomManager::doCreateInitedRoomObject(uint32_t nRoomID,const Json::Value& vJsValue ) 
{
	IRoomInterface* pRoom = doCreateRoomObject(eRoomType::eRoom_NiuNiu,true) ;
	pRoom->onFirstBeCreated(this,nRoomID,vJsValue);
	return pRoom ;
}

IRoomInterface* CNiuNiuRoomManager::doCreateRoomObject(eRoomType reqSubRoomType,bool isPrivateRoom)
{
	IRoomInterface* pRoom = nullptr ;
	if ( 0 )
	{
		//pRoom = new CSystemRoom<CNiuNiuRoom> ;
	}
	else
	{
		pRoom = new CPrivateRoom<CNiuNiuRoom> ;
	}
	return pRoom ;
}
