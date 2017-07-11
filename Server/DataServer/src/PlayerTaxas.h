#pragma once
#include "IPlayerComponent.h"
#include <set>
class CPlayerTaxas
	:public IPlayerComponent
{
public:
	typedef std::set<uint32_t> SET_ROOM_ID ;
	typedef std::map<uint32_t,stMyOwnRoom> MAP_ID_MYROOW ;
public:
	CPlayerTaxas(CPlayer* pPlayer):IPlayerComponent(pPlayer){ m_eType = ePlayerComponent_PlayerTaxas ; }
	virtual void Reset();
	virtual void Init();
	virtual bool OnMessage( stMsg* pMessage , eMsgPort eSenderPort)override ;
	virtual bool onCrossServerRequest(stMsgCrossServerRequest* pRequest , eMsgPort eSenderPort,Json::Value* vJsValue = nullptr)override;
	virtual bool onCrossServerRequestRet(stMsgCrossServerRequestRet* pResult,Json::Value* vJsValue = nullptr )override;
	virtual void OnPlayerDisconnect()override;
	virtual void OnOtherWillLogined()override;
	virtual void TimerSave();
    void OnReactive(uint32_t nSessionID )override{ sendTaxaDataToClient(); }
	void OnOtherDoLogined() override{sendTaxaDataToClient();}
	void getTaxasData(stPlayerTaxasData* pData );
	uint32_t getCurRoomID(){ return m_nCurTaxasRoomID ;}
	void addOwnRoom(uint32_t nRoomID , uint16_t nConfigID );
	bool isCreateRoomCntReachLimit();
	bool deleteOwnRoom(uint32_t nRoomID );
	uint16_t getMyOwnRoomConfig( uint32_t nRoomID ) ;
	bool isRoomIDMyOwn(uint32_t nRoomID);
protected:
	void sendTaxaDataToClient();
protected:
	uint32_t m_nCurTaxasRoomID ;
	bool m_bDirty ;
	stPlayerTaxasData m_tData ;
	MAP_ID_MYROOW m_vMyOwnRooms;
	SET_ROOM_ID m_vFollowedRooms ;
};