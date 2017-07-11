//
//  Utility.cpp
//  God
//
//  Created by Xu BILL on 12-11-23.
//
//

#include "Utility.h"

void StringSplit( const char* pString, char pSplitChar , VEC_STRING& vOutString)
{
    std::string tTemp = "" ;
    while ( *pString )
    {
        if ( *pString == pSplitChar )
        {
            if ( tTemp.empty() == false )
            {
                vOutString.push_back(tTemp) ;
                tTemp = "" ;
            }
        }
        else if ( *pString != ' ' && *pString != '\t' )
        {
            tTemp.push_back(*pString) ;
        }
        ++pString ;
    }
    
    if ( !tTemp.empty())
    {
        vOutString.push_back(tTemp) ;
    }
}

std::string TimeToStringFormate( unsigned int nSec )
{
    char pBuffer[100] = { 0 } ;
#ifdef SERVER
    unsigned int nSecond = nSec % 60 ;
    unsigned int nMin = (nSec - nSecond ) / 60 ;
    unsigned int nMinite = nMin % 60 ;
    unsigned int nHou = ( nMin - nMinite ) / 60 ;
    unsigned int nHour = nHou % 24 ;
    unsigned int nDay = ( nHou - nHour ) / 24 ;
    if ( nDay > 0 )
    {
        sprintf_s(pBuffer, "%d天 : %d : %d : %d", nDay,nHour,nMinite,nSecond );
    }
    else
    {
        sprintf_s(pBuffer, "%d : %d : %d",nHour,nMinite,nSecond );
    }
#endif
    return pBuffer ;
}