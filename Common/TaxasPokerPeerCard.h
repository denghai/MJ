#pragma once
#include<vector>
#include <string>
#include "CardPoker.h"
class CTaxasPokerPeerCard
{
public:
	enum eCardType
	{
		eCard_HuangJiaTongHuaShun,
		eCard_TongHuaShun,
		eCard_SiTiao,
		eCard_HuLu,
		eCard_TongHua,
		eCard_ShunZi,
		eCard_SanTiao,
		eCard_LiangDui,
		eCard_YiDui,
		eCard_GaoPai,
		eCard_Max,
		eCard_Robot_GaoPai4TongHua = eCard_Max,
		eCard_Robot_GaoPai4ShunZi,
		eCard_Robot_Max,
	};
public:
	typedef std::vector<CCard*> VEC_CARD ;
public:
	CTaxasPokerPeerCard(){Reset(); m_vReservs.clear() ;}
	~CTaxasPokerPeerCard();
	CCard* AddCardByCompsiteNum(unsigned char nCardNum );
	bool removePublicCompsiteNum( unsigned char nCardNum );
	char PK(CTaxasPokerPeerCard* pPeerCard );  // -1 failed 0 same , 1 win ;
	const char* GetTypeName();
	eCardType GetCardType();
	unsigned char GetPrivateCard(unsigned char nIdx){  if (m_vDefaul.size() > nIdx ) return m_vDefaul[nIdx]->GetCardCompositeNum(); return 0 ;}
	void Reset();
	void LogInfo();
	void GetFinalCard( unsigned char vMaxCard[5]);
	void GetHoldCard(unsigned char vHoldeCard[2] );
	unsigned char GetCardTypeForRobot(unsigned char& nContriButeCnt,unsigned char& nKeyCardFaceNum);
	void adjustPosForSpecailShunZi();
	unsigned char getAllCardCnt(){ return m_vAllCard.size() ;}
    bool checkIsSelected(unsigned char nCardNum);
	CTaxasPokerPeerCard& operator = (CTaxasPokerPeerCard& peerCard );
protected:
	void CaculateFinalCard();
	void ClearVecCard(VEC_CARD& vCards );
	void CheckShunZi(VEC_CARD& AllCard , bool bSpecailA, VEC_CARD& vResultChardOut );
	void robotCheck4ShunZi( VEC_CARD& AllCard , bool bSpecailA, VEC_CARD& vResultChardOut );
	unsigned char robotGetContribute(VEC_CARD& vFinalCard, VEC_CARD& vCheckCard, unsigned char& vOutKeyCardFaceNum );
	CCard* getReseveCardPtr();
protected:
	VEC_CARD m_vDefaul ;
	VEC_CARD m_vAllCard ;
	VEC_CARD m_vFinalCard ;
	VEC_CARD m_vPairs[2] ; // used when have pairs ;
	std::string m_strCardName ; 
	eCardType m_eType ;

	VEC_CARD m_vReservs;
	bool m_isCardDirty ;
};