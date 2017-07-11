#pragma once 
#include "IPlayerComponent.h"
#include "NativeTypes.h"
#include <string>
#include "CommonDefine.h"
#include <list>
class CPlayer;
struct stMail;
class CPlayerMailComponent
	:public IPlayerComponent
{
public:
	enum eOfflineEvent 
	{
		Event_Reward, // {rankIdx : 2 rewardID: 23,gameType: 2 , roomName : 234 }
		Event_AddCoin, // { comment: "why add coin" addCoin : 34556 }
		Event_SyncGameResult, // { serialNum : 234, createUID : 2345 , roomID : 234 , roomType : 2 , clubID : 34, finishTime : 234554, duiringTime : 2345 , offset : -234 , finalCoin : 23456 , buyIn : 2345 , configID : 23 , roomName :  "roomName" } 
		Event_Max,
	};
	struct stRecievedMail
	{
		uint32_t nRecvTime ;
		eMailType eType ;
		std::string strContent ;
	};
	typedef std::list<stRecievedMail> LIST_MAIL ;
public:
	CPlayerMailComponent(CPlayer* pPlayer ):IPlayerComponent(pPlayer){ClearMails();}
	~CPlayerMailComponent(){ ClearMails() ;}
	virtual bool OnMessage( stMsg* pMessage , eMsgPort eSenderPort );
	void onPlayerReconnected()override ;
	virtual void Reset();
	virtual void Init();
	virtual void OnOtherDoLogined();
	void OnReactive(uint32_t nSessionID ) override ;

	static void PostOfflineEvent( eOfflineEvent eEvntType ,Json::Value& pEventArg ,uint32_t nTargetUID );
	static void PostDlgNotice( eNoticeType eNotice ,Json::Value& pEventArg ,uint32_t nTargetUID );

	static void PostMailToPlayer( eMailType eType ,const char* pContent, uint16_t nContentLen ,uint32_t nTargetUID );
	static void PostPublicMail(stRecievedMail& pMail);

	static uint16_t getPublicMailsByTime( uint32_t nTimeTag, LIST_MAIL* vOutMail = nullptr,uint16_t nMaxOutCnt = 20);

	void ReciveMail(eMailType eType ,const char* pContent, uint16_t nContentLen );
protected:
	void saveReadTimeTag();
	void ClearMails();
	void SendMailListToClient();
	void InformRecievedUnreadMails();
	void ProcessSpecailMail();
	bool ProcessMail(stRecievedMail& pMail);
	void processSysOfflineEvent(stRecievedMail& pMail);
protected:
	LIST_MAIL m_vAllMail ;
	static LIST_MAIL s_vPublicMails ;
	static bool s_isReadedPublic ;
	uint32_t m_tReadTimeTag ;
};