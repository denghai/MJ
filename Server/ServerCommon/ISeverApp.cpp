#include <windows.h>
#include "ISeverApp.h"
#include "log4z.h"
#include "MessageDefine.h"
#include "ServerMessageDefine.h"
#include <time.h>
#include <assert.h>
#include <synchapi.h>
#include "IGlobalModule.h"
#include "AutoBuffer.h"
#include "AsyncRequestQuene.h"
#define TIME_WAIT_FOR_RECONNECT 5
bool IServerApp::init()
{
	m_bRunning = true;
	m_nTargetSvrNetworkID = INVALID_CONNECT_ID;
	
	m_pNetWork = new CNetWorkMgr ;
	m_pNetWork->SetupNetwork(1);
	m_pNetWork->AddMessageDelegate(this);

	m_pTimerMgr = CTimerManager::getInstance() ;

	m_fReconnectTick = 0 ;

	m_nFrameCnt = 0;
	m_fFrameTicket = 0;
	m_fOutputfpsTickt = 0;

	for ( uint16_t nIdx = eDefMod_None ; nIdx < eDefMod_Max ;  ++nIdx )
	{
		installModule(nIdx);
	}
	return true ;
}

IServerApp::IServerApp()
{
	m_pTimerMgr = nullptr ;
	m_pNetWork = nullptr ;
	m_eConnectState = CNetWorkMgr::eConnectType_None ;
	memset(&m_stConnectConfig,0,sizeof(m_stConnectConfig));
	m_fReconnectTick = 0 ;
	m_vAllModule.clear() ;
}

IServerApp::~IServerApp()
{
	for ( auto pp : m_vAllModule )
	{
		delete pp.second ;
		pp.second = nullptr ;
	}

	if ( m_pNetWork )
	{
		m_pNetWork->ShutDown() ;
		delete m_pNetWork ;
		m_pNetWork = nullptr ;
	}
}

bool IServerApp::OnMessage( Packet* pMsg )
{
	CHECK_MSG_SIZE(stMsg,pMsg->_len) ;
	stMsg* pmsg = (stMsg*)pMsg->_orgdata ;
	if ( pmsg->cSysIdentifer == ID_MSG_VERIFY )
	{
		LOGFMTI("no need recieve verify msg") ;
		return true ;
	}

	stMsg* pRet = pmsg;
	if ( pRet->usMsgType != MSG_TRANSER_DATA )
	{
		LOGFMTE("why msg type is not transfer data , type = %d",pRet->usMsgType ) ;
		return true;
	}

	stMsgTransferData* pData = (stMsgTransferData*)pRet ;
	stMsg* preal = (stMsg*)( pMsg->_orgdata + sizeof(stMsgTransferData));

	// check async request 
	if ( preal->usMsgType == MSG_ASYNC_REQUEST )
	{
		stMsgAsyncRequest* pRet = (stMsgAsyncRequest*)preal ;
		Json::Value jsReqContent ;
		if ( pRet->nReqContentLen > 0 )
		{
			char* pBuffer = (char*)pRet ;
			pBuffer += sizeof(stMsgAsyncRequest) ;
			Json::Reader jsReader ;
			jsReader.parse(pBuffer,pBuffer + pRet->nReqContentLen,jsReqContent,false);
		}

		Json::Value jsResult ;
		if ( !onAsyncRequest(pRet->nReqType,jsReqContent,jsResult) )
		{
			LOGFMTE("async request type = %u , not process from port = %u",pRet->nReqType,pData->nSenderPort) ;
			assert(0 && "must process the req" );
		}
		
		stMsgAsyncRequestRet msgBack ;
		msgBack.cSysIdentifer = (eMsgPort)pData->nSenderPort ;
		msgBack.nReqSerailID = pRet->nReqSerailID ;
		msgBack.nResultContentLen = 0 ;
		if ( jsResult.isNull() == true )
		{
			sendMsg(pData->nSessionID,(char*)&msgBack,sizeof(msgBack)) ;
		}
		else
		{
			Json::StyledWriter jsWrite ;
			auto strResult = jsWrite.write(jsResult);
			msgBack.nResultContentLen = strResult.size() ;
			CAutoBuffer auBuffer(sizeof(msgBack) + msgBack.nResultContentLen );
			auBuffer.addContent(&msgBack,sizeof(msgBack));
			auBuffer.addContent(strResult.c_str(),msgBack.nResultContentLen) ;
			sendMsg(pData->nSessionID,auBuffer.getBufferPtr(),auBuffer.getContentSize()) ;
		}
		return true ;
	}

	if ( preal->usMsgType == MSG_JSON_CONTENT  )
	{
		stMsgJsonContent* pRet = (stMsgJsonContent*)preal ;
		char* pBuffer = (char*)preal ;
		pBuffer += sizeof(stMsgJsonContent);
		//#ifdef __DEBUG
		static char pLog[1024] = { 0 };
		if ( pRet->nJsLen >= 1024 )
		{
			LOGFMTE("session id = %u send a invalid len json msg, len = %u ",pData->nSessionID,pRet->nJsLen) ;
			return true;
		}
		memset(pLog,0,sizeof(pLog)) ;
		memcpy_s(pLog,sizeof(pLog),pBuffer,pRet->nJsLen);
		LOGFMTD("session id = %u rec : %s",pData->nSessionID,pLog);
		
		//#endif // __DEBUG

		Json::Reader reader ;
		Json::Value rootValue ;
		auto bRet = reader.parse(pBuffer,pBuffer + pRet->nJsLen,rootValue,false) ;
		if ( !bRet )
		{
			LOGFMTE("session id = %u send a invalid json msg format error ",pData->nSessionID ) ;
			return true ;
		}

		if ( rootValue[JS_KEY_MSG_TYPE].isNull() || rootValue[JS_KEY_MSG_TYPE].isNumeric() == false )
		{
			LOGFMTE("not have msg key type , session id = %u rec : %s",pData->nSessionID,pLog);
			return true ;
		}

		uint16_t nMsgType = rootValue[JS_KEY_MSG_TYPE].asUInt() ;
		if ( onLogicMsg(rootValue,nMsgType,(eMsgPort)pData->nSenderPort,pData->nSessionID) )
		{
			return true ;
		}
		LOGFMTE("unprocessed json from port = %d , session id = %d js : %s",pData->nSenderPort,pData->nSessionID,pLog) ;
		return false ;
	}

	// normal logic msg ;
	if ( onLogicMsg(preal,(eMsgPort)pData->nSenderPort,pData->nSessionID) )
	{
		return true ;
	}

	LOGFMTE("unprocessed msg = %d , from port = %d , session id = %d",preal->usMsgType,pData->nSenderPort,pData->nSessionID) ;
	return false ;
}

