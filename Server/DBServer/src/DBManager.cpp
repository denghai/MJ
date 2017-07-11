#pragma warning(disable:4800)
#include "DBManager.h"
#include "DBRequest.h"
#include "ServerMessageDefine.h"
#include "DBApp.h"
#include "DataBaseThread.h"
#include "AutoBuffer.h"
#include "json/json.h"
#include <algorithm>
#include "log4z.h"
#define PLAYER_BRIF_DATA "playerName,userUID,sex,vipLevel,photoID,coin,diamond"
#define PLAYER_BRIF_DATA_DETAIL_EXT ",signature,singleWinMost,mostCoinEver,vUploadedPic,winTimes,loseTimes,longitude,latitude,offlineTime,maxCard,vJoinedClubID"
CDBManager::CDBManager(CDBServerApp* theApp )
{
	m_vReserverArgData.clear();
	m_pTheApp = theApp ;
	nCurUserUID = 0 ;  // temp asign 
}

CDBManager::~CDBManager()
{
	LIST_ARG_DATA::iterator iter = m_vReserverArgData.begin() ;
	for ( ; iter != m_vReserverArgData.end() ; ++iter )
	{
		if ( *iter )
		{
			delete *iter ;
			*iter = NULL ;
		}
	}
	m_vReserverArgData.clear() ;
}

void CDBManager::Init()
{
	// register funcs here ;
	//stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
	//pRequest->cOrder = eReq_Order_High ;
	//pRequest->eType = eRequestType_Select ;
	//pRequest->nRequestUID = -1;
	//pRequest->pUserData = NULL;
	//pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"(select max(Account.UserUID) FROM Account)") ;
	//CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
}

static const uint32_t nAsyncReq = -1 ;
bool CDBManager::onAsyncRequest(uint32_t nReqType ,uint32_t nSerialNum, uint8_t nSrcPort,Json::Value jsReqContent )
{
	if ( jsReqContent["sql"].isNull() )
	{
		LOGFMTE("sql is null reqType = %u, from srcPort = %u, serialNum = %u",nReqType,nSrcPort,nSerialNum) ;
		return false ;
	}
#ifdef _DEBUG
	std::string strSql = jsReqContent["sql"].asString() ;
	std::transform(strSql.begin(), strSql.end(),strSql.begin(), ::tolower);
	eDBRequestType nDBTypeGuess = eRequestType_Max  ;
	// check selectType 
	if ( strSql.find("select") < 10 || strSql.find("call") < 10 )
	{
		nDBTypeGuess = eRequestType_Select ;
	}
	else if ( strSql.find("update") < 10 ) 
	{
		nDBTypeGuess = eRequestType_Update ;
	}
	else if ( strSql.find("delete") < 10 ) 
	{
		nDBTypeGuess = eRequestType_Delete ;
	}
	else if ( strSql.find("insert") < 10 )
	{
		nDBTypeGuess = eRequestType_Add ;
	}
	else
	{
		LOGFMTE("can not tell from sql , what req type : sql = %s",strSql.c_str()) ;
	}
#endif

	eDBRequestType nDBType = eRequestType_Select ;
	switch ( nReqType )
	{
	case eAsync_DB_Select:
		{
			nDBType = eRequestType_Select ; 
		}
		break;
	case eAsync_DB_Update:
		{
			nDBType = eRequestType_Update;
		}
		break;
	case eAsync_DB_Add:
		{
			nDBType = eRequestType_Add ;
		}
		break ;
	case eAsync_Db_Delete:
		{
			nDBType = eRequestType_Delete ;
		}
		break;
	default:
		LOGFMTD("unknown reqType = %u, from srcPort = %u, serialNum = %u",nReqType,nSrcPort,nSerialNum) ;
		return false ;
	}

#ifdef _DEBUG
	if ( nDBType != nDBTypeGuess && nDBTypeGuess != eRequestType_Max )
	{
		LOGFMTE("intent type = %u not the same with guessType = %u sql = %s",nDBType,nDBTypeGuess,strSql.c_str()) ;
	}
#endif

	stArgData* pdata = GetReserverArgData() ;
	if ( pdata == NULL )
	{
		pdata = new stArgData ;
	}

	pdata->eFromPort = (eMsgPort)nSrcPort ;
	pdata->nSessionID = nSerialNum ;

	stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
	pRequest->cOrder = eReq_Order_Normal ;
	if ( jsReqContent["order"].isNull() == false )
	{
		pRequest->cOrder = jsReqContent["order"].asUInt() ;
	}
	pRequest->nRequestUID = nAsyncReq ;
	pRequest->pUserData = pdata;
	pRequest->eType = nDBType ;
	pRequest->nSqlBufferLen = 0 ;
	pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),"%s",jsReqContent["sql"].asCString());
	LOGFMTD("receive async db reqType = %d , srcPort = %u , serial number = %u" ,nReqType,nSrcPort,nSerialNum);

	CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
	return true ;
}

