#pragma once
#include "NativeTypes.h"
#include "json/json.h"
#include <memory>
class CAsyncRequestQuene;
class IPlayerReocrderInfo
{
public:
	virtual void setInfo(uint32_t nUserUID , int32_t nOffset ) = 0 ;
	virtual uint32_t getUserUID() = 0;
	virtual int32_t getOffset() = 0;
	virtual void getUserDetailForSave( Json::Value& jsUserDetail ) = 0;
	virtual void restoreUserDetail( Json::Value& jsUserDetail ) = 0;
	virtual void buildInfoForZhanJi( Json::Value& jsUserInfo ) = 0;
};

class ISingleRoundRecorder
{
public:
	virtual void init(uint16_t nRoundIdx, uint32_t nFinish, uint32_t nReplayID);
	uint16_t getRoundIdx();
	uint32_t getFinishTime();
	uint32_t getReplayID();
	void addPlayerRecorderInfo(std::shared_ptr<IPlayerReocrderInfo> ptrInfo);
	std::shared_ptr<IPlayerReocrderInfo> getPlayerRecorderInfo(uint32_t nUserUID);
	void doSaveRoundRecorder( CAsyncRequestQuene* pSyncQuene , uint32_t nSieralNum );
	virtual std::shared_ptr<IPlayerReocrderInfo> createPlayerRecorderInfo() = 0;
	void restoreRoundRecorder(CAsyncRequestQuene* pSyncQuene, uint32_t nSieralNum,uint16_t nRoundIdx ) ;
	virtual void buildRoundInfoForZhanJi(Json::Value& jsRoundInfo );
protected:
	std::map<uint32_t, std::shared_ptr<IPlayerReocrderInfo>> m_vPlayerRecorderInfo;
	uint32_t m_nFinishTime;
	uint32_t m_nReplayID;
	uint16_t m_nRoundIdx;
};

class IGameRoomRecorder
{
public:
	virtual void init(uint32_t nSieralNum, uint32_t nCirleCnt ,uint32_t nRoomID,uint32_t nRoomType,uint32_t nCreaterUID );
	void addSingleRoundRecorder(std::shared_ptr<ISingleRoundRecorder>& ptrSingleRecorder);
	std::shared_ptr<ISingleRoundRecorder> getSingleRoundRecorder(uint16_t nRoundUIdx);
	virtual bool isHavePlayerRecorder(uint32_t nUserUID);
	uint32_t getSieralNum();
	uint16_t getRoundRecorderCnt();
	void doSaveRoomRecorder( CAsyncRequestQuene* pSyncQuene  );
	void restoreGameRoomRecorder( CAsyncRequestQuene* pSyncQuene, Json::Value& jsRoomInfo );
	virtual std::shared_ptr<ISingleRoundRecorder> createSingleRoundRecorder() = 0 ;
	virtual void buildZhanJiMsg(Json::Value& jsMsg);
protected:
	void restoreRoundRecorders(CAsyncRequestQuene* pSyncQuene, uint8_t nRoundCnt);
	virtual void getRoomInfoOptsForSave(Json::Value& jsRoomOpts) = 0;
	virtual void restoreRoomInfoOpts(Json::Value& jsRoomOpts) = 0;
protected:
	uint32_t m_nRoomID;
	uint32_t m_nRoomType;
	uint32_t m_nSieralNum;
	uint32_t m_nCreaterUID;
	uint32_t m_nCirleCnt;
	std::map<uint16_t, std::shared_ptr<ISingleRoundRecorder>> m_vAllRoundRecorders;  // roundIdx : SingleRoundRecorder ;
};