bool IServerApp::OnLostSever(Packet* pMsg)
{
	m_nTargetSvrNetworkID = INVALID_CONNECT_ID ;
	LOGFMTE("Target server disconnected !") ;
	m_eConnectState = CNetWorkMgr::eConnectType_Disconnectd ;

	m_fReconnectTick = TIME_WAIT_FOR_RECONNECT ;// right now start reconnect ;
	return false ;
}

bool IServerApp::OnConnectStateChanged( eConnectState eSate, Packet* pMsg)
{
	m_eConnectState = eConnect_Accepted == eSate ? CNetWorkMgr::eConnectType_Connected : CNetWorkMgr::eConnectType_Disconnectd ;
	if ( eConnect_Accepted == eSate )
	{
		m_nTargetSvrNetworkID = pMsg->_connectID ;
		stMsg cMsg ;
		cMsg.cSysIdentifer = (uint8_t)getTargetSvrPortType() ;
		cMsg.usMsgType = getVerifyType() ;
		m_pNetWork->SendMsg((char*)&cMsg,sizeof(stMsg),pMsg->_connectID) ;
		LOGFMTI("Connected to Target Svr") ;
		onConnectedToSvr();
		return false ;
	}

	m_nTargetSvrNetworkID = INVALID_CONNECT_ID ;
	LOGFMTE("connect target svr failed, %d seconds later reconnect",TIME_WAIT_FOR_RECONNECT) ;
	return false ;
}

bool IServerApp::run()
{
	clock_t t = clock();
	while ( m_bRunning )
	{
		if ( m_pNetWork )
		{
			m_pNetWork->ReciveMessage();
		}

		clock_t tNow = clock();
		float fDelta = float(tNow - t ) / CLOCKS_PER_SEC ;
		t = tNow ;
		m_pTimerMgr->Update(fDelta);
		update(fDelta*m_pTimerMgr->GetTimeScale());
		Sleep(10);
	}

	onExit();
	LOGFMTI("sleep 4k mili seconds");
	Sleep(4000);
	shutDown();
	return true ;
}

void IServerApp::shutDown()
{
	if ( m_pNetWork )
	{
		m_pNetWork->ShutDown() ;
		m_eConnectState = CNetWorkMgr::eConnectType_None ;
	}
}

bool IServerApp::sendMsg( const char* pBuffer , int nLen )
{
	assert(m_pNetWork && "please invoke IServerApp init" );
	if ( isConnected() )
	{
		m_pNetWork->SendMsg(pBuffer,nLen,m_nTargetSvrNetworkID) ;
	}
	else
	{
		LOGFMTE("target is disconnect , can not send msg");
	}
	return isConnected() ;
}

