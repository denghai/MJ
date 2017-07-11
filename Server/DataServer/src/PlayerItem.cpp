#include "PlayerItem.h"
#include "ServerMessageDefine.h"
#include "LogManager.h"
#include "Player.h"
#include "ItemConfig.h"
#include "GameServerApp.h"
#include "ConfigManager.h"
#include "PlayerBaseData.h"
#include "PlayerEvent.h"
bool CPlayerItemComponent::OnMessage(stMsg* pMsg )
{
	switch ( pMsg->usMsgType )
	{
	case MSG_REQUEST_ITEM_LIST:
		{
			if ( ID_MSG_C2GAME == pMsg->cSysIdentifer )
			{
				SendToClient();
			}
			else if ( ID_MSG_DB2GM == pMsg->cSysIdentifer )
			{
				stMsgGameServerRequestItemListRet* pMsgRet = (stMsgGameServerRequestItemListRet*)pMsg ;
				if ( pMsgRet->nOwnItemKindCount <= 0 )
				{
					return true;
				}
				unsigned short nCount = pMsgRet->nOwnItemKindCount;
				char* pBuffer = (char*)pMsg ;
				pBuffer += sizeof(stMsgGameServerRequestItemListRet);
				while(nCount--)
				{
					stPlayerItem* pItem = new stPlayerItem ;
					memcpy(pItem,pBuffer,sizeof(stPlayerItem));
					pBuffer += sizeof(stPlayerItem);
					m_vAllPlayerItems[pItem->nItemID] = pItem ;
				}
			}
			else
			{
				CLogMgr::SharedLogMgr()->ErrorLog("unknown come from of msg MSG_REQUEST_ITEM_LIST") ;
			}
		}
		break;
	case MSG_PLAYER_PAWN_ASSERT:
		{
			stMsgPlayerPawnAsset* pRet = (stMsgPlayerPawnAsset*)pMsg ;
			stMsgPlayerPawnAssetRet msgBack ;
			msgBack.nAssertItemID = pRet->nAssetItemID ; 
			msgBack.nCount = pRet->nCount  ;
			msgBack.nRet = 0 ;			
			CItemConfigManager* pConfigMgr  = (CItemConfigManager*)CGameServerApp::SharedGameServerApp()->GetConfigMgr()->GetConfig(CConfigManager::eConfig_Item) ;
			stItemConfig* pConfig = pConfigMgr->GetItemConfigByItemID(pRet->nAssetItemID) ;
			if ( !pConfig || pRet->nAssetItemID >= eItem_Asset )
			{
				msgBack.nRet = 2 ;
				CLogMgr::SharedLogMgr()->ErrorLog("pawn unknown assert item id = %d",pRet->nAssetItemID) ;
			}
			else
			{
				if ( OnUserItem(pRet->nAssetItemID,pRet->nCount) == false )
				{
					msgBack.nRet = 1 ;
				}
				else
				{
					GetPlayer()->GetBaseData()->ModifyMoney(pConfig->nPrizeCoin * pRet->nCount);
					GetPlayer()->GetBaseData()->ModifyMoney(pConfig->nPrizeDiamoned * pRet->nCount,true);
				}
			}
			msgBack.nFinalCoin = GetPlayer()->GetBaseData()->GetAllCoin() ;
			msgBack.nFinalDiamond = GetPlayer()->GetBaseData()->GetAllDiamoned();
			SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
		}
		break;
	default:
		return false ;
	}
	return true ; 
}

void CPlayerItemComponent::OnPlayerDisconnect()
{
	// save to db 
	SaveToDB();
}

void CPlayerItemComponent::Reset()
{ 
	ClearAll() ;
	// send request ;
	stMsgGameServerRequestItemList msg ;
	msg.nSessionID = GetPlayer()->GetSessionID() ;
	msg.nUserUID = GetPlayer()->GetUserUID() ;
	SendMsgToDB((char*)&msg,sizeof(msg)) ;
}

