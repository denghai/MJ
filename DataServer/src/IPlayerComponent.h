#pragma once
#include "ServerMessageDefine.h"
#include <json/json.h>
class CPlayer ;

enum ePlayerComponentType
{
	ePlayerComponent_None ,
	ePlayerComponent_BaseData,
	ePlayerComponent_Friend,
	ePlayerComponent_PlayerShop,
	ePlayerComponent_PlayerItemMgr,
	ePlayerComponent_PlayerMission,
	ePlayerComponent_PlayerGameData ,
	ePlayerComponent_Mail,            // last sit the last pos ,
	ePlayerComponent_Max,
};

struct stPlayerEvetArg ;
class IPlayerComponent
{
public:
	IPlayerComponent(CPlayer* pPlayer );
	virtual ~IPlayerComponent();
	void SendMsg(stMsg* pbuffer , unsigned short nLen , bool bBrocast = false );
	void SendMsg(Json::Value& jsMsg , uint16_t nMsgType , bool bBrocast = false );
	ePlayerComponentType GetComponentType(){ return m_eType ;}
	CPlayer* GetPlayer(){ return m_pPlayer ;}
	virtual bool OnMessage( stMsg* pMessage , eMsgPort eSenderPort);
	virtual bool OnMessage( Json::Value& recvValue , uint16_t nmsgType, eMsgPort eSenderPort ){ return false ;}
	virtual bool onCrossServerRequest(stMsgCrossServerRequest* pRequest, eMsgPort eSenderPort,Json::Value* vJsValue = nullptr );
	virtual bool onCrossServerRequestRet(stMsgCrossServerRequestRet* pResult,Json::Value* vJsValue = nullptr );
	virtual void OnPlayerDisconnect(){}
	virtual void onPlayerReconnected(){}
	virtual void Reset(){}
	virtual void Init(){ }
	virtual void OnOtherWillLogined(){}
	virtual void OnOtherDoLogined(){}
	virtual bool OnPlayerEvent(stPlayerEvetArg* pArg){ return false ; }
	virtual void TimerSave(){};
	virtual void OnReactive(uint32_t nSessionID ){ }
protected:
	CPlayer* m_pPlayer ;
	ePlayerComponentType m_eType ;	
};