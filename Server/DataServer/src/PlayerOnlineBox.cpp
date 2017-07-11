#include "PlayerOnlineBox.h"
#include "GameServerApp.h"
#include "ConfigManager.h"
#include "BoxConfig.h"
#include "PlayerBaseData.h"
#include "Player.h"
#include "LogManager.h"
CPlayerOnlineBox::CPlayerOnlineBox(CPlayerBaseData* pPlayerBaseData)
{
	m_pPlayerBaseData = pPlayerBaseData ;
	m_nBeginTime = 0 ;
	m_bRunning = false ;
}

CPlayerOnlineBox::~CPlayerOnlineBox()
{

}

void CPlayerOnlineBox::StartTimer()
{
	m_nBeginTime = time(NULL) ;
	m_bRunning = true ;
}

void CPlayerOnlineBox::PauseTimer()
{
	if ( !m_bRunning )
	{
		return ;
	}
	m_bRunning = false ;
	time_t now = time(NULL) ;
	GetSeverBaseData()->nOnlineBoxPassedTime += (unsigned int)(now - m_nBeginTime );
	m_nBeginTime = 0 ;
}

bool CPlayerOnlineBox::OnMessage(stMsg* pMsg )
{
	switch ( pMsg->usMsgType )
	{
	case MSG_PLAYER_REQUEST_ONLINE_BOX_REWARD:
		{
			stMsgPlayerRequestOnlineBoxRewoard* pRet = (stMsgPlayerRequestOnlineBoxRewoard*)pMsg;
			stMsgPlayerRequestOnlineBoxRewoardRet msgBack ;
			msgBack.nRet = 0 ;
			msgBack.nFinishedBoxID = pRet->nBoxID ;
			msgBack.nFinalCoin = m_pPlayerBaseData->GetAllCoin();
			msgBack.nFinalDiamoned = m_pPlayerBaseData->GetAllDiamoned() ;
			if ( pRet->nBoxID != GetSeverBaseData()->nCurOnlineBoxID )
			{
				msgBack.nRet = 2 ;
				m_pPlayerBaseData->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
				break ;
			}

			stBoxConfig* pConfig = ( (CBoxConfigMgr*)CGameServerApp::SharedGameServerApp()->GetConfigMgr()->GetConfig(CConfigManager::eConfig_Box))->GetBoxConfigByBoxID(GetSeverBaseData()->nCurOnlineBoxID) ;
			if ( !pConfig )
			{
				msgBack.nRet = 2 ;
				m_pPlayerBaseData->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
				break ;
			}

			if ( m_bRunning )
			{
				time_t now = time(NULL) ;
				GetSeverBaseData()->nOnlineBoxPassedTime += (unsigned int)(now - m_nBeginTime );
				m_nBeginTime = now ;
			}

			if ( pConfig->nLastTime > GetSeverBaseData()->nOnlineBoxPassedTime )
			{
				msgBack.nRet = 1; 
				m_pPlayerBaseData->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
				break ;
			}

			// give prize ;
			m_pPlayerBaseData->ModifyMoney(pConfig->nRewardDiamond,true) ;
			m_pPlayerBaseData->ModifyMoney(pConfig->nRewardCoin,false) ;
			msgBack.nFinalCoin = m_pPlayerBaseData->GetAllCoin();
			msgBack.nFinalDiamoned = m_pPlayerBaseData->GetAllDiamoned() ;
			m_pPlayerBaseData->SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;

			GetSeverBaseData()->nCurOnlineBoxID = pConfig->nNextBoxID ;
			GetSeverBaseData()->nOnlineBoxPassedTime = 0 ;
			m_nBeginTime = time(NULL) ;
		}
		break;
	case MSG_PLAYER_REQUEST_ONLINE_BOX_STATE:
		{
			if ( m_bRunning )
			{
				time_t now = time(NULL) ;
				GetSeverBaseData()->nOnlineBoxPassedTime += (unsigned int)(now - m_nBeginTime );
				m_nBeginTime = now ;
			}

			stMsgPlayerRequestOnlineBoxStateRet msgBack ;
			msgBack.nRet = 0 ;
			msgBack.nBoxID = GetSeverBaseData()->nCurOnlineBoxID ;
			msgBack.nBoxRunedTime = GetSeverBaseData()->nOnlineBoxPassedTime ;
			m_pPlayerBaseData->SendMsgToClient((char*)&msgBack,sizeof(msgBack));
		}
		break;
	default:
		return false ;
	}
	return true ;
}

void CPlayerOnlineBox::OnPlayerDisconnect()
{
	Reset();
}

void CPlayerOnlineBox::Reset()
{
	GetSeverBaseData()->nOnlineBoxPassedTime = 0 ;
	m_nBeginTime = 0 ;
	m_bRunning = false ;
	GetSeverBaseData()->nCurOnlineBoxID = 0 ;
}

stServerBaseData* CPlayerOnlineBox::GetSeverBaseData()
{
	return &m_pPlayerBaseData->m_stBaseData ;
}
