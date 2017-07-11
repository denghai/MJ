#pragma warning(disable:4800)
#include "LoginDBManager.h"
#include "DBRequest.h"
#include "ServerMessageDefine.h"
#include "LoginApp.h"
#include "DataBaseThread.h"
#include <sstream>
#include "AsyncRequestQuene.h"
#include "log4z.h"
#define PLAYER_BRIF_DATA "playerName,userUID,sex,vipLevel,defaultPhotoID,isUploadPhoto,exp,coin,diamond"
#define PLAYER_DETAIL_DATA "playerName,userUID,sex,vipLevel,defaultPhotoID,isUploadPhoto,exp,coin,diamond,signature,singleWinMost,winTimes,loseTimes,yesterdayPlayTimes,todayPlayTimes,longitude,latitude,offlineTime"
CDBManager::CDBManager()
{
	m_vReserverArgData.clear();
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

void CDBManager::Init(CLoginApp* pApp)
{
	m_pTheApp = pApp ;
	if ( MAX_LEN_ACCOUNT < 20 )
	{
		LOGFMTE("MAX_LEN_ACCOUNT must big than 18 , or guset login will crash ") ;
	}
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
	switch( pmsg->usMsgType )
	{
	case MSG_PLAYER_REGISTER:
		{
			stMsgRegister* pLoginRegister = (stMsgRegister*)pmsg ;
			pdata->nSessionID = nSessionID ;

			if ( pLoginRegister->cRegisterType == 0  )
			{
				memset(pLoginRegister->cAccount,0,sizeof(pLoginRegister->cAccount)) ;
				memset(pLoginRegister->cPassword,0,sizeof(pLoginRegister->cPassword)) ;
				memset(pLoginRegister->cName,0,sizeof(pLoginRegister->cName)) ;

				// rand a name and account 
				for ( uint8_t nIdx  = 0 ; nIdx < 8 ; ++nIdx )
				{
					char acc , cName ;
					acc = rand() % 50 ;
					if ( acc <= 25 )
					{
						acc = 'a' + acc ;
					}
					else
					{
						acc = 'A' + (acc - 25 );
					}

					cName = rand() % 50 ;
					if ( cName <= 25)
					{
						cName = 'a' + cName ;
					}
					else
					{
						cName = 'A' + (cName - 25 );
					}
					pLoginRegister->cAccount[nIdx] = acc ;
					pLoginRegister->cName[nIdx] = cName ;
				}
				memset(pLoginRegister->cName,0,sizeof(pLoginRegister->cName));
				sprintf_s(pLoginRegister->cName, "guest%u",rand() % 10000 + 1 );
				sprintf_s(pLoginRegister->cPassword,"hello");
			}
			
			pdata->nExtenArg1 = pLoginRegister->cRegisterType ;
			if ( strlen(pLoginRegister->cAccount) >= MAX_LEN_ACCOUNT || strlen(pLoginRegister->cPassword) >= MAX_LEN_PASSWORD )
			{
				LOGFMTE("pLoginRegister password or account len is too long ");
				m_vReserverArgData.push_back(pdata) ;
				break; 
			}

			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
			pRequest->cOrder = eReq_Order_Super ;
			pRequest->eType = eRequestType_Select ;
			pRequest->nRequestUID = pmsg->usMsgType ;
			pRequest->pUserData = pdata ;

			auto strName = checkString(pLoginRegister->cName);
			auto strAccount = checkString(pLoginRegister->cAccount);
			auto strPassword = checkString(pLoginRegister->cPassword);
			// format sql String ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"call RegisterAccountNew('%s','%s','%s',%d,%d);",strName.c_str(),pLoginRegister->cAccount,pLoginRegister->cPassword,pLoginRegister->cRegisterType,pLoginRegister->nChannel ) ;
			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
		}
		break;
	case MSG_PLAYER_LOGIN:
		{
			stMsgLogin* pLoginCheck = (stMsgLogin*)pmsg ;
			pdata->nSessionID = nSessionID ;
			// must end with \0
			if ( strlen(pLoginCheck->cAccount) >= MAX_LEN_ACCOUNT || strlen(pLoginCheck->cPassword) >= MAX_LEN_PASSWORD )
			{
				LOGFMTE("password or account len is too long ");
				m_vReserverArgData.push_back(pdata) ;
				break; 
			}

			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
			pRequest->cOrder = eReq_Order_High ;
			pRequest->eType = eRequestType_Select ;
			pRequest->nRequestUID = pmsg->usMsgType ;
			pRequest->pUserData = pdata ;
			auto strAccount = checkString(pLoginCheck->cAccount);
			auto strPass = checkString(pLoginCheck->cPassword) ;
			// format sql String ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"call CheckAccount('%s','%s')",strAccount.c_str(),strPass.c_str()) ;
			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
		}
		break;
	case MSG_PLAYER_BIND_ACCOUNT:
		{
			stMsgRebindAccount* pMsgRet = (stMsgRebindAccount*)pmsg ;
			pdata->nSessionID = nSessionID ;
			pdata->nExtenArg1 = pMsgRet->nCurUserUID ;
			if ( strlen(pMsgRet->cAccount) >= MAX_LEN_ACCOUNT || strlen(pMsgRet->cPassword) >= MAX_LEN_PASSWORD )
			{
				LOGFMTE("MSG_PLAYER_BIND_ACCOUNT password or account len is too long ");
				m_vReserverArgData.push_back(pdata) ;
				break; 
			}

			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
			pRequest->cOrder = eReq_Order_Super ;
			pRequest->eType = eRequestType_Select ;
			pRequest->nRequestUID = pmsg->usMsgType ;
			pRequest->pUserData = pdata;
			auto strAccount = checkString(pMsgRet->cAccount);
			auto strPas = checkString(pMsgRet->cPassword);
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"call RebindAccount(%d,'%s','%s')",pMsgRet->nCurUserUID,strAccount.c_str(),strPas.c_str() ) ;
			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
		}
		break;
	case MSG_MODIFY_PASSWORD:
		{
			stMsgModifyPassword* pMsgRet = (stMsgModifyPassword*)pmsg ;
			pdata->nSessionID = nSessionID ;
			pdata->nExtenArg1 = pMsgRet->nUserUID ;
			if ( strlen(pMsgRet->cOldPassword) >= MAX_LEN_PASSWORD || strlen(pMsgRet->cNewPassword) >= MAX_LEN_PASSWORD )
			{
				LOGFMTE("MSG_MODIFY_PASSWORD password or account len is too long ");
				m_vReserverArgData.push_back(pdata) ;
				break; 
			}

			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
			pRequest->cOrder = eReq_Order_Super ;
			pRequest->eType = eRequestType_Select ;
			pRequest->nRequestUID = pmsg->usMsgType ;
			pRequest->pUserData = pdata;

			auto strOld = checkString(pMsgRet->cOldPassword) ;
			auto strNew = checkString(pMsgRet->cNewPassword) ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"call ModifyPassword(%d,'%s','%s')",pMsgRet->nUserUID,strOld.c_str(),strNew.c_str()) ;
			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
		}
		break;
	case MSG_RESET_PASSWORD:
		{
			stMsgResetPassword* pMsgRet = (stMsgResetPassword*)pmsg ;
			pdata->nSessionID = nSessionID ;
			if ( strlen(pMsgRet->cAccount) >= MAX_LEN_ACCOUNT || strlen(pMsgRet->cNewPassword) >= MAX_LEN_PASSWORD )
			{
				LOGFMTE("MSG_MODIFY_PASSWORD password or account len is too long ");
				m_vReserverArgData.push_back(pdata) ;
				break; 
			}

			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
			pRequest->cOrder = eReq_Order_Super ;
			pRequest->eType = eRequestType_Select ;
			pRequest->nRequestUID = pmsg->usMsgType ;
			pRequest->pUserData = pdata;
			auto strAcnt = checkString(pMsgRet->cAccount) ;
			auto strPas = checkString(pMsgRet->cNewPassword) ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"call ResetPassword('%s','%s')",strAcnt.c_str(),strPas.c_str() ) ;
			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
		}
		break;
	default:
		{
			m_vReserverArgData.push_back(pdata) ;
			LOGFMTE("unknown msg type = %d",pmsg->usMsgType ) ;
		}
	}
}

