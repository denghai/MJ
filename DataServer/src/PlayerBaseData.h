#pragma once
#include "MessageDefine.h"
#include "CommonDefine.h"
#include "IPlayerComponent.h"
#include <string>
#include "PlayerOnlineBox.h"
struct stMsg ;
class CPlayer ;
struct stEventArg ;
class CPlayerBaseData 
	:public IPlayerComponent
{
public:
	CPlayerBaseData(CPlayer*);
	~CPlayerBaseData();
	void Init();
	virtual bool onCrossServerRequest(stMsgCrossServerRequest* pRequest, eMsgPort eSenderPort,Json::Value* vJsValue = nullptr );
	virtual bool onCrossServerRequestRet(stMsgCrossServerRequestRet* pResult,Json::Value* vJsValue = nullptr );
	bool OnMessage( stMsg* pMessage , eMsgPort eSenderPort )override;
	bool OnMessage( Json::Value& recvValue , uint16_t nmsgType, eMsgPort eSenderPort )override ;
	void SendBaseDatToClient();
	void Reset();
	virtual void OnOtherDoLogined();
	void OnPlayerDisconnect() override ;
	void onPlayerReconnected()override;
	void OnOtherWillLogined()override ;
	void SaveCoin();
	void onRecivedLoginData();
	void OnProcessContinueLogin( bool bNewDay, time_t nLastLogin );
	void TimerSave()override;
	uint32_t GetAllCoin(){ return m_stBaseData.nCoin;}
	uint32_t getCoin(){ return m_stBaseData.nCoin ; }
	uint32_t GetAllDiamoned(){ return m_stBaseData.nDiamoned;}
	void setCoin(int64_t nCoin );
	bool AddMoney(int64_t nOffset,bool bDiamond = false );
	void addInvitePrize(uint32_t nCoinPrize );
	bool decressMoney(int64_t nOffset,bool bDiamond = false );
	bool OnPlayerEvent(stPlayerEvetArg* pArg);
	char* GetPlayerName(){ return m_stBaseData.cName ;}
	char* GetPlayerSignature(){ return m_stBaseData.cSignature ;}
	void GetPlayerBrifData(stPlayerBrifData* pData );
	void GetPlayerDetailData(stPlayerDetailData* pData );
	unsigned char GetSex(){ return m_stBaseData.nSex ;}
	unsigned char GetVipLevel(){ return m_stBaseData.nVipLevel ; }
	static bool EventFunc(void* pUserData,stEventArg* pArg);
	stServerBaseData* GetData(){ return &m_stBaseData ;}
	virtual void OnReactive(uint32_t nSessionID );
	uint16_t GetPhotoID(){ return m_stBaseData.nPhotoID ;}
	bool isPlayerRegistered();
	uint8_t getNewPlayerHaloWeight();
	void setNewPlayerHalo(uint8_t nPlayHalo );
	uint32_t getTempCoin(){ return m_nTempCoin ; }
	void setTempCoin( uint32_t nTempCoin );
	void onGetReward( uint8_t nIdx ,uint16_t nRewardID, uint16_t nGameType , const char* nRoomName );
	ePlayerType getPlayerType(){ return m_ePlayerType ;}
	void addTodayGameCoinOffset(int32_t nOffset );
	uint16_t getMaxCanCreateClubCount();
	uint16_t getMaxCanCreteRoomCount();
	std::string getIp(){ return m_strCurIP; }
	double getJ() { return m_J; }
	double getW() { return m_W; }
protected:
	bool onPlayerRequestMoney( uint64_t& nWantMoney,uint64_t nAtLeast, bool bDiamoned = false);
	void onBeInviteBy(uint32_t nInviteUID );
	void updateCardLife();
public:
	friend class CPlayerOnlineBox ;
protected:
	void SaveBaseData();
	void OnNewDay(stEventArg* pArg);
private:
	stServerBaseData m_stBaseData ;
	Json::Value m_jsJoinedClubs;
	ePlayerType m_ePlayerType ; 
	uint32_t m_nTempCoin ;

	uint8_t nReadingDataFromDB ; // 0 not read , 1 reading , 2 already read ;

	std::string m_strCurIP ; // ip address ; used by wechat pay ;
	// not store in db 
	bool m_bGivedLoginReward ;

	bool m_bMoneyDataDirty;
	bool m_bCommonLogicDataDirty ;
	bool m_bPlayerInfoDataDirty;

	double m_J;
	double m_W;
};