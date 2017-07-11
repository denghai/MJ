#include "PlayerShop.h"
#include "ServerMessageDefine.h"
#include "LogManager.h"
#include "Player.h"
#include "ShopConfg.h"
#include "GameServerApp.h"
#include "ConfigManager.h"
#include "Player.h"
#include "PlayerBaseData.h"
#include "PlayerManager.h"
#include "PlayerItem.h"
#include "PlayerMail.h"
#include "PlayerEvent.h"
CPlayerShop::CPlayerShop(CPlayer* pPlayer)
	:IPlayerComponent( pPlayer )
{
	m_vAllBuyedShopItems.clear() ;
}

CPlayerShop::~CPlayerShop()
{
	Clear();
}

void CPlayerShop::OnPlayerDisconnect()
{
	SavePlayerShopToDB() ;
}

bool CPlayerShop::OnMessage(stMsg* pMsg )
{
	switch ( pMsg->usMsgType )
	{
	case MSG_PLAYER_REQUEST_SHOP_LIST:
		{
			SendShopListToClient();
		}
		break;
	case MSG_GET_SHOP_BUY_RECORD:
		{
			//stMsgGameServerGetShopBuyRecordRet* pMsgRet = (stMsgGameServerGetShopBuyRecordRet*)pMsg ;
			//m_bInsertToDB = pMsgRet->nBufferLen <= 0 ;
			//if ( pMsgRet->nBufferLen == 0 )
			//{
			//	CLogMgr::SharedLogMgr()->PrintLog("No buy record log") ;
			//	return true;
			//}
			//char* pBuffer = (char*)pMsgRet ;
			//pBuffer += sizeof(stMsgGameServerGetShopBuyRecordRet);
			//unsigned short nCount = *((unsigned short*)pBuffer) ;
			//pBuffer += sizeof(unsigned short);
			//while ( nCount-- )
			//{
			//	stBuyedShopItem* pItem = new stBuyedShopItem ;
			//	memcpy(pItem,pBuffer,sizeof(stBuyedShopItem));
			//	pBuffer += sizeof(stBuyedShopItem);
			//	m_vAllBuyedShopItems.push_back(pItem) ;
			//}
		}
		break;
	case MSG_VERIFY_TANSACTION:
		{
			stMsgFromVerifyServer* pMsgRet = (stMsgFromVerifyServer*)pMsg ;
			// 0 success , 1 money not enough , 2 verify failed , 3 buy times limit , 4 shop item out of date, 5 shopitem don't exsit , 6 unknown error;
			stMsgPlayerBuyShopItemRet msgBack ;
			msgBack.nRet = 0 ;
			msgBack.nBuyShopItemForUserUID = pMsgRet->nBuyForPlayerUserUID ;
			msgBack.nShopItemID = pMsgRet->nShopItemID ;
			if ( pMsgRet->nRet != 4 )
			{
				msgBack.nRet = 2;
				SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
				return true ;
			}

			CPlayer* pTargetPlayer = GetPlayer() ;
			if ( pMsgRet->nBuyForPlayerUserUID != 0 )
			{
				pTargetPlayer = CGameServerApp::SharedGameServerApp()->GetPlayerMgr()->GetPlayerByUserUID(pMsgRet->nBuyForPlayerUserUID,false);
			}

			bool bDeliverByMail = pTargetPlayer == NULL ;

			CShopConfigMgr* pShopCinfig = (CShopConfigMgr*)CGameServerApp::SharedGameServerApp()->GetConfigMgr()->GetConfig(CConfigManager::eConfig_Shop) ;
			stShopItem* pShopItem = pShopCinfig->GetShopItem(pMsgRet->nShopItemID);

			// is your first recharge ?
			bool bDouble = GetPlayer()->GetBaseData()->GetRechargeTimes() == 0 ;
			// give item 
			if ( bDeliverByMail )
			{
				CPlayerMailComponent* pPlayerMail = (CPlayerMailComponent*)GetPlayer()->GetComponent(ePlayerComponent_Mail) ;
				pPlayerMail->PostGiftMail(GetPlayer(),pMsgRet->nBuyForPlayerUserUID,0,pMsgRet->nShopItemID,(bDouble ? 2 : 1)) ; // must diamoned , RMB ;
			}
			else
			{
				CPlayerShop* pPS = (CPlayerShop*)pTargetPlayer->GetComponent(ePlayerComponent_PlayerShop) ;
				pPS->OnGetShopItem(pShopItem,(bDouble ? 2 : 1),GetPlayer()->GetUserUID() == pTargetPlayer->GetUserUID(),GetPlayer()) ;
			}

			msgBack.nDiamoned = GetPlayer()->GetBaseData()->GetAllDiamoned() ;
			msgBack.nFinalyCoin = GetPlayer()->GetBaseData()->GetAllCoin() ;
			msgBack.nFinalTakeInDiamoned = (unsigned int)GetPlayer()->GetBaseData()->GetTakeInMoney(true);
			msgBack.nFinalyTakeInCoin = GetPlayer()->GetBaseData()->GetTakeInMoney();
			msgBack.nSavedMoneyForVip = 0 ;
			SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;

			// ad to buyed record 
			//stBuyedShopItem* pHadBuyItem = GetBuyedShpItem(pShopItem->nShopItemID) ;
			//if ( pHadBuyItem )
			//{
			//	++pHadBuyItem->nTimes;
			//}
			//else
			//{
			//	pHadBuyItem = new stBuyedShopItem ;
			//	pHadBuyItem->nShopItemID = pShopItem->nShopItemID ;
			//	pHadBuyItem->nTimes = 1 ;
			//	m_vAllBuyedShopItems.push_back(pHadBuyItem) ;
			//}

			// on post player event ;
			stPlayerEventArgRecharge eventArg ;
			eventArg.eEventType = ePlayerEvent_Recharge ;
			eventArg.nRMB = pShopItem->nPrize ;
			eventArg.nShopItemID = pShopItem->nShopItemID ;
			GetPlayer()->PostPlayerEvent(&eventArg) ;
		}
		break; 
	case MSG_BUY_SHOP_ITEM:
		{
			stMsgPlayerBuyShopItem* pMsgRet = (stMsgPlayerBuyShopItem*)pMsg ;
			// 0 success , 1 money not enough , 2 verify failed , 3 buy times limit , 4 shop item out of date, 5 shopitem don't exsit , 6 unknown error;
			stMsgPlayerBuyShopItemRet msgBack ;
			msgBack.nRet = 0 ;
			msgBack.nBuyShopItemForUserUID = pMsgRet->nBuyShopItemForUserUID ;
			msgBack.nShopItemID = pMsgRet->nShopItemID ;
			CShopConfigMgr* pShopCinfig = (CShopConfigMgr*)CGameServerApp::SharedGameServerApp()->GetConfigMgr()->GetConfig(CConfigManager::eConfig_Shop) ;
			stShopItem* pShopItem = pShopCinfig->GetShopItem(pMsgRet->nShopItemID);
			if ( !pShopItem )
			{
				msgBack.nRet = 5;
				SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
				return true;
			}
			else if ( !pShopItem->IsTimeLimitOk())
			{
				msgBack.nRet = 4;
				SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
				return true;
			}
			//else if (  pShopItem->nCanByTimes != 0 && GetBuyShopItemTimes(pMsgRet->nShopItemID) >= pShopItem->nCanByTimes )
			//{
			//	msgBack.nRet = 3;
			//	SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
			//	return true;
			//}

			CPlayer* pTargetPlayer = GetPlayer() ;
			if ( pMsgRet->nBuyShopItemForUserUID != 0 )
			{
				pTargetPlayer = CGameServerApp::SharedGameServerApp()->GetPlayerMgr()->GetPlayerByUserUID(pMsgRet->nBuyShopItemForUserUID);
			}

			bool bDeliverByMail = pTargetPlayer == NULL ;

			float fPrize = (float)pShopItem->nPrize * pMsgRet->nCount ;
			fPrize *= GetDiscountRate();        // may be vip disconnect ;
			unsigned int nFinalPrice = (unsigned int)fPrize ;
			msgBack.nSavedMoneyForVip = pShopItem->nPrize - nFinalPrice ;
			// prize type 
			switch ( pShopItem->nPrizeType )
			{
			case 0: //rmb
				{
					 // send to verify server ;
					stMsgToVerifyServer msg ;
					msg.nShopItemID = pShopItem->nShopItemID;
					msg.nBuyForPlayerUserUID = pTargetPlayer->GetUserUID();
					msg.nBuyerPlayerUserUID = GetPlayer()->GetUserUID() ;
					msg.nTranscationIDLen = pMsgRet->nBufLen ;
					//--------------------------------------
					char* pBufferV = new char[msg.nTranscationIDLen + sizeof(msg)] ;
					memcpy(pBufferV,&msg,sizeof(msg));
					memcpy(pBufferV + sizeof(msg), (char*)pMsgRet + sizeof(stMsgPlayerBuyShopItem),pMsgRet->nBufLen ) ;
					CGameServerApp::SharedGameServerApp()->SendMsgToVerifyServer(pBufferV,msg.nTranscationIDLen + sizeof(msg));
					delete[] pBufferV ;
					return true ;
				}
				break;
			case 1: // diamoned 
				{
					if ( GetPlayer()->GetBaseData()->ModifyMoney( -1 * nFinalPrice,true) == false )
					{
						msgBack.nRet = 1;
						SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
						return true;
					}
				}
				break; 
			case 2: // coin 
				{
					if ( GetPlayer()->GetBaseData()->ModifyMoney( -1 * nFinalPrice,false) == false )
					{
						msgBack.nRet = 1;
						SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
						return true;
					}
				}
				break;
			default:
				{
					msgBack.nRet = 6;
					SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
					CLogMgr::SharedLogMgr()->ErrorLog("unknown system prize type!") ;
					return true ;
				}
			}

			// give item 
			if ( bDeliverByMail )
			{
				CPlayerMailComponent* pPlayerMail = (CPlayerMailComponent*)GetPlayer()->GetComponent(ePlayerComponent_Mail) ;
				pPlayerMail->PostGiftMail(GetPlayer(),pMsgRet->nBuyShopItemForUserUID,0,pMsgRet->nShopItemID,pMsgRet->nCount) ; 
			}
			else
			{
				CPlayerShop* pPS = (CPlayerShop*)pTargetPlayer->GetComponent(ePlayerComponent_PlayerShop) ;
				pPS->OnGetShopItem(pShopItem,pMsgRet->nCount,GetPlayer()->GetUserUID() == pTargetPlayer->GetUserUID(),GetPlayer()) ;
			}
			msgBack.nDiamoned = GetPlayer()->GetBaseData()->GetAllDiamoned() ;
			msgBack.nFinalyCoin = GetPlayer()->GetBaseData()->GetAllCoin() ;
			msgBack.nFinalTakeInDiamoned = (unsigned int)GetPlayer()->GetBaseData()->GetTakeInMoney(true);
			msgBack.nFinalyTakeInCoin = GetPlayer()->GetBaseData()->GetTakeInMoney();
			SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;

			// ad to buyed record 
			//stBuyedShopItem* pHadBuyItem = GetBuyedShpItem(pShopItem->nShopItemID) ;
			//if ( pHadBuyItem )
			//{
			//	++pHadBuyItem->nTimes;
			//}
			//else
			//{
			//	pHadBuyItem = new stBuyedShopItem ;
			//	pHadBuyItem->nShopItemID = pShopItem->nShopItemID ;
			//	pHadBuyItem->nTimes = 1 ;
			//	m_vAllBuyedShopItems.push_back(pHadBuyItem) ;
			//}
		}
		break;
	default:
		return false ;
	}
	return true ;
}