unsigned int CPlayerItemComponent::GetItemCountByID(unsigned short nItemID )
{
	MAP_ALL_PLAYER_ITEMS::iterator iter = m_vAllPlayerItems.find(nItemID) ;
	if ( iter == m_vAllPlayerItems.end() )
	{
		return 0 ;
	}
	return iter->second->nCount ;
}

bool CPlayerItemComponent::OnUserItem( unsigned short nItemID , unsigned int nCount ,CPlayer* pTarget )
{
	if ( pTarget == NULL )
	{
		pTarget = GetPlayer() ;
	}
	// decrease item ;
	MAP_ALL_PLAYER_ITEMS::iterator iter = m_vAllPlayerItems.find(nItemID) ;
	if ( iter == m_vAllPlayerItems.end() )
	{
		CLogMgr::SharedLogMgr()->PrintLog( "can not find the item you used , item id = %d" ,nItemID) ;
		return false ;
	}
	else
	{
		if ( iter->second->nCount < nCount )
		{
			CLogMgr::SharedLogMgr()->PrintLog("lack of item , item id = %d",nItemID) ;
			return false ;
		}
		else
		{
			iter->second->nCount -= nCount ;
			if ( iter->second->nCount == 0 )
			{
				delete iter->second ;
				m_vAllPlayerItems.erase(iter) ;
			}
			m_bDirty = true ;
		}
	}

	CItemConfigManager* pConfigMgr  = (CItemConfigManager*)CGameServerApp::SharedGameServerApp()->GetConfigMgr()->GetConfig(CConfigManager::eConfig_Item) ;
	stItemConfig* pConfig = pConfigMgr->GetItemConfigByItemID(nItemID) ;
	if ( pConfig->eType == eItem_Gift )
	{
		OnUseGift((stItemGift*)pConfig) ;
	}
	return true ;
}

void CPlayerItemComponent::OnUseGift(stItemGift*pGift)
{
	double fValue = ( (double)rand() / (double)RAND_MAX ) * (double)pGift->nCoinRand ;
	uint64_t nAddCoin = pGift->nCoin + fValue; 
	GetPlayer()->GetBaseData()->ModifyMoney(nAddCoin,false);

	fValue = ( (double)rand() / (double)RAND_MAX ) * (double)pGift->nDiamoned ;
	uint64_t nAddDiamoned = pGift->nDiamoned + fValue; 
	GetPlayer()->GetBaseData()->ModifyMoney(nAddDiamoned,true);

	stMsgPlayerUseGigtRet msgBackMsg ;
	msgBackMsg.nRet = 0 ;
	msgBackMsg.nGiftID = pGift->nItemID ;
	msgBackMsg.nGetCoin = nAddCoin ;
	msgBackMsg.nGetDiamoned = nAddDiamoned ;
	msgBackMsg.nGetItemCount = pGift->vItemsAndCount.size();
	char* pBuffer = new char[sizeof(msgBackMsg) + msgBackMsg.nGetItemCount * sizeof(stPlayerItem)] ;
	unsigned short nOffset = 0 ;
	memcpy(pBuffer,&msgBackMsg,sizeof(msgBackMsg));
	nOffset += sizeof(msgBackMsg);
	// give item 
	stPlayerItem PlayerItem ;
	std::map<unsigned short, unsigned short>::iterator iter = pGift->vItemsAndCount.begin();
	for ( ; iter != pGift->vItemsAndCount.end(); ++iter )
	{
		AddItemByID(iter->first,iter->second) ;
		PlayerItem.nItemID = iter->first ;
		PlayerItem.nCount = iter->second ;
		memcpy(pBuffer + nOffset , &PlayerItem,sizeof(stPlayerItem));
		nOffset += sizeof(stPlayerItem);
	}
	SendMsgToClient(pBuffer,nOffset) ;
	delete[] pBuffer ;
}

