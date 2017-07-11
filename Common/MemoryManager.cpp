#include "MemoryManager.h"
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cstdio>
#define MAX_HOLD_MEMORY 1024*1024*10
size_t CMemory::s_nMiniChunckSize = sizeof(stMemChunck);
CMemory::CMemory()
{
	m_pMemoryBuffer = nullptr;
	m_nMemoryBufferSize = 0 ;
	m_pHeadChunck = nullptr;
}

CMemory::~CMemory()
{
	free(m_pMemoryBuffer);
	m_pMemoryBuffer = nullptr;
	m_nMemoryBufferSize = 0 ;
	m_pHeadChunck = nullptr;
}

bool CMemory::init( size_t nDefMemSize )
{
	m_pMemoryBuffer = nullptr;
	m_nMemoryBufferSize = 0 ;
	m_pHeadChunck = nullptr;

	if ( nDefMemSize < s_nMiniChunckSize )
	{
		return false ;
	}

	m_pMemoryBuffer = malloc(nDefMemSize);
	if ( m_pMemoryBuffer == nullptr )
	{
		return false ;
	}

	memset(m_pMemoryBuffer,0,nDefMemSize);
	m_nMemoryBufferSize = nDefMemSize ;
	m_pHeadChunck = (stMemChunck*)m_pMemoryBuffer ;
	m_pHeadChunck->nLen = nDefMemSize ;
	m_pHeadChunck->pLeft = nullptr ;
	m_pHeadChunck->pRight = nullptr ;

	m_nChunckArraySize = (nDefMemSize + s_nMiniChunckSize - 1 ) / s_nMiniChunckSize ;
	m_nCurChunckCnt = 0 ;
	m_vAllChuncks = (stMemChunckPtr*)malloc(sizeof(stMemChunckPtr)*m_nChunckArraySize);
	memset(m_vAllChuncks,0,sizeof(stMemChunckPtr)*m_nChunckArraySize);
    addChunck(m_pHeadChunck,0,0);
	return true ;
}

void* CMemory::allocatMemory(size_t nSize )
{
	nSize += chunk_len_size;  // 最终要的内存数量，要比调用者需要的多chunk_len_size ，因为要用来存储 返回了多少内存，回收时要用到
	if ( nSize < s_nMiniChunckSize ) // 返回的内存最少要可以存储一个chunk的信息，不然回收的时候，没法组成一个chunck 
	{
		nSize = s_nMiniChunckSize ;
	}

	if ( nSize > m_vAllChuncks[m_nCurChunckCnt-1]->nLen )
	{
		return nullptr ;
	}

	stMemChunck* pFinderChunk = nullptr ;
	size_t idx = getChunckIdxJustBiggerThanSize(nSize,0,m_nCurChunckCnt-1);
	//size_t idx = m_nCurChunckCnt - 1 ; //
	if ( idx == (size_t)-1 )
	{
		return nullptr ;
	}
	else
	{
		for ( size_t nIdx = idx ; nIdx < m_nCurChunckCnt ; ++nIdx )
		{
			if ( m_vAllChuncks[nIdx]->nLen >= nSize + s_nMiniChunckSize || m_vAllChuncks[nIdx]->nLen == nSize )
			{
				pFinderChunk = m_vAllChuncks[nIdx];
				idx = nIdx ;
				break;
			}
		}
	}

	if ( pFinderChunk == nullptr )
	{
		return nullptr ;
	}

	return allocatFromChunck(pFinderChunk,nSize,idx);
}

