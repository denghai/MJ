#pragma once 
#include "NativeTypes.h"
#include <string>
#include <list>
#include "utility.h"
#include "json/json.h"
#define FILIED_SPLIT ','
enum eValueType
{
	// warning : when add a type that need to New Buffer , we should add it to stFiled::~stFiled ; because the buffer will delect there ;
	eValue_Float,
	eValue_Int,
	eValue_Double,
	eValue_longLong,
	eValue_String,
	eValue_Binary,
	eValue_Char,
	eValue_Short,
	eValue_Max,
};
struct stMysqlField
{
	std::string strFieldName ;
	char* pBuffer ;
	int nBufferLen ;
	eValueType nValueType ;
public:
	template<class varType >
	static std::string UnIntArraryToString( varType vInt[] , uint16_t nCnt )
	{
		std::string strResult = "" ;
		char pBuffer[100] = { 0 } ;
		for ( uint16_t nIdx = 0 ; nIdx < nCnt ; ++nIdx )
		{
			memset(pBuffer,0,sizeof(pBuffer)) ;
			if ( nIdx == 0 )
			{
				sprintf_s(pBuffer,sizeof(pBuffer),"%u",vInt[nIdx] ) ;
			}
			else
			{
				sprintf_s(pBuffer,sizeof(pBuffer),"%c%u",FILIED_SPLIT,vInt[nIdx] ) ;
			}
			strResult.append(pBuffer) ;
		}
		return strResult ;
	}

	template<class varType >
	static std::string IntArraryToString( varType vInt[] , uint16_t nCnt )
	{
		std::string strResult = "" ;
		char pBuffer[100] = { 0 } ;
		for ( uint16_t nIdx = 0 ; nIdx < nCnt ; ++nIdx )
		{
			memset(pBuffer,0,sizeof(pBuffer)) ;
			if ( nIdx == 0 )
			{
				sprintf_s(pBuffer,sizeof(pBuffer),"%d",vInt[nIdx] ) ;
			}
			else
			{
				sprintf_s(pBuffer,sizeof(pBuffer),"%c%d",FILIED_SPLIT,vInt[nIdx] ) ;
			}
			strResult.append(pBuffer) ;
		}
		return strResult ;
	}
	static std::string Int64ArraryToString( int64_t vInt[] , uint16_t nCnt );
public:
	stMysqlField(const char* pName,int nLen):strFieldName(pName,nLen){ pBuffer = NULL ;}
	unsigned int IntValue();
	unsigned __int64 IntValue64();
	float FloatValue();
	double DoubleValue();
	const char* BufferData();
	const char* CStringValue(){ return BufferData() ;}
	void VecString( VEC_STRING& vOutString ,char cSplit = FILIED_SPLIT );
	void VecInt( std::vector<int>& vOutInt,char cSplit = FILIED_SPLIT );
	void VecFloat( std::vector<float>& vOutFloat,char cSplit  = FILIED_SPLIT  ) ;
	~stMysqlField();
};

class CMysqlRow
{
public:
	typedef std::list<stMysqlField*> LIST_FIELD ;
public:
	CMysqlRow();
	~CMysqlRow();
	void PushFiled(stMysqlField* pFiled );
	stMysqlField* GetFiledByName( const char* pFiledName );
	stMysqlField* operator [](const char* pFieldname );
	int GetFieldCount(){return m_vField.size() ;}
	void toJsValue(Json::Value& jsValue );
protected:
	void ClearAllFiled();
protected:
	LIST_FIELD m_vField ;
};
