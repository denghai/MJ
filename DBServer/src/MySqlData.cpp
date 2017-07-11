#include "MySqlData.h"
#include <algorithm>
unsigned int stMysqlField::IntValue()
{ 
	return strtol(pBuffer,(char**)NULL,10) ; 
}

unsigned __int64 stMysqlField::IntValue64()
{ 
	return _atoi64(pBuffer); 
}

float stMysqlField::FloatValue()
{
	return atof(pBuffer);  
}

double stMysqlField::DoubleValue()
{ 
	return atof(pBuffer);
}

const char* stMysqlField::BufferData()
{
	return pBuffer ; 
}

void stMysqlField::VecString( VEC_STRING& vOutString ,char cSplit)
{
	StringSplit(CStringValue(), cSplit, vOutString );
}

void stMysqlField::VecInt( std::vector<int>& vOutInt,char cSplit  )
{
	VEC_STRING vOutString ;
	VecString(vOutString ,cSplit);
	VEC_STRING::iterator iter = vOutString.begin() ;
	for ( ; iter != vOutString.end(); ++iter )
	{
		vOutInt.push_back(atoi(iter->c_str()));
	}
}

std::string stMysqlField::Int64ArraryToString( int64_t vInt[] , uint16_t nCnt )
{
	std::string strResult = "" ;
	char pBuffer[100] = { 0 } ;
	for ( uint16_t nIdx = 0 ; nIdx < nCnt ; ++nIdx )
	{
		memset(pBuffer,0,sizeof(pBuffer)) ;
		if ( nIdx == 0 )
		{
			sprintf_s(pBuffer,sizeof(pBuffer),"%I64d",vInt[nIdx] ) ;
		}
		else
		{
			sprintf_s(pBuffer,sizeof(pBuffer),"%c%I64d",FILIED_SPLIT,vInt[nIdx] ) ;
		}
		strResult.append(pBuffer) ;
	}
	return strResult ;
}

void stMysqlField::VecFloat( std::vector<float>& vOutFloat,char cSplit ) 
{
	VEC_STRING vOutString ;
	VecString(vOutString ,cSplit);
	VEC_STRING::iterator iter = vOutString.begin() ;
	for ( ; iter != vOutString.end(); ++iter )
	{
		vOutFloat.push_back((float)atof(iter->c_str()));
	}
}

stMysqlField::~stMysqlField()
{
	delete[] pBuffer;
}

// row 
CMysqlRow::CMysqlRow()
{
	ClearAllFiled();
}

CMysqlRow::~CMysqlRow()
{
	ClearAllFiled() ;
}

void CMysqlRow::PushFiled(stMysqlField* pFiled )
{
	if ( pFiled )
		m_vField.push_back(pFiled);
}

stMysqlField* CMysqlRow::GetFiledByName( const char* pFiledName )
{
	if ( !pFiledName )
	{
		printf("cell name is null \n") ;
		return NULL ;
	}
		
	LIST_FIELD::iterator iter = m_vField.begin() ;
	for ( ; iter != m_vField.end() ; ++iter )
	{
		stMysqlField* pField = *iter ;
		if ( pField == NULL )
			continue ;
		if ( strcmp(pFiledName,pField->strFieldName.c_str()) == 0 )
		{
			return pField ;
		}
	}
	printf("cell is null for name = %u \n",pFiledName);
	return NULL ;
}

void CMysqlRow::toJsValue(Json::Value& jsValue )
{
	std::find_if(m_vField.begin(),m_vField.end(),[&jsValue]( stMysqlField* pFiled )->bool
	{
		if ( pFiled == nullptr )
		{
			printf("why have an empty filed ?") ;
			return false ;
		}

		switch (pFiled->nValueType )
		{
		case eValue_Float:
			{
				jsValue[pFiled->strFieldName] = pFiled->FloatValue() ;
			}
			break;
		case eValue_Int:
			{
				jsValue[pFiled->strFieldName] = (int32_t)pFiled->IntValue() ;
			}
			break;
		case eValue_String:
			{
				jsValue[pFiled->strFieldName] = pFiled->CStringValue() ;
			}
			break;
		default:
			printf("for fied = %s , unknown type",pFiled->strFieldName.c_str()) ;
			return false;
		}
		return false ;
	});
}

stMysqlField* CMysqlRow::operator[](const char* pFieldname )
{
	return GetFiledByName(pFieldname) ;
}

void CMysqlRow::ClearAllFiled()
{
	LIST_FIELD::iterator iter = m_vField.begin() ;
	for ( ; iter != m_vField.end() ; ++iter )
	{
		delete *iter ;
		*iter = NULL ;
	}
	m_vField.clear() ;
}