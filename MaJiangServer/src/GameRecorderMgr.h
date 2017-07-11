#pragma once
#include "NativeTypes.h"
#include <map>
#include <vector>
#include<memory>
#include "IGlobalModule.h"
#include "IGameRecorder.h"
class GameRecorderMgr
	:public IGlobalModule
{
public:
	void addRoomRecorder( std::shared_ptr<IGameRoomRecorder> pRoomRecorder , bool isSaveToDB );
	std::shared_ptr<IGameRoomRecorder> getRoomRecorder( uint32_t nSieralNum );
	bool onMsg(Json::Value& prealMsg, uint16_t nMsgType, eMsgPort eSenderPort, uint32_t nSessionID)override;
	void onConnectedSvr()override;
	std::shared_ptr<IGameRoomRecorder> createRoomRecorder( eRoomType nRoomType );
	uint32_t getRoomRecorderCnt();
protected:
	void readRoomRecorder(uint32_t nAlreadyReadCnt);
protected:
	std::map<uint32_t, std::shared_ptr<IGameRoomRecorder>> m_vRoomRecorders; // sieralNum :  iGameRoomRecorder 
};