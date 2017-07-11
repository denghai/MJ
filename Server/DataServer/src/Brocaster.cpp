#include "Brocaster.h"
#include "InformConfig.h"
#include "GameServerApp.h"
#include "ConfigManager.h"
#include "Player.h"
#include "LogManager.h"
#include "PlayerBaseData.h"
CBrocaster::CBrocaster()
{
	m_pInformBuffer = NULL;
	m_nBufferLen = 0;
	m_nInformLoadTime = 0 ;
}

CBrocaster::~CBrocaster()
{
	if ( m_pInformBuffer )
	{
		delete[] m_pInformBuffer ;
		m_pInformBuffer = NULL ;
		m_nBufferLen = 0;
		m_nInformLoadTime = 0 ;
	}
}

void CBrocaster::PostPlayerSayMsg(CPlayer* pPlayer , char* pContent , unsigned short nLen )
{
	if ( pPlayer == NULL || NULL == pContent || 0 == nLen )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("can not brocast player say words !") ;
		return ;
	}
	stMsgBrocastPlayerSay msgReal ;
	msgReal.nBrocastType = 2 ;
	msgReal.nContentLen = nLen ;
	msgReal.nPlayerNameLen = strlen(pPlayer->GetBaseData()->GetPlayerName()) ;

	static char pBuffer[MAX_LEN_SPEAK_WORDS*3] ;
	memset(pBuffer,0,sizeof(pBuffer));
	unsigned short nOffset = 0 ;
	memcpy(pBuffer,&msgReal,sizeof(msgReal));
	nOffset += sizeof(msgReal);

	memcpy(pBuffer + nOffset , pPlayer->GetBaseData()->GetPlayerName(),msgReal.nPlayerNameLen);
	nOffset += msgReal.nPlayerNameLen ;

	memcpy(pBuffer + nOffset , pContent,nLen );
	nOffset += nLen ;
	SendBrocastMessage(pBuffer,nOffset) ;
}

void CBrocaster::SendBrocastMessage(char* pBuffer , unsigned short nLen )
{
	if ( pBuffer == 0 || 0 == nLen )
	{
		CLogMgr::SharedLogMgr()->ErrorLog( "bad brocaste content!" ) ;
		return ;
	}
	CPlayer* pPlayer = CGameServerApp::SharedGameServerApp()->GetPlayerMgr()->GetFirstActivePlayer() ;
	if ( pPlayer )
	{
		pPlayer->SendMsgToClient(pBuffer,nLen,true) ;
	}
}

void CBrocaster::SendInformsToPlayer(CPlayer* pPlayer)
{
	CInformConfig* pConfig = (CInformConfig*)CGameServerApp::SharedGameServerApp()->GetConfigMgr()->GetConfig(CConfigManager::eConfig_Informs) ;
	if ( pConfig->m_tLoadTime != m_nInformLoadTime )
	{
		if ( m_pInformBuffer )
		{
			delete[] m_pInformBuffer ;
			m_pInformBuffer = NULL ;
			m_nBufferLen = 0;
			m_nInformLoadTime = 0 ;
		}
	}

	if ( m_pInformBuffer == NULL )
	{
		m_nInformLoadTime = pConfig->m_tLoadTime ;
		stMsgRequestNewNoticeRet msg ;
		msg.nInformCount = pConfig->m_vAllInforms.size() ;
		int nSize = sizeof(stMsgRequestNewNoticeRet) ;
		CInformConfig::VEC_INFORMS::iterator iter = pConfig->m_vAllInforms.begin() ;
		for ( ; iter != pConfig->m_vAllInforms.end(); ++iter )
		{
			nSize += sizeof(stInform);
			nSize += strlen((*iter)->strTitle.c_str()) ;
			nSize += strlen((*iter)->strContent.c_str()) ;
		}
		m_pInformBuffer = new char[nSize] ;
		m_nBufferLen = nSize ;

		// write buffer ;
		unsigned int nOffset = 0 ;
		memcpy(m_pInformBuffer,&msg,sizeof(msg));
		nOffset += sizeof(msg);

		char* pTempBuffer = m_pInformBuffer + nOffset ;
		iter = pConfig->m_vAllInforms.begin() ;
		for ( ; iter != pConfig->m_vAllInforms.end(); ++iter )
		{
			CInformConfig::stInformConfig* pInfo = *iter ;

			stInform* pSendInfo = (stInform*)pTempBuffer ;
			pSendInfo->nTitleLen = strlen(pInfo->strTitle.c_str()) ;
			pSendInfo->nContentLen = strlen(pInfo->strTitle.c_str());
			pTempBuffer += sizeof(stInform);

			memcpy(pTempBuffer,pInfo->strTitle.c_str(),pSendInfo->nTitleLen);
			pTempBuffer += pSendInfo->nTitleLen ;

			memcpy(pTempBuffer,pInfo->strContent.c_str(),pSendInfo->nContentLen);
			pTempBuffer += pSendInfo->nContentLen ;
		}
	}
	pPlayer->SendMsgToClient(m_pInformBuffer,m_nBufferLen) ;
}

bool CBrocaster::OnMessage(RakNet::Packet* packet )
{
	// gm tool will use  in the future ;
	return false ;
}