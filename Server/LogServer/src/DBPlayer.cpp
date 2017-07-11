//#include "DBPlayer.h"
//#include <assert.h>
//#include "DBRequest.h"
//#include "DBRequestFlags.h"
//#include "LogManager.h"
//#include "ServerNetwork.h"
//CDBPlayer::CDBPlayer(RakNet::RakNetGUID& nFromGameServerGUID )
//	:m_eState(ePlayerState_None),m_nFromGUID(nFromGameServerGUID),m_nUserUID(0)
//{
//	
//}
//
//CDBPlayer::~CDBPlayer()
//{
//
//}
//
//void CDBPlayer::SetFromServerGUID(RakNet::RakNetGUID& nFromGameServerGUID)
//{
//	m_nFromGUID = nFromGameServerGUID ;
//}
//
//RakNet::RakNetGUID& CDBPlayer::GetFromGameServerGUID()
//{
//	return m_nFromGUID;
//}
//
//void CDBPlayer::OnDBResult(stDBResult* pResult )
//{
//	switch ( pResult->nRequestFlag )
//	{
//	case eDBRequest_BaseData:
//		{
//			if ( pResult->nAffectRow > 0 )
//			{
//				m_stBaseData.m_bInitData = false ;
//				CMysqlRow* pRow = pResult->vResultRows[0];
//				m_stBaseData.nCoin = pRow->GetFiledByName("Coin")->Value.llValue ;
//				m_stBaseData.nDiamoned = pRow->GetFiledByName("Diamond")->Value.iValue;
//				m_nUserUID = pRow->GetFiledByName("UserUID")->Value.llValue ;
//			}
//			m_eState = ePlayerState_Active;
//			SendBaseInfo();
//		}
//		break;
//	default:
//		{
//			CLogMgr::SharedLogMgr()->PrintLog("Unprocessed DBResult flag = %d", pResult->nRequestFlag) ;
//		}
//		break;
//	}
//}
//
//void CDBPlayer::OnMessage(stMsg* pMsg )
//{
//	switch ( pMsg->usMsgType )
//	{
//	default:
//		break; ;
//	}
//}
//
//void CDBPlayer::OnDisconnected()
//{
//	m_eState = ePlayerState_Resever ;
//	// save to db all data ;
//	SaveAllToDB();
//	// start Timer to delete self ;
//}
//
//void CDBPlayer::OnConnected()
//{
//
//}
//
//void CDBPlayer::OnPassAcountCheck(unsigned int nUserUID, unsigned int nTempUID, const char* pname )
//{
//	m_nUserUID = nUserUID ;
//	m_nTempUID = nTempUID ;
//	sprintf(m_stBaseData.strName,"%s",pname) ;
//	if ( m_eState == ePlayerState_Resever )
//	{
//		m_eState = ePlayerState_Active ;
//		SendBaseInfo();
//	}
//	else
//	{
//		ReadAllFromDB();
//	}
//}
//
//unsigned int CDBPlayer::GetUserUID()
//{
//	return m_nUserUID ;
//}
//
//void CDBPlayer::ReadAllFromDB()
//{
//	m_eState = ePlayerState_ReadingData;
//	// read base data ;
//	stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
//	pRequest->eType = eRequestType_Select ;
//	pRequest->nRequestFlag = eDBRequest_BaseData ;
//	pRequest->nRequestUID = GetTempUID() ;
//	pRequest->nSqlBufferLen = sprintf(pRequest->pSqlBuffer,"SELECT * FROM basedata WHERE UserUID = '%d'",GetUserUID());
//	CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
//}
//
//void CDBPlayer::SaveAllToDB()
//{
//	stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
//	pRequest->nRequestUID = m_nTempUID ;
//	if ( m_stBaseData.m_bInitData )
//	{
//		pRequest->eType = eRequestType_Add ;
//		pRequest->nRequestFlag = eDBRequest_Add_BaseData ;
//		pRequest->nSqlBufferLen = sprintf(pRequest->pSqlBuffer,"INSERT INTO gamedb.basedata (`UserUID`, `Coin`, `Diamond`) VALUES ('%d', '%I64d', '%d');",m_nUserUID,m_stBaseData.nCoin,m_stBaseData.nDiamoned);
//	}
//	else
//	{
//		pRequest->eType = eRequestType_Update ;
//		pRequest->nRequestFlag = eDBRequest_Update_BaseData ;
//		pRequest->nSqlBufferLen = sprintf(pRequest->pSqlBuffer,"UPDATE gamedb.basedata SET Coin = '%I64d', Diamond = '%d' WHERE UserUID = '%d'",m_stBaseData.nCoin,m_stBaseData.nDiamoned,m_nUserUID);
//	}
//	CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
//}
//
//void CDBPlayer::SendBaseInfo()
//{
//	assert(m_eState == ePlayerState_Active) ;
//	// send message to gameserver ;
//	stMsgPushBaseDataToGameServer msg ;
//	msg.nTargetUserUID = GetTempUID();
//	memcpy(&msg.stData,&m_stBaseData,sizeof(stBaseData));
//	CServerNetwork::SharedNetwork()->SendMsg((char*)&msg,sizeof(msg),m_nFromGUID,false) ;	
//}
