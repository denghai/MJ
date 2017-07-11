#pragma once
#include "IConfigFile.h"
#include <map>
enum eMissionType
{
	eMission_PlayRound,
	eMission_WinRound,
	eMission_WinRoundWithCardType,
	eMission_FirstLogin,
	eMission_SingleWinCoin,
	eMission_AddFriend,
	eMission_RMBConsume,
	eMission_Max,
};

struct stMissionConfig
{
	unsigned short nMissionID ;
	eMissionType eType ;
#ifndef GAME_SERVER
	std::string strName ;
	std::string stdDesc ;
	std::string strIcon ;
#endif
	unsigned int nMissionProcess ;
	unsigned int nRewardCoin ;
	unsigned int nMissionValue ;
	std::map<unsigned short, unsigned short> vItemsAndCount ;
};

class CMissionConfigMgr
	:public IConfigFile
{
public:
	typedef std::map<unsigned short, stMissionConfig*> MAP_MISSION ;
public:
	CMissionConfigMgr(){Clear();}
	~CMissionConfigMgr(){ Clear() ;}
	bool OnPaser(CReaderRow& refReaderRow );
	stMissionConfig* GetMissionConfigByID(unsigned short nMissionID );
    inline int GetMissionConfigSize(){return m_vAllMission.size();}
protected:
	void Clear();
#ifdef GAME_SERVER
	friend class CPlayerMission ;
#endif
protected:
	MAP_MISSION m_vAllMission ;
};