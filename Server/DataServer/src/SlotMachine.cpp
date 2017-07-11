#include "SlotMachine.h"
#include "LogManager.h"
#include <set>
#define PECENT_BASE 1000000
CSlotMachine::CSlotMachine()
{
	memset(m_vResultCnt,0,sizeof(m_vResultCnt)) ;
	nTotalCnt = 0 ;
	//memset(m_vAllConfig,0 , sizeof(m_vAllConfig)) ;
	m_tPoker.InitTaxasPoker();
}

CSlotMachine::~CSlotMachine()
{

}

bool CSlotMachine::OnPaser(CReaderRow& refReaderRow )
{
	unsigned char cCardType = refReaderRow["cardID"]->IntValue() ;
	m_vAllConfig[cCardType].eType = cCardType ;
	m_vAllConfig[cCardType].fRewardRate = refReaderRow["RewardRate"]->IntValue();
	std::vector<float> vFloat ;
	refReaderRow["nRate"]->VecFloat(vFloat) ;
	if ( vFloat.size() == 0 || vFloat.size() % 2 != 0 )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("slot machine config rate error ,type = %d",cCardType) ;
		return false ;
	}

	for ( unsigned int i = 0 ; (i + 1) < vFloat.size(); i += 2 )
	{
		unsigned char nKey = vFloat[i] ;
		float fValue = vFloat[i+1] ;
		m_vAllConfig[cCardType].vMapLevelRate.insert(std::map<unsigned char,float>::value_type(nKey,fValue)) ;
	}
	return true ;
}

float CSlotMachine::stSlotConfig::GetRate(unsigned char cLevel )
{
	unsigned char cTopBiggercLevel = 0 ;
	std::map<unsigned char ,float>& vMap = vMapLevelRate ;
	if ( vMap.empty() )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("rate config is null , slot config for card type = %d",eType) ;
		return 0 ;
	}

	std::map<unsigned char ,float>::iterator iter = vMap.begin();
	for ( ; iter != vMap.end() ; ++iter )
	{
		if ( cLevel > iter->first )
		{
			continue;
		}

		if ( cTopBiggercLevel == 0 )
		{
			cTopBiggercLevel = iter->first ;
			continue; 
		}

		if ( cTopBiggercLevel >= iter->first )
		{
			cTopBiggercLevel = iter->first ;
		}
	}

	if ( cTopBiggercLevel == 0 )
	{
		--iter;
		cTopBiggercLevel = iter->first ;
	}
	iter = vMap.find(cTopBiggercLevel) ;
	return iter->second ;
}

void CSlotMachine::OnFinishPaseFile()
{
	//int nTaotalRate = 0 ;
	//for ( int i = 0 ; i < CTaxasPokerPeerCard::eCard_Max ; ++i )
	//{
	//	nTaotalRate += m_vAllConfig[i].nRate ;
	//}

	//if (nTaotalRate != PECENT_BASE )
	//{
	//	CLogMgr::SharedLogMgr()->ErrorLog("slot machine card type rate total not equal %d !",PECENT_BASE) ;
	//}

	//memset(m_vResultCnt,0,sizeof(m_vResultCnt)) ;
	//nTotalCnt = 0 ;
	return ;
}

