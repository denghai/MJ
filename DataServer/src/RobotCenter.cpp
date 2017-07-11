#include "RobotCenter.h"
#include "log4z.h"
#include "ServerMessageDefine.h"
#include "ISeverApp.h"
CRobotCenter::CRobotCenter()
{
	m_vIdleRobots.clear() ;
	m_vReqRobotCmdCacher.clear() ;
}

CRobotCenter::~CRobotCenter() 
{
	for ( auto ref : m_vIdleRobots )
	{
		delete ref ;
	}

	for ( auto ref : m_vReqRobotCmdCacher )
	{
		delete ref ;
	}

	m_vIdleRobots.clear() ;
	m_vReqRobotCmdCacher.clear() ;
}

bool CRobotCenter::onMsg(stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID)
{
	switch (prealMsg->usMsgType)
	{
	case MSG_TELL_ROBOT_IDLE:
		{
			stMsgTellRobotIdle* pRet = (stMsgTellRobotIdle*)prealMsg ;
			if ( isRobotInTheList(pRet->nRobotUID) )
			{
				onRobotOtherLogin(pRet->nRobotUID,nSessionID) ;
			}
			else
			{
				stIdleRobot* pR = new stIdleRobot ;
				pR->nLevel = pRet->nRobotLevel ;
				pR->nSessionID = nSessionID ;
				pR->nUserUID = pRet->nRobotUID ;
				m_vIdleRobots.push_back(pR) ;
			}
			LOGFMTD("a robot uid = %u enter idle ",pRet->nRobotUID ) ;
			processRobotReq();
		}
		break;
	case MSG_REQ_ROBOT_ENTER_ROOM:
		{
			stMsgRequestRobotToEnterRoom* pRet = (stMsgRequestRobotToEnterRoom*)prealMsg ;
			stRequestRobotCmd* cmd = nullptr; 
			for ( auto ref : m_vReqRobotCmdCacher )
			{
				if ( ref->nRoomID == pRet->nRoomID && ref->nRoomType == pRet->nRoomType &&  ref->nSubRoomIdx == pRet->nSubRoomIdx )
				{
					cmd = ref ;
					++ref->nReqCnt;
					if ( ref->nReqCnt > 4 )
					{
						ref->nReqCnt = 4 ;
					}
					break;
				}
			}

			if ( cmd == nullptr )
			{
				cmd = new stRequestRobotCmd ;
				cmd->nReqRobotLevel = pRet->nReqRobotLevel ;
				cmd->nRoomID = pRet->nRoomID ;
				cmd->nRoomType = pRet->nRoomType ;
				cmd->nSubRoomIdx = pRet->nSubRoomIdx ;
				cmd->nReqCnt = 1 ;
				m_vReqRobotCmdCacher.push_back(cmd) ;
			}

			LOGFMTD("received req from room id = %u , type = %u",pRet->nRoomID,pRet->nRoomType) ;
			processRobotReq();
		}
		break;
	default:
		return false;
	}
	return true ;
}

void CRobotCenter::onRobotDisconnect(uint32_t nUID)
{
	auto iter = m_vIdleRobots.begin() ;
	for ( ; iter != m_vIdleRobots.end() ; ++iter )
	{
		if ( (*iter)->nUserUID == nUID )
		{
			delete *iter ;
			m_vIdleRobots.erase(iter) ;
			break; 
		}
	}
}

void CRobotCenter::onRobotOtherLogin(uint32_t nUID , uint32_t nNewSessionID )
{
	for ( auto ref : m_vIdleRobots )
	{
		if ( ref->nUserUID == nUID )
		{
			ref->nSessionID = nNewSessionID ;
			break; 
		}
	}
}

bool CRobotCenter::isRobotInTheList(uint32_t nUID )
{
	for ( auto ref : m_vIdleRobots )
	{
		if ( ref->nUserUID == nUID )
		{
			return true ;
		}
	}
	return false ;
}

void CRobotCenter::processRobotReq()
{
	if ( m_vIdleRobots.empty() || m_vReqRobotCmdCacher.empty() )
	{
		return ;
	}

	LIST_REQ_ROBOT_CMD vWillDelete ;
	for ( auto cmd : m_vReqRobotCmdCacher )
	{
		auto iter = m_vIdleRobots.begin() ;
		for ( ; iter != m_vIdleRobots.end(); ++iter )
		{
			if ( cmd->nReqRobotLevel == (*iter)->nLevel )
			{
				stMsgTellRobotEnterRoom msgBack ;
				msgBack.nRoomID = cmd->nRoomID ;
				msgBack.nRoomType = cmd->nRoomType ;
				msgBack.nSubRoomIdx = cmd->nSubRoomIdx ;
				getSvrApp()->sendMsg((*iter)->nSessionID,(char*)&msgBack,sizeof(msgBack));
				LOGFMTD("order robot uid = %u to enter room type = %u , room id = %u, sbuIdx = %u",(*iter)->nUserUID,cmd->nRoomType,cmd->nRoomID,cmd->nSubRoomIdx) ;

				delete (*iter) ;
				(*iter) = nullptr ;
				m_vIdleRobots.erase(iter) ;
				--cmd->nReqCnt;
				if ( cmd->nReqCnt <= 0 )
				{
					vWillDelete.push_back(cmd);
				}
				break;
			}
		}
	}

	for ( auto delRef : vWillDelete )
	{
		auto iterDel = m_vReqRobotCmdCacher.begin() ;
		for ( ; iterDel != m_vReqRobotCmdCacher.end() ; ++iterDel )
		{
			if ( (*iterDel) == delRef )
			{
				LOGFMTD("finish req form room type = %u ,id = %u and delete it",delRef->nRoomType,delRef->nRoomID) ;
				delete (*iterDel) ;
				m_vReqRobotCmdCacher.erase(iterDel) ;
				break; 
			}
		}
	}

	vWillDelete.clear();
}