bool CPlayerItemComponent::OnPlayerEvent(stPlayerEvetArg* pArg)
{
	if ( pArg->eEventType == ePlayerEvent_ReadDBOK )
	{
		SendToClient();
	}
	return false ;
}

void CPlayerItemComponent::AddItemByID(unsigned short nItemID , unsigned int nCount )
{
	MAP_ALL_PLAYER_ITEMS::iterator iter = m_vAllPlayerItems.find(nItemID) ;
	if ( iter == m_vAllPlayerItems.end() )
	{
		stPlayerItem* playeritem = new stPlayerItem;
		playeritem->nCount = nCount ;
		playeritem->nItemID = nItemID ;
		m_vAllPlayerItems[nItemID] = playeritem ;
	}
	else
	{
		iter->second->nCount += nCount ;
	}
	m_bDirty = true ;

	// if right now open gift ?
	CItemConfigManager* pConfigMgr  = (CItemConfigManager*)CGameServerApp::SharedGameServerApp()->GetConfigMgr()->GetConfig(CConfigManager::eConfig_Item) ;
	stItemConfig* pConfig = pConfigMgr->GetItemConfigByItemID(nItemID) ;
	if ( pConfig && pConfig->eType == eItem_Gift )
	{
		stItemGift* pGift = (stItemGift*)pConfig ;
		if ( pGift->bOpenRightNow )
		{
			while ( nCount--)
			{
				OnUseGift(pGift) ;
			}
		}
	}
	return ;
}

void CPlayerItemComponent::ClearAll()
{
	MAP_ALL_PLAYER_ITEMS::iterator iter = m_vAllPlayerItems.begin() ;
	for ( ; iter != m_vAllPlayerItems.end(); ++iter )
	{
		delete iter->second ;
		iter->second = NULL ;
	}
	m_vAllPlayerItems.clear() ;
	m_bDirty = false ;
}

void CPlayerItemComponent::SaveToDB()
{
	if ( !m_bDirty )
	{
		return ;
	}
	m_bDirty = false ;

	stMsgGameServerSaveItemList msg ;
	msg.nOwnItemKindCount = m_vAllPlayerItems.size() ;
	msg.nSessionID = GetPlayer()->GetSessionID() ;
	msg.nUserUID = GetPlayer()->GetUserUID() ;
	char* pBuffer = new char[sizeof(msg) + msg.nOwnItemKindCount * sizeof(stPlayerItem)] ;
	memcpy(pBuffer,&msg,sizeof(msg));
	unsigned short nOffset = 0 ;
	nOffset += sizeof(msg);
	MAP_ALL_PLAYER_ITEMS::iterator iter = m_vAllPlayerItems.begin() ;
	for ( ; iter != m_vAllPlayerItems.end(); ++iter )
	{
		stPlayerItem* pItem = iter->second ;
		memcpy(pBuffer + nOffset , pItem,sizeof(stPlayerItem));
		nOffset += sizeof(stPlayerItem);
	}
	SendMsgToDB(pBuffer,nOffset) ;
	delete[] pBuffer ;
}

void CPlayerItemComponent::SendToClient()
{
	stMsgPlayerRequestItemListRet msg ;
	msg.nPlayerItemCount = m_vAllPlayerItems.size() ;
	char* pBuffer = new char[sizeof(msg) + sizeof(stPlayerItem) * msg.nPlayerItemCount ];
	int nOffset = 0 ;
	memcpy(pBuffer,&msg,sizeof(msg));
	nOffset += sizeof(msg);
	MAP_ALL_PLAYER_ITEMS::iterator iter = m_vAllPlayerItems.begin() ;
	for ( ; iter != m_vAllPlayerItems.end(); ++iter )
	{
		memcpy( pBuffer + nOffset , iter->second,sizeof(stPlayerItem)) ;
		nOffset += sizeof(stPlayerItem);
	}
	SendMsgToClient(pBuffer,nOffset) ;
	delete[] pBuffer ;
}