CTaxasPokerPeerCard::eCardType CSlotMachine::RandSlotMachine(unsigned char cLevel,unsigned char* arrayFive,float& fRewardRate )
{
	CTaxasPokerPeerCard::eCardType eType = GetRandCardType(cLevel);
	fRewardRate = m_vAllConfig[eType].fRewardRate;
	switch ( eType )
	{
	case CTaxasPokerPeerCard::eCard_HuangJiaTongHuaShun:
	case CTaxasPokerPeerCard::eCard_TongHuaShun:
	case CTaxasPokerPeerCard::eCard_ShunZi:
		{
			// find shunzi start point
			unsigned char nStartNum = 10 ;
			if ( eType == CTaxasPokerPeerCard::eCard_TongHuaShun )
			{
				nStartNum = 2 + rand() % 8 ;
			}

			if ( eType == CTaxasPokerPeerCard::eCard_ShunZi )
			{
				nStartNum = 2 + rand() % 9 ;
			}

			// find shunzi face type
			unsigned char cType = rand() % CCard::eCard_NoJoker ;
			CCard card ;
			for ( int i = 0 , iUseType = cType; i < 5 ; ++i )
			{
				unsigned nNum = nStartNum + i;
				if ( nNum > 13 )
				{
					nNum -= 13 ;
				}

				if ( eType == CTaxasPokerPeerCard::eCard_ShunZi )
				{
					++iUseType ;
					cType = iUseType % CCard::eCard_NoJoker  ;
				}

				card.SetCard((CCard::eCardType)cType,nNum ) ;
				arrayFive[i] = card.GetCardCompositeNum();
				if ( arrayFive[i] > 54 )
				{
					CLogMgr::SharedLogMgr()->ErrorLog("arrayFive[i] > 54") ;
				}

			}
			DeBugCard(arrayFive,eType);
		}
		break;
	case CTaxasPokerPeerCard::eCard_SiTiao:
		{
			unsigned char nFourNum = rand() % 13 + 1 ;
			unsigned char nLeftNum = rand() % 13 + 1 ;
			while ( nFourNum == nLeftNum )
			{
				nLeftNum = rand() % 13 + 1 ;
			}
			
			CCard card ;
			for ( int i = CCard::eCard_None ;i < CCard::eCard_NoJoker; ++i )
			{
				card.SetCard((CCard::eCardType)i,nFourNum) ;
				arrayFive[i] = card.GetCardCompositeNum() ;
			}

			unsigned char ncType = rand() % CCard::eCard_NoJoker ;
			card.SetCard((CCard::eCardType)ncType,nLeftNum) ;
			arrayFive[4] = card.GetCardCompositeNum();
			DeBugCard(arrayFive,eType);
		}
		break;
	case CTaxasPokerPeerCard::eCard_HuLu:
		{
			// find 3 card ;
			CCard card ;
			unsigned char nThree = 1 + rand() % 13 ;
			unsigned char NotType = rand() % CCard::eCard_NoJoker ;
			int cardIdx  = 0 ;
			for ( int i = 0 ; i < CCard::eCard_NoJoker; ++i )
			{
				if ( i == NotType )
				{
					continue;
				}
				card.SetCard((CCard::eCardType)i,nThree);
				arrayFive[cardIdx++] = card.GetCardCompositeNum() ;
			}

			// find pair ;
			unsigned char nPairNum = rand() % 13 + 1 ;
			while ( nPairNum == nThree )
			{
				nPairNum = rand() % 13 + 1 ;
			}

			unsigned char nPairType = rand() % CCard::eCard_NoJoker ;
			unsigned char nOhter = rand() % CCard::eCard_NoJoker ;
			while ( nOhter == nPairType )
			{
				nOhter = rand() % CCard::eCard_NoJoker ;
			}

			card.SetCard((CCard::eCardType)nPairType,nPairNum);
			arrayFive[cardIdx++] = card.GetCardCompositeNum() ;

			card.SetCard((CCard::eCardType)nOhter,nPairNum);
			arrayFive[cardIdx++] = card.GetCardCompositeNum() ;
			DeBugCard(arrayFive,eType);
		}
		break;
	case CTaxasPokerPeerCard::eCard_TongHua:
		{
			std::set<unsigned char> vResult ;
			bool bShunZi = true ;
			while( bShunZi )
			{
				vResult.clear() ;
				while( vResult.size() != 5 )
				{
					vResult.insert((unsigned char)(1 + rand() % 13)) ;
				}
				// check shunzi ;
				std::set<unsigned char>::iterator iter = vResult.begin() ;
				unsigned char nLast = 0 ;
				for ( ;iter != vResult.end(); ++iter )
				{
					if ( nLast == 0 )
					{
						nLast = *iter ;
						continue;
					}

					nLast += 1 ;   // ascend arrange ; 
					if (  nLast != *iter )
					{
						bShunZi = false ;
						break; 
					}
				}
			}

			// put card ;
			unsigned char cType = rand() % CCard::eCard_NoJoker ;
			std::set<unsigned char>::iterator iter = vResult.begin() ;
			CCard card ;
			for ( int n = 0 ;iter != vResult.end(); ++iter, ++n )
			{
				card.SetCard((CCard::eCardType)cType,*iter) ;
				arrayFive[n] = card.GetCardCompositeNum() ;
			}
			DeBugCard(arrayFive,eType);
		}
		break;
	case CTaxasPokerPeerCard::eCard_SanTiao:
		{
			// find three ;
			unsigned char nThree = rand() % 13 + 1 ;
			unsigned char cType = rand() % CCard::eCard_NoJoker;
			unsigned char nIdx = 0 ;
			CCard card ;
			for ( unsigned char cUseType = cType; nIdx < 3 ; ++nIdx )
			{
				++cUseType;
				cType = cUseType ;
				if ( cType >= CCard::eCard_NoJoker )
				{
					cType -= CCard::eCard_NoJoker ;
				}
				card.SetCard((CCard::eCardType)cType,nThree) ;
				arrayFive[nIdx] = card.GetCardCompositeNum();
			}

			// find forth card 
			unsigned char nForth = rand() %13 + 1 ;
			while( nForth == nThree )
			{
				nForth = rand() %13 + 1 ;
			}
			cType = rand() % CCard::eCard_NoJoker;
			card.SetCard((CCard::eCardType)cType,nForth) ;
			arrayFive[nIdx++] = card.GetCardCompositeNum();

			// find five forth card 
			unsigned char nFifth = rand() %13 + 1 ;
			while( nFifth == nThree || nFifth == nForth  )
			{
				nFifth = rand() %13 + 1 ;
			}
			cType = rand() % CCard::eCard_NoJoker;
			card.SetCard((CCard::eCardType)cType,nFifth) ;
			arrayFive[nIdx++] = card.GetCardCompositeNum();
			DeBugCard(arrayFive,eType);
		}
		break;
	case CTaxasPokerPeerCard::eCard_LiangDui:
		{
			// find first pair 
			unsigned char nFristPair = rand() %13 + 1 ;
			unsigned char cType = rand() % CCard::eCard_NoJoker ;
			unsigned char nIdx = 0 ;

			CCard card ;
			card.SetCard((CCard::eCardType)cType,nFristPair) ;
			arrayFive[nIdx++] = card.GetCardCompositeNum() ;

			++cType ;
			if ( cType >= CCard::eCard_NoJoker )
			{
				cType -= CCard::eCard_NoJoker;
			}
			card.SetCard((CCard::eCardType)cType,nFristPair) ;
			arrayFive[nIdx++] = card.GetCardCompositeNum() ;

			// find second pair 
			unsigned char nSecondPair = rand() %13 + 1 ;
			while ( nSecondPair == nFristPair )
			{
				nSecondPair = rand() %13 + 1;
			}

			cType = rand() % CCard::eCard_NoJoker ;
			card.SetCard((CCard::eCardType)cType,nSecondPair) ;
			arrayFive[nIdx++] = card.GetCardCompositeNum() ;

			++cType ;
			if ( cType >= CCard::eCard_NoJoker )
			{
				cType -= CCard::eCard_NoJoker;
			}
			card.SetCard((CCard::eCardType)cType,nSecondPair) ;
			arrayFive[nIdx++] = card.GetCardCompositeNum() ;

			// find five card ;
			unsigned char nFifth = rand() % 13 + 1 ;
			while ( nFifth == nFristPair || nFifth == nSecondPair )
			{
				nFifth = rand() % 13 + 1 ;
			}
			cType = rand() % CCard::eCard_NoJoker ;
			card.SetCard((CCard::eCardType)cType,nFifth) ;
			arrayFive[nIdx++] = card.GetCardCompositeNum() ;
			DeBugCard(arrayFive,eType);
		}
		break;
	case CTaxasPokerPeerCard::eCard_YiDui:
		{
			unsigned char nPair = rand() % 13 + 1 ;
			std::set<unsigned char> vSet ;
			vSet.insert(nPair) ;
			while( vSet.size() != 4 )
			{
				vSet.insert( (rand() % 13 + 1) ) ;
			}
			vSet.erase(nPair) ;

			unsigned char cType = rand() % CCard::eCard_NoJoker ;
			unsigned char nIdx = 0;
			CCard card ;
			card.SetCard((CCard::eCardType)cType,nPair) ;
			arrayFive[nIdx++] = card.GetCardCompositeNum() ;

			++cType ;
			if ( cType >= CCard::eCard_NoJoker )
			{
				cType -= CCard::eCard_NoJoker ;
			}
			card.SetCard((CCard::eCardType)cType,nPair) ;
			arrayFive[nIdx++] = card.GetCardCompositeNum() ;

			std::set<unsigned char>::iterator iter = vSet.begin() ;
			for ( ; iter != vSet.end(); ++iter )
			{
				cType = rand() % CCard::eCard_NoJoker ;
				card.SetCard((CCard::eCardType)cType,*iter) ;
				arrayFive[nIdx++] = card.GetCardCompositeNum() ;
			}
			DeBugCard(arrayFive,eType);
		}
		break;
	case CTaxasPokerPeerCard::eCard_GaoPai:
		{
			bool bGaoPai = false ;
			while ( !bGaoPai  )
			{
				m_tPoker.ComfirmKeepCard(5) ;
				CTaxasPokerPeerCard PeerCard ;
				for ( int i = 0 ; i < 5 ; ++i )
				{
					arrayFive[i] = m_tPoker.GetCardWithCompositeNum() ;
					PeerCard.AddCardByCompsiteNum(arrayFive[i]) ;
				}
				bGaoPai = PeerCard.GetCardType() == CTaxasPokerPeerCard::eCard_GaoPai ;
			}
			DeBugCard(arrayFive,eType);
		}
		break;
	default:
		{
			CLogMgr::SharedLogMgr()->ErrorLog("Unknown car type when rand slot machine !") ;
		}
	}
#ifndef NDEBUG
	++nTotalCnt;
	++m_vResultCnt[eType];
	LogResultInfo();
#endif
	return eType;
}