void CDBManager::OnMessage(stMsg* pmsg , eMsgPort eSenderPort , uint32_t nSessionID )
{
	// construct sql
	stArgData* pdata = GetReserverArgData() ;
	if ( pdata == NULL )
	{
		pdata = new stArgData ;
	}

	pdata->eFromPort = eSenderPort ;
	pdata->nSessionID = nSessionID ;

	stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
	pRequest->cOrder = eReq_Order_Normal ;
	pRequest->nRequestUID = pmsg->usMsgType ;
	pRequest->pUserData = pdata;
	pRequest->eType = eRequestType_Max ;
	pRequest->nSqlBufferLen = 0 ;
	LOGFMTD("recive db req = %d",pmsg->usMsgType);
	switch( pmsg->usMsgType )
	{
	case MSG_SAVE_PRIVATE_ROOM_PLAYER:
		{
			stMsgSavePrivateRoomPlayer* pRet = (stMsgSavePrivateRoomPlayer*)pmsg ;
			char* pBuffer = (char*)pRet ;
			pBuffer += sizeof(stMsgSavePrivateRoomPlayer);

			CAutoBuffer auBuffer (pRet->nJsonLen + 1 );
			auBuffer.addContent(pBuffer,pRet->nJsonLen) ;

			pRequest->eType = eRequestType_Add ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
				"INSERT INTO privateroomplayer (serialNum,userUID,content) VALUES ('%u','%u','%s') ON DUPLICATE KEY UPDATE content = '%s';",pRet->nRoomSerialNum,pRet->nUserUID,auBuffer.getBufferPtr(),auBuffer.getBufferPtr()) ;
		}
		break ;
	case MSG_READ_PRIVATE_ROOM_PLAYER:
		{
			stMsgReadPrivateRoomPlayer* pRet = (stMsgReadPrivateRoomPlayer*)pmsg ;
			pdata->nExtenArg1 = pRet->nRoomID ;
			pRequest->eType = eRequestType_Select ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
				"SELECT * FROM privateroomplayer WHERE serialNum = '%u' limit 80", pRet->nRoomSerialNum);
		}
		break ;
	case MSG_SAVE_ENCRYPT_NUMBER:
		{
			stMsgSaveEncryptNumber* pRet = (stMsgSaveEncryptNumber*)pmsg ;
			pRequest->eType = eRequestType_Select ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
				"call saveEncryptNumber( '%llu','%u','%u','%u','%u','%u')",pRet->nEncryptNumber,pRet->nCoin,pRet->nRMB,pRet->nNumberType,pRet->nCoinType,pRet->nChannelID) ;
		}
		break ;
	case MSG_VERIFY_ENCRYPT_NUMBER:
		{
			stMsgVerifyEncryptNumber* pRet = (stMsgVerifyEncryptNumber*)pmsg ;
			pdata->nExtenArg1 = pRet->nUserUID ;
			pRequest->eType = eRequestType_Select ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
				"call playerUseEncryptNumber( '%llu','%u')",pRet->nNumber,pRet->nUserUID) ;
		}
		break ;
	case MSG_SAVE_GAME_RESULT:
		{
			stMsgSaveGameResult* pRet = (stMsgSaveGameResult*)pmsg ;
			pRequest->eType = eRequestType_Add;
			CAutoBuffer auBuffer(pRet->nJsLen + 1 );
			auBuffer.addContent((char*)pmsg + sizeof(stMsgSaveGameResult),pRet->nJsLen) ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"INSERT INTO gameresult (sieralNum,roomID, roomType,createUID,configID,time,duiringSeconds,playerDetail) VALUES (%u,'%u', '%u','%u','%u','%u','%u','%s')",
				pRet->nSieralNum, pRet->nRoomID, pRet->nRoomType, pRet->nCreaterUID, pRet->nConfigID, pRet->tTime, pRet->nDuringSeconds, auBuffer.getBufferPtr());
		}
		break;
	case MSG_SAVE_PLAYER_GAME_RECORDER:
		{
			stMsgSavePlayerGameRecorder* pRet = (stMsgSavePlayerGameRecorder*)pmsg ;
			pRequest->eType = eRequestType_Add;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"INSERT INTO playergamerecorder (sieralNum,userUID,roomID, roomType,createUID,finishTime,duiringSeconds,offset,buyIn,baseBet,roomName,clubID ) VALUES ('%u', %u,'%u','%u','%u','%u','%u','%d','%u','%u','%s',%u )",
				pRet->nSieralNum,pRet->nUserUID,pRet->nRoomID,pRet->nRoomType,pRet->nCreateUID,pRet->nFinishTime,pRet->nDuiringSeconds,pRet->nOffset,pRet->nBuyIn,pRet->nBaseBet,pRet->cRoomName,pRet->nClubID) ;
		}
		break ;
	case MSG_READ_GAME_RESULT:
		{
			stMsgReadGameResult* pRet = (stMsgReadGameResult*)pmsg ;
			pRequest->eType = eRequestType_Select ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
				"SELECT * FROM gameresult WHERE roomType = %u order by time desc limit 80",pRet->nRoomType ) ;
		}
		break ;
	case MSG_READ_PLAYER_GAME_RECORDER:
		{
			stMsgReadPlayerGameRecorder* pRet = (stMsgReadPlayerGameRecorder*)pmsg ;
			pRequest->eType = eRequestType_Select ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
				"SELECT * FROM playergamerecorder WHERE userUID = '%u'order by finishTime desc limit 15",pRet->nUserUID ) ;
		}
		break ;
	case MSG_SAVE_NOTICE_PLAYER:
		{
			stMsgSaveNoticePlayer* pRet = (stMsgSaveNoticePlayer*)pmsg ;
			if ( pRet->nOpt == 0 )  // add or update
			{
				char pTokenStr[65] = {0} ;
				m_pTheApp->GetDBThread()->EscapeString(pTokenStr,pRet->tPlayer.pToken,sizeof(pRet->tPlayer.pToken)) ;
				pRequest->eType = eRequestType_Add ;
				pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
					"INSERT INTO palyerapnstoken (userUID, flag,token) VALUES ('%u', '%u','%s') ON DUPLICATE KEY UPDATE flag = '%u',token = '%s';",pRet->tPlayer.nUserUID,pRet->tPlayer.nNoticeFlag,pTokenStr,pRet->tPlayer.nNoticeFlag,pTokenStr) ;
			}
			else  // remove flag ;
			{
				pRequest->eType = eRequestType_Update ;
				pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
					"UPDATE palyerapnstoken SET flag = '0' WHERE userUID = '%u'",pRet->tPlayer.nUserUID) ;
			}

		}
		break;
	case MSG_SAVE_EXCHANGE:
		{
			stMsgSaveExchanges* pRet = (stMsgSaveExchanges*)pmsg ;
			pRequest->eType = eRequestType_Add ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
				"INSERT INTO exchangecenter (exchangeID, count) VALUES ('%u', '%u') ON DUPLICATE KEY UPDATE count = '%u' ",pRet->nExchangeID,pRet->nCount,pRet->nCount ) ;
		}
		break;
	case MSG_READ_EXCHANGE:
		{
			pRequest->eType = eRequestType_Select ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
				"SELECT * FROM exchangecenter WHERE exchangeID != '0' " ) ;
		}
		break;
	case MSG_READ_NOTICE_PLAYER:
		{
			stMsgReadNoticePlayer* pRet = (stMsgReadNoticePlayer*)pmsg ;
			pdata->nExtenArg1 = pRet->nUserUID ;
			pRequest->eType = eRequestType_Select ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
				"SELECT * FROM palyerapnstoken WHERE userUID = '%u' ",pRet->nUserUID) ;
		}
		break;
	case MSG_CIRCLE_SAVE_ADD_TOPIC:
		{
			stMsgSaveAddCircleTopic* pRet = (stMsgSaveAddCircleTopic*)pmsg ;
			pRequest->eType = eRequestType_Add;
			CAutoBuffer auBuffer(pRet->item.nContentLen + 1 );
			auBuffer.addContent((char*)pmsg + sizeof(stMsgSaveAddCircleTopic),pRet->item.nContentLen) ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"INSERT INTO circletopic (topicID, authorUID,publishTime,content) VALUES ('%I64d', '%u','%u','%s')",
				pRet->item.nTopicID,pRet->item.nAuthorUID,pRet->item.nPublishTime,auBuffer.getBufferPtr()) ;
			pdata->nExtenArg1 = pRet->item.nTopicID;
		}
		break;
	case MSG_CIRCLE_SAVE_DELETE_TOPIC:
		{
			stMsgSaveDeleteCircleTopic* pRet = (stMsgSaveDeleteCircleTopic*)pmsg ;
			pRequest->eType = eRequestType_Update ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
				"UPDATE circletopic SET isDelete = '1' WHERE topicID = '%d' and isDelete = '0' ",pRet->nTopicID) ;
		}
		break;
	case MSG_CIRCLE_READ_TOPICS:
		{
			pRequest->eType = eRequestType_Select ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
				"SELECT * FROM circletopic WHERE isDelete = '0' order by publishTime desc limit 250") ;
		}
		break;
	case MSG_SELECT_DB_PLAYER_DATA:
		{
			stMsgSelectPlayerData* pRet = (stMsgSelectPlayerData*)pmsg ;
			pdata->nExtenArg1 = pRet->nTargetPlayerUID ;
			pdata->nExtenArg2 = pRet->isDetail ;
			pRequest->eType = eRequestType_Select ;
			if ( pRet->isDetail )
			{
				pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
					"call selectPlayerDetailClient( '%u')",pRet->nTargetPlayerUID) ;
			}
			else
			{
				pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
					"SELECT * FROM playerbasedata WHERE userUID = '%d'",pRet->nTargetPlayerUID) ;
			}
		}
		break;
	case MSG_PLAYER_SAVE_MAIL:
		{
			stMsgSaveMail* pRet = (stMsgSaveMail*)pmsg ;
			pRequest->eType = eRequestType_Add;
			CAutoBuffer auBuffer(pRet->pMailToSave.nContentLen + 1 );
			auBuffer.addContent((char*)pmsg + sizeof(stMsgSaveMail),pRet->pMailToSave.nContentLen) ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"INSERT INTO mail (userUID, postTime,mailType,mailContent) VALUES ('%u', '%u','%u','%s')",
				pRet->nUserUID,pRet->pMailToSave.nPostTime,pRet->pMailToSave.eType,auBuffer.getBufferPtr()) ;
			LOGFMTD("save  SAVE_MAIL uid = %d",pRet->nUserUID);
			pdata->nExtenArg1 = pRet->nUserUID ;
		}
		break;
	case MSG_PLAYER_READ_MAIL_LIST:
		{
			stMsgReadMailList* pRet = (stMsgReadMailList*)pmsg ;
			pdata->nExtenArg1 = pRet->nUserUID ;
			pRequest->eType = eRequestType_Select ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer, sizeof(pRequest->pSqlBuffer),
				"SELECT * FROM mail WHERE userUID = '%u' and state = '0' order by postTime desc limit 50 ;", pRet->nUserUID);
		}
		break;
	case MSG_PLAYER_SET_MAIL_STATE:
		{
			stMsgResetMailsState* pRet = (stMsgResetMailsState*)pmsg ;
			pRequest->eType = eRequestType_Update ;

			if ( pRet->eType == eMail_Sys_End )
			{
				pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
					"UPDATE mail SET state = '1' WHERE userUID = '%u' and mailType < '%d' and postTime <= %u and state = '0' ",pRet->nUserUID,eMail_Sys_End,pRet->nTimeLatest) ;
				LOGFMTD("reset mail state for uid = %d offline sys ",pRet->nUserUID);
			}
			else if ( pRet->eType == eMail_RealMail_Begin )
			{
				pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
					"UPDATE mail SET state = '1' WHERE userUID = '%u' and mailType > '%d' and state = '0' ",pRet->nUserUID,eMail_RealMail_Begin) ;
				LOGFMTD("reset mail state for uid = %d normal mail ",pRet->nUserUID);
			}
			else
			{
				pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
					"UPDATE mail SET state = '1' WHERE userUID = '%u' and mailType < '%d' and state = '0' ",pRet->nUserUID,eMail_Sys_End) ;
				LOGFMTE( "unknown mail type = %u , invlid type ,uid = %u",pRet->eType,pRet->nUserUID );
			}
		}
		break ;
	case MSG_SAVE_FRIEND_LIST:
		{
			stMsgSaveFirendList* pRet = (stMsgSaveFirendList*)pmsg ;
			pRequest->eType = eRequestType_Update ;
			CAutoBuffer str (pRet->nFriendCountLen + 1 );
			str.addContent((char*)pmsg + sizeof(stMsgSaveFirendList),pRet->nFriendCountLen) ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
				"UPDATE playerfriend SET friendUIDs = '%s' WHERE userUID = '%d'",str.getBufferPtr(),pRet->nUserUID) ;
			LOGFMTD("save player FRIEND_LIST uid = %d",pRet->nUserUID);
		}
		break;
	case MSG_READ_FRIEND_LIST:
		{
			stMsgReadFriendList* pRet = (stMsgReadFriendList*)pmsg ;
			pdata->nExtenArg1 = pRet->nUserUID ;
			pRequest->eType = eRequestType_Select ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
				"SELECT * FROM playerfriend WHERE userUID = '%d'",pRet->nUserUID) ;
		}
		break;
	//case MSG_REQUEST_CREATE_PLAYER_DATA:
	//	{
	//		stMsgRequestDBCreatePlayerData* pCreate = (stMsgRequestDBCreatePlayerData*)pmsg ;
	//		pdata->nExtenArg1 = pCreate->nUserUID ;

	//		uint16_t nRandID = rand() % 10000 ;
	//		pRequest->eType = eRequestType_Select ;
	//		pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,
	//			"call CreateNewRegisterPlayerDataNew(%d,'guest%d','%d')",pCreate->nUserUID,nRandID,pCreate->isRegister) ;
	//	}
	//	break;
	case MSG_DB_CHECK_INVITER:
		{
			stMsgDBCheckInvite* pRet = (stMsgDBCheckInvite*)pmsg ;
			pdata->nExtenArg1 = pRet->nInviteUserUID;
			pRequest->eType = eRequestType_Select ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
				"SELECT playerName FROM playerbasedata WHERE userUID = '%d'",pRet->nInviteUserUID) ;
		}
		break;
	case MSG_READ_PLAYER_BASE_DATA:
		{
			stMsgDataServerGetBaseData* pRet = (stMsgDataServerGetBaseData*)pmsg ;
			pdata->nExtenArg1 = pRet->nUserUID ;
			pRequest->eType = eRequestType_Select ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
				"SELECT * FROM playerbasedata WHERE userUID = '%d'",pRet->nUserUID) ;
		}
		break;
	case MSG_READ_PLAYER_TAXAS_DATA:
		{
			stMsgReadPlayerTaxasData* pRet = (stMsgReadPlayerTaxasData*)pmsg ;
			pdata->nExtenArg1 = pRet->nUserUID ;
			pRequest->eType = eRequestType_Select ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
				"SELECT * FROM playertaxasdata WHERE userUID = '%d'",pRet->nUserUID) ;
		}
		break;
	case MSG_READ_PLAYER_GAME_DATA:
		{
			stMsgReadPlayerGameData* pRet = (stMsgReadPlayerGameData*)pmsg ;
			pdata->nExtenArg1 = pRet->nUserUID ;
			pRequest->eType = eRequestType_Select ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
				"SELECT * FROM playergamedata WHERE userUID = '%u'",pRet->nUserUID) ;
		}
		break;
	case MSG_PLAYER_SAVE_PLAYER_INFO:
		{
			stMsgSavePlayerInfo* pRet = (stMsgSavePlayerInfo*)pmsg ;
			pRequest->eType = eRequestType_Update ;
			std::string strUploadPic = stMysqlField::UnIntArraryToString(pRet->vUploadedPic,MAX_UPLOAD_PIC) ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
				"UPDATE playerbasedata SET playerName = '%s', signature = '%s',vUploadedPic = '%s',photoID = '%d',isRegister = '%d',sex = '%d',inviterUID = '%u' WHERE userUID = '%d'",pRet->vName,pRet->vSigure,strUploadPic.c_str(),pRet->nIsRegister,pRet->nPhotoID,pRet->nSex,pRet->nInviterUID,pRet->nUserUID) ;
		}
		break;
	case MSG_SAVE_PLAYER_MONEY:
		{
			stMsgSavePlayerMoney* pRet = (stMsgSavePlayerMoney*)pmsg ;
			pRequest->eType = eRequestType_Update ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
				"UPDATE playerbasedata SET coin = '%I64d', diamond = '%d',nCupCnt = '%d' WHERE userUID = '%d'",pRet->nCoin,pRet->nDiamoned,pRet->nCupCnt,pRet->nUserUID) ;
			LOGFMTD("save player coin = %I64d uid = %d",pRet->nCoin,pRet->nUserUID);
		}
		break;
	case MSG_SAVE_PLAYER_GAME_DATA:
		{
			stMsgSavePlayerGameData* pRet = (stMsgSavePlayerGameData*)pmsg ;
			CAutoBuffer auBufer(pRet->nJsonLen + 1 );
			auBufer.addContent(((char*)pRet) + sizeof(stMsgSavePlayerGameData) , pRet->nJsonLen ) ;

			pRequest->eType = eRequestType_Add;
			switch ( pRet->nGameType )
			{
			case eRoom_NiuNiu:
				{
					pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"INSERT INTO playergamedata ( userUID,taxas,golden,niuniu ) VALUES ( '%u' ,'','','%s') ON DUPLICATE KEY UPDATE niuniu = '%s'",pRet->nUserUID,auBufer.getBufferPtr(),auBufer.getBufferPtr()) ;
				}
				break;
			case eRoom_TexasPoker:
				{
					pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"INSERT INTO playergamedata ( userUID,niuniu,golden ,taxas) VALUES ( '%u' ,'','','%s') ON DUPLICATE KEY UPDATE taxas = '%s'",pRet->nUserUID,auBufer.getBufferPtr(),auBufer.getBufferPtr()) ;
				}
				break;
			case eRoom_Golden:
				{
					pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"INSERT INTO playergamedata ( userUID,taxas,niuniu,golden ) VALUES ( '%u' ,'','','%s') ON DUPLICATE KEY UPDATE golden = '%s'",pRet->nUserUID,auBufer.getBufferPtr(),auBufer.getBufferPtr()) ;
				}
				break ;
			default:
				{
					pRequest->nSqlBufferLen = 0 ;
				}
				break;
			}
		}
		break;
	case MSG_SAVE_COMMON_LOGIC_DATA:
		{
			stMsgSavePlayerCommonLoginData* pRet = (stMsgSavePlayerCommonLoginData*)pmsg ;
			pRequest->eType = eRequestType_Update ;
			std::string strJoinedClub = stMysqlField::UnIntArraryToString(pRet->vJoinedClubID,MAX_JOINED_CLUB_CNT) ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
				"UPDATE playerbasedata SET mostCoinEver = '%I64d', vipLevel = '%d', nYesterdayCoinOffset = '%I64d', \
				nTodayCoinOffset = '%I64d',offlineTime = '%d',continueLoginDays = '%d',lastLoginTime = '%d',lastTakeCharityCoinTime = '%d', \
				longitude = '%f',latitude = '%f',vJoinedClubID = '%s',newPlayerHaloWeight = '%d',nVipCardType = '%d',vipCardEndTime = '%d',lastTakeCardGiftTime = '%u',totalInvitePrizeCoin = '%u',takeCharityTimes = '%u',totalGameCoinOffset = %d WHERE userUID = '%d' ",
				pRet->nMostCoinEver,pRet->nVipLevel,pRet->nYesterdayCoinOffset,pRet->nTodayCoinOffset,pRet->tOfflineTime,pRet->nContinueDays,pRet->tLastLoginTime,pRet->tLastTakeCharityCoinTime,pRet->dfLongitude,pRet->dfLatidue,
				strJoinedClub.c_str(),pRet->nNewPlayerHaloWeight,pRet->nCardType,pRet->nCardEndTime,pRet->nLastTakeCardGiftTime,pRet->nTotalInvitePrizeCoin,pRet->nTakeCharityTimes,pRet->nTotalGameCoinOffset,pRet->nUserUID) ;
		}
		break;

	case MSG_GET_MAX_ROOM_ID:
		{
			pRequest->eType = eRequestType_Select ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"(select max(gameroomnew.roomID) FROM gameroomnew)") ;
		}
		break;
	case MSG_SAVE_UPDATE_ROOM_INFO:
		{
			stMsgSaveUpdateRoomInfo* pRet = (stMsgSaveUpdateRoomInfo*)pmsg ;
			CAutoBuffer aBuffer(pRet->nJsonLen+1);
			aBuffer.addContent(((char*)pRet) + sizeof(stMsgSaveUpdateRoomInfo),pRet->nJsonLen) ;

			pRequest->eType = eRequestType_Add ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"INSERT INTO gameroomnew ( roomType,roomID,ownerUID,configID,jsonDetail) VALUES ( '%d' ,'%d','%u','%u','%s') ON DUPLICATE KEY UPDATE jsonDetail = '%s'",
			pRet->nRoomType,pRet->nRoomID,pRet->nRoomOwnerUID,pRet->nConfigID,aBuffer.getBufferPtr(),aBuffer.getBufferPtr()) ;

			LOGFMTD("save room update info room id = %d",pRet->nRoomID);
		}
		break;
	case MSG_READ_ROOM_INFO:
		{
			stMsgReadRoomInfo* pRet = (stMsgReadRoomInfo*)pmsg ;
			pRequest->eType = eRequestType_Select;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
				"SELECT * FROM gameroomnew WHERE roomType = '%d' and isDelete = '0' ",pRet->nRoomType) ;
			LOGFMTD("read all room rooms");
		}
		break;
	case MSG_DELETE_ROOM:
		{
			stMsgSaveDeleteRoom* pRet = (stMsgSaveDeleteRoom*)pmsg ;
			pRequest->eType = eRequestType_Update;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
				"UPDATE gameroomnew SET isDelete = '1' WHERE roomID = '%d' and roomType = '%d' ",pRet->nRoomID,pRet->nRoomType) ;
			LOGFMTD("delete room id = %d",pRet->nRoomID);
			pdata->nExtenArg1 = pRet->nRoomID;
		}
		break;
	case MSG_SAVE_TAXAS_ROOM_PLAYER:
		{
			stMsgSaveTaxasRoomPlayer* pRet = (stMsgSaveTaxasRoomPlayer*)pmsg ;
			int64_t nOffset = pRet->nFinalLeftInThisRoom - pRet->nTotalBuyInThisRoom ;
			if ( pRet->isUpdate )
			{
				pRequest->eType = eRequestType_Update;
				pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
					"UPDATE taxasroomplayers SET readInformSerial = '%u', totalBuyin = '%I64d', finalLeft = '%I64d', playTimes = '%u', winTimes = '%u', offset = '%lld' WHERE roomID = '%u' and playerUID = '%d'and flag = '0' "
					,pRet->m_nReadedInformSerial,pRet->nTotalBuyInThisRoom,pRet->nFinalLeftInThisRoom,pRet->nPlayeTimesInThisRoom,pRet->nWinTimesInThisRoom,nOffset,pRet->nRoomID,pRet->nPlayerUID) ;
				LOGFMTD("updata taxas room player data room id = %u , uid = %u",pRet->nRoomID,pRet->nPlayerUID);
			}
			else
			{
				pRequest->eType = eRequestType_Add;
				pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"INSERT INTO taxasroomplayers (roomID, playerUID,readInformSerial,totalBuyin,finalLeft,playTimes,winTimes,offset) VALUES ('%u', '%u','%u','%I64d','%I64d','%u','%u','%I64d')",
					pRet->nRoomID,pRet->nPlayerUID,pRet->m_nReadedInformSerial,pRet->nTotalBuyInThisRoom,pRet->nFinalLeftInThisRoom,pRet->nPlayeTimesInThisRoom,pRet->nWinTimesInThisRoom,nOffset) ;
				LOGFMTD("add taxas room player data room id = %u , uid = %u",pRet->nRoomID,pRet->nPlayerUID);
			}
		}
		break;
	case MSG_READ_TAXAS_ROOM_PLAYERS:
		{
			stMsgReadTaxasRoomPlayers* pRet = (stMsgReadTaxasRoomPlayers*)pmsg ;
			pRequest->eType = eRequestType_Select;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
				"SELECT * FROM taxasroomplayers WHERE roomID = '%d' and flag = '0' order by offset desc limit 50 ",pRet->nRoomID) ;
			LOGFMTD("read taxas room players room id = %d",pRet->nRoomID);
			pdata->nExtenArg1 = pRet->nRoomID;
		}
		break;
	case MSG_SAVE_REMOVE_TAXAS_ROOM_PLAYERS:
		{
			stMsgSaveRemoveTaxasRoomPlayers* pRet = (stMsgSaveRemoveTaxasRoomPlayers*)pmsg ;
			pRequest->eType = eRequestType_Update;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
				"UPDATE taxasroomplayers SET flag = '1' WHERE roomID = '%u' and flag = '0' ",pRet->nRoomID) ;
			LOGFMTD("remove taxas room player data room id = %u ",pRet->nRoomID);
		}
		break;
	case MSG_SAVE_ROOM_PLAYER:
		{
			stMsgSaveRoomPlayer* pRet = (stMsgSaveRoomPlayer*)pmsg ;
			pRequest->eType = eRequestType_Add;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"INSERT INTO roomrankplayers (serialNum, termNumber,playerUID,offsetCoin,otherOffset ) VALUES ('%u','%u' ,'%u','%d','%d') ON DUPLICATE KEY UPDATE offsetCoin = '%d', otherOffset = '%d' ",
				pRet->nRoomSerialNum,pRet->nTermNumber,pRet->savePlayer.nUserUID,pRet->savePlayer.nGameOffset,pRet->savePlayer.nOtherOffset,pRet->savePlayer.nGameOffset,pRet->savePlayer.nOtherOffset) ;
			LOGFMTD("update room player data serialNum = %u ,term = %u ,uid = %u",pRet->nRoomSerialNum,pRet->nTermNumber,pRet->savePlayer.nUserUID);
		}
		break;
	case MSG_READ_ROOM_PLAYER:
		{
			stMsgReadRoomPlayer* pRet = (stMsgReadRoomPlayer*)pmsg ;
			pRequest->eType = eRequestType_Select;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
				"SELECT * FROM roomrankplayers WHERE serialNum = '%u' and termNumber = '%u' order by offsetCoin desc limit 90 ",pRet->nRoomSerialNum,pRet->nTermNumber) ;
			LOGFMTD("read room players room id = %d , type = %d",pRet->nRoomID,pRet->nRoomSerialNum );
			pdata->nExtenArg1 = pRet->nRoomID;
			pdata->nExtenArg2 = pRet->nTermNumber ;
			pdata->eFromPort = eSenderPort ;
		}
		break;
	//case MSG_REMOVE_ROOM_PLAYER:
	//	{
	//		stMsgRemoveRoomPlayer* pRet = (stMsgRemoveRoomPlayer*)pmsg ;
	//		pRequest->eType = eRequestType_Update;
	//		pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
	//			"UPDATE roomplayers SET flag = '1' WHERE roomID = '%u' and flag = '0' and roomType = %u",pRet->nRoomID,pRet->nRoomType) ;
	//		LOGFMTD("remove room player data room id = %u, type = %u ",pRet->nRoomID,pRet->nRoomType );
	//	}
	//	break;
