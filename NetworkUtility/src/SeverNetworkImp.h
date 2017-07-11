#pragma once
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>  
#include <boost/enable_shared_from_this.hpp>  
#include "InternalBuffer.h"
#include <map>
#include <list>
#include <boost/thread.hpp>
#include "../../ServerCommon/log4z.h"
using boost::asio::ip::tcp;  
class CSession;
class CServerNetworkImp
{
public:
	typedef std::map<uint32_t,boost::shared_ptr<CSession>> MAP_SESSION ;
	typedef std::list<Packet*> LIST_PACKET;
	typedef boost::shared_ptr<CSession> Session_ptr ;

	typedef boost::shared_lock<boost::shared_mutex> ReadLock;  
	typedef boost::unique_lock<boost::shared_mutex> WriteLock;  
public:
	CServerNetworkImp();
	~CServerNetworkImp();
	bool init(uint16_t nPort );
	std::string getIPportString(uint32_t nConnectID);
	bool sendMsg(uint32_t nConnectID , const char* pData , size_t nLen );
	bool getAllPacket(LIST_PACKET& vOutPackets ); // must delete out side ;
	bool getFirstPacket(Packet** ppPacket ); // must delete out side ;
	void closePeerConnection(uint32_t nConnectID, bool bServerClose = true );
protected:
	void closeSession(uint32_t nConnectID, bool bServerClose = false );
	void startAccept() ;
	void handleAccept(Session_ptr session,const boost::system::error_code& error) ;
	Session_ptr getSessionByConnectID(uint32_t nConnectID );
	void addPacket(Packet* pPacket ) ;
	void onReivedData(uint32_t nConnectID , const char* pBuffer , size_t nLen );
protected:
	friend class CSession; 
private:  
	boost::asio::io_service m_ioService;  
	tcp::acceptor* m_acceptor; 

	boost::shared_mutex m_SessionMutex;  
	MAP_SESSION m_vActiveSessions ;

	boost::shared_mutex m_PacketMutex;
	LIST_PACKET m_vRecivedPackets ;

	boost::thread* m_pIOThread ;
	boost::thread* m_pIOThread2 ;
};