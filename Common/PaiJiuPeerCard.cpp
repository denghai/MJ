#include "PaiJiuPeerCard.h"
#include "LogManager.h"
#define CAN_PAIR(A,B,C,D) (((A) == (C) && (B) == (D)) || ( (A) == (D) && (B) == (C)) )
#define IS_THE_SAME(NUM) (AFaceN == NUM && BFaceN == NUM)
#define SPECAIL_PAIR(NUM,atype,btype) (IS_THE_SAME(NUM) && CAN_PAIR(AType,BType,atype,btype))
#define SPECAIL_PAIR_RED(NUM) SPECAIL_PAIR(NUM,CCard::eCard_Diamond,CCard::eCard_Heart)
#define SPECAIL_PAIR_BLACK(NUM) SPECAIL_PAIR(NUM,CCard::eCard_Sword,CCard::eCard_Club)
#define IS_CARD_RED( NUM) ( ((type) == CCard::eCard_Diamond || (type) == CCard::eCard_Heart) && nFaceNum == NUM )
#define IS_CARD_BLACK(NUM)( ((type) == CCard::eCard_Sword || (type) == CCard::eCard_Club) && nFaceNum == NUM )
void CPaiJiuPeerCard::SetPeerCardCompsiteNumber( unsigned char nA , unsigned char nB )
{
	m_Card[0].RsetCardByCompositeNum(nA);
	m_Card[1].RsetCardByCompositeNum(nB);

	// comfir type ;
	m_nSelftWeight = 0 ;
	unsigned char AFaceN = m_Card[0].GetCardFaceNum();
	unsigned char BFaceN = m_Card[1].GetCardFaceNum() ;
	CCard::eCardType AType = m_Card[0].GetType();
	CCard::eCardType BType = m_Card[1].GetType();
	if ( SPECAIL_PAIR_RED(12))
	{
		m_nSelftWeight = 100 ;
		m_strName = "双天";
	}
	else if ( SPECAIL_PAIR_RED(2))
	{
		m_nSelftWeight = 99 ;
		m_strName = "双地";
	}
	else if ( CAN_PAIR(AFaceN,BFaceN,1,3) && CAN_PAIR(AType,BType,CCard::eCard_Sword,CCard::eCard_Sword) )
	{
		m_nSelftWeight =98;
		m_strName = "至尊";
	}
	else if ( SPECAIL_PAIR_RED(8))
	{
		m_nSelftWeight = 97 ;
		m_strName = "双人";
	}
	else if ( SPECAIL_PAIR_RED(4) )
	{
		m_nSelftWeight = 96 ;
		m_strName = "双鹅";
	}
	else if ( SPECAIL_PAIR_RED(6) || SPECAIL_PAIR_RED(10) || SPECAIL_PAIR_BLACK(4) )
	{
		m_nSelftWeight = 95;
		if ( BFaceN == 6 )
		{
			m_strName = "双零霖六" ;
		}
		else if ( BFaceN == 10 )
		{
			m_strName = "双红头十" ;
		}
		else if ( BFaceN == 4 )
		{
			m_strName = "双板凳" ;
		}
	}
	else if ( SPECAIL_PAIR_BLACK(6) || SPECAIL_PAIR_RED(7) || SPECAIL_PAIR_BLACK(10) || SPECAIL_PAIR_BLACK(11))
	{
		m_nSelftWeight = 94;
		
		if ( AFaceN == 6 )
		{
			m_strName = "双长三";
		}
		else if ( 7 == AFaceN )
		{
			m_strName = "双高脚七";
		}
		else if ( 10 == AFaceN )
		{
			m_strName = "双梅花";
		}
		else if ( 11 == AFaceN )
		{
			m_strName = "斧头";
		}
	}

	else if ( SPECAIL_PAIR_RED(5) || SPECAIL_PAIR_BLACK(7) || SPECAIL_PAIR_BLACK(8) || SPECAIL_PAIR_RED(9))
	{
		m_nSelftWeight = 93;
		
		if ( 5 == AFaceN )
		{
			m_strName = "双杂五";
		}
		else if ( 7 == AFaceN) 
		{
			m_strName = "双杂七";
		}
		else if ( 8 == AFaceN )
		{
			m_strName = "双杂八";
		}
		else if ( 9 == AFaceN )
		{
			m_strName = "双杂九";
		}
	}
	else if ( CAN_PAIR(AFaceN,BFaceN,12,9))
	{
		m_nSelftWeight = 92 ;
		m_strName = "天九王";
	}
	else if ( CAN_PAIR(AFaceN,BFaceN,12,8 ) )
	{
		m_nSelftWeight = 91 ;
		m_strName = "天杠";
	}
	else if ( CAN_PAIR(AFaceN,BFaceN,2,8))
	{
		m_nSelftWeight = 90 ;
		m_strName = "地杠";
	}
	else
	{
		if ( AFaceN == 1 )
		{
			AFaceN = 6 ;
		}

		if ( BFaceN == 1 )
		{
			BFaceN = 6 ;
		}
		m_nSelftWeight = (AFaceN + BFaceN )%10;
		
		if ( CAN_PAIR(AFaceN,BFaceN,12,7))
		{
			m_strName = "天九" ;
		}
		else if ( CAN_PAIR(AFaceN,BFaceN,2,7))
		{
			m_strName = "地九" ;
		}
		else
		{
			char pBuffer[10] = { 0 } ;
			sprintf(pBuffer, "%d点",m_nSelftWeight ) ;
			m_strName = pBuffer ;
		}
	}

#ifdef _DEBUG
	LogInfo();
#endif
} 