bool IServerApp::sendMsg(  uint32_t nSessionID , const char* pBuffer , uint16_t nLen, bool bBroadcast )
{
	if ( isConnected() == false )
	{
		LOGFMTE("target svr is not connect , send msg failed") ;
		return false ;
	}
	stMsgTransferData msgTransData ;
	msgTransData.nSenderPort = getLocalSvrMsgPortType() ;
	msgTransData.bBroadCast = bBroadcast ;
	msgTransData.nSessionID = nSessionID ;
	int nLne = sizeof(msgTransData) ;
	if ( nLne + nLen >= MAX_MSG_BUFFER_LEN )
	{
		stMsg* pmsg = (stMsg*)pBuffer ;
		LOGFMTE("msg send to session id = %d , is too big , cannot send , msg id = %d ",nSessionID,pmsg->usMsgType) ;
		return false;
	}
	memcpy_s(m_pSendBuffer ,sizeof(m_pSendBuffer),&msgTransData,nLne);
	memcpy_s(m_pSendBuffer + nLne ,sizeof(m_pSendBuffer) - nLne, pBuffer,nLen );
	nLne += nLen ;
	sendMsg(m_pSendBuffer,nLne);
	return true ;
}

bool IServerApp::sendMsg( uint32_t nSessionID , Json::Value& recvValue, uint16_t nMsgID,uint8_t nTargetPort, bool bBroadcast )
{
	if ( nMsgID )
	{
		recvValue[JS_KEY_MSG_TYPE] = nMsgID;
		//if ( !recvValue[JS_KEY_MSG_TYPE] )
		//{
		//	recvValue[JS_KEY_MSG_TYPE] = nMsgID ;
		//}
		//else
		//{
		//	//LOGFMTE("msg id = %u ,already have this tag uid = %u",nMsgID,recvValue[JS_KEY_MSG_TYPE].asUInt() ) ;
		//}
	}

	Json::StyledWriter writerJs ;
	std::string strContent = writerJs.write(recvValue);
	LOGFMTD("session id = %u , target port = %u, send : %s",nSessionID,nTargetPort,strContent.c_str());
	stMsgJsonContent msg ;
	msg.cSysIdentifer = nTargetPort ;
	msg.nJsLen = strContent.size() ;
	CAutoBuffer bufferTemp(sizeof(msg) + msg.nJsLen);
	bufferTemp.addContent(&msg,sizeof(msg)) ;
	bufferTemp.addContent(strContent.c_str(),msg.nJsLen) ;
	//LOGFMTD("session id = %u , target port = %u, len = %u send : %s",nSessionID,nTargetPort,bufferTemp.getContentSize(),strContent.c_str());
	return sendMsg(nSessionID,bufferTemp.getBufferPtr(),bufferTemp.getContentSize(),bBroadcast) ; 
}

void IServerApp::stop()
{
	m_bRunning = false ;
}

bool IServerApp::onLogicMsg(stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID)
{
	for ( auto pp : m_vAllModule )
	{
		if ( pp.second->onMsg(prealMsg,eSenderPort,nSessionID) )
		{
			return true ;
		}
	}
	return false ;
}

bool IServerApp::onLogicMsg( Json::Value& recvValue , uint16_t nmsgType, eMsgPort eSenderPort , uint32_t nSessionID )
{
	for ( auto pp : m_vAllModule )
	{
		if ( pp.second->onMsg(recvValue,nmsgType,eSenderPort,nSessionID) )
		{
			return true ;
		}
	}
	return false ;
}

bool IServerApp::onAsyncRequest(uint16_t nRequestType , const Json::Value& jsReqContent, Json::Value& jsResult )
{
	for ( auto pp : m_vAllModule )
	{
		if ( pp.second->onAsyncRequest(nRequestType,jsReqContent,jsResult) )
		{
			return true ;
		}
	}
	return false ;
}

void IServerApp::update(float fDeta )
{
	if ( m_eConnectState == CNetWorkMgr::eConnectType_Disconnectd )
	{
		m_fReconnectTick += fDeta ;
		if ( m_fReconnectTick >= TIME_WAIT_FOR_RECONNECT )
		{
			LOGFMTI("Reconnecting....");
			doConnectToTargetSvr() ;
			m_fReconnectTick = 0 ;
		}
	}

	// moudle update ;
	for ( auto pp : m_vAllModule )
	{
		pp.second->update(fDeta);
	}

	// caculate fps 
	++m_nFrameCnt;
	m_fFrameTicket += fDeta;
	m_fOutputfpsTickt += fDeta;
	if (m_fFrameTicket >= 1.0f)
	{
		if ( m_fOutputfpsTickt > 50 )
		{
			//LOGFMTD("FPS : %u\n", m_nFrameCnt);
			m_fOutputfpsTickt = 0;
		}
		else
		{
			//printf("FPS : %u\n", m_nFrameCnt);
		}
		
		m_nFrameCnt = 0;
		m_fFrameTicket -= 1.0;
	}
}

