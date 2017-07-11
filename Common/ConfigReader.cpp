//
//  ConfigReader.cpp
//  God
//
//  Created by Xu BILL on 12-10-12.
//
//
#pragma warning(disable:4996)
#pragma warning(disable:4819)
#include "ConfigReader.h"
#include <stdlib.h>
#define MAX_ROW_BUFFER 1024*2
#define MAX_CELL_BUFFER 512*2
#include "NativeTypes.h"
CReaderCell::CReaderCell( const char* strData )
{
     m_StrData =  strData;
}

int CReaderCell::IntValue()
{
    if ( this == NULL )
        return 0;
    return atoi(m_StrData.c_str()) ;
}

float CReaderCell::FloatValue()
{
    if ( this == NULL )
        return 0;
    return (float)atof(m_StrData.c_str()) ;
}

std::string CReaderCell::StringValue()
{
    if ( this == NULL )
        return 0;
    return m_StrData ;
}

void CReaderCell::VecString( VEC_STRING& vOutString ,char cSplit)
{
    StringSplit(m_StrData.c_str(), cSplit, vOutString );
}

void CReaderCell::VecInt( std::vector<int>& vOutInt,char cSplit)
{
    VEC_STRING vOutString ;
    VecString(vOutString ,cSplit);
    VEC_STRING::iterator iter = vOutString.begin() ;
    for ( ; iter != vOutString.end(); ++iter )
    {
        vOutInt.push_back(atoi(iter->c_str()));
    }
}

void CReaderCell::VecFloat( std::vector<float>& vOutFloat ,char cSplit )
{
    VEC_STRING vOutString ;
    VecString(vOutString ,cSplit);
    VEC_STRING::iterator iter = vOutString.begin() ;
    for ( ; iter != vOutString.end(); ++iter )
    {
        vOutFloat.push_back((float)atof(iter->c_str()));
    }
}

// reader row data ;
std::map<std::string,short> CReaderRow::m_vStrToIdx ;

CReaderRow::CReaderRow( const char* pRowBuffer)
{
    if ( m_vStrToIdx.size() == 0 )
    {
        PaseRowForIndex(pRowBuffer);
    }
    else
    {
        PaseRow(pRowBuffer);
    }
}

CReaderRow::~CReaderRow()
{
    VEC_CELL::iterator iter = m_vAllCells.begin() ;
    for ( ; iter != m_vAllCells.end() ; ++iter )
    {
        delete *iter ;
        *iter = NULL ;
    }
    m_vAllCells.clear() ;
}

void CReaderRow::PaseRow(const char *pRowBuffer )
{
    static int iTablen = 0 ;
    if ( iTablen == 0 )
    {
        char pTab [20] = { 0 };
        sprintf(pTab, "\t") ;
        iTablen = (uint32_t)strlen(pTab) ;
    }
    
    m_vAllCells.reserve(m_vStrToIdx.size()) ;
    
    char pDest[MAX_CELL_BUFFER] = { 0 } ;
    if ( pRowBuffer[0] == '\xef')
    {
        pRowBuffer += 3 ; // 忽略乱码；
    }
    while ( sscanf(pRowBuffer, "%[^\t|\n]" ,pDest ) != EOF )
    {
        pRowBuffer += (strlen(pDest) + iTablen );
        //printf("%s\n",pDest) ;
        
        CReaderCell * pCell = new CReaderCell( pDest );
        m_vAllCells.push_back(pCell) ;
        
        memset(pDest, 0, sizeof(pDest)) ;
    }
}

void CReaderRow::PaseRowForIndex(const char *pRowBuffer )
{
    m_vStrToIdx.clear() ;
    int iTablen = 0 ;
    char pTab [20] = { 0 };
    sprintf(pTab, "\t") ;
    iTablen = (uint32_t)strlen(pTab) ;
    
    char pDest[MAX_CELL_BUFFER] = { 0 } ;
    int iIndex = 0 ;
    if ( pRowBuffer[0] == '\xef')
    {
        pRowBuffer += 3 ; // 忽略乱码；
    }
    while ( sscanf(pRowBuffer, "%[^\t|\n]" ,pDest ) != EOF )
    {
        pRowBuffer += (strlen(pDest) + iTablen );
        m_vStrToIdx[pDest] = iIndex++ ;
        memset(pDest, 0, sizeof(pDest)) ;
    }
}

