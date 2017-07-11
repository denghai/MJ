#pragma once
#include <cstring>
//template<typename T >
class CAutoBuffer
{
public:
	CAutoBuffer( int nSize )
	{
		if ( nSize < 2 )
		{
			nSize = 2 ;
		}
		nAllSize = nSize ;
		pInnerBuffer = new char[nAllSize];
		memset(pInnerBuffer,0,nAllSize);
		nContentSize = 0 ;
	}

	~CAutoBuffer()
	{
		delete[] pInnerBuffer ;
		pInnerBuffer = nullptr ;
	}

	template<typename T >
	int addContent(T* pData,int nLen )
	{
		const char* ptr = (char*)pData ;
		return addContent(ptr,nLen);
	}

	int addContent(const char* pData, int nLen )
	{
		if ( nLen <= 0 )
		{
			return 0 ;
		}

		if ( nLen + nContentSize > nAllSize )
		{
			nAllSize = nLen + nContentSize;
			nAllSize *= 2 ;
			char* pB = new char[nAllSize];
			memcpy_s(pB,nAllSize,pInnerBuffer,nContentSize);
			delete[] pInnerBuffer ;
			pInnerBuffer = pB ;
		}
		memcpy_s(pInnerBuffer + nContentSize,nAllSize - nContentSize , pData,nLen);
		nContentSize += nLen ;
		return nContentSize ;
	}

	int getContentSize()
	{
		return nContentSize ;
	}

	void resetContentSize()
	{
		nContentSize = 0 ;
	}

	void clearBuffer()
	{
		if ( nAllSize > 0 )
		{
			memset(pInnerBuffer,0,nAllSize);
			nContentSize = 0 ;
		}
	}

	const char* getBufferPtr()
	{
		return pInnerBuffer ;
	}

private:
	int nAllSize ;
	int nContentSize ;
	char* pInnerBuffer ;
};