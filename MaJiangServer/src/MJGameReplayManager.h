#pragma once
#include "IGameReplay.h"
#include "IGlobalModule.h"
#include <list>
class MJReplayFrame
	: public IReplayFrame
{
public:
	bool init(uint16_t nFrameType, uint32_t nTimestamp)
	{
		m_nFrameType = nFrameType;
		m_nTimestamp = nTimestamp;
		return true;
	}

	void setFrameArg(Json::Value& jsArg)
	{
		m_jsFrameArg = jsArg;
	}

	void restore(Json::Value& jsFrame)
	{
		m_nFrameType = jsFrame["T"].asUInt();
		//m_nTimestamp = jsFrame["time"].asUInt();
		m_jsFrameArg = jsFrame["A"];
	}

	void toJson( Json::Value& jsFrame)
	{
		jsFrame["T"] = m_nFrameType;
		//jsFrame["time"] = m_nTimestamp;
		jsFrame["A"] = m_jsFrameArg;
	}

	std::shared_ptr<IReplayFrame> clone()
	{
		auto ptr = std::make_shared<MJReplayFrame>();
		ptr->init(m_nFrameType,m_nTimestamp);
		ptr->setFrameArg(m_jsFrameArg);
		return ptr;
	}

protected:
	uint16_t m_nFrameType;
	uint32_t m_nTimestamp;
	Json::Value m_jsFrameArg;
};

class MJReplayGame
	:public IReplayGame
{
public:
	void reset()override 
	{
		m_vAllFrames.clear();
	}

	void setReplayID(uint32_t nReplayID)
	{
		m_nReplayID = nReplayID;
	}

	void setReplayRoomInfo( Json::Value& jsInfo )override 
	{
		m_jsGameInfo = jsInfo;
	}

	uint32_t getReplayID()override 
	{
		return m_nReplayID;
	}

	void addFrame(std::shared_ptr<IReplayFrame> ptrFrame)override
	{
		m_vAllFrames.push_back(ptrFrame);
	}

	void restore(Json::Value& jsReplay)override
	{
		m_jsGameInfo = jsReplay["info"];
		m_nReplayID = jsReplay["id"].asUInt();
		auto jsFrames = jsReplay["frames"];
		for (uint16_t nIdx = 0; nIdx < jsFrames.size(); ++nIdx)
		{
			auto ptr = std::make_shared<MJReplayFrame>();
			ptr->restore(jsFrames[nIdx]);
			addFrame(ptr);
		}
	}

	void toJson(Json::Value& jsReplay)override 
	{
		jsReplay["info"] = m_jsGameInfo;
		jsReplay["id"] = m_nReplayID;
		Json::Value jsFrames;
		for (auto& ref : m_vAllFrames)
		{
			ref->toJson(jsFrames[jsFrames.size()]);
		}
		jsReplay["frames"] = jsFrames;
	}

	std::shared_ptr<IReplayFrame> createFrame(uint16_t nFrameType, uint32_t nTimestamp) override
	{
		auto ptr = std::make_shared<MJReplayFrame>();
		ptr->init(nFrameType, nTimestamp);
		return ptr;
	}

	std::shared_ptr<IReplayGame> clone()
	{
		auto ptr = std::make_shared<MJReplayGame>();
		ptr->setReplayRoomInfo(m_jsGameInfo);
		ptr->setReplayID( m_nReplayID );
		for (auto& ref : m_vAllFrames)
		{
			auto ptf = ref->clone();
			ptr->m_vAllFrames.push_back(ptf);
		}
		return ptr;
	}

	void sendToClient(IServerApp* pApp, uint32_t nSessionID)override;
protected:
	Json::Value m_jsGameInfo;
	uint32_t m_nReplayID;
	std::list<std::shared_ptr<IReplayFrame>> m_vAllFrames;
};

// manager 
class MJGameReplayManager
	:public IGlobalModule
{
public:
	MJGameReplayManager() { m_nMaxReplayUID = 0;  }
	bool onMsg(Json::Value& prealMsg, uint16_t nMsgType, eMsgPort eSenderPort, uint32_t nSessionID)override;
	void onConnectedSvr()override;
	void addGameReplay( std::shared_ptr<IReplayGame> ptrGameReplay , bool isSaveToDB = true );
	uint32_t generateReplayID();
protected:
	uint32_t m_nMaxReplayUID;
	std::map<uint32_t, std::shared_ptr<IReplayGame>> m_vAllGames;
};




