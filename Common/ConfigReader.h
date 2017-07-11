//
//  ConfigReader.h
//  God
//
//  Created by Xu BILL on 12-10-12.
//
//
#pragma warning(disable:4819)
#ifndef God_ConfigReader_h
#define God_ConfigReader_h
#ifdef POKER_CLIENT
#include <string>
#else
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#endif
#include <map>
#include <list>
#include <vector>
#include "Utility.h"
#define BRACK_CHAR ','
class CReaderCell
{
public:
    CReaderCell( const char* strData );
    ~CReaderCell(){}
    int IntValue();
    float FloatValue();
    std::string StringValue();
    void VecString( VEC_STRING& vOutString ,char cSplit = BRACK_CHAR );
    void VecInt( std::vector<int>& vOutInt,char cSplit = BRACK_CHAR );
    void VecFloat( std::vector<float>& vOutFloat,char cSplit  = BRACK_CHAR  ) ;
private:
    std::string m_StrData;
};

class CReaderRow
{
public:
    typedef std::vector<CReaderCell*> VEC_CELL ;
public:
    CReaderRow( const char* pRowBuffer );
    ~CReaderRow();

    void PaseRow( const char* pRowBuffer);
    static void PaseRowForIndex( const char* pRowBuffer );

    CReaderCell* GetCellByTitle( const char* pTitle );
    CReaderCell* GetCellByIndex( short iIndex );
    CReaderCell* operator[]( const char* pTitle  )
    {
        return GetCellByTitle(pTitle) ;
    }

    static short GetIndexByTitle( const char* pTitle );
    static void PrintIndexContent();
protected:
    static std::map<std::string,short> m_vStrToIdx;
    VEC_CELL m_vAllCells;
};

class CConfigReader
{
public:
    typedef std::list<CReaderRow*> LIST_ROW ;
public:
    static CConfigReader* SharedConfigReader();
    CConfigReader();
    ~CConfigReader();
    
    bool LoadFile( const char* pFileName );
    void UnLoadFile();  // 解析完成一定要unload ;
    
    int GetRowCount();
    CReaderRow* GetRowByIndex( int iIndex );
    CReaderRow* EnumNextRow(); // 用于循环遍历［读取］所有的row warnning: 必须遍历完毕；
public:
    static int s_SkillRow ;
protected:
    LIST_ROW m_VAllRows ;
    LIST_ROW::iterator m_iterEnum ;
};

#endif