bool CMemory::dellocatMemory(void* pDataDel )
{
	// 把指针向前偏移chunk_len_size，从而获得实际回收的size。结合我们allocat 内存是的操作可以知道
	void* pData = (void*)((char*)pDataDel - chunk_len_size );
	size_t nSize = *((chunkLen_t*)pData);

	if ( nSize < s_nMiniChunckSize )
	{
		nSize = s_nMiniChunckSize ;
	}

	if ( (int_address)pData < (int_address)m_pMemoryBuffer || (int_address)pData + nSize > (int_address)m_pMemoryBuffer + m_nMemoryBufferSize )
	{
		return false ;
	}

	
	//clock_t s = clock();clock_t e ;double dw ;
	// 按照地址从小到大的顺序，插入链表
	stMemChunck* pFindChun = nullptr ;
	stMemChunck* pIterChunk = m_pHeadChunck ;
	stMemChunck* pLastNode = m_pHeadChunck ;
	while ( pIterChunk )
	{
		int_address nAddr = (int_address)pIterChunk ;
		if ( nAddr > (int_address)pData )
		{
			pFindChun = pIterChunk ;
			break;
		}
		pIterChunk = pIterChunk->pRight ;
		if ( pIterChunk )
		{
			pLastNode = pIterChunk ;
		}
	}
	
	memset(pData,0,nSize) ;

	//e = clock();
	//printf( "1  time = %d\n",e-s ) ;
	//e = clock();
	//s = e ;

	stMemChunck* pNewChunk = (stMemChunck*)pData ;
	pNewChunk->nLen = nSize ;
	if ( pFindChun == nullptr )
	{
		pLastNode->pRight = pNewChunk ;
		pNewChunk->pLeft = pLastNode ;
		pNewChunk->pRight = nullptr ;
	}
	else
	{
		pNewChunk->pLeft = pFindChun->pLeft ;
		pNewChunk->pRight = pFindChun ;
		pFindChun->pLeft = pNewChunk ;

		if ( pNewChunk->pLeft == nullptr )
		{
			m_pHeadChunck = pNewChunk ;
		}
	}
	
	//e = clock();
	//printf( "2  time = %d\n",e-s ) ;
	//e = clock();
	//s = e ;

	size_t nNewIdx = addChunck(pNewChunk,0,m_nCurChunckCnt - 1 );
	// 分别向前和向后 检查是否可以合并内存碎片
	stMemChunck* pCheckMerge = pNewChunk ;
	stMemChunck* pMergeLeft = pCheckMerge->pLeft ;
	stMemChunck* pMergeRight = pCheckMerge->pRight ;
	assert( pMergeLeft ==nullptr || ("memory maybe cross border" && (int_address)pCheckMerge >= (int_address)pMergeLeft + pMergeLeft->nLen));
	assert( pMergeRight == nullptr || ("memory maybe cross border" && (int_address)pMergeRight >= (int_address)pCheckMerge + pCheckMerge->nLen) );

	if ( pMergeLeft )
	{
		if ( (int_address)pCheckMerge == (int_address)pMergeLeft + pMergeLeft->nLen )
		{
			nNewIdx = removeChunck(pCheckMerge,nNewIdx,nNewIdx );
			removeChunck(pMergeLeft,0,nNewIdx );
			pMergeLeft->pRight = pMergeRight ;
			pMergeLeft->nLen += pCheckMerge->nLen ;
			pMergeRight->pLeft = pMergeLeft ;

			pCheckMerge = pMergeLeft ;
			pMergeLeft = pCheckMerge->pLeft ;
			pMergeRight = pCheckMerge->pRight ;

			if ( nNewIdx > 1 )
			{
				nNewIdx -= 1 ;
			}
			else
			{
				nNewIdx = 0 ;
			}
			nNewIdx = addChunck(pCheckMerge,nNewIdx,m_nCurChunckCnt - 1 );
		}
	}
 
	assert( pMergeRight == nullptr || ("memory maybe cross border" && (int_address)pMergeRight >= (int_address)pCheckMerge + pCheckMerge->nLen) );
	if ( pMergeRight )
	{
		if ( (int_address)pCheckMerge + pCheckMerge->nLen == (int_address)pMergeRight )
		{
			nNewIdx = removeChunck(pCheckMerge,nNewIdx,nNewIdx );
			nNewIdx = removeChunck(pMergeRight,nNewIdx,m_nCurChunckCnt - 1 );

			pCheckMerge->pRight = pMergeRight->pRight ;
			pCheckMerge->nLen += pMergeRight->nLen ;
			if ( pMergeRight->pRight )
			{
				pMergeRight->pRight->pLeft = pCheckMerge ;
			}

			nNewIdx = addChunck(pCheckMerge,nNewIdx,m_nCurChunckCnt - 1 );
		}
	}

	//e = clock();
	//printf( "3  time = %d\n",e-s ) ;
	//e = clock();
	//s = e ;
	return true ;
}

