#pragma once 
#ifndef Max_Sql_String
#define Max_Sql_String (1024*26) 
#endif 
#include "MySqlData.h"
#include <vector>
#include <list>
#include "mutex.h"
enum eDBRequestType
{
	eRequestType_Add,
	eRequestType_Delete,
	eRequestType_Update,
	eRequestType_Select,
	eRequestType_Max,
};

enum eRequestOrder
{
	eReq_Order_Low,
	eReq_Order_Normal,
	eReq_Order_High,
	eReq_Order_Super,
	eReq_Order_Max,
};

struct stDBRequest
{
	eDBRequestType eType ;
	unsigned int nRequestUID ;  // maybe msg id  ;
	char cOrder ;              // big order processed first ;  then first come , first out ;
	char pSqlBuffer[Max_Sql_String];
	int nSqlBufferLen ;
	void* pUserData ;
protected:
	stDBRequest(){ cOrder = 0 ;}
	friend class CDBRequestQueue;
};

struct stDBResult
{
public:
	typedef std::vector<CMysqlRow*> VEC_MYSQLROW ;
	~stDBResult();
public:
	unsigned int nRequestUID ;
	VEC_MYSQLROW vResultRows ;  
	unsigned int nAffectRow ;
	void* pUserData ;
};

class CDBRequestQueue
{
public:
	typedef std::list<stDBRequest*> VEC_DBREQUEST ;
	typedef std::list<stDBResult*> VEC_DBRESULT ;
public:
	static CDBRequestQueue* SharedDBRequestQueue();
	CDBRequestQueue();
	~CDBRequestQueue();
	void PushRequest(stDBRequest* request );
	void PushResult(VEC_DBRESULT& result );
	void GetAllResult(VEC_DBRESULT& vAllReslutOut ) ; // delete stDBreslut after used ;
	void GetAllRequest(VEC_DBREQUEST& vAllRequestOut ); // push reserver after used DBRequest ;
	void PushReserveRequest(VEC_DBREQUEST& request );  // stDBRequest 对象使用完　不要delete ，而是push进来。thread safe ;
	stDBRequest* GetReserveRequest(); //  stDBReuest 不能直接new， 要通过此函数获取； thread safe ;
protected:
	void ClearAllRequest();
	void ClearAllResult();
	void ClearAllReserveRequest();
protected:
	Mutex mRequestLock ;
	Mutex mResultLock ;
	Mutex mReserveQuestLock ;
	VEC_DBREQUEST m_vReserveRequest ;
	VEC_DBREQUEST m_vAllRequest ;
	VEC_DBRESULT m_vAllResult ;
};