// 	case MSG_PLAYER_SAVE_BASE_DATA:
// 		{
// 			stMsgGameServerSaveBaseData* pSaveBaseData = (stMsgGameServerSaveBaseData*)pmsg ;
// 			pdata->nSessionID = pSaveBaseData->nSessionID ;
// 			pdata->nExtenArg1 = pSaveBaseData->stBaseData.nUserUID ;
// 
// 			char pMaxCards[sizeof(pSaveBaseData->stBaseData.vMaxCards)*2 + 1 ] = {0} ;
// 			m_pTheApp->GetDBThread()->EscapeString(pMaxCards,(char*)pSaveBaseData->stBaseData.vMaxCards,sizeof(pSaveBaseData->stBaseData.vMaxCards)) ;
// 
// 			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
// 			pRequest->cOrder = eReq_Order_Low ;
// 			pRequest->eType = eRequestType_Update ;
// 			pRequest->nRequestUID = pmsg->usMsgType ;
// 			pRequest->pUserData = pdata;
// 			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer, 
// 			"UPDATE playerbasedata SET playerName = '%s', defaultPhotoID = '%u', isUploadPhoto = '%u', signature = '%s', sex = '%u', vipLevel = '%u',   \
// 			coin = '%I64u', diamond = '%u', winTimes = '%u', loseTimes = '%u', singleWinMost = '%I64u', maxCard = '%s',   \
// 			longitude = '%f', latitude = '%f', exp = '%u', offlineTime = '%u',noticeID = '%u',vipEndTime = '%u',\
// 			continueLoginDays = '%u',lastLoginTime = '%u',lastTakeCharityCoinTime = '%u',todayPlayTimes = '%u' , \
// 			yesterdayPlayTimes = '%u',todayWinCoin = '%I64d',yesterdayWinCoin = '%I64d', \
// 			takeMasterStudentRewardTime = '%u',rechargeTimes = '%u',curOnlineBoxID = '%u',onlineBoxPassedTime = '%u' WHERE userUID = '%u'", 
// 			pSaveBaseData->stBaseData.cName,pSaveBaseData->stBaseData.nDefaulPhotoID,pSaveBaseData->stBaseData.bIsUploadPhoto,pSaveBaseData->stBaseData.cSignature,pSaveBaseData->stBaseData.nSex,pSaveBaseData->stBaseData.nVipLevel,
// 			pSaveBaseData->stBaseData.nCoin,pSaveBaseData->stBaseData.nDiamoned,pSaveBaseData->stBaseData.nWinTimes,pSaveBaseData->stBaseData.nLoseTimes,pSaveBaseData->stBaseData.nSingleWinMost,pMaxCards,
// 			pSaveBaseData->stBaseData.dfLongitude,pSaveBaseData->stBaseData.dfLatidue,pSaveBaseData->stBaseData.nExp,(unsigned int)pSaveBaseData->stBaseData.tOfflineTime,pSaveBaseData->stBaseData.nNoticeID,pSaveBaseData->stBaseData.nVipEndTime,
// 			pSaveBaseData->stBaseData.nContinueDays,(unsigned int)pSaveBaseData->stBaseData.tLastLoginTime,(unsigned int)pSaveBaseData->stBaseData.tLastTakeCharityCoinTime,pSaveBaseData->stBaseData.nTodayPlayTimes,
// 			pSaveBaseData->stBaseData.nYesterdayPlayTimes,pSaveBaseData->stBaseData.nTodayWinCoin,pSaveBaseData->stBaseData.nYesterdayWinCoin,
// 			(unsigned int)pSaveBaseData->stBaseData.tTakeMasterStudentRewardTime,pSaveBaseData->stBaseData.nRechargeTimes,pSaveBaseData->stBaseData.nCurOnlineBoxID,pSaveBaseData->stBaseData.nOnlineBoxPassedTime,pSaveBaseData->stBaseData.nUserUID) ;
// 			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
// 		}
// 		break ;
// 	case MSG_SAVE_PLAYER_COIN:
// 		{
// 			stMsgGameServerSavePlayerCoin* pSaveCoin = (stMsgGameServerSavePlayerCoin*)pmsg ;
// 			pdata->nSessionID = pSaveCoin->nSessionID ;
// 			pdata->nExtenArg1 = pSaveCoin->nUserUID ;
// 
// 			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
// 			pRequest->cOrder = eReq_Order_Low ;
// 			pRequest->eType = eRequestType_Update ;
// 			pRequest->nRequestUID = pmsg->usMsgType ;
// 			pRequest->pUserData = pdata;
// 			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"UPDATE gamedb.account SET nCoin = '%I64d', nDiamoned = '%d' WHERE UserUID = '%d'",pSaveCoin->nCoin,pSaveCoin->nDiamoned,pSaveCoin->nUserUID) ;
// 			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
// 		}
// 		break;  
// 	case MSG_SAVE_FRIEND_LIST:
// 		{
// 			stMsgGameServerSaveFirendList* pSaveFriend = (stMsgGameServerSaveFirendList*)pmsg ;
// 			pdata->nSessionID = pSaveFriend->nSessionID ;
// 			pdata->nExtenArg1 = pSaveFriend->nUserUID ;
// 
// 			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
// 			pRequest->cOrder = eReq_Order_Low ;
// 			pRequest->nRequestUID = pmsg->usMsgType ;
// 			pRequest->pUserData = pdata;
// 			pRequest->eType = eRequestType_Update ;
// 
// 			char* pBuffer = (char*)pmsg ;
// 			pBuffer += sizeof(stMsgGameServerSaveFirendList);
// 
// 			char *pFriendListBuffer = new char[sizeof(stServerSaveFrienItem)*pSaveFriend->nFriendCount*2+1] ;
// 			memset(pFriendListBuffer,0,sizeof(sizeof(stServerSaveFrienItem)*pSaveFriend->nFriendCount*2+1));
// 			m_pTheApp->GetDBThread()->EscapeString(pFriendListBuffer,pBuffer,pSaveFriend->nFriendCount * sizeof(stServerSaveFrienItem) ) ;
// 			unsigned int nSaveTime = (unsigned int)time(NULL) ;
// 			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"UPDATE playerfriend SET friendCount = '%u',contentData = '%s',saveTime = '%u' WHERE userUID = '%d'",pSaveFriend->nFriendCount,pFriendListBuffer,nSaveTime,pSaveFriend->nUserUID) ;
// 			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
// 			delete[]pFriendListBuffer ;
// 		}
// 		break;
// 	case MSG_REQUEST_FRIEND_LIST:
// 		{
// 			stMsgGameServerRequestFirendList* pRequestFriend = (stMsgGameServerRequestFirendList*)pmsg ;
// 			pdata->nSessionID = pRequestFriend->nSessionID ;
// 			pdata->nExtenArg1 = 0 ;  // more than one time select ; 
// 
// 			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
// 			pRequest->cOrder = eReq_Order_High ;
// 			pRequest->eType = eRequestType_Select ;
// 			pRequest->nRequestUID = pmsg->usMsgType ;
// 			pRequest->pUserData = pdata;
// 			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"SELECT * FROM playerfriend WHERE userUID = '%u'",pRequestFriend->nUserUID ) ;
// 			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
// 		}
// 		break;
// 	case MSG_REQUEST_FRIEND_BRIFDATA_LIST:
// 		{
// 			stMsgGameServerRequestFriendBrifDataList* pRet = (stMsgGameServerRequestFriendBrifDataList*)pmsg ;
// 			pdata->nSessionID = pRet->nSessionID ;
// 
// 			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
// 			pRequest->cOrder = eReq_Order_High ;
// 			pRequest->eType = eRequestType_Select ;
// 			pRequest->nRequestUID = pmsg->usMsgType ;
// 			pRequest->pUserData = pdata;
// 			char* pBffer = (char*)pRet ;
// 			pBffer += sizeof(stMsgGameServerRequestFriendBrifDataList);
// 			unsigned int * pUserUID = (unsigned int*)pBffer ;
// 			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"SELECT %s FROM playerfriend WHERE userUID = '%u'",PLAYER_BRIF_DATA,*pUserUID ) ;
// 			--pRet->nFriendCount ;
// 			while ( pRet->nFriendCount--)
// 			{
// 				pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"%s || userUID = '%u'",pRequest->pSqlBuffer,*pUserUID ) ;
// 				++pUserUID ;
// 			}
// 			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
// 		}
// 		break;
// 	case MSG_PLAYER_SERACH_PEERS:
// 		{
// 			stMsgGameServerGetSearchFriendResult* pMsgRet = (stMsgGameServerGetSearchFriendResult*)pmsg ;
// 			pdata->nSessionID = pMsgRet->nSessionID ;
// 
// 			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
// 			pRequest->cOrder = eReq_Order_High ;
// 			pRequest->eType = eRequestType_Select ;
// 			pRequest->nRequestUID = pmsg->usMsgType ;
// 			pRequest->pUserData = pdata;
// 			char* pBuffer = new char[pMsgRet->nLen * 2 + 1 ] ;
// 			memset(pBuffer,0 ,pMsgRet->nLen * 2 + 1  ) ;
// 			m_pTheApp->GetDBThread()->EscapeString(pBuffer, (char*)pMsgRet + sizeof(stMsgGameServerGetSearchFriendResult),pMsgRet->nLen) ;
// 			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"SELECT %s FROM playerbasedata where userUID regexp( '%s' ) or playerName regexp('%s') limit 15;", PLAYER_BRIF_DATA,pBuffer, pBuffer ) ;
// 			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
// 			delete[] pBuffer ;
// 		}
// 		break;
// 	case MSG_PLAYER_REQUEST_SEARCH_PEER_DETAIL:
// 		{
// 			stMsgGameServerGetSearchedPeerDetail* pMsgRet = (stMsgGameServerGetSearchedPeerDetail*)pmsg ;
// 			pdata->nSessionID = pMsgRet->nSessionID ;
// 
// 			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
// 			pRequest->cOrder = eReq_Order_High ;
// 			pRequest->eType = eRequestType_Select ;
// 			pRequest->nRequestUID = pmsg->usMsgType ;
// 			pRequest->pUserData = pdata;
// 			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"SELECT %s FROM playerbasedata where userUID = %u", PLAYER_DETAIL_DATA,pMsgRet->nPeerUserUID) ;
// 			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
// 		}
// 		break;
// 	case MSG_PLAYER_REQUEST_FRIEND_DETAIL:
// 		{
// 			stMsgGameServerGetFriendDetail* pMsgRet = (stMsgGameServerGetFriendDetail*)pmsg ;
// 			pdata->nSessionID = pMsgRet->nSessionID ;
// 
// 			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
// 			pRequest->cOrder = eReq_Order_High ;
// 			pRequest->eType = eRequestType_Select ;
// 			pRequest->nRequestUID = pmsg->usMsgType ;
// 			pRequest->pUserData = pdata;
// 			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"SELECT %s FROM playerbasedata where userUID = %d", PLAYER_DETAIL_DATA,pMsgRet->nFriendUID) ;
// 			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
// 		}
// 		break;
// 	case MSG_PLAYER_GET_MAIL_LIST:
// 		{
// 			stMsgGameServerGetMailList* pMsgRet = (stMsgGameServerGetMailList*)pmsg;
// 			pdata->nSessionID = pMsgRet->nSessionID ;
// 
// 			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
// 			pRequest->cOrder = eReq_Order_High ;
// 			pRequest->eType = eRequestType_Select ;
// 			pRequest->nRequestUID = pmsg->usMsgType ;
// 			pRequest->pUserData = pdata;
// 			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"select * from mail where userUID = %u order by postTime desc limit %d",pMsgRet->nUserUID, MAX_KEEP_MAIL_COUNT ) ;
// 			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
// 		}
// 		break; 
// 	case MSG_PLAYER_SAVE_MAIL:
// 		{
// 			stMsgGameServerSaveMail* pMsgRet = (stMsgGameServerSaveMail*)pmsg  ;
// 			pdata->nSessionID = pMsgRet->nSessionID ;
// 			pdata->nExtenArg1 = pMsgRet->nUserUID ;
// 			char* pmsgData = (char*)pmsg ;
// 			pmsgData += sizeof(stMsgGameServerSaveMail);
// 			stMail* pMailToSave = (stMail*)pmsgData ;
// 			pdata->nExtenArg2 = (unsigned int)pMailToSave->nMailUID ;
// 
// 			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
// 			pRequest->cOrder = eReq_Order_Low ;
// 			pRequest->nRequestUID = pmsg->usMsgType ;
// 			pRequest->pUserData = pdata;
// 			switch ( pMsgRet->nOperateType ) 
// 			{
// 			case eDBAct_Update:
// 				{
// 					pRequest->eType = eRequestType_Update ;
// 					pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"UPDATE mail SET processAct = '%d' WHERE mailUID = '%I64d'",pMailToSave->eProcessAct,pMailToSave->nMailUID ) ;
// 				}
// 				break;
// 			case eDBAct_Delete:
// 				{
// 					pRequest->eType = eRequestType_Delete ;
// 					pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"DELETE FROM mail WHERE mailUID ='%I64d' limit 1",pMailToSave->nMailUID ) ;
// 				}
// 				break;
// 			case eDBAct_Add:
// 				{
// 					pRequest->eType = eRequestType_Add ;
// 					char* pContent = new char[pMailToSave->nContentLen * 2 + 1 ] ;
// 					memset(pContent,0,pMailToSave->nContentLen * 2 + 1 );
// 					
// 					char* pBuffer = (char*)pMsgRet;
// 					pBuffer += sizeof(stMsgGameServerSaveMail);
// 					pBuffer += sizeof(stMail);
// 					m_pTheApp->GetDBThread()->EscapeString(pContent,pBuffer,pMailToSave->nContentLen ) ;
// 					LOGFMTD("mail title content Len = %d",pMailToSave->nContentLen) ;
// 					pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"INSERT INTO mail (mailUID, userUID,postTime,mailType,mailContentLen,mailContent,processAct) VALUES ('%I64d', '%u','%u','%u','%u','%s','%u')",
// 						pMailToSave->nMailUID,pMsgRet->nUserUID,pMailToSave->nPostTime,pMailToSave->eType,pMailToSave->nContentLen,pContent,pMailToSave->eProcessAct) ;
// 					delete[] pContent ;
// 				}
// 				break;
// 			default:
// 				LOGFMTE("unknown save mail operation type !") ;
// 				pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"") ;
// 				break;
// 			}
// 			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
// 		}
// 		break; 
// 	case MSG_GAME_SERVER_GET_MAX_MAIL_UID:
// 		{
// 			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
// 			pRequest->cOrder = eReq_Order_Normal ;
// 			pRequest->eType = eRequestType_Select ;
// 			pRequest->nRequestUID = MSG_GAME_SERVER_GET_MAX_MAIL_UID;
// 			pRequest->pUserData = pdata;
// 			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"(select max(mail.mailUID) FROM mail)") ;
// 			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
// 		}
// 		break;
// 	case MSG_REQUEST_ITEM_LIST:
// 		{
// 			stMsgGameServerRequestItemList* pMsgRet = (stMsgGameServerRequestItemList*)pmsg ;
// 			pdata->nSessionID = pMsgRet->nSessionID ;
// 			pdata->nExtenArg1 = pMsgRet->nUserUID ;
// 
// 			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
// 			pRequest->cOrder = eReq_Order_High ;
// 			pRequest->eType = eRequestType_Select ;
// 			pRequest->nRequestUID = pmsg->usMsgType ;
// 			pRequest->pUserData = pdata;
// 			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"select * from playeritems where userUID = %u",pMsgRet->nUserUID ) ;
// 			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
// 		}
// 		break;
// 	case MSG_SAVE_ITEM_LIST:
// 		{
// 			stMsgGameServerSaveItemList* pMsgRet = (stMsgGameServerSaveItemList*)pmsg ;
// 			pdata->nSessionID = pMsgRet->nSessionID ;
// 			pdata->nExtenArg1 = pMsgRet->nUserUID ;
// 			char* pBuffer = new char[pMsgRet->nOwnItemKindCount * sizeof(stPlayerItem) * 2 + 1] ;
// 			memset(pBuffer,0,pMsgRet->nOwnItemKindCount * sizeof(stPlayerItem) * 2 + 1);
// 			m_pTheApp->GetDBThread()->EscapeString(pBuffer, (((char*)pmsg) + sizeof(stMsgGameServerSaveItemList)),pMsgRet->nOwnItemKindCount * sizeof(stPlayerItem) ) ;
// 
// 			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
// 			pRequest->cOrder = eReq_Order_Low ;
// 			pRequest->pUserData = pdata;
// 			pRequest->nRequestUID = pmsg->usMsgType ;
// 			pRequest->eType = eRequestType_Update ;
// 			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"UPDATE playeritems SET itemsData = '%s',ownItemsKindCount = '%u' WHERE userUID = '%d'",pBuffer,pMsgRet->nOwnItemKindCount,pMsgRet->nUserUID ) ;
// 			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
// 			delete[] pBuffer ;
// 		}
// 		break; 
// 	case MSG_REQUEST_RANK:
// 		{
// 			stMsgGameServerRequestRank* pMsgRet = (stMsgGameServerRequestRank*)pmsg ;
// 			pdata->nExtenArg1 = pMsgRet->eType ;
// 			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
// 			pRequest->cOrder = eReq_Order_Normal ;
// 			pRequest->eType = eRequestType_Select ;
// 			pRequest->nRequestUID = pmsg->usMsgType ;
// 			pRequest->pUserData = pdata;
// 			switch ( pMsgRet->eType)
// 			{
// 			case eRank_AllCoin:
// 				{
// 					pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"select yesterdayWinCoin ,%s from playerbasedata where userUID !=0 order by coin desc limit %d",PLAYER_DETAIL_DATA,RANK_SHOW_PEER_COUNT) ;
// 				}
// 				break;
// 			case eRank_YesterDayWin:
// 				{
// 					unsigned int tYesterDay = (unsigned int)time(NULL) - 24 * 3600 ;
// 					//struct tm tmNow = *localtime(&tNow) ;
// 					//tmNow.tm_hour = 0 ;
// 					//tmNow.tm_min = 0 ;
// 					//tmNow.tm_sec = 0 ;
// 					//time_t tZero = mktime(&tmNow) ;
// 					pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"select yesterdayWinCoin ,%s from playerbasedata where (offlineTime >= '%u' || lastLoginTime >= '%u') order by todayWinCoin desc limit %d",PLAYER_DETAIL_DATA,tYesterDay,tYesterDay,RANK_SHOW_PEER_COUNT) ;
// 				}
// 				break;
// 			case eRank_SingleWinMost:
// 				{
// 					pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"select yesterdayWinCoin ,%s from playerbasedata where userUID !=0 order by singleWinMost desc limit %d",PLAYER_DETAIL_DATA,RANK_SHOW_PEER_COUNT) ;
// 				}
// 				break;
// 			default:
// 				LOGFMTE("unknown rank type to select type = %d",pMsgRet->eType) ;
// 				break;
// 			}
// 			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
// 		}
// 		break;
// 	//case MSG_GET_SHOP_BUY_RECORD:
// 	//	{
// 	//		stMsgGameServerGetShopBuyRecord* pMsgRet = (stMsgGameServerGetShopBuyRecord*)pmsg ;
// 	//		pdata->nSessionID = pMsgRet->nSessionID ;
// 	//		pdata->nExtenArg1 = pMsgRet->nUserUID ;
// 
// 	//		stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
// 	//		pRequest->cOrder = eReq_Order_High ;
// 	//		pRequest->eType = eRequestType_Select ;
// 	//		pRequest->nRequestUID = pmsg->usMsgType ;
// 	//		pRequest->pUserData = pdata;
// 	//		pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"select * from playershopbuyrecord where nUserUID = %d",pMsgRet->nUserUID ) ;
// 	//		CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
// 	//	}
// 	//	break;
// 	//case MSG_SAVE_SHOP_BUY_RECORD:
// 	//	{
// 	//		stMsgGameServerSaveShopBuyRecord* pMsgRet = (stMsgGameServerSaveShopBuyRecord*)pmsg;
// 	//		pdata->nSessionID = pMsgRet->nSessionID ;
// 	//		pdata->nExtenArg1 = pMsgRet->nUserUID ;
// 
// 	//		stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
// 	//		pRequest->cOrder = eReq_Order_Low ;
// 	//		pRequest->nRequestUID = pmsg->usMsgType ;
// 	//		pRequest->pUserData = pdata;
// 
// 	//		char* pSaveBuffer = new char[pMsgRet->nBufferLen * 2 + 1 ] ;
// 	//		memset(pSaveBuffer,0,pMsgRet->nBufferLen * 2 + 1) ;
// 	//		char* pBuffer = (char*)pmsg ;
// 	//		pBuffer += sizeof(stMsgGameServerSaveShopBuyRecord);
// 	//		m_pTheApp->GetDBThread()->EscapeString(pSaveBuffer,pBuffer,pMsgRet->nBufferLen ) ;
// 	//		if ( pMsgRet->bAdd )
// 	//		{
// 	//			pRequest->eType = eRequestType_Add ;
// 	//			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"INSERT INTO `gamedb`.`playershopbuyrecord` (`nUserUID`, `pBuffer`) VALUES ('%d', '%s');",pMsgRet->nUserUID,pSaveBuffer) ;
// 	//		}
// 	//		else
// 	//		{
// 	//			pRequest->eType = eRequestType_Update ;
// 	//			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"UPDATE gamedb.playershopbuyrecord SET pBuffer = '%s' WHERE nUserUID = '%d'",pSaveBuffer,pMsgRet->nUserUID) ;
// 	//		}
// 	//		CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
// 	//		delete[] pSaveBuffer ;
// 	//	}
// 	//	break;
// 	case MSG_GAME_SERVER_SAVE_MISSION_DATA:
// 		{
// 			stMsgGameServerSaveMissionData* pMissionData = (stMsgGameServerSaveMissionData*)pmsg ;
// 			pdata->nSessionID = pMissionData->nSessionID ;
// 			pdata->nExtenArg1 = pMissionData->nUserUID ;
// 
// 			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
// 			pRequest->cOrder = eReq_Order_Low ;
// 			pRequest->nRequestUID = pmsg->usMsgType ;
// 			pRequest->pUserData = pdata;
// 
// 			char* pSaveBuffer = new char[pMissionData->nMissonCount * sizeof(stMissionSate) * 2 + 1 ] ;
// 			memset(pSaveBuffer,0,pMissionData->nMissonCount * sizeof(stMissionSate) * 2 + 1) ;
// 			char* pBuffer = (char*)pmsg ;
// 			pBuffer += sizeof(stMsgGameServerSaveMissionData);
// 			m_pTheApp->GetDBThread()->EscapeString(pSaveBuffer,pBuffer,pMissionData->nMissonCount * sizeof(stMissionSate)) ;
// 			pRequest->eType = eRequestType_Update ;
// 			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"UPDATE playermission SET missionData = '%s',saveTime = '%u',missionCount = '%u' WHERE userUID = '%d'",pSaveBuffer,pMissionData->nSavetime,pMissionData->nMissonCount,pMissionData->nUserUID) ;
// 			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
// 			delete[] pSaveBuffer ;
// 		}
// 		break;
// 	case MSG_GAME_SERVER_GET_MISSION_DATA:
// 		{
// 			stMsgGameServerGetMissionData* pMsgRet = (stMsgGameServerGetMissionData*)pmsg ;
// 			pdata->nSessionID = pMsgRet->nSessionID ;
// 			pdata->nExtenArg1 = pMsgRet->nUserUID ;
// 
// 			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
// 			pRequest->cOrder = eReq_Order_High ;
// 			pRequest->eType = eRequestType_Select ;
// 			pRequest->nRequestUID = pmsg->usMsgType ;
// 			pRequest->pUserData = pdata;
// 			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"select * from playermission where userUID = %d",pMsgRet->nUserUID ) ;
// 			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
// 		}
// 		break;
	default:
		{
			m_vReserverArgData.push_back(pdata) ;
			LOGFMTE("unknown msg type = %d",pmsg->usMsgType ) ;
		}
	}

	if ( pRequest->nSqlBufferLen == 0 || pRequest->eType == eRequestType_Max )
	{
		LOGFMTE("a request sql len = 0 , msg = %d" , pRequest->nRequestUID ) ;
		
		CDBRequestQueue::VEC_DBREQUEST v ;
		v.push_back(pRequest) ;
		CDBRequestQueue::SharedDBRequestQueue()->PushReserveRequest(v);
	}
	else
	{
		CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
	}
}

