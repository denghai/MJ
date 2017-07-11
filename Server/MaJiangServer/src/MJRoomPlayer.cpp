#include "MJRoomPlayer.h"
#include <string>
#include "log4z.h"
#include "ServerMessageDefine.h"
#include <cassert>
#include <json/json.h>
#include <algorithm>
void CMJRoomPlayer::reset(IRoom::stStandPlayer* pPlayer)
{
	ISitableRoomPlayer::reset(pPlayer) ;
	m_tPeerCard.reset();
	m_nGameOffset = 0 ;
	m_vecBill.clear() ;
	m_nNewFetchCard = 0 ;
	m_listWantedCard.clear();

	m_vHuedCards.clear() ;
	m_vChuedCards.clear() ;
	m_nBuGaneCard  = 0 ;
	m_isWantedCarListDirty = false ;
	m_eHuType = eFanxing_Max ;
}

void CMJRoomPlayer::onGameEnd()
{
	ISitableRoomPlayer::onGameEnd();
	m_nGameOffset = 0 ;
	m_tPeerCard.reset();

	for ( auto& ref : m_vecBill )
	{
		delete ref ;
		ref = nullptr ;
	}
	m_vecBill.clear() ;
	m_listWantedCard.clear();

	m_vHuedCards.clear() ;
	m_vChuedCards.clear() ;
	m_nBuGaneCard  = 0 ;
	m_isWantedCarListDirty = false ;
	m_eHuType = eFanxing_Max ;
}

void CMJRoomPlayer::onGameBegin()
{
	ISitableRoomPlayer::onGameBegin() ;
	m_nGameOffset = 0 ;
	m_tPeerCard.reset();
	m_listWantedCard.clear();
	m_vHuedCards.clear() ;
	m_vChuedCards.clear() ;
	m_isWantedCarListDirty = false ;
	m_eHuType = eFanxing_Max ;
}

void CMJRoomPlayer::doSitdown(uint8_t nIdx )
{
	setIdx(nIdx) ;
}

bool CMJRoomPlayer::removeCard(uint8_t nCardNumber) 
{
	m_tPeerCard.removeCardNumber(nCardNumber);

	LOGFMTD(" removeCard idx = %u, anpai : ",getIdx() ) ;
	m_tPeerCard.debugAnpaiCount();
	return true ;
}

bool CMJRoomPlayer::isHaveAnCard(uint8_t nCardNumber)
{
	return m_tPeerCard.isHaveAnCard(nCardNumber) ;
}

uint8_t CMJRoomPlayer::getCardByIdx(uint8_t nCardIdx, bool isForExchange )
{
	return m_tPeerCard.getCardByIdx(nCardIdx,isForExchange);
}

void CMJRoomPlayer::addDistributeCard(uint8_t nCardNumber )
{
	m_tPeerCard.addCard(nCardNumber);
	LOGFMTD(" addDistributeCard idx = %u, anpai : ",getIdx() ) ;
	m_tPeerCard.debugAnpaiCount();
	m_isWantedCarListDirty = true ;
}

void CMJRoomPlayer::setMustQueType(uint8_t nType )
{
	m_tPeerCard.setMustQueType((eMJCardType)nType) ;
}

bool CMJRoomPlayer::canHuPai( uint8_t nCard ) // 0 means , self hu ; 
{
	if ( nCard == 0 )
	{
		for ( auto& refHu : m_listSelfOperateCard )
		{
			if ( refHu.eCanInvokeAct == eMJAct_Hu )
			{
				return true ;
			}
		}
		 return false ;
	}

	updateWantedCardList();
	for ( auto& refWanted : m_listWantedCard )
	{
		if ( refWanted.eCanInvokeAct != eMJAct_Hu )
		{
			continue;
		}

		if ( refWanted.nNumber == nCard )
		{
			return true ;
		}
	}
	return false ;
}

