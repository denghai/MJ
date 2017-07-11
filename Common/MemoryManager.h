#pragma once
#include <list>
class CMemory
{
public:
#define chunkLen_t size_t  
	typedef struct stMemChunck
	{
		stMemChunck* pLeft ;
		stMemChunck* pRight ;
		chunkLen_t nLen;
	}* stMemChunckPtr;
#define int_address unsigned int 
#ifdef _WIN64
#define int_address unsigned long long
#endif // _WIN64

#define chunk_len_size sizeof(chunkLen_t)
public:
	CMemory();
	~CMemory();
	bool init( size_t nDefMemSize );
	void* allocatMemory(size_t nSize );
	bool dellocatMemory(void* pData);
	void debugLeftMemory(size_t nIdx );
protected:
	void* allocatFromChunck(stMemChunck* pChucnk,size_t nSize, size_t nChunckIdx );
	size_t getChunckIdxJustBiggerThanSize(size_t nSize,size_t beginIdx, size_t endIdx ); // idx are include [begin ,end]
	size_t addChunck(stMemChunckPtr pChucnk,size_t beginIdx, size_t endIdx);
	size_t removeChunck(stMemChunckPtr pChucnk,size_t beginIdx, size_t endIdx);
protected:
	static size_t s_nMiniChunckSize ;
	void* m_pMemoryBuffer ;
	size_t m_nMemoryBufferSize ;
	stMemChunckPtr m_pHeadChunck;

	stMemChunckPtr*m_vAllChuncks ;
	size_t m_nCurChunckCnt ;
	size_t m_nChunckArraySize ;
};

class CMemoryMgr
{
public:
#define MAX_MEMORY_LIST 300
private:
	CMemoryMgr();
public:
	static CMemoryMgr* getInstance();
	~CMemoryMgr();
	bool init();
	void* allocateMemory(size_t nSize );
	bool dellocateMemory(void* pData);
	void debugInfo();
protected:
	CMemory* m_vMemoryList[MAX_MEMORY_LIST] ;
	size_t m_nMemoryListUsedCnt ;
};