void CDBManager::OnDBResult(stDBResult* pResult)
{
	// process result 
// 	if ( pResult->nRequestUID == (unsigned int )-1 )   // get current max curUID ;
// 	{
// 		if ( pResult->nAffectRow > 0 )
// 		{
// 			CMysqlRow& pRow = *pResult->vResultRows.front(); 
// 			unsigned int nMaxID = pRow["max(Account.UserUID)"]->IntValue();
// 			if ( nMaxID >= nCurUserUID )
// 			{
// 				nCurUserUID = ++nMaxID ;
// 				LOGFMTI("curMaxUID is %d",nMaxID ) ;
// 			}
// 		}
// 		return;
// 	}

	stArgData*pdata = (stArgData*)pResult->pUserData ;
	LOGFMTD("processed db ret = %d",pResult->nRequestUID);
	
	switch ( pResult->nRequestUID )
	{
	case nAsyncReq:
		{
			// make result to js value ;
			Json::Value jsResult ;
			jsResult["afctRow"] = pResult->nAffectRow ;
			Json::Value jsData ;
			for ( uint16_t nIdx = 0 ; nIdx < pResult->nAffectRow && pResult->vResultRows.size() > nIdx ; ++nIdx )
			{
				CMysqlRow& pRow = *pResult->vResultRows[nIdx] ;
				Json::Value jsRow ;
				pRow.toJsValue(jsRow) ;
				jsData[jsData.size()] = jsRow ;
			}
			jsResult["data"] = jsData ;

			// send back ;
			stMsgAsyncRequestRet msgBack ;
			msgBack.cSysIdentifer = (eMsgPort)pdata->eFromPort ;
			msgBack.nReqSerailID = pdata->nSessionID ;
			msgBack.nResultContentLen = 0 ;
			if ( jsResult.isNull() == true )
			{
				m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msgBack,sizeof(msgBack)) ;
			}
			else
			{
				Json::StyledWriter jsWrite ;
				auto strResult = jsWrite.write(jsResult);
				msgBack.nResultContentLen = strResult.size() ;
				CAutoBuffer auBuffer(sizeof(msgBack) + msgBack.nResultContentLen );
				auBuffer.addContent(&msgBack,sizeof(msgBack));
				auBuffer.addContent(strResult.c_str(),msgBack.nResultContentLen) ;
				m_pTheApp->sendMsg(pdata->nSessionID,auBuffer.getBufferPtr(),auBuffer.getContentSize()) ;
			}
			LOGFMTD("processed async req from = %u , serailNum = %u",pdata->eFromPort,pdata->nSessionID);
		}
		break;
	case MSG_READ_PRIVATE_ROOM_PLAYER:
		{
			stMsgReadPrivateRoomPlayerRet msgBack ;
			msgBack.cSysIdentifer = pdata->eFromPort ;
			msgBack.nRoomID = pdata->nExtenArg1 ;
			CAutoBuffer auBuffer (sizeof(msgBack) + 200 );
			for ( uint16_t nIdx = 0 ; nIdx < pResult->nAffectRow; ++nIdx )
			{
				CMysqlRow& pRow = *pResult->vResultRows[nIdx] ;
				auBuffer.clearBuffer();
				msgBack.nRoomID = pdata->nExtenArg1 ;
				msgBack.nSubRoomIdx = 0 ;
				msgBack.nJsonLen = pRow["content"]->nBufferLen; 
				auBuffer.addContent(&msgBack,sizeof(msgBack));
				auBuffer.addContent(pRow["content"]->BufferData(),msgBack.nJsonLen);
				m_pTheApp->sendMsg(pdata->nSessionID,auBuffer.getBufferPtr(),auBuffer.getContentSize()) ;
			}
		}
		break;
	case MSG_VERIFY_ENCRYPT_NUMBER:
		{
			stMsgVerifyEncryptNumberRet msgBack ;
			msgBack.nUserUID = pdata->nExtenArg1 ;

			CMysqlRow& pRow = *pResult->vResultRows.front();
			msgBack.nAddCoin = pRow["nOutAddCoin"]->IntValue();
			msgBack.nRet = pRow["nOutRet"]->IntValue() ;
			msgBack.nCoinType = pRow["nOutCoinType"]->IntValue();
			m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msgBack,sizeof(msgBack)) ;
		}
		break ;
	case MSG_SAVE_ENCRYPT_NUMBER:
		{
			if ( pResult->nAffectRow > 0 )
			{
				CMysqlRow& pRow = *pResult->vResultRows.front();
				stMsgSaveEncryptNumberRet msgBack ;
				msgBack.nRet = pRow["nRet"]->IntValue();
				m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msgBack,sizeof(msgBack)) ;
			}
			else
			{
				LOGFMTD("save encrypt number no respone") ;
			}
		}
		break ;
	case MSG_READ_GAME_RESULT:
		{
			stMsgReadGameResultRet msgBack ;
			msgBack.cSysIdentifer = pdata->eFromPort ;
			CAutoBuffer auBuffer (sizeof(msgBack) + 200 );
			for ( uint16_t nIdx = 0 ; nIdx < pResult->nAffectRow; ++nIdx )
			{
				CMysqlRow& pRow = *pResult->vResultRows[nIdx] ;
				auBuffer.clearBuffer();
				msgBack.isFinal = nIdx + 1 == pResult->nAffectRow ;
				msgBack.nConfigID = pRow["configID"]->IntValue();
				msgBack.nCreaterUID = pRow["createUID"]->IntValue();
				msgBack.nDuringSeconds = pRow["duiringSeconds"]->IntValue(); 
				msgBack.nRoomID = pRow["roomID"]->IntValue();
				msgBack.nRoomType = pRow["roomType"]->IntValue();
				msgBack.tTime = pRow["time"]->IntValue(); 
				msgBack.nSieralNum = pRow["sieralNum"]->IntValue();
				msgBack.nJsLen = pRow["playerDetail"]->nBufferLen; 
				auBuffer.addContent(&msgBack,sizeof(msgBack));
				auBuffer.addContent(pRow["playerDetail"]->BufferData(),msgBack.nJsLen);
				m_pTheApp->sendMsg(pdata->nSessionID,auBuffer.getBufferPtr(),auBuffer.getContentSize()) ;
			}
		}
		break ;
	case MSG_READ_PLAYER_GAME_RECORDER:
		{
			stMsgReadPlayerGameRecorderRet msgBack ;
			for ( uint16_t nIdx = 0 ; nIdx < pResult->nAffectRow; ++nIdx )
			{
				CMysqlRow& pRow = *pResult->vResultRows[nIdx] ;
				msgBack.cSysIdentifer = pdata->eFromPort ;
				msgBack.isFinal = nIdx + 1 == pResult->nAffectRow ;
				msgBack.nCreateUID = pRow["createUID"]->IntValue();
				msgBack.nDuiringSeconds = pRow["duiringSeconds"]->IntValue();
				msgBack.nFinishTime = pRow["finishTime"]->IntValue();
				msgBack.nOffset = pRow["offset"]->IntValue();
				msgBack.nRoomID = pRow["roomID"]->IntValue();
				msgBack.nBuyIn = pRow["buyIn"]->IntValue() ;
				msgBack.nBaseBet = pRow["baseBet"]->IntValue() ;
				msgBack.nClubID = pRow["clubID"]->IntValue() ;
				msgBack.nSeiralNum = pRow["sieralNum"]->IntValue();
				memset(msgBack.cRoomName,0,sizeof(msgBack.cRoomName)) ;
				if ( pRow["roomName"]->nBufferLen > 0 )
				{
					memcpy(msgBack.cRoomName,pRow["roomName"]->BufferData(),pRow["roomName"]->nBufferLen);
				}
				m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msgBack,sizeof(msgBack)) ;
			}
		}
		break ;
	case MSG_READ_EXCHANGE:
		{
			stMsgReadExchangesRet msgBack ;
			msgBack.nCnt = pResult->nAffectRow ;
			CAutoBuffer auBuffer( sizeof(msgBack) + sizeof(stExchangeItem) * msgBack.nCnt ) ;
			auBuffer.addContent(&msgBack,sizeof(msgBack)) ;

			for ( uint16_t nIdx = 0 ; nIdx < pResult->nAffectRow; ++nIdx )
			{
				CMysqlRow& pRow = *pResult->vResultRows[nIdx] ;
				stExchangeItem item ;
				item.nExchangeID = pRow["exchangeID"]->IntValue();
				item.nExchangedCnt = pRow["count"]->nBufferLen;
				auBuffer.addContent(&item,sizeof(item));
			}
			m_pTheApp->sendMsg(pdata->nSessionID,auBuffer.getBufferPtr(),auBuffer.getContentSize()) ;
		}
		break;
	case MSG_CIRCLE_READ_TOPICS:
		{
			uint16_t nPageCnt = ( pResult->nAffectRow + CIRCLE_TOPIC_CNT_PER_PAGE - 1 )  / CIRCLE_TOPIC_CNT_PER_PAGE ;
			for ( uint16_t nPageIdx = 0 ; nPageIdx < nPageCnt ; ++nPageIdx )
			{
				stMsgReadCircleTopicsRet msgBack ;
				msgBack.nCnt = pResult->nAffectRow - nPageIdx * CIRCLE_TOPIC_CNT_PER_PAGE ;
				if ( msgBack.nCnt > CIRCLE_TOPIC_CNT_PER_PAGE )
				{
					msgBack.nCnt = CIRCLE_TOPIC_CNT_PER_PAGE ;
				}

				CAutoBuffer auBuffer( sizeof(msgBack) + (sizeof(stCircleTopicItem) + 150 ) * msgBack.nCnt ) ;
				auBuffer.addContent(&msgBack,sizeof(msgBack)) ;
				for ( uint8_t nIdx = 0 ; nIdx < CIRCLE_TOPIC_CNT_PER_PAGE ; ++nIdx )
				{
					uint16_t nDataIdx = nPageIdx * CIRCLE_TOPIC_CNT_PER_PAGE + nIdx ;
					if ( nDataIdx >= pResult->nAffectRow )
					{
						break; 
					}
					CMysqlRow& pRow = *pResult->vResultRows[nDataIdx] ;
					stCircleTopicItem item ;
					item.nAuthorUID = pRow["authorUID"]->IntValue();
					item.nContentLen = pRow["content"]->nBufferLen;
					item.nPublishTime = pRow["publishTime"]->IntValue();
					item.nTopicID = pRow["topicID"]->IntValue() ;
					auBuffer.addContent(&item,sizeof(item));
					auBuffer.addContent(pRow["content"]->BufferData(),pRow["content"]->nBufferLen);
				}
				m_pTheApp->sendMsg(pdata->nSessionID,auBuffer.getBufferPtr(),auBuffer.getContentSize()) ;
			}

			if ( nPageCnt == 0 )
			{
				LOGFMTI("poker circle topic cnt = 0") ;
			}
		}
		break;
	case MSG_SELECT_DB_PLAYER_DATA:
		{
			stMsgSelectPlayerDataRet msgBack;
			msgBack.nRet = pResult->nAffectRow > 0 ? 0 : 1 ;
			if ( msgBack.nRet )
			{
				m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msgBack,sizeof(msgBack)) ;
				return ;
			}
			msgBack.nDataPlayerUID = pdata->nExtenArg1 ;
			msgBack.isDetail = pdata->nExtenArg2 ;
			CAutoBuffer auB(sizeof(msgBack) + sizeof(stPlayerDetailDataClient) );
			stPlayerDetailDataClient tData;
			memset(&tData,0,sizeof(tData)) ;
			CMysqlRow& pRow = *pResult->vResultRows.front();
			if ( msgBack.isDetail )
			{
				tData.bIsOnLine = false ;
				memset(tData.cName,0,sizeof(tData.cName)) ;
				if ( pRow["cName"]->nBufferLen < sizeof(tData.cName) )
				{
					sprintf_s(tData.cName,sizeof(tData.cName),"%s",pRow["cName"]->CStringValue()) ;
				}
				
				tData.nCoin = pRow["nCoin"]->IntValue64();
				tData.nDiamoned = pRow["nDiamoned"]->IntValue();
				tData.nPhotoID = pRow["nPhotoID"]->IntValue();
				tData.nSex = pRow["nSex"]->IntValue();
				tData.nUserUID = pRow["nUserUID"]->IntValue();
				tData.nVipLevel = pRow["nVipLevel"]->IntValue();

				memset(tData.cSignature,0,sizeof(tData.cSignature)) ;
				if ( pRow["cSignature"]->nBufferLen <= sizeof(tData.cSignature) )
				{
					memcpy(tData.cSignature,pRow["cSignature"]->BufferData(),pRow["cSignature"]->nBufferLen);
				}
				else
				{
					LOGFMTE("signature is too long uid = %u",tData.nUserUID) ;
				}
				tData.nMostCoinEver = pRow["nMostCoinEver"]->IntValue64();
				tData.dfLatidue = pRow["dfLatidue"]->FloatValue();
				tData.dfLongitude = pRow["dfLongitude"]->FloatValue();

				time_t tLastOffline = pRow["tOfflineTime"]->IntValue();
				tData.tOfflineTime = tLastOffline ;

				std::vector<int> vInt ;
				vInt.clear();
				//read upload pic 
				vInt.clear();
				pRow["vUploadedPic"]->VecInt(vInt);
				memset(tData.vUploadedPic,0,sizeof(tData.vUploadedPic)) ;
				LOGFMTD("vUploadedPic size = %d uid = %d",vInt.size(),tData.nUserUID ) ;
				if ( vInt.size() == MAX_UPLOAD_PIC )
				{
					for ( uint8_t nIdx = 0 ; nIdx < MAX_UPLOAD_PIC ; ++nIdx )
					{
						tData.vUploadedPic[nIdx] = vInt[nIdx] ;
					}
				}

				// read joined club ids ;
				vInt.clear();
				pRow["vJoinedClubID"]->VecInt(vInt);
				memset(tData.vJoinedClubID,0,sizeof(tData.vJoinedClubID)) ;
				LOGFMTD("vJoinedClubID size = %d uid = %d",vInt.size(),tData.nUserUID ) ;
				if ( vInt.size() == MAX_JOINED_CLUB_CNT )
				{
					for ( uint8_t nIdx = 0 ; nIdx < MAX_JOINED_CLUB_CNT ; ++nIdx )
					{
						tData.vJoinedClubID[nIdx] = vInt[nIdx] ;
					}
				}

				// read taxas data 
				//tData.tTaxasData.nPlayTimes = pRow["nPlayTimes"]->IntValue();
				//tData.tTaxasData.nWinTimes = pRow["nWinTimes"]->IntValue();
				//tData.tTaxasData.nSingleWinMost = pRow["nSingleWinMost"]->IntValue64();

				vInt.clear();
				// read max card ;
				/*pRow["vMaxCards"]->VecInt(vInt);
				memset(tData.tTaxasData.vMaxCards,0,sizeof(tData.tTaxasData.vMaxCards)) ;
				if ( vInt.size() == MAX_TAXAS_HOLD_CARD )
				{
					for ( uint8_t nIdx = 0 ; nIdx < MAX_TAXAS_HOLD_CARD ; ++nIdx )
					{
						tData.tTaxasData.vMaxCards[nIdx] = vInt[nIdx] ;
					}
				}*/
				LOGFMTD("read select player detail uid = %d",tData.nUserUID) ;
			}
			else
			{
				GetPlayerBrifData(&tData,pRow) ;
			}
			auB.addContent(&msgBack,sizeof(msgBack)) ;
			auB.addContent(&tData,msgBack.isDetail ? sizeof(stPlayerDetailDataClient) : sizeof(stPlayerDetailDataClient)) ;
			m_pTheApp->sendMsg(pdata->nSessionID,auB.getBufferPtr(),auB.getContentSize()) ;
		}
		break;
	case MSG_PLAYER_READ_MAIL_LIST:
		{
			LOGFMTD("read mail list for uid = %d cnt = %d",pdata->nExtenArg1,pResult->nAffectRow) ;
			if ( pResult->nAffectRow < 1 )
			{
				return ;
			}

			CAutoBuffer buffer(256);
			for ( uint16_t nIdx = 0 ; nIdx < pResult->nAffectRow ; ++nIdx )
			{
				CMysqlRow& pRow = *pResult->vResultRows[nIdx] ;
				stMsgReadMailListRet msgRet ;
				msgRet.nUserUID = pdata->nExtenArg1 ;
				msgRet.bFinal = (nIdx + 1) == pResult->nAffectRow ;
				msgRet.pMails.eType = pRow["mailType"]->IntValue();
				msgRet.pMails.nPostTime = pRow["postTime"]->IntValue();
				msgRet.pMails.nContentLen = pRow["mailContent"]->nBufferLen ;
				if ( msgRet.pMails.nContentLen == 0 && msgRet.pMails.eType != eMail_ReadTimeTag )
				{
					LOGFMTE("why this mail len is null uid = %d type = %d, post time = %u",pRow["userUID"]->IntValue(),msgRet.pMails.eType,msgRet.pMails.nPostTime);
				}
 
				if ( msgRet.pMails.nContentLen > 0 )
				{
					buffer.clearBuffer();
					buffer.addContent((char*)&msgRet,sizeof(msgRet)) ;
					buffer.addContent(pRow["mailContent"]->BufferData(),msgRet.pMails.nContentLen);
					m_pTheApp->sendMsg(pdata->nSessionID,buffer.getBufferPtr(),buffer.getContentSize()) ;
				}
				else
				{
					m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msgRet,sizeof(msgRet)) ;
				}
			}
		}
		break;
	case MSG_READ_FRIEND_LIST:
		{
			if ( pResult->nAffectRow < 1 )
			{
				LOGFMTE("read friend list error uid = %d",pdata->nExtenArg1) ;
				return ;
			}

			CMysqlRow& pRow = *pResult->vResultRows.front();
			stMsgReadFriendListRet msgRet ;
			msgRet.nFriendCountLen = pRow["friendUIDs"]->nBufferLen ;
			if ( msgRet.nFriendCountLen == 0 )
			{
				m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msgRet,sizeof(msgRet)) ;
				return ;
			}

			CAutoBuffer auB(sizeof(msgRet) + msgRet.nFriendCountLen);
			auB.addContent(&msgRet,sizeof(msgRet)) ;
			auB.addContent(pRow["friendUIDs"]->BufferData(),pRow["friendUIDs"]->nBufferLen);
			m_pTheApp->sendMsg(pdata->nSessionID,auB.getBufferPtr(),auB.getContentSize()) ;
			LOGFMTD("player uid = %d read friend list ok",pdata->nExtenArg1) ;
		}
		break;
	//case MSG_REQUEST_CREATE_PLAYER_DATA:
	//	{
	//		if ( pResult->nAffectRow != 1 )
	//		{
	//			LOGFMTE("create player data error uid = %d",pdata->nExtenArg1) ;
	//		}
	//		else
	//		{
	//			CMysqlRow& pRow = *pResult->vResultRows.front();
	//			if ( pRow["nOutRet"]->IntValue() != 0 )
	//			{
	//				LOGFMTE("pp create player data error uid = %d ret = %d",pdata->nExtenArg1,pRow["nOutRet"]->IntValue() ) ;
	//			}
	//			else
	//			{
	//				LOGFMTD("create player data success uid = %d",pdata->nExtenArg1 ) ;
	//			}
	//		}
	//	}
	//	break;
	case MSG_READ_PLAYER_BASE_DATA:
		{
			stArgData* pdata = (stArgData*)pResult->pUserData ;
			stMsgDataServerGetBaseDataRet msg ;
			memset(&msg.stBaseData,0,sizeof(msg.stBaseData)) ;
			msg.nRet = 0 ;
			if ( pResult->nAffectRow <= 0 )
			{
				LOGFMTE("can not find base data with userUID = %d , session id = %d " , pdata->nExtenArg1,pdata->nSessionID ) ;
				msg.nRet = 1 ;
				m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msg,sizeof(msg)) ;
			}
			else
			{
				CMysqlRow& pRow = *pResult->vResultRows[0] ;
				GetPlayerDetailData(&msg.stBaseData,pRow);
				msg.stBaseData.nYesterdayCoinOffset = pRow["nYesterdayCoinOffset"]->IntValue() ;
				msg.stBaseData.nTodayGameCoinOffset = pRow["nTodayCoinOffset"]->IntValue() ;
				msg.stBaseData.tLastLoginTime = pRow["lastLoginTime"]->IntValue() ;
				msg.stBaseData.tLastTakeCharityCoinTime = pRow["lastTakeCharityCoinTime"]->IntValue() ;
				msg.stBaseData.nContinueDays = pRow["continueLoginDays"]->IntValue() ;
				msg.stBaseData.isRegister = pRow["isRegister"]->IntValue() ;
				msg.stBaseData.nNewPlayerHaloWeight = pRow["newPlayerHaloWeight"]->IntValue() ;
				msg.stBaseData.nInviteUID = pRow["inviterUID"]->IntValue() ;
				msg.stBaseData.nCardType = pRow["nVipCardType"]->IntValue();
				msg.stBaseData.nCardEndTime = pRow["vipCardEndTime"]->IntValue() ;
				msg.stBaseData.tLastTakeCardGiftTime = pRow["lastTakeCardGiftTime"]->IntValue();
				msg.stBaseData.nTotalInvitePrizeCoin = pRow["totalInvitePrizeCoin"]->IntValue() ;
				msg.stBaseData.nTakeCharityTimes = pRow["takeCharityTimes"]->IntValue();
				msg.stBaseData.nTotalGameCoinOffset = pRow["totalGameCoinOffset"]->IntValue() ;
				m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msg,sizeof(msg)) ;
			}
		}
		break;
	case MSG_DB_CHECK_INVITER:
		{
			stMsgDBCheckInviteRet msgBack ;
			msgBack.nInviteUseUID = pdata->nExtenArg1 ;
			msgBack.nRet = pResult->nAffectRow > 0 ? 0 : 1 ;
			m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msgBack,sizeof(msgBack)) ;
		}
		break;
	case MSG_READ_PLAYER_TAXAS_DATA:
		{
			stArgData* pdata = (stArgData*)pResult->pUserData ;
			stMsgReadPlayerTaxasDataRet msg ;
			msg.nRet = 0 ;
			msg.nUserUID = pdata->nExtenArg1 ;
			memset(&msg.tData,0,sizeof(msg.tData)) ;
			if ( pResult->nAffectRow <= 0 )
			{
				LOGFMTE("can not find TAXAS_DATA with userUID = %d , session id = %d " , pdata->nExtenArg1,pdata->nSessionID ) ;
				msg.nRet = 1 ;
				m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msg,sizeof(msg)) ;
			}
			else
			{
				CMysqlRow& pRow = *pResult->vResultRows[0] ;
				msg.tData.nPlayTimes = pRow["playTimes"]->IntValue();
				msg.tData.nWinTimes = pRow["winTimes"]->IntValue();
				msg.tData.nSingleWinMost = pRow["singleWinMost"]->IntValue64();
				msg.tData.nRun_upTimes = pRow["run_upTimes"]->IntValue() ;
				msg.tData.nChampionTimes = pRow["championTimes"]->IntValue() ;
				msg.tData.nThird_placeTimes = pRow["third_placeTimes"]->IntValue();
				msg.nUserUID = pdata->nExtenArg1 ;

				std::vector<int> vInt ;
				vInt.clear();
				// read max card ;
				pRow["maxCard"]->VecInt(vInt);
				memset(msg.tData.vMaxCards,0,sizeof(msg.tData.vMaxCards)) ;
				if ( vInt.size() == MAX_TAXAS_HOLD_CARD )
				{
					for ( uint8_t nIdx = 0 ; nIdx < MAX_TAXAS_HOLD_CARD ; ++nIdx )
					{
						msg.tData.vMaxCards[nIdx] = vInt[nIdx] ;
					}
				}
				m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msg,sizeof(msg)) ;
			}
		}
		break;
	case MSG_READ_PLAYER_GAME_DATA:
		{
			stArgData* pdata = (stArgData*)pResult->pUserData ;
			stMsgReadPlayerGameDataRet msg ;
			msg.nRet = 0 ;
			msg.nUserUID = pdata->nExtenArg1 ;
			msg.nJsonLen = 0 ;
			if ( pResult->nAffectRow <= 0 )
			{
				LOGFMTE("can not find NIU NIU data with userUID = %d , session id = %d " , pdata->nExtenArg1,pdata->nSessionID ) ;
				msg.nRet = 1 ;
				m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msg,sizeof(msg)) ;
			}
			else
			{
				CMysqlRow& pRow = *pResult->vResultRows[0] ;
				Json::Value gameDatas ;
				char* ptype[3] = {"taxas","niuniu","golden"};
				for ( uint8_t nIdx = 0 ; nIdx < 3 ; ++nIdx )
				{
					Json::Value gDaata;
					Json::Reader jsReader ;
					jsReader.parse(pRow[ptype[nIdx]]->CStringValue(),gDaata);
					gameDatas[(uint32_t)nIdx] = gDaata ;
				}

				Json::StyledWriter jsWrite ;
				std::string strJs = jsWrite.write(gameDatas) ;
				msg.nJsonLen = strJs.size() ;
				CAutoBuffer auBuffer ( sizeof(msg) + msg.nJsonLen );
				auBuffer.addContent(&msg,sizeof(msg)) ;
				auBuffer.addContent(strJs.c_str(),msg.nJsonLen) ;
				m_pTheApp->sendMsg(pdata->nSessionID,auBuffer.getBufferPtr(),auBuffer.getContentSize()) ;

				LOGFMTD("read uid = %u game data js = %s",msg.nUserUID,strJs.c_str()) ;
			}
		}
		break;
	case MSG_READ_ROOM_INFO:
		{
			stArgData* pdata = (stArgData*)pResult->pUserData ;
			if ( pResult->nAffectRow <= 0 )
			{
				LOGFMTE("can not read taxas rooms ") ;
			}
			else
			{
				CAutoBuffer buffer(256);
				for ( uint16_t nIdx = 0 ; nIdx < pResult->nAffectRow ; ++nIdx )
				{
					CMysqlRow& pRow = *pResult->vResultRows[nIdx] ;
					stMsgReadRoomInfoRet msgRet ;
					msgRet.cSysIdentifer = pdata->eFromPort ;
					msgRet.nRoomType = pRow["roomType"]->IntValue();
					msgRet.nRoomID = pRow["roomID"]->IntValue();
					msgRet.nRoomOwnerUID = pRow["ownerUID"]->IntValue();
					msgRet.nConfigID = pRow["configID"]->IntValue();
					msgRet.nJsonLen = pRow["jsonDetail"]->nBufferLen ; 
					if ( msgRet.nJsonLen > 0 )
					{
						buffer.clearBuffer();
						buffer.addContent((char*)&msgRet,sizeof(msgRet)) ;
						buffer.addContent(pRow["jsonDetail"]->BufferData(),msgRet.nJsonLen);
						m_pTheApp->sendMsg(pdata->nSessionID,buffer.getBufferPtr(),buffer.getContentSize()) ;
					}
					else
					{
						//m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msgRet,sizeof(msgRet)) ;
						LOGFMTD("read old formate room id = %d",msgRet.nRoomID);
					}
					LOGFMTD("read taxas room id = %d",msgRet.nRoomID);
				}
			}
		}
		break;
	case MSG_READ_TAXAS_ROOM_PLAYERS:
		{
			stArgData* pdata = (stArgData*)pResult->pUserData ;
			if ( pResult->nAffectRow == 0 )
			{
				LOGFMTD("room id = %d have no history players",pdata->nExtenArg1) ;
			}
			else
			{
				for ( uint16_t nIdx = 0 ; nIdx < pResult->nAffectRow ; ++nIdx )
				{
					CMysqlRow& pRow = *pResult->vResultRows[nIdx] ;
					stMsgReadTaxasRoomPlayersRet msgRet ;
					msgRet.nRoomID = pRow["roomID"]->IntValue();
					msgRet.nPlayerUID = pRow["playerUID"]->IntValue();
					msgRet.m_nReadedInformSerial = pRow["readInformSerial"]->IntValue();
					msgRet.nTotalBuyInThisRoom = pRow["totalBuyin"]->IntValue64();
					msgRet.nFinalLeftInThisRoom = pRow["finalLeft"]->IntValue64();
					msgRet.nWinTimesInThisRoom = pRow["playTimes"]->IntValue();
					msgRet.nPlayeTimesInThisRoom = pRow["winTimes"]->IntValue();
					m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msgRet,sizeof(msgRet)) ;
					LOGFMTD("read taxas room players room id = %d",msgRet.nRoomID);
				}
			}
		}
		break;
	case MSG_READ_ROOM_PLAYER:
		{
			stArgData* pdata = (stArgData*)pResult->pUserData ;
			if ( pResult->nAffectRow == 0 )
			{
				stMsgReadRoomPlayerRet msgRet ;
				msgRet.cSysIdentifer = pdata->eFromPort ; // send port ;
				msgRet.nRoomID = pdata->nExtenArg1;
				msgRet.nCnt = 0 ;
				msgRet.nTermNumber = pdata->nExtenArg2;
				msgRet.bIsLast = true;
				m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msgRet,sizeof(msgRet)) ;
				LOGFMTD("room id = %d have no history players",pdata->nExtenArg1) ;
			}
			else
			{
				uint8_t nSendCntInOnemsg  = 8;
				uint16_t nPage = ( pResult->nAffectRow + nSendCntInOnemsg - 1 ) / nSendCntInOnemsg ;
				for ( uint16_t nPageIdx = 0 ; nPageIdx < nPage ; ++nPageIdx )
				{
					uint16_t nPageStartIdx = nPageIdx * nSendCntInOnemsg ;
					uint8_t nSendCnt = nSendCntInOnemsg ;
					if ( nPageIdx == nPage - 1 )
					{
						nSendCnt = pResult->nAffectRow - nPageStartIdx ;
					}

					stMsgReadRoomPlayerRet msgRet ;
					msgRet.cSysIdentifer = pdata->eFromPort ; // send port ;
					msgRet.nRoomID = pdata->nExtenArg1;
					msgRet.nCnt = nSendCnt ;
					msgRet.nTermNumber = pdata->nExtenArg2;
					msgRet.bIsLast = nPageIdx == nPage - 1 ;
					CAutoBuffer auBuffer(sizeof(msgRet) + msgRet.nCnt * sizeof(stSaveRoomPlayerEntry));
					auBuffer.addContent(&msgRet,sizeof(msgRet)) ;
					for ( uint8_t nReadIdx = 0 ; nReadIdx < nSendCnt; ++nReadIdx )
					{
						CMysqlRow& pRow = *pResult->vResultRows[nPageStartIdx + nReadIdx] ;
						stSaveRoomPlayerEntry entryData ;
						entryData.nGameOffset = pRow["offsetCoin"]->IntValue();
						entryData.nOtherOffset = pRow["otherOffset"]->IntValue();
						//entryData.nPlayerTimes = pRow["playTimes"]->IntValue();
						//entryData.nWinTimes = pRow["winTimes"]->IntValue();
						entryData.nUserUID = pRow["playerUID"]->IntValue() ;
						auBuffer.addContent(&entryData,sizeof(entryData)) ;
					}
					m_pTheApp->sendMsg(pdata->nSessionID,auBuffer.getBufferPtr(),auBuffer.getContentSize() ) ;
					LOGFMTD("read room players room id = %d, page idx = %d ",msgRet.nRoomID,nPageIdx);
				}
			}
		}
		break;
	case MSG_GET_MAX_ROOM_ID:
		{
			stMsgGetMaxRoomIDRet msgBack ;
			msgBack.nMaxRoomID = 1 ;
			msgBack.cSysIdentifer = pdata->eFromPort ; // send port ;
			if ( pResult->nAffectRow > 0 )
			{
				CMysqlRow& pRow = *pResult->vResultRows[0] ;
				msgBack.nMaxRoomID = pRow["max(gameroomnew.roomID)"]->IntValue();
			}
			m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msgBack,sizeof(msgBack)) ;
			LOGFMTD("read max room id = %u",msgBack.nMaxRoomID);
		}
		break;
	case MSG_READ_NOTICE_PLAYER:
		{
			stMsgReadNoticePlayerRet msgBack ;
			memset(&msgBack.tPlayerEntery,0,sizeof(msgBack.tPlayerEntery)) ;
			msgBack.nRet = pResult->nAffectRow > 0 ? 0 : 1 ;
			msgBack.tPlayerEntery.nUserUID = pdata->nExtenArg1 ;
			if ( msgBack.nRet == 0 )
			{
				CMysqlRow& pRow = *pResult->vResultRows[0] ;
				msgBack.tPlayerEntery.nNoticeFlag = pRow["flag"]->IntValue() ;
				memcpy(msgBack.tPlayerEntery.pToken,pRow["token"]->BufferData(),32);
			}
			m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msgBack,sizeof(msgBack)) ;
		}
		break ;
	case MSG_SAVE_CREATE_ROOM_INFO:
	case MSG_SAVE_UPDATE_ROOM_INFO:
	case MSG_SAVE_REMOVE_TAXAS_ROOM_PLAYERS:
	case MSG_SAVE_TAXAS_ROOM_PLAYER:
	case MSG_SAVE_PLAYER_GAME_DATA:
	case MSG_SAVE_FRIEND_LIST:
	case MSG_PLAYER_SAVE_MAIL:
	case MSG_PLAYER_SET_MAIL_STATE:
	case MSG_SAVE_ROOM_PLAYER:
	case MSG_REMOVE_ROOM_PLAYER:
	case MSG_DELETE_ROOM:
	case MSG_SAVE_PLAYER_MONEY:
	case MSG_CIRCLE_SAVE_DELETE_TOPIC:
	case MSG_CIRCLE_SAVE_ADD_TOPIC:
	case MSG_SAVE_NOTICE_PLAYER:
	case MSG_SAVE_EXCHANGE:
	case MSG_SAVE_GAME_RESULT:
	case MSG_SAVE_PLAYER_GAME_RECORDER:
	case MSG_SAVE_PRIVATE_ROOM_PLAYER:
		{
			if ( pResult->nAffectRow <= 0 )
			{
				LOGFMTE(" db result msg id = %d , row cnt = %d, failed  ", pResult->nRequestUID,pResult->nAffectRow );
			}
		}
		break;