void CPlayerShop::Reset()
{
	Clear();
	// send msg to db request shop list ;
	//stMsgGameServerGetShopBuyRecord msg ;
	//msg.nSessionID = GetPlayer()->GetSessionID() ;
	//msg.nUserUID = GetPlayer()->GetUserUID() ;
	//SendMsgToDB((char*)&msg,sizeof(msg)) ;
}

void CPlayerShop::Init()
{
	m_vAllBuyedShopItems.clear() ;
	Reset() ;
}

void CPlayerShop::SavePlayerShopToDB()
{
	if ( m_vAllBuyedShopItems.empty() )
	{
		return ;
	}
	// save msg to db to save buy record ;
	//stMsgGameServerSaveShopBuyRecord msg ;
	//msg.bAdd = m_bInsertToDB ;
	//msg.nSessionID = GetPlayer()->GetSessionID() ;
	//msg.nUserUID = GetPlayer()->GetUserUID() ;
	//unsigned short nCount = m_vAllBuyedShopItems.size() ;
	//msg.nBufferLen = sizeof(msg) + nCount * sizeof(stBuyedShopItem) + sizeof(nCount);
	//char* pBuffer = new char[msg.nBufferLen] ;
	//memcpy(pBuffer,&msg,sizeof(msg));
	//unsigned short nOffset = sizeof(msg);
	//memcpy(pBuffer + nOffset , &nCount , sizeof(nCount));
	//nOffset += sizeof(nCount);
	//LIST_BUYED_SHOP_ITEM::iterator iter = m_vAllBuyedShopItems.begin() ;
	//for ( ; iter != m_vAllBuyedShopItems.end(); ++iter )
	//{
	//	memcpy(pBuffer + nOffset , *iter,sizeof(stBuyedShopItem));
	//	nOffset += sizeof(stBuyedShopItem);
	//}
	//SendMsgToDB(pBuffer,nOffset) ;
	//delete[] pBuffer ;
	//m_bInsertToDB = false ;
}

