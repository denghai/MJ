#pragma once
#include "IPlayerComponent.h"
#include <set>
#include <list>
class CPlayerGameData
	:public IPlayerComponent
{
public:
	struct stPlayerGameRecorder
	{
		uint32_t nRoomID ;
		uint32_t nFinishTime ;
		uint32_t nDuiringSeconds ;
		int32_t nOffset ;
		uint32_t nCreateUID ;
		uint32_t nBaseBet ;
		uint32_t nBuyIn ;
		uint32_t nClubID ;
		uint32_t nSieralNum;
		char cRoomName[MAX_LEN_ROOM_NAME] ;
	};

	struct stGameData
		: public stPlayerGameData
	{
		bool bDirty ;
	};
public:
	typedef std::set<uint32_t> SET_ROOM_ID ;
	typedef std::map<uint32_t,stMyOwnRoom> MAP_ID_MYROOW ;
	typedef std::list<stPlayerGameRecorder*> LIST_PLAYER_RECORDERS ;
public:
	CPlayerGameData(CPlayer* pPlayer):IPlayerComponent(pPlayer){ m_eType = ePlayerComponent_PlayerGameData ; }
	~CPlayerGameData();
	void Reset()override;
	void Init()override;
	bool OnMessage( stMsg* pMessage , eMsgPort eSenderPort)override ;
	bool OnMessage( Json::Value& recvValue , uint16_t nmsgType, eMsgPort eSenderPort )override ;
	bool onCrossServerRequest(stMsgCrossServerRequest* pRequest , eMsgPort eSenderPort,Json::Value* vJsValue = nullptr)override;
	bool onCrossServerRequestRet(stMsgCrossServerRequestRet* pResult,Json::Value* vJsValue = nullptr )override;
	void OnPlayerDisconnect()override;
	void OnOtherWillLogined()override;
	void TimerSave();
	void OnReactive(uint32_t nSessionID )override{ sendGameDataToClient(); }
	void OnOtherDoLogined() override{sendGameDataToClient();}
	uint32_t getCurRoomID(){ return m_nStateInRoomID ;}
	uint16_t getCreateRoomCntLimit();
	bool isNotInAnyRoom(){ return m_nStateInRoomID == 0 ; }
	void addPlayerGameRecorder(stPlayerGameRecorder* pRecorder , bool isSaveDB = true );
protected:
	void sendGameDataToClient();
protected:
	uint32_t m_nStateInRoomID ;
	uint8_t m_nSubRoomIdx ; 

	stGameData m_vData[eRoom_Max] ;

	LIST_PLAYER_RECORDERS m_vGameRecorders ;
};