//	case MSG_PLAYER_SAVE_BASE_DATA:
//		{
//			if ( pResult->nAffectRow > 0 )
//			{
//				LOGFMTD("save player base data ok UID = %d",pdata->nExtenArg1) ;
//			}
//			else
//			{
//				LOGFMTD("save player base data Error UID = %d",pdata->nExtenArg1) ;
//			}
//		}
//		break;
//	case MSG_SAVE_PLAYER_COIN:
//		{
//			stMsgGameServerSavePlayerCoinRet ret ;
//			ret.nSessionID = pdata->nSessionID ;
//			if ( pResult->nAffectRow > 0 )
//			{
//				ret.nRet = 0 ;
//			}
//			else
//			{
//				ret.nRet = 1 ;
//				LOGFMTD("Save player COIN Error ! UID = %d",pdata->nExtenArg1) ;
//			}
//			m_pTheApp->SendMsg((char*)&ret,sizeof(ret),pdata->m_nReqrestFromAdd); 
//		}
//		break;
//	case MSG_SAVE_FRIEND_LIST:
//		{
//			stMsgGameServerSaveFriendListRet ret ;
//			ret.nSessionID = pdata->nSessionID ;
//			if ( pResult->nAffectRow > 0 )
//			{
//				ret.nRet = 0 ;
//			}
//			else
//			{
//				ret.nRet = 1 ;
//				LOGFMTD("Save player MSG_SAVE_FRIEND_LIST ! UID = %d",pdata->nExtenArg1) ;
//			}
//			m_pTheApp->SendMsg((char*)&ret,sizeof(ret),pdata->m_nReqrestFromAdd); 
//		}
//		break; 
//	case MSG_REQUEST_FRIEND_LIST:
//		{
//			stMsgGameServerRequestFirendListRet msgBack ;
//			msgBack.nSessionID = pdata->nSessionID ; 
//			if ( pResult->nAffectRow > 0 )
//			{
//				CMysqlRow& pRow = *pResult->vResultRows[0] ;
//				msgBack.nFriendCount = pRow["friendCount"]->IntValue();
//				if ( msgBack.nFriendCount == 0 )
//				{
//					m_pTheApp->SendMsg((char*)&msgBack,sizeof(msgBack),pdata->m_nReqrestFromAdd); 
//					break;
//				}
//
//				// parse friends ;
//				char* pBuffer = new char[msgBack.nFriendCount * sizeof(stServerSaveFrienItem) + sizeof(msgBack)] ;
//				unsigned char nOffset = 0 ;
//				memcpy(pBuffer,&msgBack,sizeof(msgBack));
//				nOffset += sizeof(msgBack);
//				memcpy(pBuffer + nOffset , pRow["contentData"]->BufferData(),pRow["contentData"]->nBufferLen);
//				nOffset += pRow["contentData"]->nBufferLen ;
//#ifdef DEBUG
//				if ( pRow["contentData"]->nBufferLen != msgBack.nFriendCount * sizeof(stServerSaveFrienItem) )
//				{
//					LOGFMTE("why save buffer and read buffer is not equal len , read friend list ?") ;
//				}
//#endif
//				// update present times info ;
//				time_t tNow = time(NULL) ;
//				time_t nLastSave = pRow["saveTime"]->IntValue();
//				struct tm tmNow = *localtime(&tNow) ;
//				struct tm tmLastSave = *localtime(&nLastSave) ;
//				if ( tmNow.tm_yday != tmLastSave.tm_yday )
//				{
//					// reset present times ;
//					char* pDBuffer = pBuffer + sizeof(msgBack);
//					stServerSaveFrienItem* pSave = (stServerSaveFrienItem*)pDBuffer;
//					int nCount = msgBack.nFriendCount ;
//					while(nCount--)
//					{
//						pSave->nPresentTimes = 0 ;
//						++pSave ;
//					}
//				}
//				m_pTheApp->SendMsg(pBuffer,nOffset,pdata->m_nReqrestFromAdd); 
//				delete[] pBuffer ;
//			}
//			else
//			{
//				msgBack.nFriendCount = 0 ;
//				m_pTheApp->SendMsg((char*)&msgBack,sizeof(msgBack),pdata->m_nReqrestFromAdd); 
//				break;
//			}
//		}
//		break;
//	case MSG_REQUEST_FRIEND_BRIFDATA_LIST:
//		{
//			stMsgGameServerRequestFriendBrifDataListRet msgRet ;
//			msgRet.nCount = pResult->nAffectRow ;
//			if ( msgRet.nCount <= 0 )
//			{
//				LOGFMTE("How can fried brif info list is NULL ?") ;
//				m_pTheApp->SendMsg((char*)&msgRet,sizeof(msgRet),pdata->m_nReqrestFromAdd); 
//				break;
//			}
//
//			char* pBuffer = new char[sizeof(msgRet) + sizeof(stPlayerBrifData) * msgRet.nCount] ;
//			memcpy(pBuffer,&msgRet,sizeof(msgRet));
//			stPlayerBrifData* pInfo = (stPlayerBrifData*)(pBuffer + sizeof(msgRet));
//			for ( int i = 0 ; i < pResult->nAffectRow ; ++i )
//			{
//				CMysqlRow& pRow = *pResult->vResultRows[i];
//				GetPlayerBrifData(pInfo,pRow) ;
//				++pInfo ;
//			}
//			m_pTheApp->SendMsg(pBuffer,sizeof(msgRet) + sizeof(stPlayerBrifData) * msgRet.nCount ,pdata->m_nReqrestFromAdd); 
//			delete[] pBuffer ;
//		}
//		break;
//	case MSG_PLAYER_SERACH_PEERS:
//		{
//			stMsgGameServerGetSearchFriendResultRet msgBack ;
//			msgBack.nSessionID = pdata->nSessionID ;
//			msgBack.nResultCount = pResult->nAffectRow ;
//			if ( msgBack.nResultCount <= 0 )
//			{
//				m_pTheApp->SendMsg((char*)&msgBack,sizeof(msgBack) ,pdata->m_nReqrestFromAdd); 
//				break;
//			}
//
//			char* pBuffer = new char[sizeof(msgBack) + sizeof(stPlayerBrifData) * msgBack.nResultCount] ;
//			memcpy(pBuffer,&msgBack,sizeof(msgBack));
//			stPlayerBrifData* pInfo = (stPlayerBrifData*)(pBuffer + sizeof(msgBack));
//			for ( int i = 0 ; i < pResult->nAffectRow ; ++i )
//			{
//				CMysqlRow& pRow = *pResult->vResultRows[i];
//				GetPlayerBrifData(pInfo,pRow) ;
//				++pInfo ;
//			}
//			m_pTheApp->SendMsg(pBuffer,sizeof(msgBack) + sizeof(stPlayerBrifData) * msgBack.nResultCount ,pdata->m_nReqrestFromAdd); 
//			delete[] pBuffer ;
//		}
//		break;
//	case MSG_PLAYER_REQUEST_FRIEND_DETAIL:
//		{
//			stMsgGameServerGetFriendDetailRet msg ;
//			msg.nSessionID = pdata->nSessionID ;
//			if ( pResult->nAffectRow <= 0 )
//			{
//				msg.nRet = 1 ;
//			}
//			else
//			{
//				msg.nRet = 0 ;
//				CMysqlRow& pRow = *pResult->vResultRows[0];
//				GetPlayerDetailData(&msg.stPeerInfo,pRow) ;
//			}
//			m_pTheApp->SendMsg((char*)&msg,sizeof(msg),pdata->m_nReqrestFromAdd); 
//		}
//		break;
//	case MSG_PLAYER_REQUEST_SEARCH_PEER_DETAIL:
//		{
//			stMsgGameServerGetSearchedPeerDetailRet msg ;
//			msg.nSessionID = pdata->nSessionID ;
//			if ( pResult->nAffectRow <= 0 )
//			{
//				msg.nRet = 1 ;
//			}
//			else
//			{
//				msg.nRet = 0 ;
//				CMysqlRow& pRow = *pResult->vResultRows[0];
//				GetPlayerDetailData(&msg.stPeerInfo,pRow) ;
//			}
//			m_pTheApp->SendMsg((char*)&msg,sizeof(msg),pdata->m_nReqrestFromAdd); 
//		}
//		break;
//	case MSG_PLAYER_SAVE_MAIL:
//		{
//			if ( pResult->nAffectRow > 0 )
//			{
//				LOGFMTD("Save Mail Success") ;
//			}
//			else
//			{
//				LOGFMTE("Save Mail Failed, UserID = %d,MainID = %d",pdata->nExtenArg1,pdata->nExtenArg2) ;
//			}
//
//		}
//		break;
//	case MSG_PLAYER_GET_MAIL_LIST:
//		{
//			stMsgGameServerGetMailListRet pSendMsg ;
//			pSendMsg.nSessionID = pdata->nSessionID ;
//			pSendMsg.nMailCount = pResult->nAffectRow ;
//			std::vector<stMail*> VecMail ;
//			unsigned short nToTalLen = sizeof(pSendMsg);
//			for ( unsigned int i = 0 ; i < pResult->nAffectRow ; ++i )
//			{
//				CMysqlRow& pRow = *pResult->vResultRows[i];
//				stMail* mail = new stMail;
//				mail->eProcessAct = (bool)pRow["processAct"]->IntValue();
//				mail->eType = (eMailType)pRow["mailType"]->IntValue();
//				mail->nContentLen = pRow["mailContent"]->nBufferLen;
//				mail->nMailUID = pRow["mailUID"]->IntValue64();
//				mail->nPostTime = pRow["postTime"]->IntValue() ;
//
//				if ( 0 == mail->nContentLen )
//				{
//					mail->pContent = NULL;
//				}
//				else
//				{
//					mail->pContent = new char[mail->nContentLen]; 
//					memcpy(mail->pContent,pRow["mailContent"]->BufferData(),mail->nContentLen);
//				}
//
//				VecMail.push_back(mail) ;
//				nToTalLen += sizeof(stMail);
//				nToTalLen += mail->nContentLen ;
//			}
//
//			char* pBuffer = new char[nToTalLen] ;
//			unsigned short nOffset = 0 ;
//			memcpy(pBuffer,&pSendMsg,sizeof(pSendMsg));
//			nOffset += sizeof(pSendMsg);
//			for ( unsigned int i = 0 ; i < VecMail.size() ; ++i )
//			{
//				stMail* sMail = VecMail[i] ;
//				memcpy(pBuffer + nOffset , sMail,sizeof(stMail));
//				nOffset += sizeof(stMail);
//
//				if ( sMail->nContentLen > 0 )
//				{
//					memcpy(pBuffer + nOffset , sMail->pContent,sMail->nContentLen);
//					nOffset += sMail->nContentLen ;
//					delete[] sMail->pContent ;
//				}
//
//				delete sMail ;
//			}
//			m_pTheApp->SendMsg(pBuffer,nOffset,pdata->m_nReqrestFromAdd); 
//			delete[] pBuffer ;
//		}
//		break;
//	case MSG_GAME_SERVER_GET_MAX_MAIL_UID:
//		{
//			stMsgGameServerGetMaxMailUIDRet msg ;
//			if ( pResult->nAffectRow == 0 )
//			{
//				msg.nMaxMailUID = 10 ;
//			}
//			else
//			{
//				msg.nMaxMailUID = pResult->vResultRows[0]->GetFiledByName("max(mail.mailUID)")->IntValue();
//			}
//			m_pTheApp->SendMsg((char*)&msg,sizeof(msg),pdata->m_nReqrestFromAdd); 
//			LOGFMTI("Cur Max MailUID = %I64d",msg.nMaxMailUID) ;
//		}
//		break;
//	case MSG_REQUEST_ITEM_LIST:
//		{
//			stMsgGameServerRequestItemListRet msg;
//			msg.nSessionID = pdata->nSessionID;
//			if ( pResult->nAffectRow <= 0 )
//			{
//				LOGFMTE("why have no item recorder ? must inster when create player ") ;
//				msg.nOwnItemKindCount = 0 ;
//				m_pTheApp->SendMsg((char*)&msg,sizeof(msg),pdata->m_nReqrestFromAdd); 
//				break;
//			}
//			else
//			{
//				CMysqlRow& pRow = *pResult->vResultRows[0] ;
//				msg.nOwnItemKindCount = pRow["ownItemsKindCount"]->IntValue();
//				char* pBuffer = new char[sizeof(msg) + pRow["itemsData"]->nBufferLen] ;
//				memcpy(pBuffer,&msg,sizeof(msg));
//				unsigned short nOffset = sizeof(msg);
//				memcpy(pBuffer + nOffset ,pRow["itemsData"]->BufferData(), pRow["itemsData"]->nBufferLen );
//				nOffset += pRow["itemsData"]->nBufferLen;
//				m_pTheApp->SendMsg(pBuffer,nOffset,pdata->m_nReqrestFromAdd); 
//				delete[] pBuffer;
//			}
//		}
//		break; 
//	case MSG_SAVE_ITEM_LIST:
//		{
//			if ( pResult->nAffectRow > 0 )
//			{
//				LOGFMTD("Save Item List successed UID = %d",pdata->nExtenArg1) ;
//			}
//			else
//			{
//				LOGFMTD("Save Item List Failed UID = %d",pdata->nExtenArg1) ;
//			}
//		}
//		break; 
//	case MSG_REQUEST_RANK:
//		{
//			stMsgGameServerRequestRankRet msg ;
//			msg.eType = pdata->nExtenArg1 ;
//			msg.nPeerCount = pResult->vResultRows.size();
//			if ( msg.nPeerCount <= 0 )
//			{
//				m_pTheApp->SendMsg((char*)&msg,sizeof(msg),pdata->m_nReqrestFromAdd); 
//				break;
//			}
//
//			char* pBuffer = new char[sizeof(msg) + msg.nPeerCount * sizeof(stServerGetRankPeerInfo)];
//			char* pBufferTemp = NULL ;
//			memcpy(pBuffer,&msg,sizeof(msg));
//			pBufferTemp = pBuffer + sizeof(msg);
//			stServerGetRankPeerInfo* pInfo = (stServerGetRankPeerInfo*)pBufferTemp ;
//			for ( int i = 0 ; i < msg.nPeerCount ; ++i )
//			{
//				CMysqlRow& pRow = *pResult->vResultRows[i] ;
//				GetPlayerDetailData(&pInfo->tDetailData,pRow);
//				pInfo->nYesterDayWin = pRow["yesterdayWinCoin"]->IntValue64() ;
//				++pInfo;
//			}
//			m_pTheApp->SendMsg(pBuffer,sizeof(msg) + msg.nPeerCount * sizeof(stServerGetRankPeerInfo) ,pdata->m_nReqrestFromAdd); 
//			delete[] pBuffer ;
//		}
//		break ;
//	//case MSG_GET_SHOP_BUY_RECORD:
//	//	{
//	//		stMsgGameServerGetShopBuyRecordRet msg ;
//	//		msg.nSessionID = pdata->nSessionID ;
//	//		if ( pResult->nAffectRow == 0 )
//	//		{
//	//			msg.nBufferLen = 0 ;
//	//			m_pTheApp->SendMsg((char*)&msg,sizeof(msg) ,pdata->m_nReqrestFromAdd); 
//	//		}
//	//		else
//	//		{
//	//			msg.nBufferLen = pResult->vResultRows[0]->GetFiledByName("pBuffer")->nBufferLen ;
//	//			char* pBuffer = new char[sizeof(msg) + msg.nBufferLen ] ;
//	//			memcpy(pBuffer,&msg,sizeof(msg));
//	//			memcpy(pBuffer + sizeof(msg),pResult->vResultRows[0]->GetFiledByName("pBuffer")->BufferData(),msg.nBufferLen );
//	//			m_pTheApp->SendMsg(pBuffer,sizeof(msg) + msg.nBufferLen ,pdata->m_nReqrestFromAdd); 
//	//			delete[] pBuffer ;
//	//		}
//	//	}
//	//	break;
//	//case MSG_SAVE_SHOP_BUY_RECORD:
//	//	{
//	//		if ( pResult->nAffectRow > 0 )
//	//		{
//	//			LOGFMTD("Save SHOP_BUY_RECORD successed UID = %d",pdata->nExtenArg1) ;
//	//		}
//	//		else
//	//		{
//	//			LOGFMTD("Save SHOP_BUY_RECORD Failed UID = %d",pdata->nExtenArg1) ;
//	//		}
//	//	}
//	//	break;
//	case MSG_GAME_SERVER_SAVE_MISSION_DATA:
//		{
//			if ( pResult->nAffectRow > 0 )
//			{
//				LOGFMTD("Save MSG_GAME_SERVER_SAVE_MISSION_DATA successed UID = %d",pdata->nExtenArg1) ;
//			}
//			else
//			{
//				LOGFMTD("Save MSG_GAME_SERVER_SAVE_MISSION_DATA Failed UID = %d",pdata->nExtenArg1) ;
//			}
//		}
//		break;
//	case MSG_GAME_SERVER_GET_MISSION_DATA:
//		{
//			stMsgGameServerGetMissionDataRet msg ;
//			msg.nSessionID = pdata->nSessionID ;
//			if ( pResult->nAffectRow <= 0 )
//			{
//				msg.nMissonCount = 0 ;
//				msg.nLastSaveTime = 0 ;
//				m_pTheApp->SendMsg((char*)&msg,sizeof(msg) ,pdata->m_nReqrestFromAdd); 
//				LOGFMTE("Mission recorder can not be null , inster one when create player ") ;
//			}
//			else
//			{
//				CMysqlRow& pRow = *pResult->vResultRows[0];
//				msg.nMissonCount = pRow["missionCount"]->IntValue() ;
//				msg.nLastSaveTime = pRow["saveTime"]->IntValue(); 
//				char* pBuffer = new char[sizeof(msg) + pRow["missionData"]->nBufferLen ] ;
//				memcpy(pBuffer,&msg,sizeof(msg));
//				memcpy(pBuffer + sizeof(msg),pRow["missionData"]->BufferData(),pRow["missionData"]->nBufferLen );
//				m_pTheApp->SendMsg(pBuffer,sizeof(msg) + pRow["missionData"]->nBufferLen ,pdata->m_nReqrestFromAdd); 
//				delete[] pBuffer ;
//			}
//		}
//		break;
	default:
		{
			if ( pResult->nAffectRow <= 0 )
			{
				LOGFMTE("unprocessed db result msg id = %d , row cnt = %d  ", pResult->nRequestUID,pResult->nAffectRow );
			}
			else
			{
				LOGFMTI("unprocessed db result msg id = %d , row cnt = %d  ", pResult->nRequestUID,pResult->nAffectRow );
			}
		}
	}
	m_vReserverArgData.push_back(pdata) ;
}

