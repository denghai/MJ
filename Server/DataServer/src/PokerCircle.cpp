#include "PokerCircle.h"
#include "GameServerApp.h"
#include "PlayerManager.h"
#include "Player.h"
#include "PlayerBaseData.h"
#include "AutoBuffer.h"
#include "LogManager.h"
#include <algorithm>  
CPokerCircle::CPokerCircle()
{
	m_vListTopics.clear() ;
	m_nMaxTopicUID = 0 ;
}

CPokerCircle::~CPokerCircle()
{
	for ( stTopicDetail* pD : m_vListTopics )
	{
		delete pD ;
		pD = nullptr ;
	}
	m_vListTopics.clear() ;
}

void CPokerCircle::readTopics()
{
	if ( m_vListTopics.empty() == false )
	{
		return ;
	}
	stMsgReadCircleTopics msg ;
	CGameServerApp::SharedGameServerApp()->sendMsg(0,(char*)&msg,sizeof(msg)) ;
}

bool TopicSort(CPokerCircle::stTopicDetail* pLeft , CPokerCircle::stTopicDetail* pRight )
{
	return pLeft->nTopicID < pRight->nTopicID ;
}

bool CPokerCircle::onMessage(stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID)
{
	switch ( prealMsg->usMsgType )
	{
	case MSG_CIRCLE_READ_TOPICS:
		{
			stMsgReadCircleTopicsRet* pRet = (stMsgReadCircleTopicsRet*)prealMsg ;
			stCircleTopicItem* pItem = (stCircleTopicItem*)(((char*)prealMsg) + sizeof(stMsgReadCircleTopicsRet));
			while ( pRet->nCnt-- )
			{
				stTopicDetail* pDetail = new stTopicDetail ;
				pDetail->nAuthorUID = pItem->nAuthorUID ;
				pDetail->nPublishTime = pItem->nPublishTime ;
				pDetail->nTopicID = pItem->nTopicID ;
				char* pContent = (char*)pItem;
				pContent = pContent + sizeof(stCircleTopicItem);
				pDetail->strContent.append(pContent,pItem->nContentLen) ;
				m_vListTopics.push_back(pDetail);
				pContent = pContent + pItem->nContentLen ;
				pItem = (stCircleTopicItem*)pContent ;
				if ( pDetail->nTopicID > m_nMaxTopicUID )
				{
					m_nMaxTopicUID = pDetail->nTopicID ;
				}
				//CLogMgr::SharedLogMgr()->SystemLog("read topic id = %d , content: %s",pDetail->nTopicID,pDetail->strContent.c_str()) ;
				std::sort(m_vListTopics.begin(),m_vListTopics.end(),TopicSort);
			}
		}
		break;
	case MSG_CIRCLE_PUBLISH_TOPIC:
		{
			stMsgPublishTopicRet msgBack ;
			stMsgPublishTopic* pRet = (stMsgPublishTopic*)prealMsg ;
			CPlayer* pPlayer = CGameServerApp::SharedGameServerApp()->GetPlayerMgr()->GetPlayerBySessionID(nSessionID);
			if ( pRet->nContentLen >= MAX_CIRCLE_CONTENT_LEN || pRet->nContentLen < 8 )
			{
				msgBack.nRet = 2 ;
				msgBack.nTopicID = 0 ;
				CGameServerApp::SharedGameServerApp()->sendMsg(nSessionID,(char*)&msgBack,sizeof(msgBack)) ;
				return true ;
			}

			//if ( pPlayer->GetBaseData()->GetData()->isRegister == 0 )
			//{
			//	msgBack.nRet = 3 ;
			//	msgBack.nTopicID = 0 ;
			//	CGameServerApp::SharedGameServerApp()->sendMsg(nSessionID,(char*)&msgBack,sizeof(msgBack)) ;
			//	return true ;
			//}

			if ( pPlayer->GetBaseData()->GetAllCoin() < 200 )
			{
				msgBack.nRet = 1 ;
				msgBack.nTopicID = 0 ;
				CGameServerApp::SharedGameServerApp()->sendMsg(nSessionID,(char*)&msgBack,sizeof(msgBack)) ;
				return true ;
			}
			pPlayer->GetBaseData()->decressMoney(200);
			stTopicDetail* pDetail = new stTopicDetail ;
			pDetail->nAuthorUID = pPlayer->GetUserUID() ;
			pDetail->nPublishTime = time(nullptr) ;
			pDetail->nTopicID = ++m_nMaxTopicUID ;
			pDetail->strContent.append(((char*)prealMsg) + sizeof(stMsgPublishTopic),pRet->nContentLen) ;
			CLogMgr::SharedLogMgr()->PrintLog("recive content = %s",pDetail->strContent.c_str()) ;
			m_vListTopics.push_back(pDetail);

			msgBack.nRet = 0 ;
			msgBack.nTopicID = pDetail->nTopicID ;
			CGameServerApp::SharedGameServerApp()->sendMsg(nSessionID,(char*)&msgBack,sizeof(msgBack)) ;

			stMsgSaveAddCircleTopic msgAdd ;
			msgAdd.item.nAuthorUID = pDetail->nAuthorUID ;
			msgAdd.item.nPublishTime = pDetail->nPublishTime ;
			msgAdd.item.nTopicID = pDetail->nTopicID ;
			msgAdd.item.nContentLen = pDetail->strContent.size() ;
			CAutoBuffer auAddTopic(sizeof(msgAdd) + msgAdd.item.nContentLen );
			auAddTopic.addContent(&msgAdd,sizeof(msgAdd)) ;
			auAddTopic.addContent(pDetail->strContent.c_str(),msgAdd.item.nContentLen) ;
			CGameServerApp::SharedGameServerApp()->sendMsg(0,auAddTopic.getBufferPtr(),auAddTopic.getContentSize()) ;
		}
		break;
	case MSG_CIRCLE_DELETE_TOPIC:
		{
			stMsgDeleteTopic* pDelete = (stMsgDeleteTopic*)prealMsg ;
			CPlayer* pPlayer = CGameServerApp::SharedGameServerApp()->GetPlayerMgr()->GetPlayerBySessionID(nSessionID);
			if ( pPlayer->GetUserUID() != MATCH_MGR_UID )
			{
				return true ;
			}

			auto iter = m_vListTopics.begin() ;
			for ( ; iter != m_vListTopics.end() ; ++iter )
			{
				if ( (*iter)->nTopicID == pDelete->nDelTopicID )
				{
					delete (*iter) ;
					(*iter) = nullptr ;
					m_vListTopics.erase(iter) ;

					stMsgSaveDeleteCircleTopic msgDel ;
					msgDel.nTopicID = pDelete->nDelTopicID ;
					CGameServerApp::SharedGameServerApp()->sendMsg(0,(char*)&msgDel,sizeof(msgDel)) ;
					return true ;
				}
			}
		}
		break;
	case MSG_CIRCLE_REQUEST_TOPIC_LIST:
		{
			stMsgRequestTopicList* pRet = (stMsgRequestTopicList*)prealMsg ;
			stMsgRequestTopicListRet msgBack ;
			msgBack.nRet = 0 ;
			msgBack.nPageIdx = pRet->nPageIdx ;
			msgBack.nTotalPageCnt = ( m_vListTopics.size() + CIRCLE_TOPIC_CNT_PER_PAGE - 1 ) / CIRCLE_TOPIC_CNT_PER_PAGE ;
			memset(msgBack.vTopicIDs,0,sizeof(msgBack.vTopicIDs)) ;
			if ( pRet->nPageIdx >= msgBack.nTotalPageCnt )
			{
				msgBack.nRet = 1 ;
				CGameServerApp::SharedGameServerApp()->sendMsg(nSessionID,(char*)&msgBack,sizeof(msgBack)) ;
				return true ;
			}

			CLogMgr::SharedLogMgr()->SystemLog("page idx = %d",pRet->nPageIdx);
			uint64_t nStartIdx = pRet->nPageIdx * CIRCLE_TOPIC_CNT_PER_PAGE ;
			for ( uint8_t nIdx =0 ; nStartIdx < m_vListTopics.size() && nIdx < CIRCLE_TOPIC_CNT_PER_PAGE ; ++nStartIdx, ++nIdx )
			{
				uint64_t nReverIdx = m_vListTopics.size() - 1 - nStartIdx ;
				msgBack.vTopicIDs[nIdx] = m_vListTopics[nReverIdx]->nTopicID ;
				CLogMgr::SharedLogMgr()->SystemLog("topic array idx = %d ,id = %I64d",nIdx,msgBack.vTopicIDs[nIdx]) ;
			}
			CGameServerApp::SharedGameServerApp()->sendMsg(nSessionID,(char*)&msgBack,sizeof(msgBack)) ;
		}
		break;
	case MSG_CIRCLE_REQUEST_TOPIC_DETAIL:
		{
			stMsgRequestTopicDetail* pRet = (stMsgRequestTopicDetail*)prealMsg ;
			auto topic = getTopicByID(pRet->nTopicID) ;
			stMsgRequestTopicDetailRet msgBack ;
			msgBack.nRet = 0 ;
			if ( topic == nullptr )
			{
				msgBack.nRet = 1 ;
				CGameServerApp::SharedGameServerApp()->sendMsg(nSessionID,(char*)&msgBack,sizeof(msgBack)) ;
				return true ;
			}

			msgBack.nAuthorUID = topic->nAuthorUID ;
			msgBack.nPublishTime = topic->nPublishTime ;
			msgBack.nTopicID = topic->nTopicID ;
			msgBack.nContentLen = topic->strContent.size() ;
			
			CAutoBuffer aubuff(sizeof(msgBack) + msgBack.nContentLen );
			aubuff.addContent(&msgBack,sizeof(msgBack)) ;
			aubuff.addContent(topic->strContent.c_str(),msgBack.nContentLen) ;
			CGameServerApp::SharedGameServerApp()->sendMsg(nSessionID,aubuff.getBufferPtr(),aubuff.getContentSize()) ;
			CLogMgr::SharedLogMgr()->PrintLog("send content = %s",topic->strContent.c_str()) ;
		}
		break;
	default:
		return false;
	}
	return true ;
}

CPokerCircle::stTopicDetail* CPokerCircle::getTopicByID(uint64_t nTopicID )
{
	for ( stTopicDetail* pTopic : m_vListTopics )
	{
		if ( pTopic->nTopicID == nTopicID )
		{
			return pTopic ;
		}
	}
	return nullptr ;
}