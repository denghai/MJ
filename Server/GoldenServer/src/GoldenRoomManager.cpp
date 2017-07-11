#include "GoldenRoomManager.h"
#include "GoldenServer.h"
#include "log4z.h"
#include "SystemRoom.h"
#include "PrivateRoom.h"
#include "GoldenRoom.h"
IRoomInterface* CGoldenRoomManager::doCreateInitedRoomObject(uint32_t nRoomID,const Json::Value& vJsValue ) 
{
	IRoomInterface* pRoom = doCreateRoomObject(getMgrRoomType(),true) ;
	pRoom->onFirstBeCreated(this,nRoomID,vJsValue);
	return pRoom ;
}

IRoomInterface* CGoldenRoomManager::doCreateRoomObject(eRoomType reqSubRoomType,bool isPrivateRoom)
{
	IRoomInterface* pRoom = nullptr ;
	if ( 0 )
	{
		//pRoom = new CSystemRoom<CGoldenRoom> ;
	}
	else
	{
		pRoom = new CPrivateRoom<CGoldenRoom> ;
	}
	return pRoom ;
}