void CPlayerShop::SendShopListToClient()
{
	// pareare list ;
	std::vector<unsigned short> vCanBuyShopItemIDs ;
	CShopConfigMgr* pShopCinfig = (CShopConfigMgr*)CGameServerApp::SharedGameServerApp()->GetConfigMgr()->GetConfig(CConfigManager::eConfig_Shop) ;
	CShopConfigMgr::MAP_SHOP_ITEMS::iterator iter = pShopCinfig->m_vAllShopItems.begin() ;
	for ( ;iter != pShopCinfig->m_vAllShopItems.end(); ++iter )
	{
		stShopItem* pShopItem = iter->second ;
		if ( !pShopItem->IsTimeLimitOk())
		{
			continue;
		}

		unsigned short nShopItemID = pShopItem->nShopItemID ;
		//if ( pShopItem->nCanByTimes != 0 && GetBuyShopItemTimes(nShopItemID) >= pShopItem->nCanByTimes )
		//{
		//	continue;
		//}
		vCanBuyShopItemIDs.push_back(nShopItemID) ;
	}

	// paree msg ;
	stMsgPlayerRequestShopListRet msg ;
	msg.nShopItemCount = vCanBuyShopItemIDs.size() ;
	char* pBuffer = new char[sizeof(msg) + sizeof(unsigned short) * msg.nShopItemCount ] ;
	unsigned short nOffset = 0 ;
	memcpy(pBuffer,&msg,sizeof(msg));
	nOffset += sizeof(msg);
	for ( unsigned int i = 0 ; i < vCanBuyShopItemIDs.size(); ++i )
	{
		memcpy(pBuffer + nOffset , &vCanBuyShopItemIDs[i],sizeof(unsigned short));
		nOffset += sizeof(unsigned short);
	}
	SendMsgToClient(pBuffer,nOffset) ;
	delete[] pBuffer ;
	pBuffer = NULL ;
}

