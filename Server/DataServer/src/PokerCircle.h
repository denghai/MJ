#pragma once
#include "MessageDefine.h"
#include "CommonDefine.h"
#include <string>
#include <vector>
class CPokerCircle
{
public:
	struct stTopicDetail
	{
		stTopicDetail(){ strContent = "" ;}
		uint64_t nTopicID ;
		uint32_t nAuthorUID ;
		uint32_t nPublishTime ;
		std::string strContent ;
	};
	typedef std::vector<stTopicDetail*> VEC_TOPIC ;
public:
	CPokerCircle();
	~CPokerCircle();
	void readTopics();
	bool onMessage(stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID);
	stTopicDetail* getTopicByID(uint64_t nTopicID );
protected:
	uint64_t m_nMaxTopicUID ;
	VEC_TOPIC m_vListTopics ;
};