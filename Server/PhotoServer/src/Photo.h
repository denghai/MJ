#pragma once
#include <list>
#include "mutex.h"
struct stPhoto
{
	char* pBuffer ;
	unsigned int nBufferLen ;
	unsigned int nPhotoUID ;
};

 class CPhotoMgr
 {
 public:
	 typedef std::list<stPhoto*> LIST_PHOTOS ;
 public:
	 ~CPhotoMgr() ;
	 stPhoto* GetPhotoByPhotoID( unsigned int nPhotoID,bool bRead );
	 stPhoto* AddPhoto(unsigned int nPhotoID , char* pBuffer, unsigned int nBufferLen );
 protected:
	 static const char* cRootDir ;
 protected:
	 LIST_PHOTOS m_vAllPhotos ;

	 Mutex m_nMutex ;
 };