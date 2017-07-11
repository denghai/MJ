#include "ConfigManager.h"
#include <string>
#include "ContinueLoginConfig.h"
#include "LogManager.h"
#include "ItemConfig.h"
#include "InformConfig.h"
#include "ShopConfg.h"
#include "MissionConfig.h"
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

void CConfigManager::LoadAllConfigFile()
{
	CLogMgr::SharedLogMgr()->SystemLog("load all config") ;

	// go on login 
	m_vConfigs[eConfig_ContinueLogin] = new CContiuneLoginConfigMgr ;
	m_vConfigs[eConfig_ContinueLogin]->LoadFile("ContiuneLoginConfig.txt") ;

	// item config ;
	m_vConfigs[eConfig_Item] = new CItemConfigManager ;
	m_vConfigs[eConfig_Item]->LoadFile("ItemConfig.txt") ;

#ifdef GAME_SERVER
	// inform config 
	m_vConfigs[eConfig_Informs] = new CInformConfig ;
	m_vConfigs[eConfig_Informs]->LoadFile("Inform.txt") ;
#endif 

	// shop config 
	m_vConfigs[eConfig_Shop] = new CShopConfigMgr ;
	m_vConfigs[eConfig_Shop]->LoadFile("ShopConfig.txt") ;

	// mission config
	m_vConfigs[eConfig_Mission] = new CMissionConfigMgr ;
	m_vConfigs[eConfig_Mission]->LoadFile("MissionConfig.txt") ;
}

IConfigFile* CConfigManager::GetConfig( eConfigType eConfig )
{
	if ( eConfig >= eConfig_Max || eConfig < eConfig_One )
		return NULL ;
	return m_vConfigs[eConfig] ;
}