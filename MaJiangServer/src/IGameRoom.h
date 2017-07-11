#pragma once
#include "NativeTypes.h"
#include "json/json.h"
#include "MessageIdentifer.h"
#include <memory>
class IGameRoomManager;
struct stBaseRoomConfig;
struct stMsg;
struct stEnterRoomData;
class IGameRoomRecorder;
class IGameRoom
{
public:
	virtual ~IGameRoom(){}
	virtual bool init(IGameRoomManager* pRoomMgr, stBaseRoomConfig* pConfig, uint32_t nSeialNum, uint32_t nRoomID, Json::Value& vJsValue) = 0;
	virtual uint8_t checkPlayerCanEnter(stEnterRoomData* pEnterRoomPlayer ) = 0 ;
	virtual bool onPlayerEnter(stEnterRoomData* pEnterRoomPlayer) = 0;
	virtual bool onPlayerApplyLeave( uint32_t nPlayerUID ) = 0 ;
	virtual bool isRoomFull() = 0;

	//virtual void roomItemDetailVisitor(Json::Value& vOutJsValue) = 0;
	virtual uint32_t getRoomID() = 0;
	virtual uint32_t getSeiralNum() = 0;
	virtual uint8_t getRoomType() = 0;
	virtual void update(float fDelta) = 0;
	virtual void sendRoomMsg(Json::Value& prealMsg, uint16_t nMsgType) = 0;
	virtual void sendMsgToPlayer(Json::Value& prealMsg, uint16_t nMsgType, uint32_t nSessionID) = 0;
	virtual bool onMessage(stMsg* prealMsg, eMsgPort eSenderPort, uint32_t nPlayerSessionID) = 0;
	virtual bool onMsg(Json::Value& prealMsg, uint16_t nMsgType, eMsgPort eSenderPort, uint32_t nSessionID) = 0;
	virtual stBaseRoomConfig* getRoomConfig() = 0;
	virtual void sendRoomInfo(uint32_t nSessionID) = 0;
	virtual uint32_t getCoinNeedToSitDown() = 0;
	virtual bool isInternalShouldClosedAll() = 0;
	virtual bool isOneCirleEnd() = 0;
	virtual std::shared_ptr<IGameRoomRecorder> getRoomRecorder() = 0;
};