CDBManager::stArgData* CDBManager::GetReserverArgData()
{
	LIST_ARG_DATA::iterator iter = m_vReserverArgData.begin() ;
	if ( iter != m_vReserverArgData.end() )
	{
		stArgData* p = *iter ;
		m_vReserverArgData.erase(iter) ;
		p->Reset();
		return p ;
	}
	return NULL ;
}

void CDBManager::GetPlayerDetailData(stPlayerDetailData* pData, CMysqlRow&prow)
{
	GetPlayerBrifData(pData,prow) ;
	memset(pData->cSignature,0,sizeof(pData->cSignature)) ;
	sprintf_s(pData->cSignature,sizeof(pData->cSignature),"%s",prow["signature"]->CStringValue()) ;
	pData->nMostCoinEver = prow["mostCoinEver"]->IntValue64();
	pData->nCupCnt = prow["nCupCnt"]->IntValue();
	pData->dfLatidue = prow["latitude"]->FloatValue();
	pData->dfLongitude = prow["longitude"]->FloatValue();

	time_t tLastOffline = prow["offlineTime"]->IntValue();
	pData->tOfflineTime = tLastOffline ;
	
	std::vector<int> vInt ;
	vInt.clear();
	//read upload pic 
	vInt.clear();
	prow["vUploadedPic"]->VecInt(vInt);
	memset(pData->vUploadedPic,0,sizeof(pData->vUploadedPic)) ;
	LOGFMTD("vUploadedPic size = %d uid = %d",vInt.size(),pData->nUserUID ) ;
	if ( vInt.size() == MAX_UPLOAD_PIC )
	{
		for ( uint8_t nIdx = 0 ; nIdx < MAX_UPLOAD_PIC ; ++nIdx )
		{
			pData->vUploadedPic[nIdx] = vInt[nIdx] ;
		}
	}

	// read joined club ids ;
	vInt.clear();
	prow["vJoinedClubID"]->VecInt(vInt);
	memset(pData->vJoinedClubID,0,sizeof(pData->vJoinedClubID)) ;
	LOGFMTD("vJoinedClubID size = %d uid = %d",vInt.size(),pData->nUserUID ) ;
	if ( vInt.size() == MAX_JOINED_CLUB_CNT )
	{
		for ( uint8_t nIdx = 0 ; nIdx < MAX_JOINED_CLUB_CNT ; ++nIdx )
		{
			pData->vJoinedClubID[nIdx] = vInt[nIdx] ;
		}
	}

// 	time_t tNow = time(NULL) ;
// 	struct tm tLast = *localtime(&tLastOffline) ;
// 	struct tm tNowt = *localtime(&tNow) ;
// 	if ( tLast.tm_yday == tNowt.tm_yday - 1 )  // yesterday offline ;
// 	{
// 		//pData->nYesterDayPlayTimes = prow["todayPlayTimes"]->IntValue();
// 	}
// 	else if ( tLast.tm_yday < tNowt.tm_yday -1 )
// 	{
// 		//pData->nYesterDayPlayTimes = 0 ;
// 	}
}

