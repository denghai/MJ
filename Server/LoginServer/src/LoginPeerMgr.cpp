#include "LoginPeerMgr.h"
#include "LoginPeer.h"
#include "LoginApp.h"
#include "MessageDefine.h"
#include "LogManager.h"
#include "ServerMessageDefine.h"
CLoginPeerMgr::CLoginPeerMgr(CLoginApp* app )
{
	ClearAll();
	m_pApp = app ;
}

CLoginPeerMgr::~CLoginPeerMgr()
{
	ClearAll();
}

void CLoginPeerMgr::OnMessage(Packet* pMsg )
{
	CHECK_MSG_SIZE_VOID(stMsg,pMsg->_len);
	
	stMsg* pRet = (stMsg*)pMsg->_orgdata;
	//if ( MSG_REQUEST_PEER_IP == pRet->usMsgType )
	//{
	//	stMsgRequestPeerIPRet* pReal = (stMsgRequestPeerIPRet*)pRet ;
	//	if ( pReal->nRet == 0 )
	//	{
	//		if ( pReal->nExArg == 1 ) // register save ip 
	//		{

	//		}
	//		else if ( 2 == pReal->nExArg )  // login save ip ;
	//		{

	//		}
	//		else
	//		{
	//			CLogMgr::SharedLogMgr()->ErrorLog("unknown situation save ip address %d",pReal->nExArg) ;
	//			return ;
	//		}
	//		// send msg to db save 
	//		stMsgServerSavePeerIPAddress msgSave ;
	//		msgSave.nExArg = pReal->nExArg ;
	//		msgSave.nSessionID = pReal->nSessionID ;
	//		msgSave.nUserUID = pReal->nUserUID ;
	//		memset(msgSave.pIPAddress,0,sizeof(msgSave.pIPAddress)) ;
	//		memcpy(msgSave.pIPAddress,pReal->ipAddress,sizeof(msgSave.pIPAddress));
	//		SendMsgToDB((char*)&msgSave,sizeof(msgSave)) ;
	//	}
	//	else
	//	{
	//		CLogMgr::SharedLogMgr()->ErrorLog("can not find peer ip , sessionid = %u,uid = %u",pReal->nSessionID,pReal->nUserUID ) ;
	//	}
	//	return ;
	//}

	if ( pRet->usMsgType == MSG_TRANSER_DATA )
	{
		CHECK_MSG_SIZE_VOID(stMsgTransferData,pMsg->_len);
		stMsgTransferData* pReal = (stMsgTransferData*)pRet ;
		char* pbuffer = (char*)pReal ;
		pbuffer += sizeof(stMsgTransferData) ;
		OnGateMessage((stMsg*)pbuffer,pReal->nSessionID ) ;
		return ;
	}

	if ( pRet->cSysIdentifer == ID_MSG_DB2LOGIN )
	{
		OnDBMessage(pRet) ;
		return ;
	}

	if ( MSG_DISCONNECT_CLIENT == pRet->usMsgType )
	{
		return ;
	}

	CLogMgr::SharedLogMgr()->PrintLog("unprocessed msg type = %d",pRet->usMsgType ) ;
	return ;
}

