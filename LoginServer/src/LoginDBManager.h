#pragma once
#include "ServerMessageDefine.h"
#include <list>
#include <map>
#include <assert.h>
#include "json/json.h"
struct stDBResult ;
class CLoginApp;
class CDBManager
{
public:
	struct stArgData
	{
		eMsgPort eFromPort;
		unsigned int nSessionID ; // always , refer to client session with serveper , used in GameServer and LoginServer , to rresent a Player ;
		unsigned int nExtenArg1 ; // reserver argument , for later use ;
		unsigned int nExtenArg2 ; // reserver argument , for later use ;
		void* pUserData ;   // maybe need data ;
		stArgData(){ nSessionID = 0 ; nExtenArg2 = nExtenArg1 = 0 ; pUserData = NULL ;}
		void Reset(){nSessionID = 0 ; nExtenArg2 = nExtenArg1 = 0 ; assert(pUserData==NULL);}
	};
	typedef std::list<stArgData*> LIST_ARG_DATA ;
public:
	CDBManager();
	~CDBManager();
	void Init(CLoginApp* pApp );
	void OnMessage(stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID );
	void OnMessage( Json::Value& recvValue , uint16_t nmsgType, eMsgPort eSenderPort , uint32_t nSessionID );
	void OnDBResult(stDBResult* pResult);
	stArgData* GetReserverArgData();
	std::string checkString(const char* pstr);
protected:
	LIST_ARG_DATA m_vReserverArgData ;
	CLoginApp* m_pTheApp ;
};