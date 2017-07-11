#include "ConfigManager.h"
#include <string>
#include "ContinueLoginConfig.h"
#include "log4z.h"
#include "ItemConfig.h"
#ifdef DATA_SERVER
#include "../Server/DataServer/src/InformConfig.h"
#include "../Server/DataServer/src/SlotMachine.h"
#endif
#include "RoomConfig.h"
#include "ShopConfg.h"
#include "MissionConfig.h"
#include "BoxConfig.h"
#include "TitleLevelConfig.h"
CConfigManager::CConfigManager()
{
	memset(m_vConfigs,0,sizeof(m_vConfigs)) ;
}

CConfigManager::~CConfigManager()
{
	for ( int i = eConfig_One; i < eConfig_Max ; ++i )
	{
		delete m_vConfigs[i] ;
		m_vConfigs[i] = NULL ;
	}
}

void CConfigManager::LoadAllConfigFile( const char* pConfigRootPath )
{
	LOGFMTI("load all config") ;

	std::string strCL = pConfigRootPath ;
	if (strCL.at(strCL.size() -1 ) != '/')
	{
		strCL.append("/");
	}
	std::string pConfgiPath[eConfig_Max] ;
	pConfgiPath[eConfig_ContinueLogin] = strCL + "ContiuneLoginConfig.txt" ;
	pConfgiPath[eConfig_Item] = strCL + "ItemConfig.txt" ;
	pConfgiPath[eConfig_Informs] = strCL + "Inform.txt" ;
	pConfgiPath[eConfig_Shop] = strCL + "ShopConfig.txt" ;
	pConfgiPath[eConfig_Mission] = strCL + "MissionConfig.txt" ;
	pConfgiPath[eConfig_SlotMachine] = strCL + "SlotMachine.txt";
	pConfgiPath[eConfig_Box] = strCL + "BoxConfig.txt";
	//pConfgiPath[eConfig_Room] = strCL + "RoomConfig.txt";
	//pConfgiPath[eConfig_TitleLevel] = strCL + "TitleLevelConfig.txt";
	// go on login 
	m_vConfigs[eConfig_ContinueLogin] = new CContiuneLoginConfigMgr ;
	// item config ;
	m_vConfigs[eConfig_Item] = new CItemConfigManager ;
#ifdef SERVER
	// inform config 
	//m_vConfigs[eConfig_Informs] = new CInformConfig ;

	// slot manchine
	//m_vConfigs[eConfig_SlotMachine] = new CSlotMachine ;

	// room config
	m_vConfigs[eConfig_Room] = new CRoomConfigMgr ;
#endif 
	// shop config 
	m_vConfigs[eConfig_Shop] = new CShopConfigMgr ;
	// mission config
	m_vConfigs[eConfig_Mission] = new CMissionConfigMgr ;

	// online box
	m_vConfigs[eConfig_Box] = new CBoxConfigMgr ;

	// title level config 
	//m_vConfigs[eConfig_TitleLevel] = new CTitleLevelConfig ;

	for ( int i = eConfig_One; i < eConfig_Max ; ++i )
	{
		if ( m_vConfigs[i] )
		{
			m_vConfigs[i]->LoadFile(pConfgiPath[i].c_str()) ;
		}
	}

	// gift config
	strCL = strCL + "gift.txt";
	m_vConfigs[eConfig_Item]->LoadFile(strCL.c_str());
}

IConfigFile* CConfigManager::GetConfig( eConfigType eConfig )
{
	if ( eConfig >= eConfig_Max || eConfig < eConfig_One )
		return NULL ;
	return m_vConfigs[eConfig] ;
}