#pragma once
#include "NativeTypes.h"
#include "MessageIdentifer.h"
struct stBaseRoomConfig;
struct stEnterRoomData ;
class IRoomManager ;
struct stMsg ;
namespace Json
{
	class Value ;
};

class IRoomInterface
{
public:
	virtual ~IRoomInterface(){ }
	virtual bool onFirstBeCreated(IRoomManager* pRoomMgr,uint32_t nRoomID, const Json::Value& vJsValue ) = 0;
	virtual void serializationFromDB(IRoomManager* pRoomMgr,stBaseRoomConfig* pConfig,uint32_t nRoomID , Json::Value& vJsValue ) = 0;
	virtual void serializationToDB() = 0;

	virtual uint8_t canPlayerEnterRoom( stEnterRoomData* pEnterRoomPlayer ) = 0;  // return 0 means ok ;
	virtual void onPlayerEnterRoom(stEnterRoomData* pEnterRoomPlayer,int8_t& nSubIdx ) = 0;
	virtual bool onPlayerApplyLeaveRoom(uint32_t nUserUID ) = 0;
	virtual void roomItemDetailVisitor(Json::Value& vOutJsValue) = 0;
	virtual uint32_t getRoomID() = 0;
	virtual uint8_t getRoomType() = 0 ;
	virtual void update(float fDelta) = 0;
	virtual void onTimeSave() = 0;
	virtual bool onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID ) = 0;
	virtual bool onMessage( Json::Value& prealMsg ,uint16_t nMsgType, eMsgPort eSenderPort , uint32_t nSessionID ){ return false ;}
	virtual bool isDeleteRoom() = 0;
	virtual void deleteRoom() = 0 ;
	virtual uint32_t getOwnerUID() = 0 ;
	virtual int32_t getPlayerOffsetByUID( uint32_t nUserUID ) { return 0 ;}  ;
	virtual bool isPlaying() = 0 ;
	virtual void setLeftTime(uint32_t nLeftSec ){  }
	virtual void setRoomState(uint32_t nRoomState ){}
	virtual bool onCreateFromDB(IRoomManager* pRoomMgr, uint32_t nRoomID, const Json::Value& vJsValue) { return true; };
};