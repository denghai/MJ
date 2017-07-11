#include "PeerCard.h"
#include <memory>
#include <assert.h>

#ifdef SERVER
#include "log4z.h"
#endif
#ifndef SERVER
#include "Language.h"
#endif
CPeerCard::CPeerCard()
{
	for ( int i = 0 ; i < PEER_CARD_COUNT ; ++i )
	{
		m_vCard[i] = NULL ;
	}
	 m_eCardType = ePeerCard_None ;
	 m_vRepacedCard.clear();
	 m_vShowedCard.clear() ;
	 m_pPairCardNum = 0 ;
}

CPeerCard::~CPeerCard()
{
	for ( int i = 0 ; i < PEER_CARD_COUNT ; ++i )
	{
		if ( m_vCard[i])
			delete m_vCard[i] ;
		m_vCard[i] = NULL ;
	}
}

void CPeerCard::SetPeerCardByNumber( unsigned char nNumber , unsigned char nNum, unsigned char nNum2 )
{
	unsigned char vNum[] = {nNumber,nNum,nNum2 };
	for ( int i = 0 ; i < PEER_CARD_COUNT ; ++i )
	{
		if ( m_vCard[i] == NULL )
		{
			m_vCard[i] = new CCard(vNum[i]) ;
		}
		else 
		{
			m_vCard[i]->RsetCardByCompositeNum(vNum[i]) ;
		}
	}
	ArrangeCard();
	m_vRepacedCard.clear();
	m_vShowedCard.clear() ;
}

int8_t CPeerCard::PKPeerCard(CPeerCard* pPeerCard)
{
	if ( GetType() != pPeerCard->GetType() )
	{
		if ( GetType() > pPeerCard->GetType() )
		{
			return 1 ;
		}
		else
		{
			return -1 ;
		}
	}

	if ( GetType() == ePeerCard_Pair )
	{
		if ( m_pPairCardNum > pPeerCard->m_pPairCardNum )
		{
			return 1 ;
		}
		else if ( m_pPairCardNum < pPeerCard->m_pPairCardNum ) 
		{
			return -1 ;
		}
	}

	int myBig = 0 ;
	int nOtherBig = 0 ;
	for ( int i = PEER_CARD_COUNT -1 ; i >= 0 ; --i )
	{
		myBig = m_vCard[i]->GetCardFaceNum() == 1 ? 14 : m_vCard[i]->GetCardFaceNum() ;
		nOtherBig = pPeerCard->m_vCard[i]->GetCardFaceNum() == 1 ? 14 : pPeerCard->m_vCard[i]->GetCardFaceNum() ;
		if ( myBig > nOtherBig )
		{
			return 1 ;
		}
		else if ( myBig < nOtherBig )
		{
			return -1 ;
		}
	}
	return 0 ;	
}

void CPeerCard::ReplaceCardByNumber( unsigned char nOld , unsigned char nNew )
{
	for ( int i = 0 ; i < PEER_CARD_COUNT ; ++i )
	{
		if ( m_vCard[i]->GetCardCompositeNum() == nOld )
		{
			m_vCard[i]->RsetCardByCompositeNum(nNew) ;
			ArrangeCard();
			m_vRepacedCard.push_back(nOld) ;
			return ;
		}
	}
	assert(0);
   #ifdef SERVER
	LOGFMTE("this is no card in my peer card : old = %d ",nOld ) ;
#endif
}

void CPeerCard::ShowCardByNumber(unsigned char nCardNum)
{
	for ( int i = 0 ; i < PEER_CARD_COUNT ; ++i )
	{
		if ( m_vCard[i]->GetCardCompositeNum() == nCardNum )
		{
			m_vShowedCard.push_back(nCardNum) ;
			return ;
		}
	}
	assert(0);
    #ifdef SERVER
	LOGFMTE("Can not show Card : %d . do not exist it ",nCardNum ) ;
#endif
	return  ;
}

