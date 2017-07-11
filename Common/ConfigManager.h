#pragma once
class IConfigFile ;
class CContiuneLoginConfigMgr ;
class CTitleLevelConfig;

class CConfigManager
{
public:
	enum eConfigType
	{
		eConfig_One,
		eConfig_ContinueLogin = eConfig_One,
		eConfig_Item,
		eConfig_Informs,
		eConfig_Shop,
		eConfig_Mission,
		eConfig_SlotMachine,
		eConfig_Box,
		eConfig_Room,
		eConfig_TitleLevel,
		eConfig_Max,
	};
public:
	CConfigManager();
	~CConfigManager();
	void LoadAllConfigFile( const char* pConfigRootPath );
	IConfigFile* GetConfig( eConfigType eConfig );
	CContiuneLoginConfigMgr* GetContinueLoginConfig(){ return (CContiuneLoginConfigMgr*)GetConfig(eConfig_ContinueLogin) ; }
	CTitleLevelConfig* GetTitleLevelConfig(){ return (CTitleLevelConfig*)GetConfig(eConfig_TitleLevel) ; }
protected:
	IConfigFile* m_vConfigs[eConfig_Max] ;
};