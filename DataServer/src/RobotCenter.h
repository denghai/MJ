#pragma once
#pragma once
#include "MessageDefine.h"
#include "IGlobalModule.h"
#include <list>
class CRobotCenter
	:public IGlobalModule
{
public:
	enum 
	{
		eModule_Type = 101,
	};
public:
	struct stIdleRobot
	{
		uint32_t nUserUID ;
		uint32_t nSessionID ;
		uint8_t nLevel ;
	};

	struct stRequestRobotCmd
	{
		uint32_t nRoomID ;
		uint16_t nSubRoomIdx ;
		uint8_t nRoomType ;
		uint8_t nReqRobotLevel ;
		uint8_t nReqCnt ;
	};

	typedef std::list<stIdleRobot*> LIST_ROBOT ;
	typedef std::list<stRequestRobotCmd*>  LIST_REQ_ROBOT_CMD ;
public:
	CRobotCenter();
	~CRobotCenter() ;
	uint16_t getModuleType(){ return eModule_Type ; }
	bool onMsg(stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID)override ;
	void onRobotDisconnect(uint32_t nUID);
	void onRobotOtherLogin(uint32_t nUID , uint32_t nNewSessionID );
	bool isRobotInTheList(uint32_t nUID );
	void processRobotReq();
protected:
	LIST_ROBOT m_vIdleRobots ;
	LIST_REQ_ROBOT_CMD m_vReqRobotCmdCacher ;
};