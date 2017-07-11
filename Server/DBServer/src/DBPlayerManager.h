//#pragma once
//#include "ServerNetwork.h"
//#include <map>
//struct stDBResult;
//class CDBPlayer;
//struct stMsg ;
//class CDBPlayerManager
//:public CServerNetworkDelegate
//{
//public:
//	//enum ePlayersType
//	//{
//	//	ePlayerType_None,
//	//	ePlayerType_Active = ePlayerType_None,
//	//	ePlayerType_Reserve,
//	//	ePlayerType_WaitForCheck,
//	//	ePlayerType_Max,
//	//};
//	struct stAccountCheckAndRegister 
//	{
//		RakNet::RakNetGUID nFromServerID ;
//		unsigned int nTempUsrUID ;
//		std::string strAccount ;
//		std::string strPassword ;
//		std::string strCharacterName ;
//		bool bCheck ;
//		unsigned char nAccountType ;  // [used when register ] 0 visitor login register , 1 ordinary register ,2 SINA WEIBO login register  3 QQ login register .  
//	};
//
//	typedef std::map<unsigned int,CDBPlayer*> MAP_DBPLAYER ;
//	typedef std::map<unsigned int ,stAccountCheckAndRegister*> MAP_ACCOUNT_CHECK_REGISTER ;
//public:
//	CDBPlayerManager();
//	~CDBPlayerManager();
//
//	// network delegate ;
//	virtual bool OnMessage( RakNet::Packet* pData );
//	virtual void OnNewPeerConnected(RakNet::RakNetGUID& nNewPeer, RakNet::Packet* pData );
//	virtual void OnPeerDisconnected(RakNet::RakNetGUID& nPeerDisconnected, RakNet::Packet* pData );
//
//	void ProcessDBResults();
//
//	CDBPlayer* GetPlayer( unsigned int nTempUID);
//	CDBPlayer* CDBPlayerManager::GetPlayerByUserUID( unsigned int nUserUID );
//protected:
//	void ProcessTransferedMsg( stMsg* pMsg ,unsigned int nTargetUserUID , RakNet::RakNetGUID& nFromNetUID );
//	void OnProcessAccountCheckResult(stDBResult* pResult);
//	void OnProcessRegisterResult(stDBResult* pResult);
//	void OnProcessDBResult(stDBResult* pResult );
//	//void RemoveDBPlayer(LIST_DBPLAYER& vPlayers , CDBPlayer* pPlayer );
//	//void DeleteDBPlayer(LIST_DBPLAYER& vPlayers , CDBPlayer* pPlayer);
//	void ClearAllPlayers();
//	void ClearAccountCheck();
//public:
//	static char* s_gBuffer ;
//protected:
//	MAP_DBPLAYER m_vPlayers ;
//	MAP_ACCOUNT_CHECK_REGISTER m_vAccountChecks ;
//};