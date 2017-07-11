#include "WebsocketNetwork.h"
#include <functional>
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
IWebsocketNetwork::IWebsocketNetwork()
{
	m_nCurMaxNetID = 1 ;
}

IWebsocketNetwork::~IWebsocketNetwork() 
{
	for ( auto ref : m_vNetPeers )
	{
		delete ref.second ;
		ref.second = nullptr ;
	}
	m_vNetPeers.clear() ;


}

bool IWebsocketNetwork::init(server::io_service_ptr pIos_service, unsigned short nPort )
{
	m_InternalSvr.init_asio(pIos_service);

	m_InternalSvr.set_open_handler(bind(&IWebsocketNetwork::on_open,this,::_1));
	m_InternalSvr.set_close_handler(bind(&IWebsocketNetwork::on_close,this,::_1));
	m_InternalSvr.set_message_handler(bind(&IWebsocketNetwork::on_message,this,::_1,::_2));
	//m_InternalSvr.set_http_handler(bind(&IWebsocketNetwork::validate_handler,this,::_1));
	m_InternalSvr.listen(nPort);
	m_InternalSvr.start_accept();
	m_timer = m_InternalSvr.set_timer(18, bind(&IWebsocketNetwork::trySendMsg,this,::_1));
	return true ;
}

void IWebsocketNetwork::addDelegate(IWebSocketDelegate* pDlete )
{
	for ( auto del : m_vWillAdd )
	{
		 if ( del == pDlete )
		 {
			 return ;
		 }
	}

	for ( auto rd : m_vDelegates )
	{
		if ( rd == pDlete )
		{
			return ;
		}
	}

	m_vWillAdd.push_back(pDlete);
}

void IWebsocketNetwork::removeDelegate(IWebSocketDelegate* pDele)
{
	m_vWillRemove.push_back(pDele);
}

void IWebsocketNetwork::processDelegateChange()
{
	for ( auto tt : m_vWillAdd )
	{
		m_vDelegates.push_back(tt);
	}
	m_vWillAdd.clear();

	for ( auto dd : m_vWillRemove )
	{
		auto iter = m_vDelegates.begin() ;
		for ( ; iter != m_vDelegates.end(); ++iter )
		{
			if ( (*iter) == dd )
			{
				m_vDelegates.erase(iter) ;
				break;
			}
		}
	}
	m_vWillRemove.clear();
}

void IWebsocketNetwork::distributeMsg()
{
	processDelegateChange() ;
	stPacket* pPacket = nullptr ;

	{
		std::lock_guard<std::mutex> lock(m_mutexRecivedPacket);  
		auto iter = m_vRecivedPacket.begin() ;
		if ( iter != m_vRecivedPacket.end() )
		{
			pPacket = *iter ;
			m_vRecivedPacket.erase(iter) ;
		}
	}

	if ( pPacket == nullptr )
	{
		return ;
	}

	if ( pPacket->_packetType == _PACKET_TYPE_MSG )
	{
		for ( auto dele : m_vDelegates )
		{
			dele->onMsg(pPacket->nTarget,pPacket->strContent.c_str()) ;
		}
	}
	else if ( _PACKET_TYPE_CONNECTED == pPacket->_packetType )
	{
		for ( auto dele : m_vDelegates )
		{
			dele->onPeerConnected(pPacket->strContent.c_str(),pPacket->nTarget) ;
		}
	}
	else if ( _PACKET_TYPE_DISCONNECT == pPacket->_packetType )
	{
		for ( auto dele : m_vDelegates )
		{
			dele->onPeerClosed(pPacket->nTarget) ;
		}
	}
	else
	{
		printf("unknown web msg type = %u\n",pPacket->_packetType) ;
	}

	delete pPacket ;
	pPacket = nullptr ;
}

void IWebsocketNetwork::on_open(websocketpp::connection_hdl hdl)
{
	 stPacket* pPackt = new stPacket ;
	 pPackt->nTarget = ++m_nCurMaxNetID ;
	 auto pp = m_InternalSvr.get_con_from_hdl(hdl) ;
	 pPackt->strContent = pp->get_host();
	 pPackt->_packetType = _PACKET_TYPE_CONNECTED ;

	 stNetPeer* pPeer = new stNetPeer ;
	 pPeer->nNetID = pPackt->nTarget ;
	 pPeer->nConnectHandle = hdl ;
	 m_vNetPeers[hdl] = pPeer ;

	 std::lock_guard<std::mutex> lock(m_mutexRecivedPacket);  
	 m_vRecivedPacket.push_back(pPackt) ;
}

void IWebsocketNetwork::on_close(websocketpp::connection_hdl hdl)
{
	auto iter = m_vNetPeers.find(hdl) ;
	if ( iter == m_vNetPeers.end() )
	{
		printf("not connected player close\n") ;
		return ;
	}

	stPacket* pPackt = new stPacket ;
	pPackt->nTarget = iter->second->nNetID;
	pPackt->_packetType = _PACKET_TYPE_DISCONNECT ;
	std::lock_guard<std::mutex> lock(m_mutexRecivedPacket);  
	m_vRecivedPacket.push_back(pPackt) ;
	m_vNetPeers.erase(hdl) ;
}

void IWebsocketNetwork::validate_handler(websocketpp::connection_hdl hdl)
{
	auto pp = m_InternalSvr.get_con_from_hdl(hdl) ;
	//printf("origin : %s \n",pp->get_origin());
	//pp->append_header();
	pp->replace_header("Access-Control-Allow-Origin","*");
	return  ;
}

void IWebsocketNetwork::on_message(websocketpp::connection_hdl hdl, server::message_ptr msg)
{
	auto iter = m_vNetPeers.find(hdl) ;
	if ( iter == m_vNetPeers.end() )
	{
		printf("not connected player send msg \n") ;
		return ;
	}

	stPacket* pPackt = new stPacket ;
	pPackt->nTarget = iter->second->nNetID;
	pPackt->_packetType = _PACKET_TYPE_MSG ;
	pPackt->strContent = msg->get_payload();
	std::lock_guard<std::mutex> lock(m_mutexRecivedPacket);  
	m_vRecivedPacket.push_back(pPackt) ;
}

void IWebsocketNetwork::trySendMsg(const websocketpp::lib::error_code & ec)
{
	m_timer = m_InternalSvr.set_timer(18, bind(&IWebsocketNetwork::trySendMsg,this,::_1));
	std::lock_guard<std::mutex> lock(m_mutexSendPacket);  
	auto iter = m_vWillSendPacket.begin() ;
	if ( iter == m_vWillSendPacket.end() )
	{
		return ;
	}
	
	for ( auto pp : m_vNetPeers )
	{
		if ( pp.second->nNetID == (*iter)->nTarget )
		{
			m_InternalSvr.send(pp.first,(*iter)->strContent.c_str(),websocketpp::frame::opcode::text);
			break;
		}
	}

	delete (*iter);
	m_vWillSendPacket.erase(iter) ;
}

void IWebsocketNetwork::sendMsg(CONNECT_ID nNetID , const char* pContent )
{
	std::lock_guard<std::mutex> lock(m_mutexSendPacket);  
	stPacket* pp = new stPacket ;
	pp->nTarget = nNetID ;
	pp->strContent = pContent ;
	pp->_packetType = _PACKET_TYPE_MSG ;
	m_vWillSendPacket.push_back(pp) ;
}