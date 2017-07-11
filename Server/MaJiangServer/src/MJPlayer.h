#pragma once
#include "IMJPlayer.h"
#include "Timer.h"
class MJPlayer
	:public IMJPlayer
{
public:
	~MJPlayer();
	void init(stEnterRoomData* pData)override;
	void onComeBackRoom(stEnterRoomData* pData)override;
	void onWillStartGame()override;
	void onStartGame()override;
	void onGameDidEnd()override;
	void onGameEnd()override;
	uint8_t getIdx() final;
	void setIdx(uint8_t nIdx)  final;
	int32_t getCoin() final;
	void setCoin(int32_t nNewCoin)final;
	virtual void addOffsetCoin(int32_t nOffset) override;
	int32_t getOffsetCoin() override;
	void setState(uint32_t eState) override;
	bool haveState(uint32_t eState) override;
	uint32_t getState() final;
	uint32_t getSessionID()final;
	uint32_t getUID()final;
	void signGangFlag()final;
	void clearGangFlag()final;
	bool haveGangFalg()final;
	void signDecareBuGangFlag()final;
	void clearDecareBuGangFlag()final;
	bool haveDecareBuGangFalg()final;
	int32_t onRecievedSupplyCoin(uint32_t nSupplyCoin)final;
	bool isRobot()final;
	bool isTrusteed()final;
	void switchTrusteed( bool isTrusted )override;
	void setTrusteeActFunc(CTimer::time_func pFunc)final;
	bool isTempLeaveRoom()final;
	bool doTempLeaveRoom()final;

	uint8_t getDianPaoCnt() final;
	void addDianPaoCnt() final;

	uint8_t getHuCnt() final;
	void addHuCnt()final;

	uint8_t getZiMoCnt() final;
	void addZiMoCnt()final;

	uint8_t getAnGangCnt()final;
	void addAnGangCnt()final;

	uint8_t getMingGangCnt()final;
	void addMingGangCnt()final;

	void setIsOnline(bool isOnline) final;
	bool isOnline() final;
protected:
	bool m_isTempLeave;
	uint8_t m_nPlayerType;
	uint32_t m_eState;
	uint32_t m_nUserUID;
	uint32_t m_nSessioID;
	uint8_t m_nIdx;
	int32_t m_nOffset;
	uint32_t m_nCoin;
	bool m_isHaveGang;
	bool m_isDeclareBuGang;
	bool m_isTrusteed; // is tuo guan ?

	CTimer m_tTrusteedActTimer;

	uint8_t m_nHuCnt;
	uint8_t m_nZiMoCnt; 
	uint8_t m_nDianPaoCnt;
	uint8_t m_nMingGangCnt;
	uint8_t m_nAnGangCnt;

	bool m_isOnline;
};