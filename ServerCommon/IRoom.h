#pragma once
#include "IRoomInterface.h"
#include "CardPoker.h"
#include <list>
#include <map>
#include "CommonDefine.h"
#include "ServerDefine.h"
#include <vector>
#include <functional>
class IRoomState ;
class IRoomPlayer ;
struct stMsg ;
struct stBaseRoomConfig ;
class IRoomDelegate ;
namespace Json
{
	class Value ;
};

class IRoom
	:public IRoomInterface
{
public:
	struct stStandPlayer
		:public stEnterRoomData
	{
		uint32_t nWinTimes ;
		uint32_t nPlayerTimes ;
		uint32_t nSingleWinMost ;
		int32_t nGameOffset ;
	};

public:
	typedef std::list<stStandPlayer*> LIST_STAND_PLAYER ;
	typedef std::map<uint32_t,stStandPlayer*> MAP_UID_STAND_PLAYER ;
	typedef std::map<uint16_t,IRoomState*>	MAP_ID_ROOM_STATE;
	typedef MAP_UID_STAND_PLAYER::iterator STAND_PLAYER_ITER ;
public:
	IRoom();
	virtual ~IRoom();
	bool onFirstBeCreated(IRoomManager* pRoomMgr,uint32_t nRoomID, const Json::Value& vJsValue )override;
	void serializationFromDB(IRoomManager* pRoomMgr,stBaseRoomConfig* pConfig,uint32_t nRoomID , Json::Value& vJsValue )override;
	virtual void willSerializtionToDB(Json::Value& vOutJsValue);
	virtual void prepareState();
	void serializationToDB()final;
	uint32_t getRoomID()final;
	void update(float fDelta)override;
	void setDelegate(IRoomDelegate* pDelegate ){ m_pDelegate = pDelegate ; }
	IRoomDelegate* getDelegate(){ return m_pDelegate ;}
	bool onPlayerApplyLeaveRoom(uint32_t nUserUID )override ;
	void deleteRoom()final{}
	uint32_t getOwnerUID()final{ return 0 ; }

	// event function 
	uint8_t canPlayerEnterRoom( stEnterRoomData* pEnterRoomPlayer )override;  // return 0 means ok ;
	void onPlayerEnterRoom(stEnterRoomData* pEnterRoomPlayer,int8_t& nSubIdx )override;;
	virtual void onPlayerWillLeaveRoom(stStandPlayer* pPlayer );
	virtual bool canStartGame() ;
	virtual void prepareCards() = 0 ;
	virtual void doProcessNewPlayerHalo() = 0 ;
	void roomItemDetailVisitor(Json::Value& vOutJsValue)override;
	void playerDoLeaveRoom(stStandPlayer* pp );
	void setDeskFee(uint32_t nDeskFee){ m_nDeskFree = nDeskFee ;}
	uint32_t getDeskFee(){ return m_nDeskFree ;}
	bool isHaveRealPlayer(); // not robot 
	bool isPlaying()override ;
private:
	bool addRoomPlayer(stStandPlayer* pPlayer );
	void removePlayer(stStandPlayer* pPlayer );
public:
	stStandPlayer* getPlayerByUserUID(uint32_t nUserUID );
	stStandPlayer* getPlayerBySessionID(uint32_t nSessionID );
	bool isPlayerInRoom(stStandPlayer* pPlayer );
	bool isPlayerInRoomWithSessionID(uint32_t nSessioID );
	bool isPlayerInRoomWithUserUID(uint32_t nUserUID );
	uint16_t getPlayerCount();
	STAND_PLAYER_ITER beginIterForPlayers();
	STAND_PLAYER_ITER endIterForPlayers();

	void sendRoomMsg( stMsg* pmsg , uint16_t nLen );
	void sendMsgToPlayer( stMsg* pmsg , uint16_t nLen , uint32_t nSessionID ) ;
	void sendRoomMsg( Json::Value& recvValue, uint16_t nMsgID );
	void sendMsgToPlayer( uint32_t nSessionID , Json::Value& recvValue, uint16_t nMsgID  ) ;

	bool onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID )override;
	bool onMessage( Json::Value& prealMsg ,uint16_t nMsgType, eMsgPort eSenderPort , uint32_t nSessionID )override;
	virtual void roomInfoVisitor(Json::Value& vOutJsValue) = 0 ;
	virtual void sendRoomPlayersInfo(uint32_t nSessionID) = 0 ;
	virtual void onGameWillBegin(){}
	virtual void onGameDidEnd(){}

	void onTimeSave()override;;
	void goToState(IRoomState* pTargetState );
	void goToState( uint16_t nStateID );
	void setInitState(IRoomState* pDefaultState );
	IRoomState* getCurRoomState();
	IRoomState* getRoomStateByID(uint16_t nStateID );
	CPoker* getPoker(){ return &m_tPoker ; }

	// room attribute
	uint32_t getTotalProfit(){ return m_nTotalProfit ;}
	void addTotoalProfit(uint32_t nAdd ){ m_nTotalProfit += nAdd ; m_bRoomInfoDiry = true; }
	void setTotalProfit( uint32_t nProfit ){ m_nTotalProfit = nProfit ;m_bRoomInfoDiry = true; }
	bool isRoomInfoDirty(){ return m_bRoomInfoDiry  ; }
	void setRoomInfoDirty( bool isDirty ){ m_bRoomInfoDiry = isDirty;}
	void setChatRoomID(uint32_t nChatRoomID );
	uint32_t getChatRoomID(){ return m_nChatRoomID ; }
	bool isDeleteRoom()override;
	void forcePlayersLeaveRoom();
	float getChouShuiRate(){ return m_fDividFeeRate ;}
	void setChouShuiRate(float fDividFeeRate ){ m_fDividFeeRate = fDividFeeRate ; }
	void enumAudientsPlayer(std::function<void (stStandPlayer*)> lpFunc );
	virtual void sendResultToPlayerWhenDuringResultState(uint32_t nSessionID) { }
protected:
	bool addRoomState(IRoomState* pRoomState );
private:
	IRoomManager* m_pRoomMgr ; 
	IRoomDelegate* m_pDelegate ;
	bool m_bRoomInfoDiry ;

	uint32_t m_nRoomID ;
	LIST_STAND_PLAYER m_vReseverPlayerObjects;

	MAP_UID_STAND_PLAYER m_vInRoomPlayers ;
	IRoomState* m_pCurRoomState ;
	MAP_ID_ROOM_STATE m_vRoomStates ;

	CPoker m_tPoker ;

	uint32_t m_nTotalProfit ;
	uint32_t m_nChatRoomID ;

	uint32_t m_nDeskFree ;
	float m_fDividFeeRate;
};