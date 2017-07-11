//#pragma once
//#pragma pack(push)
//#pragma pack(1)
//#include "MessageDefine.h"
//#include "NativeTypes.h"
//enum ePJActionRet
//{
//	ePJ_ActRet_Success,
//	ePJ_ActRet_State_Not_Fit,
//	ePJ_ActRet_Self_Money_Not_Enough,
//	ePJ_ActRet_Banker_Money_Not_Enough,
//	ePJ_ActRet_Apply_List_Full,
//	ePJ_ActRet_SELF_NOT_BANKER,
//	ePJ_ActRet_Identifer_Wrong,
//	ePJ_ActRet_Aready_In_ApplyList, 
//	ePJ_ActRe_UnknowError,
//};
//
//struct stApplyBankerItem
//{
//	unsigned int nSessionID;
//	uint64_t nCoin ;
//	char nName[MAX_LEN_CHARACTER_NAME] ;
//};
//
//struct stHistroyRecorder
//{
//	unsigned char nResult ; // 0 default , 1 win ,2 failed ;
//	unsigned int nPokerShuffleRound ;  // nCardRound + 1 , when shuffle ;
//	unsigned int nBankerSessionID ; 
//	stHistroyRecorder():nResult(0),nPokerShuffleRound(0),nBankerSessionID(0){ }
//};
//
//struct stMsgPJRoomInfo
//	:public stMsg
//{
//	stMsgPJRoomInfo()
//	{
//		cSysIdentifer = ID_MSG_GAME2C; usMsgType = MSG_PJ_ROOM_INFO ;
//	}
//	unsigned int nRoomID ;
//	unsigned char eRoomState ; // eRoomState ;
//	float fStateTick ; 
//	unsigned char vPortCard[ePJ_BetPort_Normal_Max][2] ;
//	uint64_t nBetCoin[ePJ_BetPort_Max] ;
//	char cBankerName[MAX_LEN_CHARACTER_NAME] ;
//	unsigned short nBankerPhotoID ;
//	uint64_t nBankerCoin ;
//	stHistroyRecorder nHistroy[ePJ_BetPort_Normal_Max][MAX_PAIJIU_HISTROY_RECORDER];
//	unsigned char nLeftCard ; 
//	unsigned char nApplyBankerCount ;
//	stApplyBankerItem*	applyers;
//};
//
//struct stMsgPJActionRet
//	:public stMsg
//{
//	stMsgPJActionRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_PJ_ACTION_RET ; }
//	unsigned short nAcionType ;  // usMsgType from client ;
//	unsigned nRet ; 
//};
//
//struct stMsgPJRoomStateChanged
//	:public stMsg
//{
//public:
//	stMsgPJRoomStateChanged(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_PJ_STATE_CHANGED ; }
//	unsigned char nCurrentState ;   // eRoomState
//};
//
//struct stMsgPJNewBankerChoseShuffle
//	:public stMsg
//{
//	stMsgPJNewBankerChoseShuffle(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_PJ_BANKER_CHOSE_SHUFFLE ; }
//};
//
//struct stMsgPJBet
//	:public stMsg
//{
//	stMsgPJBet(){ cSysIdentifer = ID_MSG_C2GAME; usMsgType = MSG_PJ_BET ; }
//	uint64_t nBetCoin ;
//	unsigned char nBetPort ; //ePaiJiuBetPort 
//};
//
//struct stMsgPJBetRet
//	:public stMsgPJActionRet
//{
//	stMsgPJBetRet(){ nAcionType = MSG_PJ_BET ; }
//	uint64_t nBetCoin ;
//	unsigned char nBetPort ; //ePaiJiuBetPort 
//};
//
//struct stMsgPJOtherBet
//	:public stMsg
//{
//	stMsgPJOtherBet(){cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_PJ_OTHER_BET ; }
//	uint64_t nBetCoin ;
//	unsigned char nBetPort ; //ePaiJiuBetPort 
//};
//
//struct stMsgPJApplyToBeBanker
//	:public stMsg
//{
//	stMsgPJApplyToBeBanker(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_PJ_APPLY_BANKER ; }
//
//};
//
//struct stMsgPJBankerChoseGoOnOrCanncel
//	:public stMsg
//{
//	stMsgPJBankerChoseGoOnOrCanncel(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_PJ_BANKER_CHOSE_CONTINUE_CANCEL ; }
//	unsigned char nChoice ; // 0  , go on player , 2 unbanker ;
//};
//
////struct stMsgPJApplyToBeBankerRet
////	:public stMsg
////{
////	stMsgPJApplyToBeBankerRet()
////	{
////		cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_PJ_APPLY_BANKER ;
////	}
////	unsigned char nRet ; // 0 success , 1 money is not enough , 2 apply list is full ;
////};
//
//struct stMsgPJOtherApplyToBeBanker
//	:public stMsg 
//{
//	stMsgPJOtherApplyToBeBanker(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_PJ_OTHER_APPLY_BANKER ; }
//	char cApplyerName[MAX_LEN_CHARACTER_NAME] ;
//	uint64_t nApplyerCoin ;
//};
//
//struct stMsgPJBankerChanged
//	:public stMsg
//{
//	stMsgPJBankerChanged(){ cSysIdentifer = ID_MSG_GAME2C; usMsgType = MSG_PJ_BANKER_CHANGED ; }
//	char cNewBankerName[MAX_LEN_CHARACTER_NAME] ;
//	uint64_t nNewBankerCoin;
//	unsigned int newBankerSessionID ;
//};
//
//struct stMsgPJDice
//	:public stMsgPJRoomStateChanged
//{
//	stMsgPJDice() { nCurrentState = eRoomState_PJ_Dice ; }
//	unsigned char nPoint[2] ;
//};
//
//struct stMsgPJDistribute
//	:public stMsgPJRoomStateChanged 
//{
//	stMsgPJDistribute(){ nCurrentState = eRoomState_PJ_Distribute; }
//	unsigned char vCardZhuang[2] ;  // ePJ_BetPort_Banker
//	unsigned char vCardShun[2] ;  // ePJ_BetPort_Shun
//	unsigned char vCardTian[2] ; // ePJ_BetPort_Tian 
//	unsigned char vCardDao[2] ; // ePJ_BetPort_Dao 
//};
//
////struct stMsgPJBankerViewCard
////	:public stMsg
////{
////	stMsgPJBankerViewCard(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_PJ_BANKER_VIEWCARD ; }
////};
//
////struct stMsgPJBankerShowCard
////	:public stMsg 
////{
////	stMsgPJBankerShowCard(){ cSysIdentifer = ID_MSG_C2GAME; usMsgType = MSG_PJ_BANKER_SHOW_CARD ; }
////};
////
////struct stMsgPJShowBankerCard
////	:public stMsg
////{
////	stMsgPJShowBankerCard(){ cSysIdentifer = ID_MSG_GAME2C; usMsgType = MSG_PJ_SHOW_BANKER_CARD ; }
////};
//
////struct stMsgPJShuffle
////	:public stMsg
////{
////	stMsgPJShuffle()
////	{
////		cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_PJ_SHUFFLE ;
////	}
////};
//
//struct stMsgPJSettlement
//	:public stMsgPJRoomStateChanged
//{
//	stMsgPJSettlement(){ nCurrentState = eRoomState_PJ_Settlement; }
//	int64_t nResultCoinOffset ; // > 0 win , < 0 lose ;
//	int64_t nBankerCoinOffset; // > 0 win , < 0 lose ;
//};
//
//struct stMsgPJApplyUnbanker
//	:public stMsg
//{
//	stMsgPJApplyUnbanker(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_PJ_APPLY_UNBANKER ; }
//};
//
//struct stMsgPJRoomApplyList
//	:public stMsg 
//{
//	stMsgPJRoomApplyList(){ cSysIdentifer = ID_MSG_GAME2C; usMsgType = MSG_PJ_ROOM_APPLY_BANKER_LIST ; }
//	unsigned char nCount ;
//	stApplyBankerItem* pApplyItems ;
//};
//
//struct stMsgPJRoomHistoryRecord
//	:public stMsg
//{
//	stMsgPJRoomHistoryRecord(){cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_PJ_ROOM_RECORD ; }
//	stHistroyRecorder nHistroy[ePJ_BetPort_Normal_Max][MAX_PAIJIU_HISTROY_RECORDER];
//};
//
////struct stMsgPJApplyUnbackerRet
////	:public stMsg
////{
////	stMsgPJApplyUnbackerRet(){ cSysIdentifer = ID_MSG_GAME2C; usMsgType = MSG_PJ_APPLY_UNBANKER ; }
////	unsigned char nRet ; // 0 success , 1 you are not banker now ;
////};
//#pragma pack(pop)