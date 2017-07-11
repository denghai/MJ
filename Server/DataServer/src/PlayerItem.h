#pragma once
#include "IPlayerComponent.h"
#include <map>
#include "CommonDefine.h"
class CPlayer;
struct stPlayerItem ;
struct stItemGift ;
class CPlayerItemComponent
	:public IPlayerComponent
{
public:
	typedef std::map<unsigned short,stPlayerItem*> MAP_ALL_PLAYER_ITEMS ;
public:
	CPlayerItemComponent(CPlayer* pPlayer):IPlayerComponent(pPlayer){ m_bDirty = false ; ClearAll(); }
	~CPlayerItemComponent(){ClearAll();}
	virtual bool OnMessage(stMsg* pMsg ) ;
	virtual void OnPlayerDisconnect() ;
	virtual void Reset();
	virtual void Init(){ Reset() ;}
	unsigned int GetItemCountByID(unsigned short nItemID );
	bool OnUserItem( unsigned short nItemID , unsigned int nCount = 1 ,CPlayer* pTarget = NULL );
	void AddItemByID(unsigned short nItemID , unsigned int nCount = 1 );
	virtual void TimerSave(){ SaveToDB();}
	void OnUseGift(stItemGift*pGift);
	bool OnPlayerEvent(stPlayerEvetArg* pArg);
protected:
	void ClearAll();
	void SaveToDB();
	void SendToClient();
protected:
	MAP_ALL_PLAYER_ITEMS m_vAllPlayerItems ;
	bool m_bDirty ;
};