//{
	void RET_ILSEQ() {
		//std::cout << "WRONG FROM OF THE SEQUENCE" << std::endl;
		//exit(1);
		LOGFMTE("WRONG FROM OF THE SEQUENCE");
	}


	void RET_TOOFEW() {
		//std::cout << "MISSING FROM THE SEQUENCE" << std::endl;
		//exit(1);
		LOGFMTE("MISSING FROM THE SEQUENCE");
	}

	std::vector<std::string> parse(std::string sin) {
		int l = sin.length();
		std::vector<std::string> ret;
		ret.clear();
		for (int p = 0; p < l;) {
			int size = 0, n = l - p;
			unsigned char c = sin[p], cc = sin[p + 1];
			if (c < 0x80) {
				size = 1;
			}
			else if (c < 0xc2) {
				RET_ILSEQ();
			}
			else if (c < 0xe0) {
				if (n < 2) {
					RET_TOOFEW();
				}
				if (!((sin[p + 1] ^ 0x80) < 0x40)) {
					RET_ILSEQ();
				}
				size = 2;
			}
			else if (c < 0xf0) {
				if (n < 3) {
					RET_TOOFEW();
				}
				if (!((sin[p + 1] ^ 0x80) < 0x40 &&
					(sin[p + 2] ^ 0x80) < 0x40 &&
					(c >= 0xe1 || cc >= 0xa0))) {
					RET_ILSEQ();
				}
				size = 3;
			}
			else if (c < 0xf8) {
				if (n < 4) {
					RET_TOOFEW();
				}
				if (!((sin[p + 1] ^ 0x80) < 0x40 &&
					(sin[p + 2] ^ 0x80) < 0x40 &&
					(sin[p + 3] ^ 0x80) < 0x40 &&
					(c >= 0xf1 || cc >= 0x90))) {
					RET_ILSEQ();
				}
				size = 4;
			}
			else if (c < 0xfc) {
				if (n < 5) {
					RET_TOOFEW();
				}
				if (!((sin[p + 1] ^ 0x80) < 0x40 &&
					(sin[p + 2] ^ 0x80) < 0x40 &&
					(sin[p + 3] ^ 0x80) < 0x40 &&
					(sin[p + 4] ^ 0x80) < 0x40 &&
					(c >= 0xfd || cc >= 0x88))) {
					RET_ILSEQ();
				}
				size = 5;
			}
			else if (c < 0xfe) {
				if (n < 6) {
					RET_TOOFEW();
				}
				if (!((sin[p + 1] ^ 0x80) < 0x40 &&
					(sin[p + 2] ^ 0x80) < 0x40 &&
					(sin[p + 3] ^ 0x80) < 0x40 &&
					(sin[p + 4] ^ 0x80) < 0x40 &&
					(sin[p + 5] ^ 0x80) < 0x40 &&
					(c >= 0xfd || cc >= 0x84))) {
					RET_ILSEQ();
				}
				size = 6;
			}
			else {
				RET_ILSEQ();
			}
			std::string temp = "";
			temp = sin.substr(p, size);
			ret.push_back(temp);
			p += size;
		}
		return ret;
	}

	std::string getSubStr(std::string & str, uint32_t length){
		if (str.length() > length){
			std::vector<std::string> ans = parse(str);
			if (length < ans.size()) {
				std::string returnString = "";
				for (int i = 0; i < length; i++) {
					returnString += ans[i];
				}
				return returnString;
			}
			else{
				return str.c_str();
			}
		}
		return str.c_str();
	}
