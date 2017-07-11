#pragma once
#include "NetworkDefine.h"
#include <list>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include "InternalBuffer.h"
#include <deque> 
using boost::asio::ip::tcp;  
class CClientNetworkImp
{
public:
	typedef std::list<Packet*> LIST_PACKET;
	typedef boost::shared_ptr<CInternalBuffer> InternalBuffer_ptr;  
	typedef std::deque<InternalBuffer_ptr> BufferQueue;
	typedef boost::unique_lock<boost::shared_mutex> WriteLock;  
	enum  
	{
		eState_None,
		eState_Connecting,
		eState_Connected,
		eState_ConnectedFailed,
		eState_Max,
	};
public:
	CClientNetworkImp();
	~CClientNetworkImp();
	bool init();
	void shutdown();
	bool connectToServer(const char* pIP, unsigned short nPort );
	bool getAllPacket(LIST_PACKET& vOutPackets ); // must delete out side ;
	bool getFirstPacket(Packet** ppPacket ); // must delete out side ;
	void addPacket(Packet* pPacket ) ;
	bool sendMsg(const char* pData , size_t nLen ) ;
	boost::asio::io_service* getServicePtr(){ return &m_io_service ; }
private:  
	void handleConnect(const boost::system::error_code& error);

	void handleReadHeader(const boost::system::error_code& error) ; 

	void handleReadBody(const boost::system::error_code& error) ;

	void doWrite(InternalBuffer_ptr msg) ;

	void handleWrite(const boost::system::error_code& error) ;

	void sendHeatBeat( const boost::system::error_code& ec );
	void onKeepIoServeicWork( const boost::system::error_code& ec );
	void doClose();
private:  
	boost::asio::io_service m_io_service;  
	tcp::socket* m_socket;  

	boost::shared_mutex m_PacketMutex;
	LIST_PACKET m_vRecivedPackets ;

	boost::shared_mutex m_SendBuffersMutex;
	BufferQueue m_vWillSendBuffers ;

	InternalBuffer_ptr m_pReadIngBuffer ;
	unsigned char m_nState ;

	boost::thread* m_pIOThread ;
	boost::asio::ip::tcp::endpoint* m_pEndpoint ;

	boost::asio::deadline_timer m_tHeatBeat;
	int8_t m_nHeatBeatTimes ;

	boost::asio::deadline_timer m_tKeepIOServiecWork;
};