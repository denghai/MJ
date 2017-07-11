#include "RoomManager.h"
#include "TaxasServerApp.h"
#include "log4z.h"
#include "TaxasRoom.h"
#include "ServerMessageDefine.h"
#include "RoomConfig.h"
#include "AutoBuffer.h"
#include "SystemRoom.h"
#include "PrivateRoom.h"
IRoomInterface* CRoomManager::doCreateInitedRoomObject(uint32_t nRoomID ,const Json::Value& vJsValue)
{
	IRoomInterface* pR = doCreateRoomObject(getMgrRoomType(),true);
	pR->onFirstBeCreated(this,nRoomID,vJsValue) ;
	return pR ;
}

IRoomInterface* CRoomManager::doCreateRoomObject(eRoomType reqSubRoomType ,bool isPrivateRoom)
{
	IRoomInterface* pRoom = nullptr ;
	if ( 0 )
	{
		//pRoom = new CSystemRoom<CTaxasRoom>() ;
	}
	else
	{
		pRoom = new CPrivateRoom<CTaxasRoom> ;
	}
	return pRoom ;
}
