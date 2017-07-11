#pragma once
#include "ITask.h"
#include "my_global.h"
#include "mysql.h"
struct stDBResult ;
struct stDBRequest ;
class CDBTask
	:public ITask
{
public:
	typedef std::shared_ptr<stDBResult> DBResult_ptr ;
	typedef std::shared_ptr<stDBRequest> DBRequest_ptr ;
public:
	CDBTask( uint32_t nTaskID,const char* pIP,unsigned pPort , const char* pUserName,const char* pPassword, const char* pDBName ) ;
	~CDBTask();
	uint8_t performTask()override ;
	void setDBRequest( DBRequest_ptr ptr );
	DBRequest_ptr getDBRequest();
	DBResult_ptr getDBResult();
protected:
	bool setupMysqlConnection();
	uint8_t doRequest(DBRequest_ptr ptr );
protected:
	std::string m_strIP ;
	uint16_t m_nPort ;
	std::string m_strUserName ;
	std::string m_strPassword ;
	std::string m_strDBName ;

	MYSQL* m_pMySql ;
	time_t m_tNextMysqlPingTime;
	DBResult_ptr m_pResult ;
	DBRequest_ptr m_pRequest ;
};