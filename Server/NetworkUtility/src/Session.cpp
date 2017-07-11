#include <WinSock2.h>
#include "Session.h"
#include "SeverNetworkImp.h"
uint32_t CSession::s_ConnectID = 1 ;
CSession::CSession(boost::asio::io_service& io_service,CServerNetworkImp* network )
	:m_socket(io_service),m_pNetwork(network),m_pReadIngBuffer(new CInternalBuffer()),m_tHeatBeat(io_service),m_tWaitFirstMsg(io_service)
{
	m_nConnectID = ++s_ConnectID ;
	m_bRecivedMsg = false ;
}

bool CSession::sendData(const char* pData , uint16_t nLen )
{
	InternalBuffer_ptr pBuffer( new CInternalBuffer() );
	if ( ! pBuffer->setData(pData,nLen) )
	{
		return false ;
	}
	
	bool bSending = false;
	WriteLock wLock(m_SendBuffersMutex);
	bSending = m_vWillSendBuffers.empty() == false;
	m_vWillSendBuffers.push_back(pBuffer);
	if ( bSending == false )
	{
		boost::asio::async_write(m_socket,  
			boost::asio::buffer(m_vWillSendBuffers.front()->data(),  
			m_vWillSendBuffers.front()->length()),   
			boost::bind(&CSession::handleWrite, shared_from_this(),  
			boost::asio::placeholders::error)); 
	}
	return true ;
}

uint32_t CSession::getConnectID()
{
	return m_nConnectID ;
}

std::string CSession::getIPString()
{
	if (m_socket.is_open())
	{
		return m_socket.remote_endpoint().address().to_string();
	}
	return "socket is close";
}

void CSession::start()
{
	boost::asio::async_read(m_socket,  
		boost::asio::buffer(m_pReadIngBuffer->data(), CInternalBuffer::header_length),  
		boost::bind(  
		&CSession::handleReadHeader, shared_from_this(),  
		boost::asio::placeholders::error)); //异步读客户端发来的消息
}

void CSession::close()
{
	LOGFMTD("serssion close close") ;
	boost::system::error_code ec;
	m_socket.close(ec);
	m_tHeatBeat.cancel();
	m_tWaitFirstMsg.cancel();
}
// handle function ;
void CSession::handleReadHeader(const boost::system::error_code& error)
{
	if (!error && m_pReadIngBuffer->decodeHeader())  
	{  
		boost::asio::async_read(m_socket,  
			boost::asio::buffer(m_pReadIngBuffer->body(), m_pReadIngBuffer->bodyLength()),  
			boost::bind(&CSession::handleReadBody, shared_from_this(),  
			boost::asio::placeholders::error));  
	}  
	else  
	{  
		LOGFMTD("handleReadHeader close id = %u",getConnectID());
		//m_pNetwork->closeSession(getConnectID());
		m_pNetwork->closePeerConnection(getConnectID(),false);
	} 
}

void CSession::handleReadBody(const boost::system::error_code& error) 
{
	if (!error)  
	{  
		if ( m_pReadIngBuffer->bodyLength() )
		{
			m_pNetwork->onReivedData(getConnectID(),m_pReadIngBuffer->body(),m_pReadIngBuffer->bodyLength());
			m_bRecivedMsg = true ;
		}

		boost::asio::async_read(m_socket,  
			boost::asio::buffer(m_pReadIngBuffer->data(), CInternalBuffer::header_length),  
			boost::bind(  
			&CSession::handleReadHeader, shared_from_this(),  
			boost::asio::placeholders::error));
	}  
	else  
	{  
		//printf("handleReadBody close\n") ;
		LOGFMTD("handleReadBody close id = %u", getConnectID());
		//m_pNetwork->closeSession(getConnectID());
		m_pNetwork->closePeerConnection(getConnectID(),false);
	}  
}

void CSession::handleWrite(const boost::system::error_code& error) 
{
	if (!error)  
	{  
		WriteLock wLock(m_SendBuffersMutex);
		m_vWillSendBuffers.pop_front();

		if (!m_vWillSendBuffers.empty())  
		{  
			boost::asio::async_write(m_socket,  
				boost::asio::buffer(m_vWillSendBuffers.front()->data(),  
				m_vWillSendBuffers.front()->length()),  
				boost::bind(&CSession::handleWrite, shared_from_this(),  
				boost::asio::placeholders::error));
		}  
	}  
	else  
	{  
		//m_pNetwork->closeSession(getConnectID());
		LOGFMTD("handleWrite close id = %u", getConnectID());
		m_pNetwork->closePeerConnection(getConnectID(), false);
	} 
}

void CSession::startWaitFirstMsg()
{
	m_tWaitFirstMsg.expires_from_now(boost::posix_time::seconds(TIME_CHECK_FIRST_MSG));
	m_tWaitFirstMsg.async_wait(boost::bind(&CSession::handleCheckFirstMsg, shared_from_this(), boost::asio::placeholders::error));
}

void CSession::handleCheckFirstMsg( const boost::system::error_code& ec )
{
	if ( !ec )
	{
		if ( !m_bRecivedMsg )
		{
			LOGFMTE("find a dead connect id = %u ",getConnectID());
			//m_pNetwork->closeSession(getConnectID());
			m_pNetwork->closePeerConnection(getConnectID(), false);
		}
	}
}

void CSession::startHeartbeatTimer()
{
	m_tHeatBeat.expires_from_now(boost::posix_time::seconds(TIME_HEAT_BET));
	m_tHeatBeat.async_wait(boost::bind(&CSession::sendHeatBeat, shared_from_this(), boost::asio::placeholders::error));
}

void CSession::handleWriteHeartbeat(const boost::system::error_code& ec)
{
	if(!ec){
		startHeartbeatTimer();
		//printf("send beat ok \n");
	}
	else{
		// close ;
		LOGFMTE("heat beat failed id = %u",getConnectID());
		//m_pNetwork->closeSession(getConnectID());
		m_pNetwork->closePeerConnection(getConnectID(), false);
	}
}

void CSession::sendHeatBeat( const boost::system::error_code& ec )
{
	if ( !ec )
	{
		char p[4] = { 0 } ;
		unsigned short* psh = (unsigned short*)p;
		*psh = 3;
		p[2] = 0 ;
		p[3] = 0 ;
		boost::asio::async_write(m_socket, boost::asio::buffer(p, sizeof(p)),
			boost::bind(&CSession::handleWriteHeartbeat, shared_from_this(),
			boost::asio::placeholders::error ));
	}
}