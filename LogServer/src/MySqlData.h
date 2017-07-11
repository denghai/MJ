#pragma once 
#include <string>
#include <list>
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
public:
	stMysqlField(const char* pName,int nLen):strFieldName(pName,nLen){ pBuffer = NULL ;}
	unsigned int IntValue();
	unsigned __int64 IntValue64();
	float FloatValue();
	double DoubleValue();
	const char* BufferData();
	const char* CStringValue(){ return BufferData() ;}
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
protected:
	void ClearAllFiled();
protected:
	LIST_FIELD m_vField ;
};
