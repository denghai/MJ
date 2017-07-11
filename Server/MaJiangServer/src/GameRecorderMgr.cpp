#include "GameRecorderMgr.h"
#include "log4z.h"
#include "ISeverApp.h"
#include "AsyncRequestQuene.h"
#include "NJMJPlayerRecorderInfo.h"
#include "SZMJPlayerRecorderInfo.h"
#include "CYMJPlayerRecorderInfo.h"
#include "HHMJPlayerRecorderInfo.h"
#include "JSMJPlayerRecorderInfo.h"
#include "YZMJPlayerRecorderInfo.h"
#include "BPMJPlayerRecorderInfo.h"
// game recorder mgr ;
void GameRecorderMgr::addRoomRecorder(std::shared_ptr<IGameRoomRecorder> pRoomRecorder, bool isSaveToDB)
{
	auto iter = m_vRoomRecorders.find( pRoomRecorder->getSieralNum() );
	if (iter != m_vRoomRecorders.end())
	{
		LOGFMTE("already add this room recorder sieral = %u",pRoomRecorder->getSieralNum() );
		m_vRoomRecorders.erase( iter );
	}
	m_vRoomRecorders[pRoomRecorder->getSieralNum()] = pRoomRecorder;
	
#ifdef _DEBUG
	return;
#endif
	// if need to save to db 
	if (!isSaveToDB )
	{
		return;
	}

	auto pSvr = getSvrApp()->getAsynReqQueue();
	pRoomRecorder->doSaveRoomRecorder(pSvr);
}

std::shared_ptr<IGameRoomRecorder> GameRecorderMgr::getRoomRecorder(uint32_t nSieralNum)
{
	auto iter = m_vRoomRecorders.find(nSieralNum);
	if ( iter == m_vRoomRecorders.end() )
	{
		return nullptr;
	}
	return iter->second;
}

bool GameRecorderMgr::onMsg(Json::Value& prealMsg, uint16_t nMsgType, eMsgPort eSenderPort, uint32_t nSessionID)
{
	if (nMsgType != MSG_REQ_ZHAN_JI)
	{
		return false;
	}

	uint32_t nUserUID = prealMsg["userUID"].asUInt();
	uint32_t nSiealNum = prealMsg["curSerial"].asUInt();
	
	Json::Value jsMsgBack;
	auto iter = m_vRoomRecorders.find(nSiealNum);
	if (iter == m_vRoomRecorders.end() )
	{
		jsMsgBack["nRet"] = 1;
		getSvrApp()->sendMsg(nSessionID, jsMsgBack, MSG_REQ_ZHAN_JI);
		return true;
	}

	if (iter->second->isHavePlayerRecorder(nUserUID) == false)
	{
		jsMsgBack["nRet"] = 2;
		getSvrApp()->sendMsg(nSessionID, jsMsgBack, MSG_REQ_ZHAN_JI);
		return true;
	}
	jsMsgBack["nRet"] = 0;
	iter->second->buildZhanJiMsg(jsMsgBack);
	getSvrApp()->sendMsg(nSessionID, jsMsgBack, MSG_REQ_ZHAN_JI);
	return true;
}

void GameRecorderMgr::onConnectedSvr()
{
#ifdef _DEBUG
	return;
#endif
	if (m_vRoomRecorders.empty() == false)
	{
		return;
	}

	readRoomRecorder( 0 );
}

uint32_t GameRecorderMgr::getRoomRecorderCnt()
{
	return m_vRoomRecorders.size();
}

void GameRecorderMgr::readRoomRecorder(uint32_t nAlreadyReadCnt)
{
	auto asyq = getSvrApp()->getAsynReqQueue();
	std::ostringstream ss;
	ss << "SELECT sieralNum,roomID,createrUID,roomType,cirleCnt, circleRecorderCnt,roomOpts FROM recorderroominfo order by sieralNum desc limit 10 offset " << nAlreadyReadCnt << ";";
	Json::Value jsReq;
	jsReq["sql"] = ss.str();
	asyq->pushAsyncRequest(ID_MSG_PORT_DB, eAsync_DB_Select, jsReq, [this, asyq ](uint16_t nReqType, const Json::Value& retContent, Json::Value& jsUserData) {
		uint32_t nAft = retContent["afctRow"].asUInt();
		auto jsData = retContent["data"];
		if (nAft == 0 || jsData.isNull())
		{
			LOGFMTE("read max bill id error ");
			return;
		}

		for (uint32_t nRowIdx = 0; nRowIdx < nAft; ++nRowIdx)
		{
			auto jsRow = jsData[nRowIdx];
			uint32_t nRoomType = jsRow["roomType"].asUInt();

			auto ptrRoomRecorder = createRoomRecorder((eRoomType)nRoomType);
			ptrRoomRecorder->restoreGameRoomRecorder(asyq, jsRow);
			addRoomRecorder(ptrRoomRecorder, false);
		}

		auto nNewOffset = getRoomRecorderCnt();
		if (nNewOffset > 80 || nAft < 10)  // only read recent 80 items ;
		{
			return;
		}

		// not finish , go on read 
		readRoomRecorder(nNewOffset);

	});
}

std::shared_ptr<IGameRoomRecorder> GameRecorderMgr::createRoomRecorder(eRoomType nRoomType)
{
	switch (nRoomType )
	{
	case eRoom_MJ_NanJing:
	{
		return std::make_shared<NJMJRoomRecorder>();
	}
	break;
	//case eRoom_MJ_SuZhou:
	//{
	//	return std::make_shared<SZMJRoomRecorder>();
	//}
	//break;
	case eRoom_MJ_CaoYang:
	{
		return std::make_shared<CYMJRoomRecorder>();
	}
	case eRoom_MJ_HongHe:
	{
		return std::make_shared<HHMJRoomRecorder>();
	}
	case eRoom_MJ_JianShui:
	{
		return std::make_shared<JSMJRoomRecorder>();
	}
	case eRoom_MJ_YangZhou:
	{
		return std::make_shared<YZMJRoomRecorder>();
	}
	case eRoom_MJ_BeiPiao:
	{
		return std::make_shared<BPMJRoomRecorder>();
	}
	default:
		LOGFMTE("unknown room type = %u",nRoomType );
	}
	return nullptr;
}