uint16_t IServerApp::getTargetSvrPortType()
{
	return ID_MSG_PORT_CENTER ;
}

bool IServerApp::isConnected()
{
	return m_eConnectState == CNetWorkMgr::eConnectType_Connected;
}

void IServerApp::setConnectServerConfig(stServerConfig* pConfig )
{
	if ( pConfig == nullptr )
	{
		LOGFMTE("connect config is null") ;
		return ;
	}

	m_stConnectConfig = *pConfig ;

	if ( m_eConnectState != CNetWorkMgr::eConnectType_Connected && CNetWorkMgr::eConnectType_Connecting != m_eConnectState )
	{
		doConnectToTargetSvr();
	}
}

void IServerApp::doConnectToTargetSvr()
{
	if ( m_eConnectState == CNetWorkMgr::eConnectType_Connecting || isConnected() )
	{
		return ;
	}

	assert(m_pNetWork && "IServer init not invoke" ) ;
	assert(m_stConnectConfig.nPort && "please set connect config" ) ;
	m_pNetWork->ConnectToServer(m_stConnectConfig.strIPAddress,m_stConnectConfig.nPort,m_stConnectConfig.strPassword) ;
	m_eConnectState = CNetWorkMgr::eConnectType_Connecting ;
	LOGFMTI("connecting to target svr ip = %s", m_stConnectConfig.strIPAddress );
}
uint16_t IServerApp::getVerifyType()
{
	switch ( getLocalSvrMsgPortType() )
	{
	case ID_MSG_PORT_CLIENT:
		return MSG_VERIFY_CLIENT ;
	case ID_MSG_PORT_GATE:
		return MSG_VERIFY_GATE ;
	case ID_MSG_PORT_LOGIN:
		return MSG_VERIFY_LOGIN ;
	case ID_MSG_PORT_VERIFY:
		return MSG_VERIFY_VERIYF ;
	case ID_MSG_PORT_MJ:
		return MSG_VERIFY_MJ;
	case ID_MSG_PORT_APNS:
		return MSG_VERIFY_APNS ;
	case ID_MSG_PORT_LOG:
		return MSG_VERIFY_LOG; 
	case ID_MSG_PORT_DATA:
		return MSG_VERIFY_DATA ;
	case ID_MSG_PORT_TAXAS:
		return MSG_VERIFY_TAXAS;
	case ID_MSG_PORT_DB:
		return MSG_VERIFY_DB ;
	case ID_MSG_PORT_NIU_NIU:
		return MSG_VERIFY_NIU_NIU ;
	case ID_MSG_PORT_GOLDEN:
		return MSG_VERIFY_GOLDEN;
	default:
		assert(0 && "what verify type for the svr ? " ) ;
		break;
	}
	return MSG_VERIFY_END ;
}

void IServerApp::onExit()
{
	for ( auto pp : m_vAllModule )
	{
		pp.second->onExit();
	}
	getNetwork()->RemoveAllDelegate();
}

void IServerApp::onConnectedToSvr()
{
	for ( auto pp : m_vAllModule )
	{
		pp.second->onConnectedSvr();
	}
}

bool IServerApp::registerModule(IGlobalModule* pModule,uint16_t eModuleType)
{
	//assert(pModule && "this module is null" );
	if ( pModule == nullptr )
	{
		return false;
	}

	if ( eModuleType == IGlobalModule::INVALID_MODULE_TYPE )
	{
		assert(0&&"please set the module type ");
		return false ;
	}

	pModule->setModuleType(eModuleType);
	auto pp = getModuleByType(eModuleType) ;
	assert(pp == nullptr && "already have this module" );
	if ( pp )
	{
		return false ;
	}

	m_vAllModule[pModule->getModuleType()] = pModule ;
	pModule->init(this);
	return true ;
}

bool IServerApp::installModule( uint16_t nModuleType )
{
	auto pAsy = createModule(nModuleType) ;
	return registerModule(pAsy,nModuleType) ;
}

IGlobalModule* IServerApp::getModuleByType(uint16_t nType )
{
	auto pp = m_vAllModule.find(nType) ;
	if ( pp != m_vAllModule.end() )
	{
		return pp->second ;
	}
	return nullptr;
}

IGlobalModule* IServerApp::createModule( uint16_t eModuleType )
{
	IGlobalModule* pModule = nullptr ;
	switch (eModuleType)
	{
	case eDefMod_AsyncRequestQueu:
		{
			pModule = new CAsyncRequestQuene ;
		}
		break;
	default:
		break;
	}

	return pModule ;
}

CAsyncRequestQuene* IServerApp::getAsynReqQueue()
{
	return (CAsyncRequestQuene*)getModuleByType(eDefMod_AsyncRequestQueu);
}