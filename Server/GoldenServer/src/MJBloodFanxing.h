#pragma once
#include "MJCardFanXing.h"
#include <vector>
class CBloodFanxingPingHu
	:public IMJCardFanXing
{
public:
	enum eCheckResultType
	{
		eResult_ChunShun,
		eResult_ZhiYouJiang,
		eResult_YouJiangQueShun,
		eResult_ZhiQueJiang,
		eResult_WuJiangQueShun,
		eResult_Max,
	};

	struct stCheckResult
	{
		eCheckResultType eType ;
		std::vector<uint8_t> vQueCardValue ;
	};

	struct stCardTypeCheckResult
	{
		eMJCardType eType ;
		std::vector<stCheckResult> vResults ;
	};

	struct stCarItem 
	{
		uint8_t nCardValue ;
		uint8_t nCount ;
	};
	typedef std::vector<stCarItem> VEC_CARD_ITEM ;
public:
	CBloodFanxingPingHu();
	uint8_t getFanRate()override { return 1 ; }
	eFanxingType getType()override{ return eFanxing_PingHu ;}
	bool checkFanXingWantedCards(CMJPeerCard& peerCard, LIST_WANTED_CARD& vWaited)override;
	bool checkType(CMJPeerCard& peerCard)override ;
protected:
	bool checkChunShun(VEC_CARD_ITEM& vec);
	bool checkZhiYouJiang(VEC_CARD_ITEM& vec, std::vector<uint8_t>& vecOutQue);
	bool checkYouJiangQueShun(VEC_CARD_ITEM& vec , std::vector<uint8_t>& vecOutQue );
	bool checkZhiQueJiang(VEC_CARD_ITEM& vec  , std::vector<uint8_t>& vecOutQue );
	bool checkWuJiangQueShun( VEC_CARD_ITEM& vec  , std::vector<uint8_t>& vecOutQue );

	// help function 
	bool checkQueShun( VEC_CARD_ITEM& vec , std::vector<uint8_t>& vecOutQue );
	bool findQueShun( std::vector<uint8_t>& vec ,uint8_t& nQueIdx );
	bool findAllShun( std::vector<uint8_t>& vec , std::vector<std::vector<uint8_t>>& vecOutShunzi );
};

class CBloodQingYiSe
	:public IMJCardFanXing
{
public:
	uint8_t getFanRate()override{ return 4 ; } ;
	eFanxingType getType()override{ return eFanxing_QingYiSe ;}
	bool checkType(CMJPeerCard& peerCard)override ;
};

class CBloodFanxingQiDui
	:public IMJCardFanXing
{
public:
	CBloodFanxingQiDui();
	uint8_t getFanRate()override{ return 4 ; } ;
	eFanxingType getType()override{ return eFanxing_QiDui ;}
	bool checkFanXingWantedCards(CMJPeerCard& peerCard, LIST_WANTED_CARD& vWaited)override;
	bool checkType(CMJPeerCard& peerCard)override ;
};

class CBloodFanxingQingQiDui
	:public CBloodQingYiSe
{
public:
	uint8_t getFanRate()override{ return 16 ; } ;
	eFanxingType getType()override{ return eFanxing_QingDui ;}
};

class CBloodFanxingLongQiDui
	:public IMJCardFanXing
{
public:
	CBloodFanxingLongQiDui();
	uint8_t getFanRate()override{ return 16 ; } ;
	eFanxingType getType()override{ return eFanxing_LongQiDui ;}
	bool checkType(CMJPeerCard& peerCard)override ;
};

class CBloodFanxingQingLongQiDui
	:public CBloodQingYiSe
{
public:
	uint8_t getFanRate()override{ return 32 ; } ;
	eFanxingType getType()override{ return eFanxing_QingLongQiDui ;}
};

class CBloodFanxingDuiDuiHu
	:public IMJCardFanXing
{
public:
	CBloodFanxingDuiDuiHu();
	uint8_t getFanRate()override{ return 2 ; } ;
	eFanxingType getType()override{ return eFanxing_DuiDuiHu ;}
	bool checkFanXingWantedCards(CMJPeerCard& peerCard, LIST_WANTED_CARD& vWaited)override;
	bool checkType(CMJPeerCard& peerCard)override ;
};

class CBloodFanxingQingDuiDuiHu
	:public CBloodQingYiSe
{
public:
	uint8_t getFanRate()override{ return 8 ; } ;
	eFanxingType getType()override{ return eFanxing_QingDuiDuiHu ;}
};

class CBloodFanxingJinGouDiao
	:public IMJCardFanXing
{
public:
	CBloodFanxingJinGouDiao();
	uint8_t getFanRate()override{ return 4 ; } ;
	eFanxingType getType()override{ return eFanxing_JinGouDiao ;}
	bool checkType(CMJPeerCard& peerCard)override ;
};

class CBloodFanxingQingJinGouDiao
	:public CBloodQingYiSe
{
public:
	uint8_t getFanRate()override{ return 16 ; } ;
	eFanxingType getType()override{ return eFanxing_QingJinGouDiao ;}
};

class CBloodFanxingShiBaLuoHan
	:public IMJCardFanXing
{
public:
	uint8_t getFanRate()override{ return 32 ; } ;
	eFanxingType getType()override{ return eFanxing_ShiBaLuoHan ;}
	bool checkType(CMJPeerCard& peerCard)override ;
};

class CBloodFanxingJiangJinGouDiao
	:public IMJCardFanXing
{
public:
	uint8_t getFanRate()override{ return 16 ; } ;
	eFanxingType getType()override{ return eFanxing_JiangJinGouDiao ;}
	bool checkType(CMJPeerCard& peerCard)override ;
};


class CBloodFanxingDaiYaoJiu
	:public IMJCardFanXing
{
public:
	CBloodFanxingDaiYaoJiu();
	uint8_t getFanRate()override{ return 4 ; } ;
	eFanxingType getType()override{ return eFanxing_DaiYaoJiu ;}
	bool checkFanXingWantedCards(CMJPeerCard& peerCard, LIST_WANTED_CARD& vWaited)override;
	bool checkType(CMJPeerCard& peerCard)override ;
};

class CBloodFanxingQingDaiYaoJiu
	:public CBloodQingYiSe
{
public:
	uint8_t getFanRate()override{ return 16 ; } ;
	eFanxingType getType()override{ return eFanxing_QingDaiYaoJiu ;}
};