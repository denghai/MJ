#pragma once
#include "IPlayerComponent.h"
class CPlayerNiuNiu
	:public IPlayerComponent
{
public:
	typedef std::map<uint32_t,stMyOwnRoom> MAP_ID_MYROOW ;
public:
	CPlayerNiuNiu(CPlayer* pPlayer):IPlayerComponent(pPlayer){ m_eType = ePlayerComponent_PlayerNiuNiu ; }
	void Reset()override;
	void Init()override;
	bool OnMessage( stMsg* pMessage , eMsgPort eSenderPort)override ;
	bool onCrossServerRequest(stMsgCrossServerRequest* pRequest , eMsgPort eSenderPort,Json::Value* vJsValue = nullptr)override;
	bool onCrossServerRequestRet(stMsgCrossServerRequestRet* pResult,Json::Value* vJsValue = nullptr )override;
	void OnPlayerDisconnect()override;
	void OnOtherWillLogined()override;
	void TimerSave();
	void OnReactive(uint32_t nSessionID )override;
	void OnOtherDoLogined();
	uint32_t getCurRoomID(){ return m_nCurRoomID ;}
	void addOwnRoom(uint32_t nRoomID , uint16_t nConfigID );
	bool isCreateRoomCntReachLimit();
	bool deleteOwnRoom(uint32_t nRoomID );
	uint16_t getMyOwnRoomConfig( uint32_t nRoomID ) ;
	bool isRoomIDMyOwn(uint32_t nRoomID);
protected:
	void sendNiuNiuDataToClient();
protected:
	uint16_t m_nCurRoomID ;
	stPlayerNiuNiuData m_tData ;
	MAP_ID_MYROOW m_vMyOwnRooms;
	bool m_bDirty ;
};