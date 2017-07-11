#pragma once
#include "ServerMessageDefine.h"
#include <map>
#include <json/json.h>
#include "httpRequest.h"
#include "IGlobalModule.h"
#include <list>
#include "Timer.h"
class CRoomConfigMgr ;
class IRoomInterface ;
class IRoom ;

struct stPrivateRoomRecorder
{
	uint32_t nSieralNum;
	uint32_t nRoomID ;
	uint32_t nCreaterUID ;
	uint16_t nConfigID ;
	time_t nTime ;
	uint32_t nDuringSeconds ;
	Json::Value playerDetail ;
};

class IRoomManager
	:public CHttpRequestDelegate
	,public IGlobalModule
{
public:
	enum eHttpReq
	{
		eHttpReq_CreateChatRoom,
		eHttpReq_ChatRoomList,
		eHttpReq_Token,
		eHttpReq_DeleteChatRoomID,
		eHttpReq_Max,
	};

	typedef std::list<IRoomInterface*> LIST_ROOM ;
	typedef std::map<uint32_t, IRoomInterface*> MAP_ID_ROOM;
	struct stRoomCreatorInfo
	{
		uint32_t nPlayerUID ;
		LIST_ROOM vRooms ;
	};
	typedef std::map<uint32_t,stRoomCreatorInfo> MAP_UID_CR;
public:
	IRoomManager(CRoomConfigMgr* pConfigMgr);
	~IRoomManager();
	void init( IServerApp* svrApp )override; 
	bool onMsg(stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID)override;
	bool onMsg(Json::Value& prealMsg ,uint16_t nMsgType, eMsgPort eSenderPort , uint32_t nSessionID)override ;
	virtual bool onPublicMsg(stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID);
	IRoomInterface* GetRoomByID(uint32_t nRoomID );
	void sendMsg(stMsg* pmsg, uint32_t nLen , uint32_t nSessionID ) ;
	bool sendMsg( uint32_t nSessionID , Json::Value& recvValue, uint16_t nMsgID = 0 ,uint8_t nTargetPort = ID_MSG_PORT_CLIENT );
	void onHttpCallBack(char* pResultData, size_t nDatalen , void* pUserData , size_t nUserTypeArg);
	void update(float fDeta )override;
	void onTimeSave()override;
	void onConnectedSvr()override;
	bool reqeustChatRoomID(IRoom* pRoom);
	void deleteRoomChatID(uint32_t nChatID );
	void addPrivateRoomRecorder( stPrivateRoomRecorder* pRecorder, bool isSaveDB = true );
	bool onAsyncRequest(uint16_t nRequestType , const Json::Value& jsReqContent, Json::Value& jsResult )override ;
	//bool requestChatRoomIDList();
	//bool requestGotypeToken();
	void onExit()override ;
	//float getTimeSave()override{ return 10; }
protected:
	virtual bool onCrossServerRequest(stMsgCrossServerRequest* pRequest , eMsgPort eSenderPort,Json::Value* vJsValue = nullptr);
	virtual IRoomInterface* doCreateRoomObject( eRoomType cRoomType,bool isPrivateRoom ) = 0 ;
	virtual IRoomInterface* doCreateInitedRoomObject(uint32_t nRoomID,const Json::Value& vJsValue) = 0 ;
	void removeRoom(IRoomInterface* pRoom );
	virtual eRoomType getMgrRoomType() = 0 ;
	void readRoomSerails();;
	void readRoomInfo(uint32_t nSeailNum, uint32_t nChatRoomID );
protected:
	MAP_ID_ROOM m_vRooms ;

	CHttpRequest m_pGoTyeAPI;
	CRoomConfigMgr* m_pConfigMgr ;
	CTimer m_tReaderRoomSerials ;

	CTimer m_tRequestGoTyeToken ;

	std::map<uint32_t,stPrivateRoomRecorder*> m_mapPrivateRecorder ;
	std::map<uint32_t, stPrivateRoomRecorder*> m_mapSieralPrivateRecorder;
	//eOperateStage m_eRequestChatRoomIDs ;
};