void* CMemory::allocatFromChunck(stMemChunck* pChucnk,size_t nSize, size_t nChunckIdx )
{
	if ( pChucnk == nullptr )
	{
		return nullptr ;
	}
	removeChunck(pChucnk,nChunckIdx,nChunckIdx);
	stMemChunck* pLeft = pChucnk->pLeft ;
	stMemChunck* pRight = pChucnk->pRight ;
	void* pOutMem = nullptr ;

	if ( pChucnk->nLen - nSize >= s_nMiniChunckSize )
	{
		char* pChunkMem = (char*)pChucnk ;
		stMemChunck* pNewChunck = (stMemChunck*)(pChunkMem + nSize);
		pNewChunck->pLeft = pLeft ;
		pNewChunck->pRight = pRight ;
		pNewChunck->nLen = pChucnk->nLen - nSize ;
		if ( pLeft )
		{
			pLeft->pRight = pNewChunck ;
		}
		
		if ( pRight )
		{
			pRight->pLeft = pNewChunck ;
		}

		if ( pLeft == nullptr )
		{
			m_pHeadChunck = pNewChunck ;
		}

		pOutMem = (void*)pChunkMem ;
		addChunck(pNewChunck,0,nChunckIdx);
	}
	else if ( nSize == pChucnk->nLen )
	{
		if ( pRight )
		{
			pRight->pLeft = pLeft ;
		}

		if ( pLeft )
		{
			pLeft->pRight = pRight ;
		}
		else
		{
			m_pHeadChunck = pRight ;
		}

		pOutMem = (void*)pChucnk ;
	}
	else
	{
		assert(0&&"size error alloc from chunck");
	}

	memset(pOutMem,0,nSize);
	// 把实际返回的内存存储在返回内存的前面，回收的时候，方便获取；
	chunkLen_t* plen = (chunkLen_t*)pOutMem ;
	*plen = nSize ;
	return ((char*)pOutMem + chunk_len_size);
}

 size_t CMemory::getChunckIdxJustBiggerThanSize( size_t nSize,size_t beginIdx, size_t endIdx )
{
	if ( m_vAllChuncks[endIdx] == nullptr || m_vAllChuncks[endIdx]->nLen < nSize )
		return -1 ;
	if ( m_vAllChuncks[beginIdx]->nLen >= nSize )
	{
		return beginIdx ;
	}

#define SPLIT_CNT 30
	if ( endIdx - beginIdx <= SPLIT_CNT )
	{
		for ( size_t idx = beginIdx ; idx <= endIdx ; ++idx )
		{
			if (  m_vAllChuncks[idx]->nLen >= nSize )
			{
				return idx ;
			}
		}
		return -1 ;
	}

	size_t nStepIdx = ( endIdx - beginIdx ) / SPLIT_CNT ;
	size_t nS = beginIdx ;
	size_t rS = endIdx ;
	size_t nFlag = 0 ;
	for ( ; nFlag < 2 ; )
	{

		if ( m_vAllChuncks[nS]->nLen >= nSize )
		{
			return getChunckIdxJustBiggerThanSize(nSize,nS - nStepIdx,nS) ;
		}

		if ( m_vAllChuncks[rS]->nLen <= nSize )
		{


			return getChunckIdxJustBiggerThanSize(nSize,rS,rS + nStepIdx) ;
		}

		if ( endIdx == nS )
		{
			++nFlag ;
		}

		if ( rS == beginIdx )
		{
			++nFlag ;
		}

		nS += nStepIdx  ;
		if ( nS > endIdx )
		{
			nS = endIdx ;
		}

		if ( rS < nStepIdx )
		{
			rS = beginIdx ;
		}
		else
		{
			rS -= nStepIdx ;
		}
	}
	return -1 ;
	///-----new ;
	//size_t idx = beginIdx ;
	//size_t nStepIdx = ( endIdx - beginIdx ) / SPLIT_CNT ;
	//if ( nStepIdx < 1 )
	//{
	//	nStepIdx = 1 ;
	//}
	//while ( idx != endIdx || nStepIdx != 1 )
	//{
	//	if ( m_vAllChuncks[idx]->nLen >= nSize )
	//	{
	//		if ( nStepIdx == 1 )
	//		{
	//			return idx ;
	//		}
	//		else
	//		{
	//			endIdx = idx ;
	//			idx = idx - nStepIdx ;

	//			nStepIdx *= 0.5; //( endIdx - idx ) / SPLIT_CNT ;
	//			if (endIdx - idx <= SPLIT_CNT )
	//			{
	//				nStepIdx = 1 ;
	//			}
	//		}
	//	}
	//	else
	//	{
	//		idx += nStepIdx ;
	//		if ( idx > endIdx )
	//		{
	//			idx = endIdx ;
	//		}
	//	}
	//}

	return -1 ;
}

 size_t CMemory::addChunck(stMemChunckPtr pChucnk,size_t beginIdx, size_t endIdx)
 {
	  //clock_t s = clock(); clock_t e; float dw ;
	 if ( m_nCurChunckCnt == 0 )
	 {
		 m_vAllChuncks[m_nCurChunckCnt++] = pChucnk ;
		 return 0;
	 }

	 size_t idx = getChunckIdxJustBiggerThanSize(pChucnk->nLen,beginIdx,endIdx);
	 if ( idx == (size_t)-1 )
	 {
		m_vAllChuncks[m_nCurChunckCnt++] = pChucnk ;
		return m_nCurChunckCnt - 1 ;
	 }

	 memcpy(&m_vAllChuncks[idx+1],&m_vAllChuncks[idx],sizeof(stMemChunckPtr) * ( m_nCurChunckCnt - idx) );
	 m_vAllChuncks[idx] = pChucnk ;
	 ++m_nCurChunckCnt ;
	 return idx ;
 }

 size_t CMemory::removeChunck(stMemChunckPtr pChucnk,size_t beginIdx, size_t endIdx)
 {
	 size_t idx = getChunckIdxJustBiggerThanSize(pChucnk->nLen,beginIdx,endIdx);
	 size_t iOutIdx = idx ;
	 assert("must can find"&&idx != (size_t)-1) ;
	 if ( idx != m_nCurChunckCnt - 1 )
	 {
		 memcpy(&m_vAllChuncks[idx],&m_vAllChuncks[idx+1],m_nCurChunckCnt - 1 - idx );
	 }
	 else
	 {
		 if ( iOutIdx > 0 )
		 --iOutIdx;
	 }
	 m_vAllChuncks[m_nCurChunckCnt - 1] = nullptr;
	 --m_nCurChunckCnt ;
	 return iOutIdx ;
 }

