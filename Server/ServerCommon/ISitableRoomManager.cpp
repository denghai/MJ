#include "ISitableRoomManager.h"
#include "ISitableRoom.h"

IRoom* ISitableRoomManager::getRoomByConfigID(uint32_t nRoomConfigID )
{
	MAP_CONFIG_ROOMS::iterator iter = m_vCongfigIDRooms.find(nRoomConfigID) ;
	if ( iter == m_vCongfigIDRooms.end() )
	{
		return nullptr ;
	}

	LIST_ROOM& vRooms = iter->second ;
	if ( vRooms.empty() )
	{
		return nullptr ;
	}

	LIST_ROOM vAcitveRooms ;
	LIST_ROOM vEmptyRooms ;
	for ( IRoom* pRoom : vRooms )
	{
		if ( pRoom == nullptr || pRoom->isRoomAlive() == false )
		{
			continue; 
		}

		if ( ((ISitableRoom*)pRoom)->getPlayerCntWithState(eRoomPeer_SitDown) )
		{
			vAcitveRooms.push_back(pRoom) ;
		}
		else
		{
			vEmptyRooms.push_back(pRoom) ;
		}
	}

	if ( vAcitveRooms.empty() && vEmptyRooms.empty() )
	{
		return nullptr ;
	}

	if ( vAcitveRooms.empty() )  // if all room is empty , then just rand a room to enter ;
	{
		vAcitveRooms.insert(vAcitveRooms.begin(),vEmptyRooms.begin(),vEmptyRooms.end()) ;
	}
	else if ( vAcitveRooms.size() <= 10 )  // put some empty rooms in 
	{
		uint8_t naddEmtpy = 0 ;
		for ( IRoom* pRoom : vEmptyRooms )
		{
			if ( naddEmtpy > 8 )
			{
				break; ;
			}

			if ( naddEmtpy % 2 == 0 )
			{
				vAcitveRooms.push_back(pRoom) ;
			}
			else
			{
				vAcitveRooms.insert(vAcitveRooms.begin(),pRoom) ;
			}

			++naddEmtpy ;
		}
	}

	uint16_t nStartIdx = rand() % vAcitveRooms.size() ;
	uint16_t iter_idx = 0 ;
	for( IRoom* pRoom : vAcitveRooms )
	{
		if ( iter_idx != nStartIdx )
		{
			++iter_idx ;
			continue;
		}

		return pRoom ;

	}
	return nullptr ;
}