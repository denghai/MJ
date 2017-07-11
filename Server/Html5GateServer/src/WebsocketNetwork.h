#pragma once
#define _WEBSOCKETPP_NO_CPP11_SYSTEM_ERROR_ 
#include "NetworkDefine.h"
#include "websocketpp/server.hpp"
#include "websocketpp/config/asio_no_tls.hpp"
#include <map>
#include <list>
#include <mutex>
class IWebSocketDelegate
{
public:
	virtual ~IWebSocketDelegate(){}
	virtual bool onPeerConnected(const char* pIPAddress , CONNECT_ID nConnectID ){ return false ;}
	virtual bool onPeerClosed(CONNECT_ID nNetID ){ return false ;}
	virtual bool onMsg(CONNECT_ID id , const char* pmsg ) = 0 ;
};

class IWebsocketNetwork
{
public:
	struct stPacket 
	{
		CONNECT_ID nTarget ;
		uint8_t _packetType ; // 
		std::string strContent ;
		stPacket(){ nTarget = 0 ; _packetType = _PACKET_TYPE_MSG ; strContent = "";}
	};
	typedef websocketpp::server<websocketpp::config::asio> server;

	struct stNetPeer
	{
		CONNECT_ID nNetID ;
		websocketpp::connection_hdl nConnectHandle ;
	};

	typedef std::map<websocketpp::connection_hdl,stNetPeer*,std::owner_less<websocketpp::connection_hdl>> MAP_NET_PEER ;
	typedef std::list<stPacket*> LIST_PACKET ;
	typedef std::list<IWebSocketDelegate*> LIST_DELEGATE ;
public:
	IWebsocketNetwork();
	~IWebsocketNetwork();
	bool init(server::io_service_ptr pIos_service, unsigned short nPort );
	void addDelegate(IWebSocketDelegate* pDlete );
	void removeDelegate(IWebSocketDelegate* pDele);
	void distributeMsg();  // invoker by main loop ;
	void sendMsg(CONNECT_ID nNetID , const char* pContent );
public:
	void on_open(websocketpp::connection_hdl hdl);
	void on_close(websocketpp::connection_hdl hdl);
	void on_message(websocketpp::connection_hdl hdl, server::message_ptr msg) ;
	void validate_handler(websocketpp::connection_hdl hdl);
	void trySendMsg(const websocketpp::lib::error_code & ec);
protected:
	void processDelegateChange();
protected:
	server m_InternalSvr ;
	server::timer_ptr m_timer;

	LIST_DELEGATE m_vWillAdd ;
	LIST_DELEGATE m_vWillRemove ;
	LIST_DELEGATE m_vDelegates ;

	LIST_PACKET m_vRecivedPacket ;
	std::mutex m_mutexRecivedPacket;

	LIST_PACKET m_vWillSendPacket ;
	std::mutex m_mutexSendPacket;

	MAP_NET_PEER m_vNetPeers ;

	CONNECT_ID m_nCurMaxNetID ;
};