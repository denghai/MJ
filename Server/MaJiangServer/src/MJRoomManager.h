#pragma once
#include "IGameRoomManager.h"
#include <memory>
#include <map>
#include <set>
#include <list>
#include "ServerMessageDefine.h"
#include "MJGoldRoom.h"

class MJRoomManager
	:public IGameRoomManager
{
public:
	struct stVipRoomBill
	{
		uint32_t nBillID;
		uint32_t nRoomID;
		uint32_t nCreateUID;
		uint32_t nRoomType;
		uint32_t nRoomInitCoin;
		uint16_t nCircleCnt;
		uint32_t nBillTime;
		Json::Value jsDetail;
	};

	struct stReqVipRoomBillPlayers
	{
		uint32_t nReqBillID;
		std::set<uint32_t> vReqPlayers;
	};

	typedef std::map<uint32_t, IGameRoom*> MAP_MJROOM;
	typedef std::vector<uint32_t> VEC_ROOMID;
	typedef std::map<uint16_t, VEC_ROOMID> MAP_CONFIG_ROOMID;

	typedef std::shared_ptr<stVipRoomBill> VIP_ROOM_BILL_SHARED_PTR;
	typedef std::map<uint32_t, std::shared_ptr<stVipRoomBill>> MAP_VIP_BILL;
public:
	~MJRoomManager();
	void init(IServerApp* svrApp)override;
	IGameRoom* getRoomByID(uint32_t nRoomID)override;
	void sendMsg(stMsg* pmsg, uint32_t nLen, uint32_t nSessionID) override;
	void sendMsg(Json::Value& jsContent, unsigned short nMsgType, uint32_t nSessionID, eMsgPort ePort = ID_MSG_PORT_CLIENT) override;
	void update(float fDeta)override;
	bool onMsg(stMsg* prealMsg, eMsgPort eSenderPort, uint32_t nSessionID)override;
	bool onMsg(Json::Value& prealMsg, uint16_t nMsgType, eMsgPort eSenderPort, uint32_t nSessionID)override;
	bool onAsyncRequest(uint16_t nRequestType, const Json::Value& jsReqContent, Json::Value& jsResult)override;
	void onConnectedSvr()override;
public:
	void addVipRoomBill(std::shared_ptr<stVipRoomBill>& pBill, bool isAddtoDB);
	bool isHaveVipRoomBill(uint32_t nVipBillID);
	std::shared_ptr<stVipRoomBill> createVipRoomBill();
	void sendVipRoomBillToPlayer(uint32_t nBillID, uint32_t nTargetSessionD);
	void addWillDeleteRoomID(uint32_t nDelRoomID );
protected:
	bool processEnterRoomMsg(stMsg* prealMsg, eMsgPort eSenderPort, uint32_t nSessionID);
	bool processEnterGoldRoomMsg(stMsg* prealMsg, eMsgPort eSenderPort, uint32_t nSessionID);
	uint32_t generateRoomID();
	//IGameRoom* randRoomToEnterByConfigID( uint32_t nRoomConfigID );
	IGameRoom* doCreatePublicRoom(Json::Value& jsArg);
	IGameRoom* doCreatePrivateRoom( uint16_t nConfigID , Json::Value& jsArg);
	uint32_t genPrivateRoomID();

	void enterWaitingQueue(stMsgSvrEnterGoldRoom* data);
	uint32_t getRoomIdByUserUID(uint32_t nUserUID);
	bool addUserToRoom(MJGoldRoom* pRoom);
	void createGoldRoomAndAddUserIn();
protected:
	MAP_MJROOM m_vRooms;
	MAP_CONFIG_ROOMID m_vPublicRooms;
	std::map<uint32_t, std::shared_ptr<stReqVipRoomBillPlayers>> m_vReqingBillInfoPlayers;

	std::map<uint32_t, uint32_t> m_mapUserUIDandRoomID;
	std::list<uint32_t> m_lstShorthandedRoomID;
	std::list<stMsgSvrEnterGoldRoom> m_lstUserEnterRoomData;
	std::list<stMsgSvrEnterGoldRoom> m_lstRobotEnterRoomData;
	time_t m_lastRoomCreatedTime;
protected:
	MAP_VIP_BILL m_vVipRoomBills;
	std::vector<uint32_t> m_vWillDeleteRoomIDs;
public:
	static uint32_t s_MaxBillID;
};