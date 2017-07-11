#include "PushNotificationThread.h"
#include "PushRequestQueue.h"
#include "log4z.h"
#pragma comment(lib, "ws2_32.lib")
#define RECONNECT_TIME (60*26)
#define CHK_NULL(x) if ((x)==NULL) exit (1)
#define CHK_ERR(err,s) if ((err)==-1) { perror(s); exit(1); }
#define CHK_SSL(err) if ((err)==-1) { ERR_print_errors_fp(stderr); exit(2); }


#define CERTF "./pem/ck.pem" /*服务端的证书(需经CA签名)*/
#define KEYF "./pem/ck.pem" /*服务端的私钥(建议加密存储)*/

#ifdef NDEBUG
#define HOST_NAME "gateway.push.apple.com"
#else
#define HOST_NAME "gateway.sandbox.push.apple.com"
#endif

#define PORT 2195 /*服务端的端口*/

CPushNotificationThread::CPushNotificationThread()
{
	m_bRunning = true;
	m_nSocket = 0 ;  
	m_pctx = NULL;  
	m_pSSL = NULL;  
	memset(m_vSeed_int,0,sizeof(m_vSeed_int)) ;
}

CPushNotificationThread::~CPushNotificationThread()
{
	Disconnect();
}

bool CPushNotificationThread::InitSSLContex()
{
	//SSL_library_init(); 
	OpenSSL_add_ssl_algorithms(); /*初始化*/  
	SSL_load_error_strings(); /*为打印调试信息作准备*/  
	m_pctx = SSL_CTX_new (SSLv23_client_method());  
	CHK_NULL(m_pctx); 

	if (SSL_CTX_use_certificate_file(m_pctx, CERTF, SSL_FILETYPE_PEM) <= 0)   
	{  
		LOGFMTE("客户端证书检查失败！！\n");
		return false;  
	}  

	if (SSL_CTX_use_PrivateKey_file(m_pctx, KEYF, SSL_FILETYPE_PEM) <= 0)   
	{  
		LOGFMTE("客户端key检查失败！\n");
		return false;  
	}  

	if (!SSL_CTX_check_private_key(m_pctx))   
	{  
		LOGFMTE("客户端证书和key不匹配!\n");
		return false;  
	}
	return true ;
}

bool CPushNotificationThread::ConnectToAPNs()
{
	WSADATA wsaData;  

	if(WSAStartup(MAKEWORD(2,2),&wsaData) != 0){  
		printf("WSAStartup()fail:%d\n",GetLastError());  
		return false;  
	}


	srand( (unsigned)time( NULL ) );  
	for( int i = 0; i < 100;i++ ) m_vSeed_int[i] = rand();  

	RAND_seed(m_vSeed_int, sizeof(m_vSeed_int));  

	m_nSocket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);   
	if(m_nSocket == INVALID_SOCKET)  
	{  
		LOGFMTE("套接字创建失败!\n");
		return false ;
	}  

	struct sockaddr_in sa;  
	memset(&sa,'\0', sizeof(sa));  
	struct hostent *hp; 
	if(!(hp=gethostbyname(HOST_NAME))) 
	{
		LOGFMTE("GET HOST BY NAME EEROR , name = %s\n",HOST_NAME) ;
		return false ;
	}
	sa.sin_family = AF_INET;  
	sa.sin_addr = *(struct in_addr*)hp->h_addr_list[0]; 
	sa.sin_port = htons (PORT); /* Server Port number */  

	//TCP连接  
	int err = 0;   
	err = connect(m_nSocket, (struct sockaddr*) &sa,sizeof(sa));  
	if(err == -1)  
	{  
		LOGFMTE( "TCP连接失败！\n");
		return false;  
	}  
	else  
	{  
		LOGFMTI( "TCP连接成功！\n");
	}  
	//SSL连接  
	//新建SSL  
	m_pSSL = SSL_new (m_pctx);   
	if(m_pSSL == NULL)  
	{  
		LOGFMTE("新建SSL失败!\n");
		return false ;
	}  
	//套接字和SSL绑定  
	SSL_set_fd (m_pSSL, m_nSocket);  
	//SLL连接  
	err = SSL_connect (m_pSSL);  
	if(err <= 0 )  
	{  
		int nRet = SSL_get_error(m_pSSL,err);
		LOGFMTE("SSL连接失败 nRet = %d \n",nRet) ;
		return false ;
	}  
	else  
	{  
		LOGFMTI("SSL连接成功\n") ;
	}  
	//打印连接信息  
#ifndef NDEBUG
	printf("SSL连接算法信息：%s\n",SSL_get_cipher (m_pSSL));
	/*得到服务端的证书并打印些信息(可选) */  
	X509* server_cert  = SSL_get_peer_certificate (m_pSSL);  
	CHK_NULL(server_cert);
	if (server_cert != NULL)
	{  
		LOGFMTI("服务器证书:\n");
		LOGFMTI("subject: %s \n",X509_NAME_oneline (X509_get_subject_name (server_cert),0,0));
		LOGFMTI("issuer: %s",X509_NAME_oneline(X509_get_issuer_name(server_cert),0,0)); 
		X509_free (server_cert);/*如不再需要,需将证书释放 */  
	}  
	else
	{
		LOGFMTE("服务器没有证书信息！\n") ;//服务器端认证失败  
	}
