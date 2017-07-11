//
//  Utility.h
//  God
//
//  Created by Xu BILL on 12-11-23.
//
//

#ifndef God_Utility_h
#define God_Utility_h
#include <vector>
#include <string>
#include <stdio.h>
typedef std::vector<std::string> VEC_STRING ;
void StringSplit( const char* pString, char pSplitChar , VEC_STRING& vOutString);

std::string TimeToStringFormate( unsigned int nSec );
#endif
