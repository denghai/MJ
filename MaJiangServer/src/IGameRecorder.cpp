#include "IGameRecorder.h"
#include "log4z.h"
#include "AsyncRequestQuene.h"
void ISingleRoundRecorder::init(uint16_t nRoundIdx, uint32_t nFinish, uint32_t nReplayID)
{
	m_vPlayerRecorderInfo.clear();
	m_nRoundIdx = nRoundIdx;
	m_nFinishTime = nFinish;
	m_nReplayID = nReplayID;
}

uint16_t ISingleRoundRecorder::getRoundIdx()
{
	return m_nRoundIdx;
}

uint32_t ISingleRoundRecorder::getFinishTime()
{
	return m_nFinishTime;
}

uint32_t ISingleRoundRecorder::getReplayID()
{
	return m_nReplayID;
}

void ISingleRoundRecorder::addPlayerRecorderInfo(std::shared_ptr<IPlayerReocrderInfo> ptrInfo)
{
	auto iter = m_vPlayerRecorderInfo.find(ptrInfo->getUserUID());
	if (iter != m_vPlayerRecorderInfo.end())
	{
		LOGFMTE("already add this player info uid = %u", ptrInfo->getUserUID());
		m_vPlayerRecorderInfo.erase(iter);
	}
	m_vPlayerRecorderInfo[ptrInfo->getUserUID()] = ptrInfo;
}

std::shared_ptr<IPlayerReocrderInfo> ISingleRoundRecorder::getPlayerRecorderInfo(uint32_t nUserUID)
{
	auto iter = m_vPlayerRecorderInfo.find(nUserUID);
	if (iter == m_vPlayerRecorderInfo.end())
	{
		return nullptr;
	}
	return iter->second;
}

void ISingleRoundRecorder::doSaveRoundRecorder( CAsyncRequestQuene* pSyncQuene , uint32_t nSieralNum )
{
	for (auto& ref : m_vPlayerRecorderInfo)
	{
		// get opts str 
		std::string strUserDetail = "";
		Json::Value jsUserDetail;
		ref.second->getUserDetailForSave(jsUserDetail);
		if (jsUserDetail.isNull() == false)
		{
			Json::StyledWriter jswrite;
			strUserDetail = jswrite.write(jsUserDetail);
		}

		// do save sql 
		Json::Value jssql;
		char pBuffer[1024] = { 0 };
		sprintf_s(pBuffer, sizeof(pBuffer), "insert into recorderplayerinfo ( sieralNum,roundIdx,userUID,offset,timestamp,replayID,userDetail ) values (%u,%u,%u,%d,%u,%u,'%s');", 
			nSieralNum, m_nRoundIdx, ref.second->getUserUID(), ref.second->getOffset(),m_nFinishTime ,m_nReplayID,strUserDetail.c_str() );
		jssql["sql"] = pBuffer;
		pSyncQuene->pushAsyncRequest(ID_MSG_PORT_DB, eAsync_DB_Add, jssql);
	}

}

