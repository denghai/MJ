#include "ServerConfig.h"
#include "log4z.h"
CSeverConfigMgr::CSeverConfigMgr()
{
	memset(m_vAllSvrConfig,0,sizeof(m_vAllSvrConfig)) ;
}

CSeverConfigMgr::~CSeverConfigMgr()
{

}

bool CSeverConfigMgr::OnPaser(CReaderRow& refReaderRow )
{
	unsigned char cSvrType = refReaderRow["svrType"]->IntValue();
	if ( cSvrType >= eSvrType_Max )
	{
		LOGFMTE("server config error , type error , type = %d",cSvrType) ;
		return false;
	}

	stServerConfig configItem ;
	memset(&configItem,0,sizeof(configItem));
	configItem.nPort = refReaderRow["svrPort"]->IntValue();
	configItem.nSvrType = cSvrType;
	if (strlen(refReaderRow["svrIP"]->StringValue().c_str()) >= 261 )
	{
		LOGFMTE("too long ip address = %s",refReaderRow["svrIP"]->StringValue().c_str()) ;
		return false ;
	}
	memcpy_s(configItem.strIPAddress,sizeof(configItem.strIPAddress),refReaderRow["svrIP"]->StringValue().c_str(),strlen(refReaderRow["svrIP"]->StringValue().c_str()));
	memcpy_s(configItem.strAccount,sizeof(configItem.strAccount),refReaderRow["account"]->StringValue().c_str(),strlen(refReaderRow["account"]->StringValue().c_str()));
	memcpy_s(configItem.strPassword,sizeof(configItem.strPassword),refReaderRow["password"]->StringValue().c_str(),strlen(refReaderRow["password"]->StringValue().c_str()));
	m_vAllSvrConfig[cSvrType].push_back(configItem);
	return true ;
}

stServerConfig* CSeverConfigMgr::GetServerConfig(eServerType cSvrType, uint16_t nIdx )
{
	if ( cSvrType >= eSvrType_Max )
		return NULL ;
	VEC_SERVER_CONFIG& v = m_vAllSvrConfig[cSvrType];
	if ( nIdx < v.size() )
	{
		return &v[nIdx] ;
	}
	return NULL ;
}