void CLoginPeerMgr::OnGateMessage(stMsg* pmsg ,unsigned int nSessionID )
{
	//switch ( pmsg->usMsgType )
	//{
	//case MSG_PLAYER_REGISTER:
	//	{
	//		stMsgRegister* pMsg = (stMsgRegister*)pmsg ;

	//		stMsgLoginRegister msgRegister ;
	//		int nLen = sizeof(msgRegister) + pMsg->nAccLen + pMsg->nPasswordLen;
	//		msgRegister.bAutoRegister = pMsg->bAutoQuickEnter ;
	//		msgRegister.cSex = pMsg->bSex ;
	//		msgRegister.pAcoundLen = pMsg->nAccLen ;
	//		msgRegister.ppPasswordLen = pMsg->nPasswordLen ;
	//		msgRegister.nSessionID = nSessionID ;
	//		msgRegister.ncharNameLen = pMsg->ncharNameLen ;
	//		msgRegister.nCoinPasswordLen = pMsg->nCoinPasswordLen ;
	//		msgRegister.nBirthDay = pMsg->nBirthDay ;
	//		
	//		if ( msgRegister.ncharNameLen > MAX_LEN_CHARACTER_NAME || msgRegister.ppPasswordLen > MAX_LEN_PASSWORD || msgRegister.pAcoundLen > MAX_LEN_ACCOUNT )
	//		{
	//			CLogMgr::SharedLogMgr()->ErrorLog("some buffer is too long nameLen = %d, acccountLen = %d, passwordLen = %d",msgRegister.ncharNameLen,pMsg->nPasswordLen,pMsg->nPasswordLen) ;
	//			return ;
	//		}
	//		int nOffset = 0 ;
	//		SAFE_MEMCPY(m_pMaxBuffer + nOffset , &msgRegister, sizeof(msgRegister) );
	//		nOffset += sizeof(msgRegister);

	//		char* pbuffer = (char*)pmsg ;
	//		pbuffer += sizeof(stMsgRegister);

	//		SAFE_MEMCPY(m_pMaxBuffer + nOffset , pbuffer, msgRegister.pAcoundLen) ;
	//		nOffset += msgRegister.pAcoundLen ;
	//		pbuffer += msgRegister.pAcoundLen ;

	//		SAFE_MEMCPY(m_pMaxBuffer + nOffset , pbuffer, msgRegister.ppPasswordLen) ;
	//		nOffset += msgRegister.ppPasswordLen ;
	//		pbuffer += msgRegister.ppPasswordLen ;

	//		SAFE_MEMCPY(m_pMaxBuffer + nOffset , pbuffer, msgRegister.ncharNameLen) ;
	//		nOffset += msgRegister.ncharNameLen ;
	//		pbuffer += msgRegister.ncharNameLen ;

	//		SAFE_MEMCPY(m_pMaxBuffer + nOffset , pbuffer, msgRegister.nCoinPasswordLen) ;
	//		nOffset += msgRegister.nCoinPasswordLen ;
	//		pbuffer += msgRegister.nCoinPasswordLen ;

	//		SendMsgToDB(m_pMaxBuffer, nOffset ) ;
	//	}
	//	break;
	//case MSG_PLAYER_CHECK_ACCOUNT:
	//	{
	//		stMsgCheckAccount* pMsgCheck = (stMsgCheckAccount*)pmsg ;
	//		stMsgLoginAccountCheck msg ;
	//		msg.nSessionID = nSessionID ;
	//		msg.nAccountLen = pMsgCheck->nAccountLen ;
	//		msg.nPasswordLen = pMsgCheck->nPasswordlen ;
	//		
	//		if ( msg.nPasswordLen > MAX_LEN_PASSWORD || msg.nAccountLen > MAX_LEN_ACCOUNT )
	//		{
	//			CLogMgr::SharedLogMgr()->ErrorLog("MSG_PLAYER_CHECK_ACCOUNT some buffer is too long acccountLen = %d, passwordLen = %d",msg.nAccountLen,msg.nPasswordLen) ;
	//			return ;
	//		}

	//		int nOffset = 0 ;
	//		SAFE_MEMCPY(m_pMaxBuffer + nOffset ,&msg, sizeof(msg) );
	//		nOffset += sizeof(msg);

	//		char* pbuffer = (char*)pmsg ;
	//		pbuffer += sizeof(stMsgCheckAccount);

	//		SAFE_MEMCPY(m_pMaxBuffer + nOffset ,pbuffer, pMsgCheck->nAccountLen );
	//		nOffset += pMsgCheck->nAccountLen ;
	//		pbuffer += pMsgCheck->nAccountLen ;

	//		SAFE_MEMCPY(m_pMaxBuffer + nOffset ,pbuffer, pMsgCheck->nPasswordlen );
	//		nOffset += pMsgCheck->nPasswordlen ;
	//		pbuffer += pMsgCheck->nPasswordlen ;
	//		SendMsgToDB(m_pMaxBuffer,nOffset) ;
	//	}
	//	break;
	//case MSG_PLAYER_BIND_ACCOUNT:
	//	{
	//		stMsgRebindAccount* pMsg = (stMsgRebindAccount*)pmsg ;
	//		stMsgLoginBindAccount msgSend ;
	//		msgSend.nSex = pMsg->nSex ;
	//		msgSend.nAccountLen = pMsg->nAccLen ;
	//		msgSend.nCurUserUID = pMsg->nCurUserUID ;
	//		msgSend.nPasswordLen = pMsg->nPasswordLen ;
	//		msgSend.nCoinPasswordLen = pMsg->nCoinPasswordLen;
	//		msgSend.nSessionID = nSessionID ;
	//		
	//		if ( msgSend.nCoinPasswordLen > MAX_LEN_COIN_PASSWORD || msgSend.nPasswordLen > MAX_LEN_PASSWORD || msgSend.nAccountLen > MAX_LEN_ACCOUNT )
	//		{
	//			CLogMgr::SharedLogMgr()->ErrorLog("MSG_PLAYER_BIND_ACCOUNT some buffer is too long nameLen = %d, acccountLen = %d, passwordLen = %d",msgSend.nCoinPasswordLen,msgSend.nPasswordLen,msgSend.nAccountLen) ;
	//			return ;
	//		}
	//		
	//		char* pBuffer = new char[sizeof(msgSend) +msgSend.nPasswordLen + msgSend.nAccountLen+msgSend.nCoinPasswordLen] ;
	//		SAFE_MEMCPY(pBuffer,&msgSend,sizeof(msgSend));
	//		SAFE_MEMCPY(pBuffer + sizeof(msgSend) , ((char*)pmsg) + sizeof(stMsgRebindAccount),msgSend.nPasswordLen + msgSend.nAccountLen + msgSend.nCoinPasswordLen );
	//		SendMsgToDB(pBuffer,sizeof(msgSend) +msgSend.nPasswordLen + msgSend.nAccountLen + msgSend.nCoinPasswordLen);
	//		delete[] pBuffer ;
	//	}
	//	break;
	//case MSG_PLAYER_MODIFY_PASSWORD:
	//	{
	//		stMsgModifyPassword* pMsgRet = (stMsgModifyPassword*)pmsg ;
	//		stMsgLoginModifyPassword msg ;
	//		msg.nSessionID = nSessionID ;
	//		msg.nUserUID = pMsgRet->nUserUID ;
	//		SAFE_MEMCPY(msg.pNewPassword,pMsgRet->pNewPassword,sizeof(msg.pNewPassword));
	//		SAFE_MEMCPY(msg.pOldPassword,pMsgRet->pOldPassword,sizeof(msg.pOldPassword));
	//		SendMsgToDB((char*)&msg,sizeof(msg)) ;
	//	} 
	//	break;
	//default:
	//	{
	//		CLogMgr::SharedLogMgr()->ErrorLog("Unknown message from gate type = %d",pmsg->usMsgType );
	//		return ;
	//	}	   

	//}
}