#endif
	return true;
}

void CPushNotificationThread::ProcessWork()
{
	static time_t nLastActive = time(NULL);
	while(m_bRunning)  
	{     
		LIST_NOTICES vGetNotice ;
		CPushRequestQueue::SharedPushRequestQueue()->GetAllNoticeToProcess(vGetNotice);
		LIST_NOTICES::iterator iter = vGetNotice.begin() ;
		char* pBuffer = 0  ;
		int nLen = 0 ;
		stNotice* pNotice = NULL ;
		time_t nCurNow = time(NULL) ;
		time_t nTimeIdle = 0 ;
		for ( ; iter != vGetNotice.end() ; ++iter )
		{
			nTimeIdle = nCurNow - nLastActive;
			if (  nTimeIdle > RECONNECT_TIME )
			{
				LOGFMTI("idle too long , we reconnected ; idle Time = %d ",nTimeIdle ) ;
				Reconnect();
			}
			pNotice = *iter ;
			ProduceSendBuffer(&pBuffer,nLen,pNotice);
			if ( nLen > 0 && pBuffer != NULL )
			{	
				unsigned char nTryTimes = 1 ;
				
				while( m_pSSL == nullptr || SSL_write(m_pSSL, pBuffer, nLen) <= 0 )  // maybe disconnect ;
				{
					time_t nRunnedTime = time(NULL) - nLastActive ;
					LOGFMTI("SSL connect to APNS Disconnected , trying to Reconnect times = %d ! runned time = %d",nTryTimes,nRunnedTime ) ;
					Reconnect();
					++nTryTimes ;
					if ( nTryTimes >= 5 )
					{
						break;   // avoid unlimit loop ;
					}
				}
				nLastActive = time(NULL) ;   /// test value ;
			}
		}
		CPushRequestQueue::SharedPushRequestQueue()->PushProcessedNotice(vGetNotice) ;
		vGetNotice.clear();
		Sleep(20) ;
	} 
}

void CPushNotificationThread::ProduceSendBuffer(char** pBuffer , int& nLen , stNotice* pSendNotice )
{
	static char payloadBinary[300] ={0};
	memset(payloadBinary,0,sizeof(payloadBinary)) ;
	if ( strlen(pSendNotice->cSound) == 0 )
	{
		sprintf_s(payloadBinary,"{\"aps\":{\"alert\":\"%s\",\"badge\":%d ,\"sound\" : \"default\"} }",pSendNotice->pAlert,pSendNotice->nBadge);
	}
	else
	{
		sprintf_s(payloadBinary,"{\"aps\":{\"alert\":\"%s\",\"badge\":%d ,\"sound\" : \"%s\"}}",pSendNotice->pAlert,pSendNotice->nBadge,pSendNotice->cSound);
	}
	static char pTemp[293] ;
	memset(pTemp,0,sizeof(pTemp)) ;
	size_t payloadLength = strlen(payloadBinary);

	// Define some variables.
	unsigned char command = 0;
	*pBuffer = pTemp ;
	char *pointer = pTemp;
	unsigned short networkTokenLength = htons(32);
	unsigned short networkPayloadLength = htons(payloadLength);

	// Compose message.
	memcpy(pointer, &command, sizeof(unsigned char));
	pointer += sizeof(unsigned char);
	memcpy(pointer, &networkTokenLength, sizeof(unsigned short));
	pointer += sizeof(unsigned short);
	memcpy(pointer,pSendNotice->pDeveiceToken, 32);
	pointer += 32;
	memcpy(pointer, &networkPayloadLength, sizeof(unsigned short));
	pointer += sizeof(unsigned short);
	memcpy(pointer, payloadBinary, payloadLength);
	pointer += payloadLength;
	nLen = pointer - pTemp ;
}

void CPushNotificationThread::Disconnect()
{
	if ( m_pSSL )
	{
		SSL_shutdown (m_pSSL);  
		SSL_free (m_pSSL);  
		SSL_CTX_free (m_pctx);  
		m_pSSL = NULL ;
		m_pctx = NULL ;
	}

	if ( m_nSocket )
	{
		shutdown (m_nSocket,2);  
		closesocket(m_nSocket);
		m_nSocket = 0 ;
	}
}

void CPushNotificationThread::Reconnect()
{
	if ( m_pSSL )
	{
		SSL_shutdown (m_pSSL);  
		SSL_free (m_pSSL);  
		m_pSSL = NULL ;
	}

	if ( m_nSocket )
	{
		shutdown(m_nSocket,2);  
		closesocket(m_nSocket);
		m_nSocket = 0 ;
	}
	ConnectToAPNs();
}