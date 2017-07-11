#pragma once
#include "TaxasMessageDefine.h"
#include "ServerMessageDefine.h"
#include <map>
#include <json/json.h>
#include "httpRequest.h"
#include <list>
#include "IRoomManager.h"
class CTaxasRoom ;
class CRoomConfigMgr ;
class CRoomManager
	:public IRoomManager
{
public:
	CRoomManager(CRoomConfigMgr* pCongig ):IRoomManager(pCongig){ }
	eRoomType getMgrRoomType()override{ return eRoom_TexasPoker ;}
protected:
	IRoomInterface* doCreateInitedRoomObject(uint32_t nRoomID , const Json::Value& vJsValue)override ;
	IRoomInterface* doCreateRoomObject(eRoomType reqSubRoomType,bool isPrivateRoom)override ;
};