void CLoginPeerMgr::OnDBMessage(stMsg* pmsg )
{
	//switch ( pmsg->usMsgType )
	//{
	//case MSG_PLAYER_REGISTER:
	//	{
	//		stMsgLoginRegisterRet* pMsgRet = (stMsgLoginRegisterRet*)pmsg ;
	//		stMsgTransferData msgData ;
	//		msgData.bBroadCast = false ;
	//		msgData.cSysIdentifer = ID_MSG_LOGIN2C ;
	//		msgData.nSessionID = pMsgRet->nSessionID ;
	//		
	//		stMsgRegisterRet msgclientRet ;
	//		msgclientRet.bAutoReigster = pMsgRet->bAuto;
	//		msgclientRet.nRet = pMsgRet->nRet ;
	//		msgclientRet.nUserID = pMsgRet->nUserID ;
	//	
	//		int nOffset = 0 ;
	//		SAFE_MEMCPY(m_pMaxBuffer+ nOffset , &msgData, sizeof( msgData ) );
	//		nOffset += sizeof(msgData);
	//		
	//		SAFE_MEMCPY(m_pMaxBuffer + nOffset, &msgclientRet , sizeof(msgclientRet) );
	//		nOffset += sizeof(msgclientRet);
	//		
	//		SendMsgToGate(m_pMaxBuffer, nOffset );

	//		// if register success ,we should save register ip address 
	//		if ( msgclientRet.nRet == 0 )
	//		{
	//			stMsgRequestPeerIP msg ;
	//			msg.nSessionID = pMsgRet->nSessionID ;
	//			msg.nUserUID = pMsgRet->nUserID ;
	//			msg.nExArg = 1 ; // register  situation ;
	//			SendMsgToGate((char*)&msg,sizeof(msg)) ;
	//		}
	//	}
	//	break;
	//case MSG_PLAYER_CHECK_ACCOUNT:
	//	{
	//		stMsgLoginAccountCheckRet* pLoginCheckRet = (stMsgLoginAccountCheckRet*)pmsg ;
	//		
	//			
	//		stMsgCheckAccountRet msgcheckret ;
	//		msgcheckret.nRet = pLoginCheckRet->nRet ;
	//		msgcheckret.nUserID = pLoginCheckRet->nUserID ;
	//		
	//		stMsgTransferData msgData ;
	//		msgData.bBroadCast = false ;
	//		msgData.nSessionID = pLoginCheckRet->nSessionID ;
	//		msgData.cSysIdentifer = ID_MSG_LOGIN2C ;
	//		
	//		int nOffset = 0 ;
	//		SAFE_MEMCPY(m_pMaxBuffer, &msgData, sizeof(msgData));
	//		nOffset += sizeof(msgData);

	//		SAFE_MEMCPY(m_pMaxBuffer+ nOffset , &msgcheckret,sizeof(msgcheckret));
	//		nOffset += sizeof(msgcheckret) ;
	//		SendMsgToGate(m_pMaxBuffer, nOffset) ;
	//		CLogMgr::SharedLogMgr()->SystemLog("player check account nSessionId = %d, userUID = %d, Result = %d",msgData.nSessionID,msgcheckret.nUserID,msgcheckret.nRet);
	//		if ( msgcheckret.nRet == 0 )
	//		{
	//			stMsgRequestPeerIP msg ;
	//			msg.nSessionID = pLoginCheckRet->nSessionID ;
	//			msg.nUserUID = pLoginCheckRet->nUserID ;
	//			msg.nExArg = 2 ; // login situation ;
	//			SendMsgToGate((char*)&msg,sizeof(msg)) ;
	//		}
	//	}
	//	break;
	//case MSG_PLAYER_BIND_ACCOUNT:
	//	{
	//		stMsgLoginBindAccountRet* pRet = (stMsgLoginBindAccountRet*)pmsg;

	//		stMsgRebindAccountRet retMsg ;
	//		retMsg.nCurUserUID = pRet->nCurUserUID ;
	//		retMsg.nRet = pRet->nRet ;
	//		
	//		stMsgTransferData msgData ;
	//		msgData.bBroadCast = false ;
	//		msgData.nSessionID = pRet->nSessionID ;
	//		msgData.cSysIdentifer = ID_MSG_LOGIN2C ;

	//		int nOffset = 0 ;
	//		SAFE_MEMCPY(m_pMaxBuffer, &msgData, sizeof(msgData));
	//		nOffset += sizeof(msgData);

	//		SAFE_MEMCPY(m_pMaxBuffer+ nOffset , &retMsg,sizeof(retMsg));
	//		nOffset += sizeof(retMsg) ;
	//		SendMsgToGate(m_pMaxBuffer, nOffset) ;
	//	}
	//	break;
	//case MSG_PLAYER_MODIFY_PASSWORD:
	//	{
	//		stMsgLoginModifyPasswordRet* pRet = (stMsgLoginModifyPasswordRet*)pmsg ;
	//		stMsgModifyPasswordRet msg ;
	//		msg.nRet = pRet->nRet ;

	//		stMsgTransferData msgData ;
	//		msgData.bBroadCast = false ;
	//		msgData.nSessionID = pRet->nSessionID ;
	//		msgData.cSysIdentifer = ID_MSG_LOGIN2C ;

	//		int nOffset = 0 ;
	//		SAFE_MEMCPY(m_pMaxBuffer, &msgData, sizeof(msgData));
	//		nOffset += sizeof(msgData);

	//		SAFE_MEMCPY(m_pMaxBuffer+ nOffset , &msg,sizeof(msg));
	//		nOffset += sizeof(msg) ;
	//		SendMsgToGate(m_pMaxBuffer, nOffset) ;
	//	}
	//	break;
	//default:
	//	{
	//		CLogMgr::SharedLogMgr()->ErrorLog("Unknown message from DB type = %d",pmsg->usMsgType );
	//		return ;
	//	}	   

	//}
}