bool CMJRoomPlayer::canGangWithCard( uint8_t nCard, bool bCardFromSelf )
{
	if ( bCardFromSelf )
	{
		for ( auto& refCard : m_listSelfOperateCard )
		{
			if ( nCard == refCard.nNumber && ( refCard.eCanInvokeAct == eMJAct_BuGang || eMJAct_AnGang == refCard.eCanInvokeAct ) )
			{
				 return true ;
			}
		}
		return false ;
	}

	updateWantedCardList();
	for ( auto& refWanted : m_listWantedCard )
	{
		if ( refWanted.eCanInvokeAct == eMJAct_MingGang )
		{
			if ( nCard == refWanted.nNumber )
			{
				return true ;
			}
		}		
	}

	return false ;
}

bool CMJRoomPlayer::canPengWithCard(uint8_t nCard)
{
	updateWantedCardList();
	for ( auto& refWanted : m_listWantedCard )
	{
		if ( refWanted.eCanInvokeAct != eMJAct_Peng )
		{
			continue;
		}

		if ( refWanted.nNumber == nCard )
		{
			return true ;
		}
	}

	return false ;
}

uint8_t CMJRoomPlayer::getNewFetchCard()
{
	return m_nNewFetchCard ;
}

void CMJRoomPlayer::fetchCard(uint8_t nCardNumber )
{
	m_nNewFetchCard = nCardNumber ;
	m_eNewFetchCardFrom = eMJAct_Mo ;
	m_tPeerCard.addCard(m_nNewFetchCard);
	//debugWantedCard();

	LOGFMTD("idx = %u fetchCard , number = %u ",getIdx(),nCardNumber ) ;
	//m_tPeerCard.debugAnpaiCount();
}

eMJActType CMJRoomPlayer::getNewFetchedFrom()
{
	return m_eNewFetchCardFrom ;
}

uint8_t CMJRoomPlayer::doHuPaiFanshu( uint8_t nCardNumber , uint8_t& nGenShu )  // nCardNumber = 0 , means self mo ;
{
	setState(eRoomPeer_AlreadyHu) ;
	uint8_t nFan = m_tPeerCard.doHuPaiFanshu(nCardNumber,nGenShu,m_eHuType) ;
	if ( nCardNumber == 0 )   // must keep 13, player can hu more than once , so must remove new added card , for self ;
	{
		removeCard(getNewFetchCard()) ;
		addHuPai(getNewFetchCard());
		m_isWantedCarListDirty = true ;
	}
	else
	{
		addHuPai(nCardNumber);
	}

	// update max win card type 
	if ( m_eHuType > getMaxWinCardType() )
	{
		setMaxWinCardType(m_eHuType) ;
	}

	return nFan ;
}

bool CMJRoomPlayer::isCardBeWanted(uint8_t nCardNumber , uint8_t& nActType , bool bFromSelf )
{
	if ( isHaveState(eRoomPeer_DecideLose) )
	{
		LOGFMTD("already decide lose , can not need card") ;
		return false ;
	}

	updateWantedCardList();

	nActType = 0 ;
	for ( auto& refWanted : m_listWantedCard )
	{
		if ( refWanted.nNumber == nCardNumber )
		{
			if ( refWanted.eWanteddCardFrom == ePos_Other || ePos_Any == refWanted.eWanteddCardFrom )
			{
				if ( isHaveState(eRoomPeer_AlreadyHu) && refWanted.eCanInvokeAct != eMJAct_Hu )
				{
					LOGFMTD("player idx = %u , already hu , so can not need the card not invoke hu",getIdx()) ;
					continue;
				}

				if ( nActType < refWanted.eCanInvokeAct )
				{
					nActType = refWanted.eCanInvokeAct ;
				}
			}
		}
	}

	if ( nActType != 0 )
	{
		return true ;
	}

	LOGFMTD("idx = %u , i need not the card : %u" ,getIdx(),nCardNumber) ;
	debugWantedCard();
	return false ;
}

void CMJRoomPlayer::onPengCard(uint8_t nCard )
{
	m_tPeerCard.doAction(eMJAct_Peng,nCard);
	m_eNewFetchCardFrom = eMJAct_Mo ;

	LOGFMTD(" onPengCard idx = %u, anpai : ",getIdx() ) ;
	m_tPeerCard.debugAnpaiCount();
}

void CMJRoomPlayer::onChuCard(uint8_t nCardNumber )
{
	removeCard(nCardNumber) ;
	addChuPai(nCardNumber) ;
	m_isWantedCarListDirty = true ;
}