bool CPaiJiuPeerCard::PKPeerCard(CPaiJiuPeerCard* pTargetIdle )
{
	if ( GetSelfWeight() > pTargetIdle->GetSelfWeight() )
	{
		return true ;
	}
	else if ( GetSelfWeight() == pTargetIdle->GetSelfWeight() )
	{
		CCard* me = GetBiggerCard() ;
		CCard* other = pTargetIdle->GetBiggerCard() ;
		return (GetSingleCardWeight(me) >= GetSingleCardWeight(other) ) ;
	}
	return false ;
}

CCard* CPaiJiuPeerCard::GetBiggerCard()
{
	return (GetSingleCardWeight(&m_Card[0]) >= GetSingleCardWeight(&m_Card[1]) ? (&m_Card[0]) : (&m_Card[1]));
}

unsigned short CPaiJiuPeerCard::GetSingleCardWeight(CCard* pacd )
{
	unsigned short nWeith = 0 ;
	unsigned char nFaceNum = pacd->GetCardFaceNum() ;
	CCard::eCardType type = pacd->GetType() ;
	if ( IS_CARD_RED(12) )
	{
		return 100 ;
	}
	else if ( IS_CARD_RED(2))
	{
		return 99 ;
	}
	else if ( (nFaceNum == 1 || 3 == nFaceNum ) && type == CCard::eCard_Sword  )
	{
		return 98 ;
	}
	else if ( IS_CARD_RED(8))
	{
		return 97; 
	}
	else if ( IS_CARD_RED(4))
	{
		return 96 ;
	}
	else if ( IS_CARD_BLACK(4) || IS_CARD_RED(6) || IS_CARD_RED(10) )
	{
		return 95 ;
	}
	else if ( IS_CARD_BLACK(6) || IS_CARD_RED(7) || IS_CARD_BLACK(10) || IS_CARD_BLACK(11))
	{
		return 94 ;
	}
	else if ( IS_CARD_RED(5)|| IS_CARD_BLACK(7) || IS_CARD_BLACK(8) || IS_CARD_RED(9) )
	{
		return 93 ;
	}
	else if ( nFaceNum == 12 || nFaceNum == 9 )
	{
		return 92 ;
	}
	else if ( 12 == nFaceNum || 8 == nFaceNum )
	{
		return 91 ;
	}
	else if ( 2 == nFaceNum || 8 == nFaceNum )
	{
		return 90 ;
	}
	return 0 ;
}

void CPaiJiuPeerCard::LogInfo()
{
	CLogMgr::SharedLogMgr()->PrintLog("%s",m_strName.c_str()) ;
	m_Card[0].LogCardInfo() ;
	m_Card[1].LogCardInfo() ;
}

