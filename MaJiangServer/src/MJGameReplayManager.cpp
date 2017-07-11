#include "MJGameReplayManager.h"
#include "ISeverApp.h"
#include "log4z.h"
#include "AsyncRequestQuene.h"
void MJReplayGame::sendToClient(IServerApp* pApp, uint32_t nSessionID)
{
	// base data 
	Json::Value jsInfo;
	jsInfo["replayID"] = m_nReplayID;
	jsInfo["ret"] = 0;
	jsInfo["roomInfo"] = m_jsGameInfo;
	pApp->sendMsg(nSessionID, jsInfo, MSG_REQ_GAME_REPLAY );

	// send frame data 
	Json::Value jsFrameData;
	jsFrameData["replayID"] = m_nReplayID;
	uint32_t nSended = 0;
	for ( auto& ref : m_vAllFrames )
	{
		if (m_vAllFrames.size() == (nSended + 1))
		{
			jsFrameData["isEnd"] = 1;
		}

		ref->toJson(jsFrameData["data"]);
		pApp->sendMsg(nSessionID, jsFrameData, MSG_REPLAY_FRAME );
		++nSended;
	}
}

// game mgr 
bool MJGameReplayManager::onMsg( Json::Value & prealMsg, uint16_t nMsgType, eMsgPort eSenderPort, uint32_t nSessionID )
{
	if ( nMsgType == MSG_REQ_GAME_REPLAY )
	{
		auto nReplayID = prealMsg["replayID"].asUInt();
		auto iter = m_vAllGames.find(nReplayID);
		if (iter == m_vAllGames.end())
		{
			Json::Value jsInfo;
			jsInfo["replayID"] = nReplayID;
			jsInfo["ret"] = 1;
			getSvrApp()->sendMsg(nSessionID, jsInfo, MSG_REQ_GAME_REPLAY);
		}
		else
		{
			iter->second->sendToClient(getSvrApp(), nSessionID);
		}
		return true;
	}
	return false;
}

void MJGameReplayManager::onConnectedSvr()
{
	auto asyq = getSvrApp()->getAsynReqQueue();
	Json::Value jsReq;
	jsReq["sql"] = "SELECT max(replayID) as maxReplayID FROM gamereplay ;";
	asyq->pushAsyncRequest(ID_MSG_PORT_DB, eAsync_DB_Select, jsReq, [this](uint16_t nReqType, const Json::Value& retContent, Json::Value& jsUserData) {
		uint32_t nAft = retContent["afctRow"].asUInt();
		auto jsData = retContent["data"];
		if (nAft == 0 || jsData.isNull())
		{
			LOGFMTW("read maxReplayID id error, but no matter ");
			return;
		}

		auto jsRow = jsData[(uint32_t)0];
		m_nMaxReplayUID = jsRow["maxReplayID"].asUInt();
		LOGFMTD("maxReplayID id  = %u", m_nMaxReplayUID);
	});
}

void MJGameReplayManager::addGameReplay( std::shared_ptr<IReplayGame> ptrGameReplay, bool isSaveToDB )
{
	//m_vAllGames[ptrGameReplay->getReplayID()] = ptrGameReplay;
	if ( !isSaveToDB )
	{
		return;
	}

	Json::Value js;
	ptrGameReplay->toJson(js);

	Json::StyledWriter jss;
	auto strJs = jss.write(js);
	LOGFMTI("replay len = %u", strJs.size() );

	// save to DB ;
	Json::Value jsreq;
	static char s_pBuffer[30014] = { 0 };
	if (strJs.size() > sizeof(s_pBuffer))
	{
		LOGFMTE("the replay data is too big ,can not store replayid = %u",ptrGameReplay->getReplayID() );
		return;
	}

	memset(s_pBuffer,0,sizeof(s_pBuffer));
	sprintf_s(s_pBuffer, sizeof(s_pBuffer), "insert into gamereplay (replayID,time,detail ) values (%u,now(),'%s'); ",ptrGameReplay->getReplayID(),strJs.c_str());
	jsreq["sql"] = s_pBuffer;
	getSvrApp()->getAsynReqQueue()->pushAsyncRequest(ID_MSG_PORT_DB, eAsync_DB_Add, jsreq);
}

uint32_t MJGameReplayManager::generateReplayID()
{
	return ++m_nMaxReplayUID;
}
