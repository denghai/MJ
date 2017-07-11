#pragma once
class CPlayer ;
enum ePlayerEvent
{
	ePlayerEvent_RoundEnd,
	ePlayerEvent_FirstLogin,
	ePlayerEvent_AddFriend,
	ePlayerEvent_Recharge,
	ePlayerEvent_ReadDBOK,
	ePlayerEvent_Max,
};

struct stPlayerEvetArg
{
	ePlayerEvent eEventType ;
};

struct stPlayerEventArgRoundEnd
	:public stPlayerEvetArg
{
	unsigned char cCardType ;
	int64_t nCoinOffset ;
	int nDiamonedOffset ;
};

struct stPlayerEventArgAddFriend
	:public stPlayerEvetArg
{
	CPlayer* pNewFirend ;
};

struct stPlayerEventArgRecharge
	:public stPlayerEvetArg
{
	unsigned int nRMB ;
	unsigned short nShopItemID ;
};