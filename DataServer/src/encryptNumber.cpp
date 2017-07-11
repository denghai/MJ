#include "encryptNumber.h"
#include <ctime>
#include "ServerMessageDefine.h"
#include "log4z.h"
#include <cassert>
#include "ISeverApp.h"
bool CEncryptNumber::onMsg(stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID)
{
	if ( IGlobalModule::onMsg(prealMsg,eSenderPort,nSessionID) )
	{
		return true ;
	}

	switch ( prealMsg->usMsgType )
	{
	case MSG_ROBOT_GENERATE_ENCRYPT_NUMBER:
		{
			stMsgRobotGenerateEncryptNumber* pRet = (stMsgRobotGenerateEncryptNumber*)prealMsg ;
			assert(pRet->nGenCount > 0 && pRet->nGenCount < 5000 && "invalid req count number" );
			auto pReq = new stEncryptRequest ;
			pReq->nCoin = pRet->nCoin ;
			pReq->nCoinType = pRet->nCoinType;
			pReq->nNeedCnt = pRet->nGenCount ;
			pReq->nType = pRet->nNumberType ;
			pReq->nRMB = pRet->nRMB ;
			pReq->nChannelID = pRet->nChannelID ;
			if ( m_pCurRequest == nullptr )
			{
				m_pCurRequest = pReq ;
				doGenerateNumber(m_pCurRequest);
			}
			else
			{
				m_vAllRequest.push_back(pReq) ;
			}
		}
		break ;
	case MSG_SAVE_ENCRYPT_NUMBER:
		{
			stMsgSaveEncryptNumberRet* pRet = (stMsgSaveEncryptNumberRet*)prealMsg ;
			if ( pRet->nRet )
			{
				doGenerateNumber(m_pCurRequest);
				break;
			}

			--m_pCurRequest->nNeedCnt;
			if ( m_pCurRequest->nNeedCnt <= 0 )
			{
				delete m_pCurRequest ;
				m_pCurRequest = nullptr ;
				if ( m_vAllRequest.empty() )
				{
					LOGFMTI("create encrypt number ok ") ;
					return true ;
				}

				m_pCurRequest = m_vAllRequest.front() ;
				m_vAllRequest.pop_front() ;
			}

			if ( m_pCurRequest )
			{
				doGenerateNumber(m_pCurRequest);
			}
		}
		break ;
	default:
		return false ;
	}
	return true ;
}

void CEncryptNumber::doGenerateNumber(stEncryptRequest* pReq )
{
	stMsgSaveEncryptNumber msgSave ;
	msgSave.nCoin = pReq->nCoin ;
	msgSave.nEncryptNumber = generateNumber() ;
	while ( msgSave.nEncryptNumber <= 999999999999999 )
	{
		LOGFMTD("number = %llu not 16 bit, try again",msgSave.nEncryptNumber);
		msgSave.nEncryptNumber = generateNumber() ;
	}
	msgSave.nNumberType = pReq->nType ;
	msgSave.nRMB = pReq->nRMB ;
	msgSave.nCoinType = pReq->nCoinType ;
	msgSave.nChannelID = pReq->nChannelID ;
	getSvrApp()->sendMsg(0,(char*)&msgSave,sizeof(msgSave));
#ifdef  _DEBUG
	if ( isNumberValid(msgSave.nEncryptNumber) )
	{
		LOGFMTD("nuber = %lld ok",msgSave.nEncryptNumber);
	}
	else
	{
		LOGFMTE("nuber = %lld invalid",msgSave.nEncryptNumber);
		assert("number error " && 0 );
	}
#endif //  _DEBUG

}

uint64_t CEncryptNumber::generateNumber()
{
	time_t t = time(nullptr) ;
	tm* pNow = localtime(&t);

	uint32_t nHour = pNow->tm_hour % 0xF;
	uint32_t nMin = pNow->tm_min ;
	uint32_t nSec = pNow->tm_sec ;

	uint32_t nYear = pNow->tm_year % 8 ;
	uint32_t nMon = pNow->tm_mon ;
	uint32_t nDay = pNow->tm_mday;
	uint32_t nMicroSec = clock() % 1000 ;

	uint64_t nYearMDM = nMon << 18 | nDay << 13 | nYear << 10 | nMicroSec ;
 	uint64_t nHourMinSec = nHour << 12 | nMin << 6 | nSec ;

	uint64_t nRandModule = rand() % 0x7FF;
	
	uint64_t nCheckSum = ( nYearMDM + nRandModule + nHourMinSec ) % 0xF + 1 ;

	uint64_t nFinalNumber = nCheckSum << 49 | nHourMinSec << 33 | nRandModule << 22 | nYearMDM;

	LOGFMTD(" nYearMDM = %lld , hourMin = %lld , rand = %lld , checkSum = %lld",nYearMDM,nHourMinSec,nRandModule,nCheckSum) ;
	return nFinalNumber ;
}

bool CEncryptNumber::isNumberValid(uint64_t nNumber )
{
	uint64_t nCheckSum = (nNumber & (uint64_t)0xF << 49  ) >> 49  ;
	uint64_t nHourMinSec = ( nNumber & (uint64_t)0xFFFF << 33 ) >> 33;
	uint64_t nRandModule = ( nNumber & (uint64_t)0x7FF << 22 ) >> 22;
	uint64_t nYearMDM = nNumber & 0x3FFFFF ;

	LOGFMTD(" nYearMDM = %lld , hourMin = %lld , rand = %lld , checkSum = %lld",nYearMDM,nHourMinSec,nRandModule,nCheckSum) ;
	return ( ( nYearMDM + nRandModule + nHourMinSec) % 0xF + 1 == nCheckSum );
}