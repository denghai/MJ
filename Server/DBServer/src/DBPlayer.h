//#pragma once
//#include <string>
//#include "BaseData.h"
//#include "RakNetTypes.h"
//#include "DBRequest.h"
//#include "ServerMessageDefine.h"
//struct stDBBaseData
//:public stBaseData
//{
//public:
//	bool m_bInitData ;
//	stDBBaseData()  // give init data here ; new reigster player ;
//	{
//		m_bInitData = true ;
//		nCoin = 1000 ;
//		nDiamoned = 1 ;
//	}
//};
//
//class CDBPlayer
//{
//public:
//	enum ePlayerState
//	{
//		ePlayerState_None ,
//		ePlayerState_Active,
//		ePlayerState_ReadingData,
//		ePlayerState_Resever,
//		ePlayerState_Max,
//	};
//public:
//	CDBPlayer(RakNet::RakNetGUID& nFromGameServerGUID );
//	~CDBPlayer();
//	void SetFromServerGUID(RakNet::RakNetGUID& nFromGameServerGUID);
//	RakNet::RakNetGUID& GetFromGameServerGUID();
//	void OnDBResult(stDBResult* pResult );
//	void OnMessage(stMsg* pMsg );
//	void OnDisconnected();
//	void OnConnected();
//	unsigned int GetUserUID();
//	unsigned int GetTempUID(){return m_nTempUID ;}
//	void OnPassAcountCheck( unsigned int nUserUID, unsigned int nTempUID, const char* pname );
//	ePlayerState GetState(){ return m_eState ; }
//protected:
//	void SaveAllToDB();
//	void ReadAllFromDB();
//	void SendBaseInfo();
//	void SetState( ePlayerState eState ){ m_eState = eState ;}
//protected:
//	stDBBaseData m_stBaseData ;
//	unsigned int m_nTempUID,m_nUserUID;
//	RakNet::RakNetGUID m_nFromGUID ;
//	ePlayerState m_eState ;
//};