CReaderCell* CReaderRow::GetCellByTitle(const char *pTitle)
{
    short Index = GetIndexByTitle(pTitle) ;
    return GetCellByIndex(Index) ;
}

CReaderCell* CReaderRow::GetCellByIndex(short iIndex)
{
    if ( iIndex >= (short)m_vAllCells.size() || iIndex < 0 )
        return NULL ;
    return m_vAllCells[iIndex] ;
}

void CReaderRow::PrintIndexContent()
{
    std::map<std::string,short>::iterator iter = m_vStrToIdx.begin();
    char pbuffer[200] = {0} ;
    for ( ; iter != m_vStrToIdx.end() ; ++iter )
    {
        sprintf(pbuffer, "title: %s$  index = %d \n", iter->first.c_str(),iter->second ) ;
        //printf(pbuffer) ;
        memset(pbuffer, 0, sizeof(pbuffer)) ;
    }
}

short CReaderRow::GetIndexByTitle(const char *pTitle)
{
    std::map<std::string,short>::iterator iter = m_vStrToIdx.begin();
    for ( ; iter != m_vStrToIdx.end() ; ++iter )
    {
        const char* p =  iter->first.c_str() ;
        if ( strcmp(pTitle, p ) == 0 )
            return iter->second ;
    }
    return -1 ;
}

// configer reader ;
int CConfigReader::s_SkillRow = 0 ;
CConfigReader* CConfigReader::SharedConfigReader()
{
    static CConfigReader g_sReader ;
    return &g_sReader ;
}

CConfigReader::CConfigReader()
{
    m_VAllRows.clear() ;
    m_iterEnum = m_VAllRows.end() ;
}

CConfigReader::~CConfigReader()
{
    UnLoadFile();
}

bool CConfigReader::LoadFile(const char *pFileName)
{
    UnLoadFile();
    FILE* pFile = fopen(pFileName, "r");
    if ( pFile == NULL )
        return false ;
    char pbuffer[MAX_ROW_BUFFER] = {0} ;
    int iLine = 0 ;
//#ifdef POKER_CLIENT
//    while (fscanf(pFile, "%[^\r]\n",pbuffer) != EOF )
//#else
	while (fscanf(pFile, "%[^\n|\r]\n\r",pbuffer) != EOF )
//#endif
    {
        if ( iLine < s_SkillRow )
        {
            ++iLine ;
            continue ;
        }
        
        if ( iLine == s_SkillRow ) // title row ;
        {
            CReaderRow::PaseRowForIndex(pbuffer) ;
        }
        else
        {
            CReaderRow* pRow = new CReaderRow(pbuffer);
            m_VAllRows.push_back(pRow);
        }
        ++iLine ;
        memset(pbuffer, 0, sizeof(pbuffer)) ;
    }
    fclose(pFile) ;
    m_iterEnum = m_VAllRows.begin() ;
    return true ;
}

void CConfigReader::UnLoadFile()
{
    LIST_ROW::iterator iter = m_VAllRows.begin();
    for ( ; iter != m_VAllRows.end() ; ++iter )
    {
        delete *iter ;
        *iter = NULL ;
    }
    m_VAllRows.clear() ;
    m_iterEnum = m_VAllRows.end() ;
}

int CConfigReader::GetRowCount()
{
    return (uint32_t)m_VAllRows.size() ;
}

CReaderRow* CConfigReader::GetRowByIndex( int iIndex )
{
    if ( iIndex < 0 || iIndex >= GetRowCount() )
        return NULL ;
    int i = 0 ;
    CReaderRow* pRet = NULL ;
    while ( (pRet = EnumNextRow()))
    {
        if ( i == iIndex )
        {
            break ;
        }
        ++i ;
    }
    m_iterEnum = m_VAllRows.begin() ;
    return pRet ;
}

CReaderRow* CConfigReader::EnumNextRow()
{
    if ( m_iterEnum == m_VAllRows.end() )
    {
        m_iterEnum = m_VAllRows.begin() ;
        return NULL ;
    }
    CReaderRow* pRet = *m_iterEnum ;
    ++m_iterEnum ;
    return pRet ;
}



