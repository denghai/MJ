#pragma once
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>  
#include <boost/enable_shared_from_this.hpp>  
#include <boost/thread.hpp>
#include <deque> 
#include "InternalBuffer.h"
using boost::asio::ip::tcp;  
class CServerNetworkImp ;
class CSession
	: public boost::enable_shared_from_this<CSession>
{
public:
	typedef boost::shared_ptr<CInternalBuffer> InternalBuffer_ptr;  
	typedef std::deque<InternalBuffer_ptr> BufferQueue;
	typedef boost::unique_lock<boost::shared_mutex> WriteLock;  
public:
	CSession(boost::asio::io_service& io_service,CServerNetworkImp* network );
	bool sendData(const char* pData , uint16_t nLen );
	uint32_t getConnectID();
	tcp::socket& socket(){return m_socket;}
	std::string getIPString();
	void start();
	void close();  // only invoke by network ;
	// handle function ;
	void handleReadHeader(const boost::system::error_code& error) ;
	void handleReadBody(const boost::system::error_code& error) ;
	void handleWrite(const boost::system::error_code& error) ;

	// handle heatBeat
	void startWaitFirstMsg();
	void handleCheckFirstMsg(const boost::system::error_code& ec);

	void startHeartbeatTimer();
	void sendHeatBeat( const boost::system::error_code& ec );
	void handleWriteHeartbeat(const boost::system::error_code& ec);
protected:
	static uint32_t s_ConnectID ;
protected:
	tcp::socket m_socket; 
	uint32_t m_nConnectID ;
	CServerNetworkImp* m_pNetwork ;

	boost::shared_mutex m_SendBuffersMutex;
	BufferQueue m_vWillSendBuffers ;

	InternalBuffer_ptr m_pReadIngBuffer ;

	boost::asio::deadline_timer m_tHeatBeat;
	boost::asio::deadline_timer m_tWaitFirstMsg;
	bool m_bRecivedMsg ;  // 防止一些坏的连接，连上啦什么都不干，所以连上来如果10秒内，没有向sever 发消息，就踢掉。
};