void CPeerCard::ArrangeCard()
{
	for ( uint8_t nIdx = 0 ; nIdx < PEER_CARD_COUNT - 1 ; ++nIdx )
	{
		uint8_t nPosNum = m_vCard[nIdx]->GetCardFaceNum(true) ;
		for ( uint8_t nSIdx = nIdx + 1 ; nSIdx < PEER_CARD_COUNT ; ++nSIdx )
		{
			uint8_t nSNum = m_vCard[nSIdx]->GetCardFaceNum(true) ;
			if ( nSNum < nPosNum ) // switch ;
			{
				nPosNum = nSNum ;
				CCard* pCard = m_vCard[nSIdx] ;
				m_vCard[nSIdx]= m_vCard[nIdx] ;
				m_vCard[nIdx]=pCard ;
			}
		}
	}

	int iNum[PEER_CARD_COUNT] = { 0 } ;
	for ( int i = 0 ; i < PEER_CARD_COUNT ; ++i )
	{
		iNum[i] = m_vCard[i]->GetCardFaceNum(true);
	}

	// decide type ;
	if ( m_vCard[0]->GetCardFaceNum() == m_vCard[1]->GetCardFaceNum() && m_vCard[1]->GetCardFaceNum() == m_vCard[2]->GetCardFaceNum() )
	{
		m_eCardType = ePeerCard_Bomb ;
	}
	else if ( m_vCard[0]->GetType() == m_vCard[1]->GetType() && m_vCard[1]->GetType() == m_vCard[2]->GetType() )
	{
		m_eCardType = ePeerCard_SameColor ;
		if ( iNum[0] +1 == iNum[1] && iNum[1] + 1 == iNum[2] )
		{
			m_eCardType =  ePeerCard_SameColorSequence ;
		}

		if ( iNum[0] == 2 && iNum[1] == 3 && 14 == iNum[2] )
		{
			m_eCardType =  ePeerCard_SameColorSequence ;
		}
	}
	else if ( (iNum[0] +1 == iNum[1] && iNum[1] + 1 == iNum[2]) || ( iNum[0] == 2 && iNum[1] == 3 && 14 == iNum[2] ) )
	{
		m_eCardType = ePeerCard_Sequence ;
	}
	else if ( iNum[0] == iNum[1] || iNum[2] == iNum[1]) 
	{
		m_eCardType = ePeerCard_Pair ;
		m_pPairCardNum = iNum[1];
	}
	else
	{
		m_eCardType = ePeerCard_None ;
	}
}

void CPeerCard::LogInfo()
{
	const char* pType = NULL;
	switch( GetType() )
	{
		case ePeerCard_None:
			{
				pType = "普通牌型   "   ;
			}
			break;;
		case ePeerCard_Pair:
			{
				pType = "对子";
				break;
			}
		case  ePeerCard_Sequence:
			{
				pType = "顺子";
				break;
			}
		case ePeerCard_SameColor:
			{
				pType = "金花";
				break;
			}
		case ePeerCard_SameColorSequence:
			{
				pType = "顺金";
				break;
			}
		case ePeerCard_Bomb:
			{
				pType = "豹子";
				break;
			}
		default:
			pType = "未知牌型" ;
			break;

	}
    #ifdef SERVER
	LOGFMTD("%s",pType) ;
#endif
	for ( int i = 0 ; i < PEER_CARD_COUNT ; ++i )
	{
		if ( m_vCard[i])
			m_vCard[i]->LogCardInfo();
	}
}

void CPeerCard::Reset()
{
	m_eCardType = ePeerCard_None ;
	m_vRepacedCard.clear();
	m_vShowedCard.clear() ;
	m_pPairCardNum = 0 ;
}

void CPeerCard::GetCompositeCardRepresent(char* vCard )
{
	for ( int i = 0 ; i < PEER_CARD_COUNT ; ++i )
	{
		vCard[i] = m_vCard[i]->GetCardCompositeNum() ;
	}
}

#ifndef SERVER
const char*  CPeerCard::getNameString(){
    std::string str = "niuniu_meiniu";
    switch (m_eCardType) {
        case ePeerCard_None:
            str = "paixing_danzhang";
            break;
        case ePeerCard_Pair:
            str = "paixing_duizi";
            break;
        case ePeerCard_Sequence:
            str = "paixing_shunzi";
            break;
        case ePeerCard_SameColor:
            str = "paixing_tonghua";
            break;
        case ePeerCard_SameColorSequence:
            str = "paixing_tonghuashun";
            break;
        case ePeerCard_Bomb:
            str = "paixing_baozi";
            break;
        default:
            str = "paixing_danzhang";
            break;
    }
    return Language::getInstance()->get(str.c_str());

}
#endif