void CDBManager::OnMessage( Json::Value& recvValue , uint16_t nmsgType, eMsgPort eSenderPort , uint32_t nSessionID )
{
	if ( nmsgType == MSG_CHECK_REG_ACCOUNT )
	{
		//recvValue["ret"] = 0 ;
		//m_pTheApp->sendMsg(nSessionID,recvValue,MSG_CHECK_REG_ACCOUNT) ;
		//LOGFMTI("replay check ok") ;
		//return ;

		auto pAsy = m_pTheApp->getAsynReqQueue();
		Json::Value jsmsg ;
		std::ostringstream ss ;
		ss << "select userUID from account where account = '" << recvValue["account"].asString() << "' ;" ; 
		jsmsg["sql"] = ss.str();
		Json::Value jsUserData ;
		jsUserData["sessionID"] = nSessionID ;
		jsUserData["account"] = recvValue["account"] ;
		pAsy->pushAsyncRequest(ID_MSG_PORT_DB,eAsync_DB_Select,jsmsg,[this]( uint16_t nReqType ,const Json::Value& retContent,Json::Value& jsUserData )
		{
			uint32_t nCnt = retContent["afctRow"].asUInt();
			uint32_t nSessionID = jsUserData["sessionID"].asUInt();
			Json::Value jsMsg ;
			jsMsg["ret"] = nCnt ;
			jsMsg["account"] = jsUserData["account"] ;
			m_pTheApp->sendMsg(nSessionID,jsMsg,MSG_CHECK_REG_ACCOUNT) ;
			LOGFMTI("replay check ok") ;
		} ,jsUserData) ;
	}
}

