#pragma once
#include "RoomBaseNew.h"
class CTaxasRoomData;
class CRoomTaxasNew
	:public CRoomBaseNew
{
public:
	virtual bool Init(stBaseRoomConfig* pConfig);
	virtual void Enter(CPlayer* pEnter );
	virtual void Leave(CPlayer* pLeaver);
	virtual void Update(float fTimeElpas, unsigned int nTimerID ) ;
	virtual bool OnMessage(CPlayer*pSender, stMsg* pmsg);
	virtual void SitDown(CPlayer* pPlayer );
	virtual void StandUp(CPlayer* pPlayer) ;
	virtual void SendRoomInfoToPlayer(CPlayer* pPlayer);
	virtual unsigned char CheckCanJoinThisRoom(CPlayer* pPlayer); // 0 means ok , other value means failed ;
	void GotoState(unsigned char TargetState);
	CTaxasRoomData* GetRoomData();
	void NextPlayerAct();
	void ProcessAutoTakeInAndAutoStandUp();
protected:

};