CLoginPeer* CLoginPeerMgr::GetPeerBySessionID(unsigned int nSessionID )
{
	MAP_LOGIN_PEER::iterator iter = m_vAllPeers.find(nSessionID) ;
	if ( iter != m_vAllPeers.end())
	{
		return iter->second ;
	}
	return NULL ;
}

CLoginPeer* CLoginPeerMgr::GetReserverPeer()
{
	if ( m_vReseverLoginPeers.empty() )
	{
		return NULL ;
	}
	LIST_LOGIN_PEER::iterator iter = m_vReseverLoginPeers.begin() ;
	CLoginPeer* pPeer = *iter ;
	m_vReseverLoginPeers.erase(iter) ;
	return pPeer ;
}

bool CLoginPeerMgr::SendMsgToDB(const char* pBuffer , int nLen )
{
	// net work object 
	m_pApp->SendMsg(pBuffer,nLen,false);
	return true ;
}

bool CLoginPeerMgr::SendMsgToGate(const char* pBuffer , int nLen )
{
	// net work object
	m_pApp->SendMsg(pBuffer,nLen,true);
	return true ;
}

void CLoginPeerMgr::ClearAll()
{
	MAP_LOGIN_PEER::iterator iter = m_vAllPeers.begin() ;
	for ( ; iter != m_vAllPeers.end(); ++iter )
	{
		if ( iter->second )
		{
			delete iter->second ;
			iter->second = NULL ;
		}

	}
	m_vAllPeers.clear() ;

	LIST_LOGIN_PEER::iterator iter_list = m_vReseverLoginPeers.begin() ;
	for ( ; iter_list != m_vReseverLoginPeers.end(); ++iter_list )
	{
		if ( *iter_list )
		{
			delete *iter_list ;
		}
	}
	m_vReseverLoginPeers.clear() ;
}