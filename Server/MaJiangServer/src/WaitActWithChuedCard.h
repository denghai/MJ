#pragma once
#include "IRoomState.h"
#include <memory>
#include <list>
#include <algorithm>
class CWaitActWithChuedCard
	:public IRoomState
{
public:
	struct stWaitPeerInfo
	{
		uint8_t nIdx ;
		std::list<eMJActType> vCanActList ;
		eMJActType nExpectedMaxAct ;
		eMJActType nChosedAct ;
		uint8_t nWithCardA ;  // used when eat act 
		uint8_t nWithCardB ;  // used when eat act 
		stWaitPeerInfo(){ nChosedAct = eMJAct_Max; vCanActList.clear() ;}
		bool isCanDoAct( eMJActType eTy )
		{
			auto iter = std::find(vCanActList.begin(),vCanActList.end(),eTy) ;
			return iter != vCanActList.end() ;
		}
	};
	typedef std::shared_ptr<stWaitPeerInfo> WAIT_PEER_INFO_PTR ;
public:
	void enterState(IRoom* pRoom,Json::Value& jsTransferData)override;
	void leaveState()override;
	void onStateDuringTimeUp()override;
	bool onMsg(Json::Value& prealMsg ,uint16_t nMsgType, eMsgPort eSenderPort , uint32_t nSessionID)override ;
	uint16_t getStateID(){ return eRoomState_WaitOtherPlayerAct ;}
protected:
	bool doExcutingAct();

	WAIT_PEER_INFO_PTR getReusePeerInfo();
	void recyclePeerInfoObject(WAIT_PEER_INFO_PTR& peerInfo );
	bool isWaitingEnd();
protected:
	bool m_isWaitingChoseAct ;  // is waiting player chose do act , or  executing m_ePlayerAct

	uint8_t m_nInvokeIdx ;
	uint8_t m_nTargetCard ;
	eMJActType m_eCardFrom ;

	uint8_t m_nMaxActedIdx ;
	eMJActType m_nCurMaxChosedActPriority ;
	std::list<WAIT_PEER_INFO_PTR> m_vWaitingObject ;
private:
	std::list<WAIT_PEER_INFO_PTR> m_vResueObject ;
};