void CMemory::debugLeftMemory( size_t nIdx )
{
	unsigned int nChunkCnt = 0 ;
	unsigned int nMemoryLen = 0 ;
	stMemChunck* pIter = m_pHeadChunck ;
	while (pIter)
	{
		++nChunkCnt ;
		nMemoryLen += pIter->nLen ;
		pIter = pIter->pRight ;
	}
	printf("Number %d memory info : used size = %d, left size = %d, chunkCnt = %d\n",nIdx,m_nMemoryBufferSize - nMemoryLen ,nMemoryLen,nChunkCnt);
}

CMemoryMgr* CMemoryMgr::getInstance()
{
	static CMemoryMgr s_memMgr ;
	return &s_memMgr ;
}

CMemoryMgr::CMemoryMgr()
{
	memset(m_vMemoryList,0,sizeof(m_vMemoryList));
	init();
}

CMemoryMgr::~CMemoryMgr()
{
	for each (CMemory* pmem in m_vMemoryList )
	{
		if ( pmem==0 )
		{
			break ;
		}

		pmem->~CMemory();
		free(pmem);
	}

	memset(m_vMemoryList,0,sizeof(m_vMemoryList));
}

bool CMemoryMgr::init()
{
	m_nMemoryListUsedCnt = 0 ;
	size_t nCnt = 2 ;
	while (nCnt--)
	{
		CMemory* tMemory = (CMemory*)malloc(sizeof(CMemory)) ;
		tMemory->init(MAX_HOLD_MEMORY) ;
		m_vMemoryList[m_nMemoryListUsedCnt++] = tMemory;
	}
	return true ;
}

void* CMemoryMgr::allocateMemory(size_t nSize )
{
	if ( nSize > MAX_HOLD_MEMORY )
	{
		void* p = malloc(nSize) ;
		printf("request memory too big ,so i can not offer");
		return p ;
	}


	for each( CMemory* pMemory in m_vMemoryList )
	{
		if ( pMemory == nullptr )
		{
			break;
		}

		void* pMem = pMemory->allocatMemory(nSize) ;
		if ( pMem != nullptr )
		{
			return pMem;
		}
	}

	CMemory* tMemory = (CMemory*)malloc(sizeof(CMemory)) ;
	tMemory->init(MAX_HOLD_MEMORY) ;
	m_vMemoryList[m_nMemoryListUsedCnt++] = tMemory;
	void* pMem = tMemory->allocatMemory(nSize) ;
	return pMem ;
}

bool CMemoryMgr::dellocateMemory( void* pData )
{
	for each( CMemory* pMem in m_vMemoryList )
	{
		if ( pMem == nullptr )
		{
			break ;
		}

		if ( pMem->dellocatMemory(pData) )
		{
			return true ;
		}
	}

	assert(0&&"this memory may already freed or not allocate by mgr, you can clear this assert");
	free(pData);
	return false ;
}

void CMemoryMgr::debugInfo()
{
	size_t idx = 0 ;
	for each( CMemory* pMem in m_vMemoryList )
	{
		if ( pMem == nullptr )
		{
			break ;
		}

		pMem->debugLeftMemory(idx++);
	}
	return  ;
}

//
//void * operator new(size_t size)
//{
//	void *p = CMemoryMgr::getInstance()->allocateMemory(size);
//	return (p);
//}
//
//void operator delete(void *p)
//{
//	CMemoryMgr::getInstance()->dellocateMemory(p);
//}
//
//void * operator new[](size_t size)
//{
//	void *p = CMemoryMgr::getInstance()->allocateMemory(size);
//	return (p);
//}
//
//void operator delete[](void *p)
//{
//	CMemoryMgr::getInstance()->dellocateMemory(p) ;
//}