bool CMJRoomPlayer::isHuaZhu()
{
	return m_tPeerCard.isContainMustQue() ;
}

bool CMJRoomPlayer::isTingPai()
{
	for ( auto& refWanted : m_listWantedCard )
	{
		if ( refWanted.eCanInvokeAct == eMJAct_Hu )
		{
			return true ;
		}
	}

	return false ;
}

uint8_t CMJRoomPlayer::getMaxCanHuFanShu( uint8_t& nGenShu )
{
	return m_tPeerCard.getMaxHuPaiFanShu(nGenShu) ;
}

void CMJRoomPlayer::addBill( stBill* pBill )
{
	m_vecBill.push_back(pBill) ;
	if ((pBill->eType & eBill_Win) == eBill_Win)
	{
		m_nGameOffset += pBill->nOffset;
		return;
	}
	m_nGameOffset -= pBill->nOffset;
}

void CMJRoomPlayer::gangPai( uint8_t nGangPai, eMJActType eGangType,uint8_t nNewCard  )
{
	m_nNewFetchCard = nNewCard ;
	m_eNewFetchCardFrom = eGangType ;
	m_tPeerCard.doAction(eGangType,nGangPai);

	m_tPeerCard.addCard(m_nNewFetchCard);

	
	//debugWantedCard();
	LOGFMTD("idx = %u gangPai , GangCard = %u, nNew = %u ",getIdx(),nGangPai,nNewCard ) ;
	//m_tPeerCard.debugAnpaiCount();

	if ( eGangType == eMJAct_BuGang_Done || eMJAct_BuGang == eGangType )
	{
		m_nBuGaneCard = 0 ;
	}
}

void CMJRoomPlayer::declareBuGang(uint8_t nCardNumber )
{
	m_nBuGaneCard = nCardNumber ;
}

void CMJRoomPlayer::beRobotGang( uint8_t nCardNumber )
{
	assert(nCardNumber == m_nBuGaneCard && "robot gang card is not declared card");
	m_nBuGaneCard = 0 ;
	removeCard(nCardNumber) ;
	m_isWantedCarListDirty = true ;
}

void CMJRoomPlayer::getGangWinBill( std::vector<stBill*>& vecGangWin )
{
	for ( auto& refBill : m_vecBill )
	{
		if ( refBill->eType == eBill_GangWin )
		{
			vecGangWin.push_back(refBill) ;
		}
	}
}

void CMJRoomPlayer::onChuedPaiBePengOrGang(uint8_t nCardNum )
{
	uint8_t ncard = m_vChuedCards.back() ;
	assert(ncard == nCardNum && "must be newest chu pai" );
	m_vChuedCards.pop_back() ;
}

void CMJRoomPlayer::addChuPai(uint8_t nCardNum )
{
	m_vChuedCards.push_back(nCardNum) ;
}

void CMJRoomPlayer::addHuPai(uint8_t nCardNum )
{
	m_vHuedCards.push_back(nCardNum) ;
}

void CMJRoomPlayer::getCardInfo( Json::Value& vCardInFoValue )
{
	Json::Value vAnPai ;
	Json::Value vMingPai ;
	Json::Value vHuPai ;
	Json::Value vChuPai ;

	m_tPeerCard.getAnPai(vAnPai) ;
	m_tPeerCard.getMingPai(vMingPai) ;
	for ( uint8_t nIdx = 0 ; nIdx < m_vChuedCards.size() ; ++nIdx )
	{
		vChuPai[nIdx] = m_vChuedCards[nIdx] ;
	}

	for ( uint8_t nIdx = 0 ; nIdx < m_vHuedCards.size() ; ++nIdx )
	{
		vHuPai[nIdx] = m_vHuedCards[nIdx] ;
	}

	vCardInFoValue["idx"] = getIdx() ;
	vCardInFoValue["anPai"] = vAnPai ;
	vCardInFoValue["mingPai"] = vMingPai ;
	vCardInFoValue["huPai"] = vHuPai ;
	vCardInFoValue["chuPai"] = vChuPai ;
	vCardInFoValue["queType"] = m_tPeerCard.getMustQueType();
}