//}

void CDBManager::GetPlayerBrifData(stPlayerBrifData*pData,CMysqlRow&prow)
{
	pData->bIsOnLine = false ;
	memset(pData->cName,0,sizeof(pData->cName)) ;
	if (prow["playerName"]->nBufferLen >= sizeof(pData->cName))
	{
		LOGFMTD("you name is too long");
		std::string str = prow["playerName"]->CStringValue();
		std::string strName = getSubStr(str,6);
		if (strName.size() >= sizeof(pData->cName))
		{
			LOGFMTE("name still too long");
			sprintf_s(pData->cName, sizeof(pData->cName), "%s", "Name too long");
		}
		else
		{
			sprintf_s(pData->cName, sizeof(pData->cName), "%s", strName.c_str());
		}
	}
	else
	{
		sprintf_s(pData->cName, sizeof(pData->cName), "%s", prow["playerName"]->CStringValue());
	}
	
	pData->nCoin = prow["coin"]->IntValue64();
	pData->nDiamoned = prow["diamond"]->IntValue();
	pData->nPhotoID = prow["photoID"]->IntValue();
	pData->nSex = prow["sex"]->IntValue();
	pData->nUserUID = prow["userUID"]->IntValue();
	pData->nVipLevel = prow["vipLevel"]->IntValue();
}