void CPlayerShop::Clear()
{
	LIST_BUYED_SHOP_ITEM::iterator iter = m_vAllBuyedShopItems.begin() ;
	for ( ; iter != m_vAllBuyedShopItems.end(); ++iter )
	{
		if ( *iter )
		{
			delete *iter ;
			*iter = NULL ;
		}
	}
	m_vAllBuyedShopItems.clear() ;
}

//unsigned int CPlayerShop::GetBuyShopItemTimes(unsigned short nShopItemID )
//{
//	stBuyedShopItem* pItem = GetBuyedShpItem(nShopItemID) ;
//	if ( !pItem )
//	{
//		return 0 ;
//	}
//	return pItem->nTimes ;
//}

CPlayerShop::stBuyedShopItem* CPlayerShop::GetBuyedShpItem(unsigned short nShopItemID )
{
	LIST_BUYED_SHOP_ITEM::iterator iter = m_vAllBuyedShopItems.begin() ;
	for ( ; iter != m_vAllBuyedShopItems.end(); ++iter )
	{
		if ( (*iter)->nShopItemID == nShopItemID  )
		{
			return (*iter) ;
		}
	}
	return NULL ;
}

float CPlayerShop::GetDiscountRate()
{
	unsigned char nVipLevel = GetPlayer()->GetBaseData()->GetVipLevel() ;
	if ( nVipLevel == 1 )
	{
		return 0.9f ;
	}
	else if ( 2 == nVipLevel )
	{
		return 0.8f ;
	}
	else if ( 3 == nVipLevel )
	{
		return 0.7f;
	}
	return 1.0f ;
}

void CPlayerShop::OnGetShopItem(stShopItem* pShopItem ,unsigned int nCount , bool bSelfBuy, CPlayer* pBuyer )
{
	/*eShopItem_Diamoned,
	eShopItem_Coin,
	eShopItem_Assets,
	eShopItem_UseItem,
	eShopItem_Other, **/
	
	if ( pShopItem->eType == eShopItem_Diamoned )
	{
		GetPlayer()->GetBaseData()->ModifyMoney(pShopItem->nCount * nCount,true) ;
	}
	else if ( eShopItem_Coin == pShopItem->eType )
	{
		GetPlayer()->GetBaseData()->ModifyMoney(pShopItem->nCount*nCount,false) ;
	}
	else  // items ;
	{
		CPlayerItemComponent* pPlayerItemMgr = (CPlayerItemComponent*)GetPlayer()->GetComponent(ePlayerComponent_PlayerItemMgr);
		pPlayerItemMgr->AddItemByID(pShopItem->nItemID,pShopItem->nCount*nCount) ;
	}

	if ( !bSelfBuy )
	{
		// give a notice ;
		// send a message to tell ?
		stMsgPlayerRecievedShopItemGift msg ;
		msg.nFinalCoin = GetPlayer()->GetBaseData()->GetAllCoin() ;
		msg.nFinalDiamond = GetPlayer()->GetBaseData()->GetAllDiamoned() ;
		memcpy(msg.pPresenterName,pBuyer->GetBaseData()->GetPlayerName(),sizeof(msg.pPresenterName));
		msg.nPresenterUID = pBuyer->GetUserUID() ;
		msg.nShopItemID = pShopItem->nShopItemID ;
		msg.nShopItemCount = nCount ;
		SendMsgToClient((char*)&msg,sizeof(msg)) ;
	}
}

void CPlayerShop::TimerSave()
{
	SavePlayerShopToDB();
}