void CSlotMachine::DeBugCard(unsigned char* vCard , unsigned cType )
{
	return ;
	for ( int i = 0 ; i < 5 ; ++i )
	{
		CLogMgr::SharedLogMgr()->PrintLog("idx card =%d",vCard[i]) ;
	}

	CTaxasPokerPeerCard peer ;
	for ( int i = 0 ; i < 5 ; ++i )
	{
		peer.AddCardByCompsiteNum(vCard[i]) ;
		CLogMgr::SharedLogMgr()->PrintLog("idx card in add num =%d",vCard[i]) ;
	}
	unsigned char nRealType = peer.GetCardType();
	peer.LogInfo();
	if ( nRealType !=cType && cType != 0 )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("Error error !") ;
	}
}

CTaxasPokerPeerCard::eCardType CSlotMachine::GetRandCardType( unsigned cLevel )
{
	float nRandom = float(rand()) /float(RAND_MAX)  ;
	nRandom *= PECENT_BASE ;  // [0,PECENT_BASE );
	float TestRandom = 0 ;
	int nIdx =  0 ;
	float nRate = 0;
	for ( int i = rand() % CTaxasPokerPeerCard::eCard_Max ; i < CTaxasPokerPeerCard::eCard_Max * 2; ++i )
	{
		nIdx = i % CTaxasPokerPeerCard::eCard_Max ;
		nRate = m_vAllConfig[nIdx].GetRate(cLevel);
		if ( TestRandom <= nRandom && nRandom < (TestRandom + nRate) )
		{
			return (CTaxasPokerPeerCard::eCardType)nIdx ;
		}
		else
		{
			TestRandom += nRate ;
		}
	}
	CLogMgr::SharedLogMgr()->ErrorLog("no proper rate nRandom = %.3f",nRandom ) ;
	return CTaxasPokerPeerCard::eCard_GaoPai ;
}

