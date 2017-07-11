#pragma once
#include "RoomBaseNew.h"
class CGoldenRoomData;

class CRoomGoldenNew
	:public CRoomBaseNew
{
public:
	virtual bool Init(stBaseRoomConfig* pConfig);
	virtual void Enter(CPlayer* pEnter );
	virtual void Leave(CPlayer* pLeaver);
	virtual unsigned char CheckCanJoinThisRoom(CPlayer* pPlayer); // 0 means ok , other value means failed ;
	virtual void SendRoomInfoToPlayer(CPlayer* pPlayer);
	virtual void Update(float fTimeElpas, unsigned int nTimerID ) ;
	void GoToState(unsigned char cTargetState);
	virtual bool OnMessage(CPlayer*pSender, stMsg* pmsg);

	bool GameOverCheckAndProcess();
	void NextPlayerAct();
	CGoldenRoomData* GetRoomData();

};