void CDBManager::OnDBResult(stDBResult* pResult)
{
	stArgData*pdata = (stArgData*)pResult->pUserData ;
	switch ( pResult->nRequestUID )
	{
	case  MSG_PLAYER_REGISTER:
		{
			stMsgRegisterRet msgRet ;
			msgRet.cRegisterType = pdata->nExtenArg1 ;
			memset(msgRet.cAccount,0,sizeof(msgRet.cAccount));
			memset(msgRet.cPassword,0,sizeof(msgRet.cPassword)) ;
			msgRet.nUserID = 0 ;
			if ( pResult->nAffectRow <= 0 )
			{
				msgRet.nRet = 1 ;
				m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msgRet,sizeof(msgRet));
				LOGFMTE("why register affect row = 0 ") ;
				return ;
			}

			CMysqlRow& pRow = *pResult->vResultRows.front() ;
			 msgRet.nRet = pRow["nOutRet"]->IntValue();
			 if ( msgRet.nRet != 0 )
			 {
				 m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msgRet,sizeof(msgRet));
				 LOGFMTD("register failed duplicate account = %s",pRow["strAccount"]->CStringValue() );
				 return ;
			 }

			sprintf_s(msgRet.cAccount,"%s",pRow["strAccount"]->CStringValue());
			sprintf_s(msgRet.cPassword,"%s",pRow["strPassword"]->CStringValue());
			msgRet.nUserID = pRow["nOutUserUID"]->IntValue();

			// request db to create new player data 
			//stMsgRequestDBCreatePlayerData msgCreateData ;
			//msgCreateData.nUserUID = msgRet.nUserID ;
			//msgCreateData.isRegister = msgRet.cRegisterType != 0 ;
			//m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msgCreateData,sizeof(msgCreateData)) ;

			// tell client the success register result ;
			m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msgRet,sizeof(msgRet));
			LOGFMTD("register success account = %s",pRow["strAccount"]->CStringValue() );

			stMsgLoginSvrInformGateSaveLog msglog ;
			msglog.nlogType = eLog_Register ;
			msglog.nUserUID = msgRet.nUserID  ;
			m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msglog,sizeof(msglog)) ;
		}
		break;
	case MSG_PLAYER_LOGIN:
		{
			stMsgLoginRet msgRet ;
			msgRet.nAccountType = 0 ;
			uint32_t nUserUID = 0 ;
			if ( pResult->nAffectRow > 0 )
			{
				CMysqlRow& pRow = *pResult->vResultRows.front() ;
				msgRet.nRet = pRow["nOutRet"]->IntValue() ;
				msgRet.nAccountType = pRow["nOutRegisterType"]->IntValue() ;
				nUserUID = pRow["nOutUID"]->IntValue() ;
				LOGFMTD("check accout = %s  ret = %d",pRow["strAccount"]->CStringValue(),msgRet.nRet  ) ;
			}
			else
			{
				msgRet.nRet = 1 ;  // account error ;   
				LOGFMTE("check account  why affect row = 0 ? ") ;
			}
			m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msgRet,sizeof(msgRet) ) ;
			// tell data svr login success 
			if (msgRet.nRet == 0 )
			{
				stMsgOnPlayerLogin msgData ;
				msgData.nUserUID =  nUserUID;
				m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msgData,sizeof(msgData) ) ;

				stMsgLoginSvrInformGateSaveLog msglog ;
				msglog.nlogType = eLog_Login ;
				msglog.nUserUID = nUserUID ;
				m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msglog,sizeof(msglog)) ;
			}
		}
		break;
	case MSG_PLAYER_BIND_ACCOUNT:
		{
			stMsgRebindAccountRet msgBack ;
			msgBack.nRet = 0 ;
			if ( pResult->nAffectRow > 0 )
			{
				CMysqlRow& pRow = *pResult->vResultRows.front() ;
				msgBack.nRet = pRow["nOutRet"]->IntValue() ;
			}
			else
			{
				msgBack.nRet = 3 ;
				LOGFMTE("uid = %d ,bind account error db ",pdata->nExtenArg1) ;
			}
			m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msgBack,sizeof(msgBack)); 
			LOGFMTD("rebind account ret = %d , userUID = %d",msgBack.nRet,pdata->nExtenArg1 ) ;

			if ( msgBack.nRet == 0 )
			{
				stMsgOnPlayerBindAccount msgInfom ;
				m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msgInfom,sizeof(msgInfom)); 

				stMsgLoginSvrInformGateSaveLog msglog ;
				msglog.nlogType = eLog_BindAccount ;
				msglog.nUserUID = pdata->nExtenArg1 ;
				m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msglog,sizeof(msglog)) ;
			}
		}
		break;
	case MSG_MODIFY_PASSWORD:
		{
			stMsgModifyPasswordRet msgBack ;
			msgBack.nRet = 0 ;
			if ( pResult->nAffectRow > 0 )
			{
				CMysqlRow& pRow = *pResult->vResultRows.front() ;
				msgBack.nRet = pRow["nOutRet"]->IntValue() ;
			}
			else
			{
				msgBack.nRet = 3 ;
				LOGFMTE("uid = %d , modify password error db ",pdata->nExtenArg1) ;
			}
			LOGFMTD("uid = %d modify password ret = %d",pdata->nExtenArg1,msgBack.nRet ) ;
			m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msgBack,sizeof(msgBack)); 

			if ( msgBack.nRet == 0 )
			{
				stMsgLoginSvrInformGateSaveLog msglog ;
				msglog.nlogType = eLog_ModifyPwd ;
				msglog.nUserUID = pdata->nExtenArg1 ;
				m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msglog,sizeof(msglog)) ;
			}
		}
		break;
	case MSG_RESET_PASSWORD:
		{
			stMsgResetPasswordRet msgBack ;
			msgBack.nRet = 0 ;
			uint32_t nUID = 0 ;
			if ( pResult->nAffectRow > 0 )
			{
				CMysqlRow& pRow = *pResult->vResultRows.front() ;
				msgBack.nRet = pRow["nOutRet"]->IntValue() ;
				nUID = pRow["nUID"]->IntValue() ;
			}
			else
			{
				msgBack.nRet = 1 ;
			}
			LOGFMTD("uid = %d modify password ret = %d",nUID,msgBack.nRet ) ;
			m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msgBack,sizeof(msgBack)); 

			if ( msgBack.nRet == 0 )
			{
				stMsgLoginSvrInformGateSaveLog msglog ;
				msglog.nlogType = eLog_ResetPassword ;
				msglog.nUserUID = nUID ;
				m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msglog,sizeof(msglog)) ;
			}
		}
		break;
	default:
		{
			LOGFMTE("unprocessed login db result msg id = %d ", pResult->nRequestUID );
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

std::vector<std::string> parse(std::string sin, bool& bValid ) {
	int l = sin.length();
	std::vector<std::string> ret;
	ret.clear();
	bValid = true;
	for (int p = 0; p < l;) {
		int size = 0, n = l - p;
		unsigned char c = sin[p], cc = sin[p + 1];
		if (c < 0x80) {
			size = 1;
		}
		else if (c < 0xc2) {
			RET_ILSEQ();
			bValid = false;
			break;
		}
		else if (c < 0xe0) {
			if (n < 2) {
				RET_TOOFEW();
				bValid = false;
				break;
			}
			if (!((sin[p + 1] ^ 0x80) < 0x40)) {
				RET_ILSEQ();
				bValid = false;
				break;
			}
			size = 2;
		}
		else if (c < 0xf0) {
			if (n < 3) {
				RET_TOOFEW();
				bValid = false;
				break;
			}
			if (!((sin[p + 1] ^ 0x80) < 0x40 &&
				(sin[p + 2] ^ 0x80) < 0x40 &&
				(c >= 0xe1 || cc >= 0xa0))) {
				RET_ILSEQ();
				bValid = false;
				break;
			}
			size = 3;
		}
		else if (c < 0xf8) {
			if (n < 4) {
				RET_TOOFEW();
				bValid = false;
				break;
			}
			if (!((sin[p + 1] ^ 0x80) < 0x40 &&
				(sin[p + 2] ^ 0x80) < 0x40 &&
				(sin[p + 3] ^ 0x80) < 0x40 &&
				(c >= 0xf1 || cc >= 0x90))) {
				RET_ILSEQ();
				bValid = false;
				break;
			}
			size = 4;
		}
		else if (c < 0xfc) {
			if (n < 5) {
				RET_TOOFEW();
				bValid = false;
				break;
			}
			if (!((sin[p + 1] ^ 0x80) < 0x40 &&
				(sin[p + 2] ^ 0x80) < 0x40 &&
				(sin[p + 3] ^ 0x80) < 0x40 &&
				(sin[p + 4] ^ 0x80) < 0x40 &&
				(c >= 0xfd || cc >= 0x88))) {
				RET_ILSEQ();
				bValid = false;
				break;
			}
			size = 5;
		}
		else if (c < 0xfe) {
			if (n < 6) {
				RET_TOOFEW();
				bValid = false;
				break;
			}
			if (!((sin[p + 1] ^ 0x80) < 0x40 &&
				(sin[p + 2] ^ 0x80) < 0x40 &&
				(sin[p + 3] ^ 0x80) < 0x40 &&
				(sin[p + 4] ^ 0x80) < 0x40 &&
				(sin[p + 5] ^ 0x80) < 0x40 &&
				(c >= 0xfd || cc >= 0x84))) {
				RET_ILSEQ();
				bValid = false;
				break;
			}
			size = 6;
		}
		else {
			RET_ILSEQ();
			bValid = false;
			break;
		}
		std::string temp = "";
		temp = sin.substr(p, size);
		ret.push_back(temp);
		p += size;
	}
	return ret;
}

std::string CDBManager::checkString(const char* pstr)
{
	bool bValid = false;
	std::vector<std::string> strArray = parse(pstr, bValid);
	if ( false == bValid)
	{
		LOGFMTE("error invlid name str %s",pstr);
		std::string strTemp = "";
		char pBuffer[200] = { 0 };
		sprintf_s(pBuffer,sizeof(pBuffer) ,"guest%u", rand() % 10000 + 1);
		strTemp = pBuffer;
		return strTemp;
	}

	std::string strout;
	for (auto& ref : strArray)
	{
		strout += ref;
		if (ref == "'")
		{
			strout += ref;
		}
	}

	return strout;

	//uint16_t nLen = strlen(pstr)*2 ;
	//char* pBuffer = new char[nLen] ;
	//memset(pBuffer,0,nLen);
	//uint16_t nWriteIdx = 0 ;
	//while (*pstr)
	//{
	//	pBuffer[nWriteIdx++] = *pstr;
	//	if ( *pstr == '\'' )
	//	{
	//		pBuffer[nWriteIdx++] = '\'';
	//	}
	//	++pstr ;
	//}
	//std::string str = pBuffer ;
	//return str ;
}

