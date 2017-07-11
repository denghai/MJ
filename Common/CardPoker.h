#pragma once 
class CCard
{
public:
	enum eCardType
	{
		eCard_None,
		eCard_Diamond = eCard_None, // fangkuai
        eCard_Club, // cao hua
		eCard_Heart, // hong tao
		eCard_Sword, // hei tao 
		eCard_NoJoker,
		eCard_Joker = eCard_NoJoker, // xiao wang
		eCard_BigJoker, // da wang
		eCard_Max,
	};
public:
	CCard( unsigned char nCompositeNum =2 );
	~CCard();
	void RsetCardByCompositeNum( unsigned char nCompositeNum );
	unsigned char GetCardFaceNum( bool bSpecailA = false ){ if ( bSpecailA && m_nCardFaceNum == 1 )return 14 ;return m_nCardFaceNum ; } // face num is the num show on the card ;
	unsigned char GetCardCompositeNum();  // Composite are make of face num and card type ;
	eCardType GetType(){ return m_eType ;}
	void LogCardInfo();
	CCard& SetCard(eCardType etype, unsigned char nFaceNum );
protected:
	eCardType m_eType ;
	unsigned char m_nCardFaceNum ;
};

class CPoker
{
public:
	CPoker();
	~CPoker();
	void InitPaiJiu() ;
	void InitTaxasPoker();
	void InitBaccarat();
	void InitGolden();
	unsigned char GetCardWithCompositeNum();
	unsigned short GetLeftCard(){ return m_nCardCount - m_nCurIdx; }
	unsigned short GetAllCard(){ return m_nCardCount;  }
	void ComfirmKeepCard( unsigned char nCardLeft = 3 * 5 ); // 开局发牌之前，一定要确认牌堆里是否有足够的牌。
	void RestAllPoker();
	unsigned char getCardNum( unsigned char nIdx );
protected:
	void AddCard(unsigned char nCard );   // invoke when init
	void AddCard(CCard* pcard);   // invoke when init
	void SetupCardCount(unsigned short nCount );
	void LogCardInfo();
protected:
	unsigned char* m_vCards ;
	unsigned short m_nCardCount ;
	unsigned short m_nCurIdx ;
};