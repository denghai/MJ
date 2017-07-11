#include "MJPlayerCard.h"
#include "MJCard.h"
#include "XLMJPlayerCard.h"
#include "SZMJPlayerCard.h"
#include "CYMJPlayerCard.h"
#include "HHMJPlayerCard.h"
#include "JSMJPlayerCard.h"
#include "YZMJPlayerCard.h"
#include "BPMJPlayerCard.h"
#include "log4z.h"
void tempTest()
{
	//	// test new chard ;
	auto pPlayerCard = new BPMJPlayerCard();
	pPlayerCard->reset();
	//pPlayerCard->setBaoCard(CMJCard::makeCardNumber(eCT_Tong, 1));
	//pPlayerCard->setBanZiCard(CMJCard::makeCardNumber(eCT_Tiao, 9));
		
	pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 3));
	pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 4));
	pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 5));
	//pPlayerCard->onMingGang(CMJCard::makeCardNumber(eCT_Tiao, 1), CMJCard::makeCardNumber(eCT_Tong, 8));
	//pPlayerCard->onPeng(CMJCard::makeCardNumber(eCT_Jian, 1));

	pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 2));
	pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 3));
	//pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 4));
	//pPlayerCard->onPeng(CMJCard::makeCardNumber(eCT_Jian, 2));
	pPlayerCard->onEat(CMJCard::makeCardNumber(eCT_Wan, 4), CMJCard::makeCardNumber(eCT_Wan, 2), CMJCard::makeCardNumber(eCT_Wan, 3));

	pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Tiao, 1));
	pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Tiao, 2));
	//pPlayerCard->onPeng(CMJCard::makeCardNumber(eCT_Tong, 1));
	pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Tiao, 3));
	//pPlayerCard->onEat(CMJCard::makeCardNumber(eCT_Tiao, 4), CMJCard::makeCardNumber(eCT_Tiao, 2), CMJCard::makeCardNumber(eCT_Tiao, 3));

	pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Tong, 6));
	pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Tong, 7));
	//pPlayerCard->onPeng(CMJCard::makeCardNumber(eCT_Tong, 5));
	//pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Tong, 4));
	//pPlayerCard->onMingGang(CMJCard::makeCardNumber(eCT_Tiao, 1), CMJCard::makeCardNumber(eCT_Tiao, 3));

	//pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Tiao, 3));
	//pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Tong, 1));
	pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Jian, 3));
	pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Jian, 3));

	pPlayerCard->setIsHuiPai(true);
	pPlayerCard->setIsBiMenHu(true);
	pPlayerCard->setIs7Pair(false);
	pPlayerCard->setHuiCard(CMJCard::makeCardNumber(eCT_Jian, 1));
	//pPlayerCard->setBanZiCard(CMJCard::makeCardNumber(eCT_Tiao, 9));
	//pPlayerCard->setBaoCard(CMJCard::makeCardNumber(eCT_Tong, 1));

	//pPlayerCard->onMoCard(CMJCard::makeCardNumber(eCT_Feng, 2));

	//uint8_t nAA = pPlayerCard->checkJiaHu(CMJCard::makeCardNumber(eCT_Tiao, 7), false);

	//if (pPlayerCard->isHoldCardCanHu())
	if (pPlayerCard->canHuWitCard(CMJCard::makeCardNumber(eCT_Tong, 8)))
	{
		//std::vector<uint16_t> vHuType;
		//int32_t nFanCnt;
		//auto nRet = pPlayerCard->onDoHu(true, true, true, true, true, true, true, vHuType, nFanCnt);
		//for (auto nType : vHuType)
		//{
		//	printf("%d ", nType);
		//}
		printf("hu\n");
	}
}

#include "MJServer.h"
#include "Application.h"
int main()
{
	tempTest();
	CApplication theAplication(CMJServerApp::getInstance());
	theAplication.startApp();
	return 0;
}