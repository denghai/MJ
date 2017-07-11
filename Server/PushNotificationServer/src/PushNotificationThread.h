#pragma once
#include "openssl/rsa.h" 
#include "openssl/crypto.h"
#include "openssl/x509.h"
#include "openssl/pem.h"
#include "openssl/ssl.h"
#include "openssl/err.h"
#include "openssl/rand.h"
#include "ThreadMod.h"
struct stNotice ;
class CPushNotificationThread
	:public CThreadT
{
public:
	CPushNotificationThread();
	~CPushNotificationThread();
	bool ConnectToAPNs();
	virtual void __run(){ ProcessWork();}
	void StopServer(){ m_bRunning = false ;}
	void ProcessWork();
	bool InitSSLContex();
protected:
	void ProduceSendBuffer(char** pBuffer , int& nLen , stNotice* pSendNotice );
	void Disconnect();
	void Reconnect();
protected:
	bool m_bRunning ;
	int m_nSocket;  
	SSL_CTX* m_pctx;  
	SSL* m_pSSL;  
	int m_vSeed_int[100]; /*´æ·ÅËæ»úÐòÁÐ*/  
};