void ISingleRoundRecorder::restoreRoundRecorder( CAsyncRequestQuene* pSyncQuene, uint32_t nSieralNum, uint16_t nRoundIdx)
{
	m_nRoundIdx = nRoundIdx;

	Json::Value jssql;
	char pBuffer[1024] = { 0 };
	sprintf_s(pBuffer, sizeof(pBuffer), "select * from recorderplayerinfo where sieralNum = %u and roundIdx = %u;", nSieralNum, nRoundIdx );
	jssql["sql"] = pBuffer;

	pSyncQuene->pushAsyncRequest(ID_MSG_PORT_DB, eAsync_DB_Select, jssql, [this,nSieralNum,nRoundIdx](uint16_t nReqType, const Json::Value& retContent, Json::Value& jsUserData) {
		uint32_t nRow = retContent["afctRow"].asUInt();
		Json::Value jsData = retContent["data"];

		Json::StyledWriter jsWrite;
		auto strResult = jsWrite.write(jsData);
		LOGFMTD("===========%s", strResult.c_str());

		if (nRow == 0)
		{
			LOGFMTE("do not single room recorder sieral = %u , roundIdx = %u",nSieralNum,nRoundIdx );
		}
		else
		{
			for ( uint32_t nIdx = 0; nIdx < nRow; ++nIdx)
			{
				Json::Value jsRow = jsData[(uint32_t)nIdx];
				if (nIdx == 0)
				{
					m_nFinishTime = jsRow["timestamp"].asUInt();
					m_nReplayID = jsRow["replayID"].asUInt();
				}

				uint32_t nUserUID = jsRow["userUID"].asUInt();
				int32_t nOffset = jsRow["offset"].asInt();

				std::string strUserDetail = jsRow["userDetail"].asString();
				Json::Value jsUserDetail;
				if (strUserDetail.empty() == false)
				{
					Json::Reader jsRead;
					jsRead.parse(strUserDetail, jsUserDetail);
				}
				auto ptrUser = createPlayerRecorderInfo();
				ptrUser->setInfo(nUserUID, nOffset);
				ptrUser->restoreUserDetail(jsUserDetail);
				addPlayerRecorderInfo(ptrUser);
				LOGFMTD("read single recorder sieral = %u , round idx = %u , player uid = %u", nSieralNum,nRoundIdx,nUserUID );
			}

		}
	});
}

void ISingleRoundRecorder::buildRoundInfoForZhanJi( Json::Value& jsRoundInfo )
{
	jsRoundInfo["time"] = m_nFinishTime;
	jsRoundInfo["replayID"] = m_nReplayID;

	Json::Value jsPlayers;
	for (auto& ref : m_vPlayerRecorderInfo)
	{
		Json::Value jsInfo;
		ref.second->buildInfoForZhanJi(jsInfo);
		jsPlayers[jsPlayers.size()] = jsInfo;
	}

	jsRoundInfo["result"] = jsPlayers;
}
// room recorder 
void IGameRoomRecorder::init(uint32_t nSieralNum, uint32_t nCirleCnt, uint32_t nRoomID, uint32_t nRoomType, uint32_t nCreaterUID)
{
	m_nRoomID = nRoomID;
	m_nSieralNum = nSieralNum;
	m_nRoomType = nRoomType;
	m_nCreaterUID = nCreaterUID;
	m_nCirleCnt = nCirleCnt;
	m_vAllRoundRecorders.clear();
}

void IGameRoomRecorder::addSingleRoundRecorder(std::shared_ptr<ISingleRoundRecorder>& ptrSingleRecorder)
{
	auto iter = m_vAllRoundRecorders.find(ptrSingleRecorder->getRoundIdx());
	if (iter != m_vAllRoundRecorders.end())
	{
		LOGFMTE("duplicate round recorder for idx = %u", ptrSingleRecorder->getRoundIdx());
		m_vAllRoundRecorders.erase(iter);
	}
	m_vAllRoundRecorders[ptrSingleRecorder->getRoundIdx()] = ptrSingleRecorder;
}

std::shared_ptr<ISingleRoundRecorder> IGameRoomRecorder::getSingleRoundRecorder(uint16_t nRoundUIdx)
{
	auto iter = m_vAllRoundRecorders.find(nRoundUIdx);
	if (iter == m_vAllRoundRecorders.end())
	{
		return nullptr;
	}
	return iter->second;
}

bool IGameRoomRecorder::isHavePlayerRecorder(uint32_t nUserUID)
{
	if (m_vAllRoundRecorders.empty())
	{
		return false;
	}

	auto ptrSingle = m_vAllRoundRecorders.begin()->second;
	return ptrSingle->getPlayerRecorderInfo(nUserUID) != nullptr;
}

