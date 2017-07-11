//extern "C" 
//{
#include "Photo.h"
//};

#include <string.h>
#include <stdio.h>
const char* CPhotoMgr::cRootDir = "PlayerPhoto/";
CPhotoMgr::~CPhotoMgr()
{
	LIST_PHOTOS::iterator iter = m_vAllPhotos.begin() ;
	for ( ; iter != m_vAllPhotos.end(); ++iter )
	{
		delete[] ((*iter)->pBuffer) ;
		delete *iter ;
		*iter = NULL ;
	}
	m_vAllPhotos.clear() ;
}

stPhoto* CPhotoMgr::GetPhotoByPhotoID( unsigned int nPhotoID,bool bRead )
{
	m_nMutex.Lock();
	LIST_PHOTOS::iterator iter = m_vAllPhotos.begin() ;
	for ( ; iter != m_vAllPhotos.end(); ++iter )
	{
		if ( (*iter)->nPhotoUID == nPhotoID )
		{
			m_nMutex.Unlock();
			return *iter ;
		}
	}
	m_nMutex.Unlock();

	if ( !bRead )
	{
		return NULL;
	}
	//printf("first not find\n") ;
	static char pBuffer[100] = { 0 } ; 
	memset(pBuffer,0,sizeof(pBuffer)) ;
	sprintf(pBuffer,"%s%d.png",cRootDir,nPhotoID );
	//sprintf(pBuffer,"%d.png",nPhotoID );
	//printf("open file %s\n",pBuffer ) ;
	FILE* pFile = fopen(pBuffer,"rb") ;
	//printf("after open \n",pBuffer ) ;
	if ( !pFile )
	{
		//printf("after if comdition \n",pBuffer ) ;
		//printf("Second not find\n") ;
		return NULL ;
	}

	stPhoto* pPhoto = new stPhoto ;
	pPhoto->nPhotoUID = nPhotoID ;
	fseek(pFile,0,SEEK_END);
	pPhoto->nBufferLen = ftell(pFile);
	fseek(pFile,0,SEEK_SET) ;
	pPhoto->pBuffer = new char[pPhoto->nBufferLen] ;
	fread(pPhoto->pBuffer,pPhoto->nBufferLen,1,pFile);
	fclose(pFile);
	m_nMutex.Lock();
	//---------------mutex ;
	m_vAllPhotos.push_back(pPhoto) ;
	m_nMutex.Unlock();
	//------------mutex ;
	return pPhoto ;
}

stPhoto* CPhotoMgr::AddPhoto(unsigned int nPhotoID , char* pBufferPhoto, unsigned int nBufferLen )
{
	stPhoto* pPhoto = GetPhotoByPhotoID(nPhotoID,false) ;
	if ( pPhoto == NULL )
	{
		pPhoto = new stPhoto ;
		//---------------mutex ;
		m_nMutex.Lock();
		m_vAllPhotos.push_back(pPhoto) ;
		m_nMutex.Unlock();
		//------------mutex ;
	}
	else
	{
		delete[] (pPhoto->pBuffer) ;
		pPhoto->pBuffer = NULL ;
	}
	pPhoto->nPhotoUID = nPhotoID ;
	pPhoto->nBufferLen = nBufferLen ;
	pPhoto->pBuffer = new char[nBufferLen] ;
	memcpy(pPhoto->pBuffer,pBufferPhoto,nBufferLen) ;

	static char pBuffer[100] = { 0 } ; 
	memset(pBuffer,0,sizeof(pBuffer)) ;
	sprintf(pBuffer,"%s%d.png",cRootDir,nPhotoID );
	//printf("finaly name = %s\n",pBuffer);
	FILE* pFile = fopen(pBuffer,"wb") ;
	if ( !pFile )
	{
		return NULL ;
	}
	//printf("set new photo\n");

	fwrite(pBufferPhoto,nBufferLen,1,pFile) ;
	fclose(pFile);
	return pPhoto ;
}