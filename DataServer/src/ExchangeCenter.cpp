#include "ExchangeCenter.h"
#include "PlayerManager.h"
#include "GameServerApp.h"
#include "Player.h"
#include "PlayerBaseData.h"
#include "AutoBuffer.h"
#include "log4z.h"
CExchangeCenter::CExchangeCenter(const char* pConfigFilePath)
{
	pItemsBuffer = nullptr ;
	nItemsBufferLen = 0 ;
	bItemBufferDirty = false ;

	m_tConfig.LoadFile(pConfigFilePath) ;
}

CExchangeCenter::~CExchangeCenter() 
{
	if ( pItemsBuffer )
	{
		delete pItemsBuffer ;
		pItemsBuffer = nullptr  ;
		nItemsBufferLen = 0 ;
	}

	for ( auto p : vExchangeEntrys )
	{
		delete p.second ;
		p.second = nullptr ;
	}
	vExchangeEntrys.clear() ;
}

bool CExchangeCenter::onMsg(stMsg* pMsg , eMsgPort eSenderPort , uint32_t nSessionID)
{
	switch ( pMsg->usMsgType )
	{
	case MSG_READ_EXCHANGE:
		{
			stMsgReadExchangesRet* pRet = (stMsgReadExchangesRet*)pMsg ;
			stExchangeItem* pItem = (stExchangeItem*)(((char*)pMsg) + sizeof(stMsgReadExchangesRet));
			while (pRet->nCnt--)
			{
				stExchangeEntry* pentry = new stExchangeEntry ;
				memset(pentry,0,sizeof(stExchangeEntry));
				pentry->bDirty = false ;
				pentry->nExchangedCnt = pItem->nExchangedCnt ;
				pentry->nExchangeID = pItem->nExchangeID ;
				vExchangeEntrys[pentry->nExchangeID] = pentry ;
				++pItem ;
			}
			LOGFMTD("recived exchange recorders") ;
		}
		break;
	case MSG_PLAYER_EXCHANGE:
		{
			stMsgPlayerExchange* pRet = (stMsgPlayerExchange*)pMsg ;
			stMsgPlayerExchangeRet msgRet ;
			msgRet.nExchageID = pRet->nExchangeID ;
			auto pPlayer = CGameServerApp::SharedGameServerApp()->GetPlayerMgr()->GetPlayerBySessionID(nSessionID) ;
			if ( !pPlayer )
			{
				msgRet.nRet = 3 ;
				getSvrApp()->sendMsg(nSessionID,(char*)&msgRet,sizeof(msgRet));
				break ;
			}

			auto pExchangeItem = m_tConfig.getExchangeByID(pRet->nExchangeID);
			if ( !pExchangeItem )
			{
				msgRet.nRet = 1 ;
				getSvrApp()->sendMsg(nSessionID,(char*)&msgRet,sizeof(msgRet));
				LOGFMTD("can not find exchange item uid = %d , exchange id = %d",pPlayer->GetUserUID(),pExchangeItem->nConfigID) ;
				break ;
			}

			if ( pPlayer->GetBaseData()->GetAllDiamoned() < pExchangeItem->nDiamondNeed )
			{
				msgRet.nRet = 2 ;
				getSvrApp()->sendMsg(nSessionID,(char*)&msgRet,sizeof(msgRet));
				LOGFMTD("uid = %d , you diamond is not enough",pPlayer->GetUserUID()) ;
				break ;
			}
			msgRet.nRet = 0 ;
			getSvrApp()->sendMsg(nSessionID,(char*)&msgRet,sizeof(msgRet));

			pPlayer->GetBaseData()->decressMoney(pExchangeItem->nDiamondNeed,true) ;
			// get remarks 
			Json::Reader reader;
			Json::Value rootValue ;
			char* pstr = ((char*)pRet) + sizeof(stMsgPlayerExchange) ;
			reader.parse(pstr,pstr + pRet->nJsonLen,rootValue,false);
			// give order 
			stMsgSaveLog msgLog ;
			msgLog.nLogType = eLog_ExchangeOrder ;
			msgLog.nTargetID = pPlayer->GetUserUID() ;
			memset(msgLog.vArg,0,sizeof(msgLog.vArg));
			msgLog.vArg[0] = pExchangeItem->nConfigID ;

			Json::Value jValue ;
			jValue["playerName"] = pPlayer->GetBaseData()->GetPlayerName() ;
			jValue["excDesc"] = pExchangeItem->strDesc ;
			jValue["address"] = rootValue["address"].asString() ;
			jValue["phoneNumber"] = rootValue["phoneNumber"].asString() ;

			Json::StyledWriter jWriter ;
			std::string  strArg = jWriter.write(jValue) ;
			msgLog.nJsonExtnerLen = strArg.size() ;
			 
			CAutoBuffer auBuffer(sizeof(msgLog) + msgLog.nJsonExtnerLen );
			auBuffer.addContent(&msgLog,sizeof(msgLog)) ;
			auBuffer.addContent(strArg.c_str(),msgLog.nJsonExtnerLen) ;
			getSvrApp()->sendMsg(nSessionID,auBuffer.getBufferPtr(),auBuffer.getContentSize()) ;
			LOGFMTD("uid = %d do exchange item id = %d, remark = %s",pPlayer->GetUserUID(),pExchangeItem->nConfigID,rootValue["address"].asString()) ;

			// update recorder ;
			auto pRec = vExchangeEntrys.find(pRet->nExchangeID);
			if ( pRec != vExchangeEntrys.end() )
			{
				++pRec->second->nExchangedCnt ;
				pRec->second->bDirty = true ;
			}
			else
			{
				auto pp = new stExchangeEntry ;
				pp->bDirty = true ;
				pp->nExchangedCnt = 1 ;
				pp->nExchangeID = pRet->nExchangeID ;
				vExchangeEntrys[pp->nExchangeID] = pp ;
			}

			bItemBufferDirty = true ;
		}
		break;
	case MSG_REQUEST_EXCHANGE_DETAIL:
		{
			stMsgRequestExchangeDetail* pRet = (stMsgRequestExchangeDetail*)pMsg ;
			
			auto item = m_tConfig.getExchangeByID(pRet->nExchangeID) ;

			stMsgRequestExchangeDetailRet msgBack ;
			msgBack.nExchangeID = pRet->nExchangeID ;
			msgBack.nJsonLen = 0 ;
			msgBack.nRet = 0 ; 
			if ( item == nullptr )
			{
				msgBack.nRet = 1 ;
				getSvrApp()->sendMsg(nSessionID,(char*)&msgBack,sizeof(msgBack));
				break ;
			}

			Json::Value jValue ;
			jValue["diamond"] = item->nDiamondNeed ;
			jValue["desc"] = item->strDesc ;
			jValue["icon"] = item->strIcon ;

			Json::StyledWriter jWriter ;
			std::string  strArg = jWriter.write(jValue) ;
			msgBack.nJsonLen = strArg.size() ;
			CAutoBuffer auBuffer(sizeof(msgBack) + msgBack.nJsonLen );
			auBuffer.addContent(&msgBack,sizeof(msgBack)) ;
			auBuffer.addContent(strArg.c_str(),msgBack.nJsonLen) ;
			getSvrApp()->sendMsg(nSessionID,auBuffer.getBufferPtr(),auBuffer.getContentSize()) ;
			LOGFMTD("session id = %d get exchange detail",nSessionID) ;
		}
		break;
	case MSG_REQUEST_EXCHANGE_LIST:
		{
			if ( bItemBufferDirty )
			{
				bItemBufferDirty = false ;
				delete [] pItemsBuffer;
				pItemsBuffer = nullptr ;
				nItemsBufferLen = 0 ;
			}

			if ( pItemsBuffer == nullptr )
			{
				stMsgRequestExchangeListRet msgBack ;
				msgBack.nCnt = m_tConfig.getExchangeItemCnt() ;
				CAutoBuffer auBuffer(sizeof(msgBack) + sizeof(stExchangeItem) * msgBack.nCnt );
				auBuffer.addContent(&msgBack,sizeof(msgBack)) ;
				auto iter = m_tConfig.getBeginIter() ;
				stExchangeItem item ;
				for ( ; iter != m_tConfig.getEndItem() ; ++iter )
				{
					item.nExchangeID = iter->second->nConfigID ;
					item.nExchangedCnt = 0 ;

					auto p = vExchangeEntrys.find(item.nExchangeID) ;
					if ( p != vExchangeEntrys.end() )
					{
						item.nExchangedCnt = p->second->nExchangedCnt ;
					}
					auBuffer.addContent(&item,sizeof(item)) ;
				}
				getSvrApp()->sendMsg(nSessionID,auBuffer.getBufferPtr(),auBuffer.getContentSize()) ;

				pItemsBuffer = new char[auBuffer.getContentSize()] ;
				memcpy(pItemsBuffer,auBuffer.getBufferPtr(),auBuffer.getContentSize());
				nItemsBufferLen = auBuffer.getContentSize() ;
			}
			else
			{
				getSvrApp()->sendMsg(nSessionID,pItemsBuffer,nItemsBufferLen) ;
			}

			LOGFMTD("session id = %d get exchange list",nSessionID) ;
		}
		break;
	default:
		return false ;
	}
	return true ;
}

void CExchangeCenter::onTimeSave()
{
	IGlobalModule::onTimeSave();
	for ( auto pp : vExchangeEntrys )
	{
		if ( pp.second->bDirty )
		{
			pp.second->bDirty = false ;

			stMsgSaveExchanges msgSave ;
			msgSave.nCount = pp.second->nExchangedCnt ;
			msgSave.nExchangeID = pp.second->nExchangeID ;
			getSvrApp()->sendMsg(0,(char*)&msgSave,sizeof(msgSave));
		}
	}
}

void CExchangeCenter::onConnectedSvr()
{
	IGlobalModule::onConnectedSvr();
	stMsgReadExchanges msg ;
	getSvrApp()->sendMsg(0,(char*)&msg,sizeof(msg));
}