uint32_t IGameRoomRecorder::getSieralNum()
{
	return m_nSieralNum;
}

uint16_t IGameRoomRecorder::getRoundRecorderCnt()
{
	return m_vAllRoundRecorders.size();
}

void IGameRoomRecorder::doSaveRoomRecorder( CAsyncRequestQuene* pSyncQuene )
{
	// get opts str 
	std::string strOpts = "";
	Json::Value jsOpts;
	getRoomInfoOptsForSave(jsOpts);
	if (jsOpts.isNull() == false)
	{
		Json::StyledWriter jswrite;
		strOpts = jswrite.write(jsOpts);
	}

	// do save sql 
	Json::Value jssql;
	char pBuffer[1024] = { 0 };
	sprintf_s(pBuffer,sizeof(pBuffer) ,"insert into recorderroominfo ( sieralNum,roomID,createrUID,roomType,timestamp,cirleCnt,circleRecorderCnt,roomOpts ) values (%u,%u,%u,%u,now(),'%u','%u','%s');", m_nSieralNum, m_nRoomID,m_nCreaterUID, m_nRoomType, m_nCirleCnt,m_vAllRoundRecorders.size(),strOpts.c_str() );
	jssql["sql"] = pBuffer;
	pSyncQuene->pushAsyncRequest(ID_MSG_PORT_DB, eAsync_DB_Add, jssql );

	// save sub round info ;
	for (auto& ref : m_vAllRoundRecorders)
	{
		ref.second->doSaveRoundRecorder(pSyncQuene, getSieralNum());
	}
}

void IGameRoomRecorder::restoreGameRoomRecorder(CAsyncRequestQuene* pSyncQuene, Json::Value& jsRoomInfo)
{
	uint32_t nSieralNum = jsRoomInfo["sieralNum"].asUInt();
	uint32_t nRoomID = jsRoomInfo["roomID"].asUInt();
	uint32_t nCreaterUID = jsRoomInfo["createrUID"].asUInt();
	uint32_t nRoomType = jsRoomInfo["roomType"].asUInt();
	uint32_t nCircleRecordCnt = jsRoomInfo["circleRecorderCnt"].asUInt();
	uint32_t nCircleCnt = jsRoomInfo["cirleCnt"].asUInt();

	auto jsRoomOptsStr = jsRoomInfo["roomOpts"].asString();
	Json::Value jsOpts;
	Json::Reader jsReader;
	jsReader.parse(jsRoomOptsStr, jsOpts);

	init(nSieralNum, nCircleCnt, nRoomID, nRoomType, nCreaterUID);
	restoreRoomInfoOpts(jsOpts);
	restoreRoundRecorders(pSyncQuene, nCircleRecordCnt);
}

void IGameRoomRecorder::restoreRoundRecorders( CAsyncRequestQuene* pSyncQuene, uint8_t nRoundCnt )
{
	for ( uint8_t nIdx = 0; nIdx < nRoundCnt; ++nIdx )
	{
		auto ptr = createSingleRoundRecorder();
		ptr->init(nIdx, 0, 0);
		ptr->restoreRoundRecorder(pSyncQuene,getSieralNum(),nIdx);
		addSingleRoundRecorder(ptr);
	}
}

void IGameRoomRecorder::buildZhanJiMsg( Json::Value& jsMsg )
{
	jsMsg["sieral"] = m_nSieralNum;
	jsMsg["cirleCnt"] = m_nCirleCnt;
	jsMsg["roomID"] = m_nRoomID;
	jsMsg["createrUID"] = m_nCreaterUID;

	Json::Value jsRounds;
	for (auto& ref : m_vAllRoundRecorders)
	{
		Json::Value jsRoundInfo;
		ref.second->buildRoundInfoForZhanJi(jsRoundInfo);
		jsRounds[jsRounds.size()] = jsRoundInfo;
	}
	jsMsg["rounds"] = jsRounds;
}