void CMJRoomPlayer::debugWantedCard()
{
	updateWantedCardList();
	LOGFMTD("idx = %u wanted card : ",getIdx()) ;
	for (stWantedCard& var : m_listWantedCard )
	{
		LOGFMTD("card number : %u, invoke act = %u, wanted card from = %u",var.nNumber,var.eCanInvokeAct,var.eWanteddCardFrom ) ;
	}
}

void CMJRoomPlayer::updateWantedCardList()
{
	if ( m_isWantedCarListDirty )
	{
		m_listWantedCard.clear() ;
		m_tPeerCard.updateWantedCard(m_listWantedCard) ;
	}
	m_isWantedCarListDirty = false ;
}

bool CMJRoomPlayer::updateSelfOperateCards()
{
	m_listSelfOperateCard.clear() ;
	m_tPeerCard.updateSelfOperateCard(m_listSelfOperateCard,getNewFetchCard()) ;
	return m_listSelfOperateCard.empty() == false ;
}

bool CMJRoomPlayer::getOperateListJoson(Json::Value& vActList )
{
	if ( m_listSelfOperateCard.empty() )
	{
		return false ;
	}

	for_each(m_listSelfOperateCard.begin(),m_listSelfOperateCard.end(),[&vActList,this](stWantedCard& ar)
	{
		if ( ar.eCanInvokeAct == eMJAct_AnGang && isHaveState(eRoomPeer_AlreadyHu ) )
		{
			return ;
		}

		if (ar.eCanInvokeAct == eMJAct_BuGang && isHaveState(eRoomPeer_AlreadyHu) && ar.nNumber != getNewFetchCard())
		{
			return;
		}

		Json::Value js ;
		js["act"] = ar.eCanInvokeAct ;
		js["cardNum"] = ar.nNumber ;
		vActList[vActList.size()] = js;
	}
	) ;
	return vActList.size() > 0  ;
}

void CMJRoomPlayer::getAllBillForMsg(Json::Value& jsbillInfoMsg)
{
	// svr : { idx : 2 , winBills : [ { billType :eBill_HuWin, huType : 23, fanShu : 23 ,detail : [ {loseIdx : 2 , coin: 234 }, {loseIdx : 2 , coin: 234 }] } ,....] } , .....] , loseBills : [ { billType : eBill_HuLose, , huType : 23, fanShu : 23 ,winIdx : 2 , coin : 23 } , { billType : eBill_HuLose , huType : 23, fanShu : 23, winIdx : 1 , coin : 234 }, ..... ] }
	Json::Value jsArrayWinBills;
	Json::Value jsArrayLoseBills;
	for (auto& ref : m_vecBill)
	{
		Json::Value jsBill;
		jsBill["billType"] = ref->eType;
		if (eBill_Win == (ref->eType & eBill_Win))
		{
			stBillWin* pWin = (stBillWin*)ref;
			jsBill["huType"] = pWin->nHuType;
			jsBill["fanShu"] = pWin->nFanShu;
			Json::Value jsLoseArray;
			for (auto& refD : pWin->vLoseIdxAndCoin)
			{
				Json::Value jsLose;
				jsLose["loseIdx"] = refD.first;
				jsLose["coin"] = refD.second;
				jsLoseArray[jsLoseArray.size()] = jsLose;
			}
			jsBill["detail"] = jsLoseArray;
			jsArrayWinBills[jsArrayWinBills.size()] = jsBill;
			continue;
		}
		
		// lose ;
		auto pLoseBill = (stBillLose*)ref;
		jsBill["winIdx"] = pLoseBill->nWinnerIdx;
		jsBill["coin"] = pLoseBill->nOffset;
		jsBill["huType"] = pLoseBill->nHuType;
		jsBill["fanShu"] = pLoseBill->nFanShu;
		jsArrayLoseBills[jsArrayLoseBills.size()] = jsBill;
	}

	jsbillInfoMsg["idx"] = getIdx();
	jsbillInfoMsg["loseBills"] = jsArrayLoseBills;
	jsbillInfoMsg["winBills"] = jsArrayWinBills;
}