void CSlotMachine::LogResultInfo()
{
	if ( nTotalCnt % 1000 != 0 )
	{
		return ;
	}

	//CLogMgr::SharedLogMgr()->ErrorLog("老虎机执行情况: 总次数：%d",nTotalCnt) ;

	//for ( int eType = 0 ; CTaxasPokerPeerCard::eCard_Max > eType ; ++eType )
	//{
	//	switch ( eType )
	//	{
	//	case CTaxasPokerPeerCard::eCard_HuangJiaTongHuaShun:
	//		{
	//			float fRate = (float)m_vResultCnt[eType] / (float)nTotalCnt ;
	//			fRate *= 100 ;
	//			CLogMgr::SharedLogMgr()->SystemLog("皇家顺 实际概率：%.3f, 配置概率: %.3f",fRate,m_vAllConfig[eType].nRate) ;
	//		}
	//		break;
	//	case CTaxasPokerPeerCard::eCard_TongHuaShun:
	//		{
	//			float fRate = (float)m_vResultCnt[eType] / (float)nTotalCnt ;
	//			fRate *= 100 ;
	//			CLogMgr::SharedLogMgr()->SystemLog("同花顺 实际概率：%.3f, 配置概率: %.3f",fRate,m_vAllConfig[eType].nRate) ;
	//		}
	//		break;
	//	case CTaxasPokerPeerCard::eCard_SiTiao:
	//		{
	//			float fRate = (float)m_vResultCnt[eType] / (float)nTotalCnt ;
	//			fRate *= 100 ;
	//			CLogMgr::SharedLogMgr()->SystemLog("四条 实际概率：%.3f, 配置概率: %.3f",fRate,m_vAllConfig[eType].nRate) ;
	//		}
	//		break;
	//	case CTaxasPokerPeerCard::eCard_HuLu:
	//		{
	//			float fRate = (float)m_vResultCnt[eType] / (float)nTotalCnt ;
	//			fRate *= 100 ;
	//			CLogMgr::SharedLogMgr()->SystemLog("葫芦 实际概率：%.3f, 配置概率: %.3f",fRate,m_vAllConfig[eType].nRate) ;
	//		}
	//		break;
	//	case CTaxasPokerPeerCard::eCard_TongHua:
	//		{
	//			float fRate = (float)m_vResultCnt[eType] / (float)nTotalCnt ;
	//			fRate *= 100 ;
	//			CLogMgr::SharedLogMgr()->SystemLog("同花 实际概率：%.3f, 配置概率: %.3f",fRate,m_vAllConfig[eType].nRate) ;
	//		}
	//		break;
	//	case CTaxasPokerPeerCard::eCard_ShunZi:
	//		{
	//			float fRate = (float)m_vResultCnt[eType] / (float)nTotalCnt ;
	//			fRate *= 100 ;
	//			CLogMgr::SharedLogMgr()->SystemLog("顺子 实际概率：%.3f, 配置概率: %.3f",fRate,m_vAllConfig[eType].nRate) ;
	//		}
	//		break;
	//	case CTaxasPokerPeerCard::eCard_SanTiao:
	//		{
	//			float fRate = (float)m_vResultCnt[eType] / (float)nTotalCnt ;
	//			fRate *= 100 ;
	//			CLogMgr::SharedLogMgr()->SystemLog("三条 实际概率：%.3f, 配置概率: %.3f",fRate,m_vAllConfig[eType].nRate) ;
	//		}
	//		break;
	//	case CTaxasPokerPeerCard::eCard_LiangDui:
	//		{
	//			float fRate = (float)m_vResultCnt[eType] / (float)nTotalCnt ;
	//			fRate *= 100 ;
	//			CLogMgr::SharedLogMgr()->SystemLog("两对 实际概率：%.3f, 配置概率: %.3f",fRate,m_vAllConfig[eType].nRate) ;
	//		}
	//		break;
	//	case CTaxasPokerPeerCard::eCard_YiDui:
	//		{
	//			float fRate = (float)m_vResultCnt[eType] / (float)nTotalCnt ;
	//			fRate *= 100 ;
	//			CLogMgr::SharedLogMgr()->SystemLog("一对 实际概率：%.3f, 配置概率: %.3f",fRate,m_vAllConfig[eType].nRate) ;
	//		}
	//		break;
	//	case CTaxasPokerPeerCard::eCard_GaoPai:
	//		{
	//			float fRate = (float)m_vResultCnt[eType] / (float)nTotalCnt ;
	//			fRate *= 100 ;
	//			CLogMgr::SharedLogMgr()->SystemLog("高牌 实际概率：%.3f, 配置概率: %.3f",fRate,m_vAllConfig[eType].nRate) ;
	//		}
	//		break;
	//	default:
	//		{
	//			CLogMgr::SharedLogMgr()->ErrorLog("Unknown card type when rand slot machine !") ;
	//		}
	//	}
	//}
}