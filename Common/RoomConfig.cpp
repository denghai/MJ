#include "RoomConfig.h"
#include "CommonDefine.h"
#ifdef SERVER
#include "log4z.h"
#endif
#include <assert.h>
bool CRoomConfigMgr::OnPaser(CReaderRow& refReaderRow )
{
	return true;

	unsigned char cType = refReaderRow["GameType"]->IntValue() ;
	stBaseRoomConfig* pRoomConfig = NULL ;
	switch ( cType )
	{
	case eRoom_NiuNiu:
    case eRoom_Golden:
		{
			stNiuNiuRoomConfig* pConfig = NULL;
            if (cType == eRoom_Golden) {
                pConfig = new stGoldenRoomConfig();
            }else{
                pConfig = new stNiuNiuRoomConfig();
            }
			pConfig->nBaseBet = refReaderRow["BigBlind"]->IntValue();
			//pConfig->nMiniTakeInCoin = refReaderRow["miniTakeIn"]->IntValue() ;
			pConfig->nMaxSeat = refReaderRow["MaxSeat"]->IntValue();
			pConfig->nBaseTakeIn = refReaderRow["baseTakeIn"]->IntValue();
			pRoomConfig = pConfig ;
		}
		break;
	case eRoom_TexasPoker:
		{
			stTaxasRoomConfig* pConfig = new stTaxasRoomConfig ;
			pConfig->nBigBlind = refReaderRow["BigBlind"]->IntValue();
			//pConfig->nMaxTakeInCoin = refReaderRow["maxTakeIn"]->IntValue() ;
			//pConfig->nMiniTakeInCoin = refReaderRow["miniTakeIn"]->IntValue() ;
			pConfig->nMaxSeat = refReaderRow["MaxSeat"]->IntValue();
			pConfig->nBaseTakeIn = refReaderRow["baseTakeIn"]->IntValue();
			pRoomConfig = pConfig ;
		}
		break;
//	case eRoom_Golden:
//		{
//			stGoldenRoomConfig* pConfig = new stGoldenRoomConfig ;
//			pConfig->bCanDoublePK = (bool)refReaderRow["CanDoublePK"]->IntValue();
//			pConfig->nChangeCardRound = refReaderRow["ChangeCardRound"]->IntValue();
//			pConfig->nMiniBet = refReaderRow["MiniBet"]->IntValue();
//			pConfig->nTitleNeedToEnter = refReaderRow["TitleNeedToEnter"]->IntValue();
// #ifdef SERVER
// 			char pBuffer[256] = {0};
// 			for ( int i = 0 ; i < GOLDEN_ROOM_COIN_LEVEL_CNT ; ++i )
// 			{
// 				memset(pBuffer,0,sizeof(pBuffer));
// 				sprintf_s(pBuffer,"CoinLevel%d",i);
// 				pConfig->vCoinLevels[i] = refReaderRow[pBuffer]->IntValue();
// 			}
// #endif
//			pRoomConfig = pConfig ;
//		}
//		break;
	default:
#ifdef SERVER
		LOGFMTE( "unknown room config ,room type = %d",cType ) ;
#endif
		return false;
	}
	pRoomConfig->nGameType = cType ;
	pRoomConfig->nConfigID = refReaderRow["configID"]->IntValue();
	pRoomConfig->nRentFeePerDay = refReaderRow["RendFeePerDay"]->IntValue() ;
	pRoomConfig->nDeskFee = refReaderRow["DeskFee"]->IntValue() ;
	pRoomConfig->fDividFeeRate = refReaderRow["DividFeeRate"]->FloatValue() ;
	//pRoomConfig->nCoinLowLimit = refReaderRow["CoinLowLimit"]->IntValue();
	//pRoomConfig->nCoinTopLimit = refReaderRow["CoinTopLimit"]->IntValue();
	//pRoomConfig->bIsNeedRegistered = (bool)(refReaderRow["NeedRegister"]->IntValue());
	//pRoomConfig->bIsOmitNewPlayerHalo = (bool)(refReaderRow["isOmitNewPlayerHalo"]->IntValue());
	//pRoomConfig->nMaxLose = refReaderRow["MaxLose"]->IntValue() ;
#ifdef SERVER
	pRoomConfig->nNeedRobotLevel = refReaderRow["NeedRobotLevel"]->IntValue() ;
#endif // SERVER

	//refReaderRow["Reward"]->VecInt(pRoomConfig->vRewardID);
	pRoomConfig->fDividFeeRate = pRoomConfig->fDividFeeRate / 100.00f ;
//	if ( pRoomConfig->vRewardID.empty() )
//	{
//		LOGFMTE("room config id = %d reward is null",pRoomConfig->nConfigID) ;
//	}
	m_vAllConfig.push_back(pRoomConfig) ;
	return true ;
}

void CRoomConfigMgr::Clear()
{

	LIST_ROOM_CONFIG::iterator iter = m_vAllConfig.begin();
	for ( ; iter != m_vAllConfig.end() ; ++iter )
	{
		delete *iter ;
		*iter = NULL ;
	}
	m_vAllConfig.clear() ;
}

void CRoomConfigMgr::OnFinishPaseFile()
{
	IConfigFile::OnFinishPaseFile();
}

int CRoomConfigMgr::GetConfigCnt( )
{
	return (uint32_t)m_vAllConfig.size() ;
}

stBaseRoomConfig* CRoomConfigMgr::GetConfigByConfigID( uint16_t nConfigID )
{
	LIST_ITER iter = GetBeginIter() ;
	for ( ; iter != GetEndIter() ; ++iter )
	{
		if ( (*iter)->nConfigID == nConfigID )
		{
			return (stBaseRoomConfig*)(*iter) ;
		}
	}
	return nullptr ;
}

#ifndef SERVER
int CRoomConfigMgr::GetConfigCntByRoomType(int roomType){
    int count = 0;
    LIST_ITER iter = GetBeginIter() ;
    for ( ; iter != GetEndIter() ; ++iter )
    {
        if ( (*iter)->nGameType == roomType)
        {
            count++;
        }
    }
    return count;
}
stBaseRoomConfig* CRoomConfigMgr::GetConfigByConfigID( uint16_t idx , int roomType){
    LIST_ITER iter = GetBeginIter() ;
    for ( ; iter != GetEndIter() ; ++iter )
    {
        if ( (*iter)->nGameType == roomType )
        {
            idx--;
            if (!idx) {
                return (stBaseRoomConfig*)(*iter) ;
            }
        }
    }
    return nullptr ;
}
#endif


