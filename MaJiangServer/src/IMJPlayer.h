#pragma once 
#include "NativeTypes.h"
#include "timer.h"
class IMJPlayerCard;
struct stEnterRoomData;
class IMJPlayer
{
public:
	virtual ~IMJPlayer(){}
	virtual void init(stEnterRoomData* pData) = 0;
	virtual void onComeBackRoom(stEnterRoomData* pData) = 0 ;
	virtual void onWillStartGame() = 0;
	virtual void onStartGame() = 0;
	virtual void onGameDidEnd() = 0;
	virtual void onGameEnd() = 0 ;
	virtual uint8_t getIdx() = 0;
	virtual void setIdx(uint8_t nIdx) = 0;
	virtual int32_t getCoin() = 0;
	virtual void setCoin( int32_t nNewCoin ) = 0 ;  // only used for comsume desk fee
	virtual int32_t onRecievedSupplyCoin( uint32_t nSupplyCoin ) = 0;
	virtual void addOffsetCoin( int32_t nOffset ) = 0 ;
	virtual int32_t getOffsetCoin() = 0;
	virtual IMJPlayerCard* getPlayerCard() = 0;
	virtual void setState( uint32_t eState ) = 0;
	virtual bool haveState( uint32_t eState ) = 0;
	virtual uint32_t getState() = 0;
	virtual uint32_t getSessionID() = 0;
	virtual uint32_t getUID() = 0;
	virtual void signGangFlag() = 0;
	virtual void clearGangFlag() = 0;
	virtual bool haveGangFalg() = 0;
	virtual void signDecareBuGangFlag() = 0;
	virtual void clearDecareBuGangFlag() = 0;
	virtual bool haveDecareBuGangFalg() = 0;
	virtual bool isRobot() = 0;
	virtual bool isTrusteed() = 0;
	virtual void switchTrusteed(bool isTrusted) = 0;
	virtual void setTrusteeActFunc(CTimer::time_func pFunc) = 0;
	virtual bool isTempLeaveRoom() = 0;
	virtual bool doTempLeaveRoom() = 0 ;

	virtual uint8_t getDianPaoCnt() = 0;
	virtual void addDianPaoCnt() = 0 ;
	
	virtual uint8_t getHuCnt() = 0;
	virtual void addHuCnt() = 0;

	virtual uint8_t getZiMoCnt() = 0 ;
	virtual void addZiMoCnt() = 0;

	virtual uint8_t getAnGangCnt() = 0 ;
	virtual void addAnGangCnt() = 0;

	virtual uint8_t getMingGangCnt() = 0 ;
	virtual void addMingGangCnt() = 0;
	virtual void setIsOnline(bool isOnline) = 0;
	virtual bool isOnline() = 0;

};