#pragma once 
#include "IGlobalModule.h"
#include "./HttpServer/server.hpp"
class CHttpModule
	:public IGlobalModule
{
public:
	typedef boost::function<bool (http::server::connection_ptr)> httpHandle;
public:
	void init(IServerApp* svrApp)override;
	void update(float fDeta)override;
	bool registerHttpHandle(std::string strURI, httpHandle pHandle );
protected:
	bool onHandleVXPayResult(http::server::connection_ptr ptr );
	bool handleGetPlayerInfo(http::server::connection_ptr ptr );
	bool handleAddRoomCard(http::server::connection_ptr ptr);
	bool handleAnySdkLogin(http::server::connection_ptr ptr);

	// handle config file 
	bool onHandleConfigN(http::server::connection_ptr ptr);
	bool onHandleConfigNA(http::server::connection_ptr ptr);
	bool onHandleConfigGolden(http::server::connection_ptr ptr);
	bool onHandleConfigGoldenA(http::server::connection_ptr ptr);
	bool onHandleConfigRefresh(http::server::connection_ptr ptr);

	bool readFileToString(std::string file_name, std::string& fileData);
	bool responeConfigToConnect(const char* pFileName, http::server::connection_ptr ptr );
protected:
	boost::shared_ptr<http::server::server> mHttpServer;
	std::map<std::string, httpHandle> vHttphandles;
	std::map<std::string, std::string> vConfigFile;
};