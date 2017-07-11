#include "NoticePlayerManager.h"
#include "log4z.h"
#include "PushNotificationServer.h"
#include "PushRequestQueue.h"
#include "json/json.h"
void stNoticePlayer::pushNotice(const char* pNotices , uint32_t nNoticFlag)
{
	if ( strlen(pNotices) > 218 )
	{
		LOGFMTE("uid = %u, notice is too long . %s ",this->nUserUID, pNotices) ;
		return ;
	}

	if ( bReadingData )
	{
		stWaitNotice notice ;
		notice.nType = nNoticFlag ;
		notice.strContent = pNotices ;
		vWaitPushNotices.push_back(notice) ;
		return ;
	}

	if ( isHaveFlag(nNoticFlag) == false )
	{
		return ;
	}

	// do send msg ;
	stNotice* p = CPushRequestQueue::SharedPushRequestQueue()->GetReserverNotice();
	memcpy(p->pDeveiceToken,this->pToken,32);
	p->nBadge = 1 ;
	sprintf_s(p->pAlert,sizeof(p->pAlert),"%s",pNotices) ;
	CPushRequestQueue::SharedPushRequestQueue()->PushNotice(p) ;
	LOGFMTD("uid = %d push notice: %s",this->nUserUID,pNotices) ;
}

void stNoticePlayer::doReadData()
{
	bReadingData = false ;
	if ( nNoticeFlag == 0 )
	{
		vWaitPushNotices.clear() ;
	}

	bReadingData = false ;
	// do send msg ;
	for ( auto& notice : vWaitPushNotices )
	{
		pushNotice(notice.strContent.c_str(),notice.nType);
	}

	vWaitPushNotices.clear() ;
}

CNoticePlayerManager::CNoticePlayerManager()
{
	m_vNoticePlayers.clear() ;
}

CNoticePlayerManager::~CNoticePlayerManager() 
{
	for ( auto ref : m_vNoticePlayers )
	{
		if ( ref.second )
		{
			delete ref.second ;
			ref.second = nullptr ;
		}
	}
	m_vNoticePlayers.clear() ;
}

bool CNoticePlayerManager::onMsg(stMsg* pmsg,uint32_t nSessionID )
{
	switch (pmsg->usMsgType )
	{
	case MSG_READ_NOTICE_PLAYER:
		{
			stMsgReadNoticePlayerRet* pRet = (stMsgReadNoticePlayerRet*)pmsg ;
			auto pp = getNoticePlayer(pRet->tPlayerEntery.nUserUID) ;
			if ( !pp )
			{
				LOGFMTE("why read notice player uid = null ret = %d",pRet->nRet) ;
				break; 
			}

			if ( pp->bReadingData == false )
			{
				LOGFMTI("read player data , but flag not reading uid = %d , disable when reading ?",pp->nUserUID) ;
				pp->doReadData() ;
				break; 
			}

			pp->bReadingData = false ;
			if ( pRet->nRet )
			{
				pp->nNoticeFlag = 0 ;
			}
			else
			{
				memcpy(pp,&pRet->tPlayerEntery,sizeof(pRet->tPlayerEntery));
			}
			pp->doReadData() ;
			LOGFMTI("read uid = %d player data ok ,flag = %d",pp->nUserUID,pp->nNoticeFlag) ;
		}
		break;
	case MSG_PUSH_NOTICE:
		{
			stMsgPushNotice* pRet = (stMsgPushNotice*)pmsg ;
			Json::Reader reader ;
			Json::Value jsRoot, target ;
			char* pBuffer = (char*)pmsg + sizeof(stMsgPushNotice) ;
			reader.parse(pBuffer,pBuffer + pRet->nJonsLen,jsRoot);
			target = jsRoot["targets"];
			if (target.isNull())
			{
				LOGFMTE("push target is null , skip ") ;
				break; 
			}

			for ( uint16_t nIdx = 0 ; nIdx < target.size(); ++nIdx )
			{
				pushNotice(target[nIdx].asUInt(),jsRoot["content"].asCString(),jsRoot["flag"].asUInt()) ;
			}
		}
		break;
	case MSG_PUSH_APNS_TOKEN:
		{
			stMsgPushAPNSToken* pRet = (stMsgPushAPNSToken*)pmsg ;

			stMsgPushAPNSTokenRet msgBack ;
			msgBack.nReqTokenRet = pRet->nReqTokenRet;
			CPushNotificationServer::getInstance()->sendMsg(nSessionID,(char*)&msgBack,sizeof(msgBack));

			auto pp = getNoticePlayer(pRet->nUserUID) ;
			if ( pRet->nReqTokenRet )
			{
				 if ( pp )
				 {
					 pp->nNoticeFlag = 0 ;
					 pp->bReadingData = false ;
				 }

				 stMsgSaveNoticePlayer msgSave ;
				 msgSave.nOpt = 1 ;
				 msgSave.tPlayer.nUserUID = pRet->nUserUID ;
				 CPushNotificationServer::getInstance()->sendMsg(0,(char*)&msgSave,sizeof(msgSave));
				 LOGFMTD(" uid = %d remove token",pRet->nUserUID) ;
			}
			else
			{
				LOGFMTD(" uid = %d add token",pRet->nUserUID) ;
				stMsgSaveNoticePlayer msgSave ;
				msgSave.nOpt = 0 ;
				msgSave.tPlayer.nUserUID = pRet->nUserUID ;
				msgSave.tPlayer.nNoticeFlag = pRet->nFlag ;
				memcpy(msgSave.tPlayer.pToken,pRet->vAPNsToken,32);
				CPushNotificationServer::getInstance()->sendMsg(0,(char*)&msgSave,sizeof(msgSave));

				if ( pp )
				{
					pp->bReadingData = false ;
					pp->nNoticeFlag = pRet->nFlag ;
					memcpy(pp->pToken,pRet->vAPNsToken,32);
				}
			}
		}
		break;
	default:
		return false ; 
	}
	return true ;
}

void CNoticePlayerManager::pushNotice(uint32_t nUserUID , const char* pNoticeContent, uint32_t nNoticFlag )
{
	auto pp = getNoticePlayer(nUserUID) ;
	if ( !pp )
	{
		pp = new stNoticePlayer ;
		pp->bReadingData = true ;
		pp->tStartReadingTime = time(nullptr);
		pp->nUserUID = nUserUID ;
		m_vNoticePlayers[pp->nUserUID] = pp ;

		stMsgReadNoticePlayer msg ;
		msg.nUserUID = nUserUID ;
		CPushNotificationServer::getInstance()->sendMsg(0,(char*)&msg,sizeof(msg));
		LOGFMTD("start to read uid = %d token data ",msg.nUserUID ) ;
	}

	pp->pushNotice(pNoticeContent,nNoticFlag) ;

	if ( pp->bReadingData )
	{
		time_t tNow = time(nullptr) ;
		if ( tNow - pp->tStartReadingTime > 6 )
		{
			stMsgReadNoticePlayer msg ;
			msg.nUserUID = nUserUID ;
			CPushNotificationServer::getInstance()->sendMsg(0,(char*)&msg,sizeof(msg));
			pp->tStartReadingTime = time(nullptr);
			LOGFMTD("read uid = %d token data time out try agian",msg.nUserUID ) ;
		}
	}
}

stNoticePlayer* CNoticePlayerManager::getNoticePlayer(uint32_t nUserUID )
{
	auto iter = m_vNoticePlayers.find(nUserUID) ;
	if ( iter != m_vNoticePlayers.end() )
	{
		return iter->second ;
	}
	return nullptr ;
}