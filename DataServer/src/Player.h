#pragma once 
#include "IPlayerComponent.h"
#include "Timer.h"
#include "ServerMessageDefine.h"
class CPlayerBaseData ;
struct stMsg ;
class CRoomBaseNew ;
class CPlayer
{
public:
	enum ePlayerState
	{
		ePlayerState_Offline = 1 ,
		ePlayerState_Online = 1 << 1,
		ePlayerState_WaitReconnect = 1 << 2,
		ePlayerState_InTaxasRoom = (1<<2)| ePlayerState_Online,
		ePlayerState_Max,
	};
public:
	CPlayer();
	~CPlayer();
	void Init( unsigned int nUserUID,unsigned int nSessionID );
	//void Reset(unsigned int nUserUID,unsigned int nSessionID ) ; // for reuse the object ;
	bool OnMessage( stMsg* pMessage , eMsgPort eSenderPort );
	bool OnMessage( Json::Value& recvValue , uint16_t nmsgType, eMsgPort eSenderPort );
	void OnPlayerDisconnect();
	void SendMsgToClient(const char* pBuffer, unsigned short nLen,bool bBrocat = false );
	void SendMsgToClient(Json::Value& jsMsg, uint16_t nMsgType,bool bBrocat = false );
	unsigned int GetUserUID(){ return m_nUserUID ;}
	unsigned int GetSessionID(){ return m_nSessionID ;}
	IPlayerComponent* GetComponent(ePlayerComponentType eType ){ return m_vAllComponents[eType];}
	CPlayerBaseData* GetBaseData(){ return (CPlayerBaseData*)GetComponent(ePlayerComponent_BaseData);}
	bool IsState( ePlayerState eState ); 
	void SetState(ePlayerState eSate ){ m_eSate = eSate ; }
	void OnAnotherClientLoginThisPeer(unsigned int nSessionID );
	void PostPlayerEvent(stPlayerEvetArg* pEventArg );
	void OnTimerSave(CTimer* pTimer, float fTimeElaps );
	void OnReactive(uint32_t nSessionID );
	time_t GetDisconnectTime(){ return m_nDisconnectTime ;}
	virtual bool onCrossServerRequest(stMsgCrossServerRequest* pRequest, eMsgPort eSenderPort,Json::Value* vJsValue = nullptr );
	virtual bool onCrossServerRequestRet(stMsgCrossServerRequestRet* pResult,Json::Value* vJsValue = nullptr );
	static uint8_t getMsgPortByRoomType(uint8_t nType );
	void delayDelete();
	time_t getCanDelayTime(){ return m_nDisconnectTime ;}
	void startAsyncReq(){ m_bIsWaitingAsyncRequest = true ; }
	void endAsyncReq(){ m_bIsWaitingAsyncRequest = false ; }
	bool isWaitAsyncReq(){ return m_bIsWaitingAsyncRequest ;}
protected:
	bool ProcessPublicPlayerMsg( stMsg* pMessage , eMsgPort eSenderPort );
	void PushTestAPNs();
protected:
	unsigned int m_nUserUID ;
	unsigned int m_nSessionID ;  // comunicate with the client ;
	IPlayerComponent* m_vAllComponents[ePlayerComponent_Max] ;
	ePlayerState m_eSate ;
	time_t m_nDisconnectTime ;
	CTimer m_tTimerSave ;

	bool m_bIsWaitingAsyncRequest ;
}; 