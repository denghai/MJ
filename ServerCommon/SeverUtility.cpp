#include "SeverUtility.h"
#include "AutoBuffer.h"
#include "ServerMessageDefine.h"
#include "AsyncRequestQuene.h"
void CSendPushNotification::reset()
{
	return;
	m_arrayTargetIDs.clear() ;
	m_strApns.clear() ;
}

void CSendPushNotification::addTarget(uint32_t nUserUID )
{
	return;
	m_arrayTargetIDs[m_arrayTargetIDs.size()] = nUserUID ;
}

void CSendPushNotification::setContent(const char* pContent,uint32_t nFlag )
{
	return;
	m_strApns["content"] = pContent ;
}

void CSendPushNotification::postApns( CAsyncRequestQuene* pAsync , bool isGroup, const char* pmsgID , const char* pmsgdesc )
{
	return;
	if ( m_arrayTargetIDs.isNull() || m_strApns["content"].isNull() )
	{
		printf("push notice argument not finish\n") ;
		return ;
	}

	m_strApns["apnsType"] = isGroup ? 0 : 1 ;
	m_strApns["targets"] = m_arrayTargetIDs ;
	m_strApns["msgID"] = pmsgID != nullptr ? pmsgID : "def" ;
	m_strApns["msgdesc"] = pmsgdesc != nullptr ? pmsgdesc : "desc" ;
	pAsync->pushAsyncRequest(ID_MSG_PORT_VERIFY,eAsync_Apns,m_strApns);
	m_strApns.clear();
}