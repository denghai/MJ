#include "IPlayerComponent.h"
#include "Player.h"
IPlayerComponent::IPlayerComponent(CPlayer* pPlayer )
	:m_pPlayer(pPlayer),m_eType(ePlayerComponent_None)
{

}

IPlayerComponent::~IPlayerComponent()
{

}

bool IPlayerComponent::OnMessage( stMsg* pMessage , eMsgPort eSenderPort)
{
	return false ;
}

bool IPlayerComponent::onCrossServerRequest(stMsgCrossServerRequest* pRequest, eMsgPort eSenderPort,Json::Value* vJsValue )
{
	return false ;
}

bool IPlayerComponent::onCrossServerRequestRet(stMsgCrossServerRequestRet* pResult,Json::Value* vJsValue )
{
	return false ;
}

void IPlayerComponent::SendMsg(stMsg* pbuffer , unsigned short nLen , bool bBrocast  )
{
	m_pPlayer->SendMsgToClient((char*)pbuffer,nLen,bBrocast);
}

void IPlayerComponent::SendMsg(Json::Value& jsMsg , uint16_t nMsgType , bool bBrocast )
{
	m_pPlayer->SendMsgToClient(jsMsg,nMsgType,bBrocast);
}
