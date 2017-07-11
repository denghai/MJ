#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include "../httpd/include/microhttpd.h"
#include "Photo.h"
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <pthread.h>
//#define USE_LOCK
#define PORT            8882
CPhotoMgr g_PhotoMgr ;
 char* ErrPage = "Error sdffffffffffffffffffffffffffffffffffffffffffffffffffffff" ;
 char* pOKPage = "Ok dfgdddddddddddddddddddddddddddddddddddddddddddddddddddd";
int ReplyClient(struct MHD_Connection *connection)
{
	struct MHD_Response * Repone = MHD_create_response_from_buffer (strlen(ErrPage),(void*)ErrPage,MHD_RESPMEM_PERSISTENT);
	MHD_add_response_header (Repone,"nResult","0");
	MHD_add_response_header (Repone,MHD_HTTP_HEADER_CONTENT_TYPE,"text/html");
	int nRet = MHD_queue_response(connection,MHD_HTTP_OK,Repone);
	MHD_destroy_response(Repone) ;
	return nRet ;
}

static int answer_to_connection (void *cls, struct MHD_Connection *connection,const char *url, const char *method,const char *version, const char *upload_data,size_t *upload_data_size, void **con_cls)
{
	if (0 == strcmp (method, "POST"))
	{
		//printf("upload_data_size = %d \n",*upload_data_size);
		//printf( "Received Len = %d\n",nLen ) ;

		const char* length = MHD_lookup_connection_value (connection, MHD_HEADER_KIND, MHD_HTTP_HEADER_CONTENT_LENGTH);
		if (length == NULL)
		{
			ReplyClient(connection) ;
			return MHD_YES ;
		}
		int nLen = atoi(length) ;

		if ( *upload_data_size  ==  nLen )
		{
			*upload_data_size = 0 ;
			const char* body = MHD_lookup_connection_value (connection, MHD_POSTDATA_KIND, NULL);
			if (body == NULL)
			{
				return ReplyClient(connection) ;
			}
			//printf("finished body\n");
			char* p = const_cast<char*>(url);
			unsigned int n =  atoi(p + 1) ;
			//printf("get id = %d\n",n) ;
			if (g_PhotoMgr.AddPhoto(n,const_cast<char*>(body),nLen))
			{
				//-----
				struct MHD_Response * Repone = MHD_create_response_from_buffer (strlen(pOKPage),(void*)pOKPage,MHD_RESPMEM_PERSISTENT);
				if (NULL == Repone )
				{
					printf("NULL Repone \n") ;
				}
				MHD_add_response_header(Repone,"nResult","1");
				MHD_add_response_header (Repone,MHD_HTTP_HEADER_CONTENT_TYPE,"text/html");
				int nRet = MHD_queue_response(connection,MHD_HTTP_OK,Repone);
				MHD_destroy_response(Repone) ;
				printf("send respone after post \n ");
				return nRet ;
			}
			else
			{
				printf("Add photo failed \n ");
				return ReplyClient(connection) ;
			}
			printf("send respone after post \n ");
		}
		else
		{
			return MHD_YES ;
		}

		//char pBufferZ[100] = {0} ;
		//static int iPro = 0 ;
		//++iPro ;
		//sprintf(pBufferZ,"a.png",iPro);
		//FILE* pFile = fopen( "a.png","wb");
		//if ( pFile == NULL )
		//{
		//	return MHD_NO ;
		//}
		//else
		//{
		//	fwrite(body,nLen,1,pFile) ;
		//	fclose(pFile);
		//	printf("Saved Success\n") ;
		//	return MHD_NO ;
		//}
		//return MHD_YES ;
	}
	else
	{
		stPhoto* pPhoto = g_PhotoMgr.GetPhotoByPhotoID(atoi(url+1),true) ;
		if ( !pPhoto )
		{
			return ReplyClient(connection) ;
		}
		else
		{
			struct MHD_Response * Repone = MHD_create_response_from_buffer (pPhoto->nBufferLen,pPhoto->pBuffer,MHD_RESPMEM_PERSISTENT);
			MHD_add_response_header (Repone,MHD_HTTP_HEADER_CONTENT_TYPE,"image/png");
			char pNum[20] = { 0 } ;
			sprintf(pNum,"%d",pPhoto->nBufferLen);
			MHD_add_response_header (Repone,MHD_HTTP_HEADER_CONTENT_LENGTH,pNum);
			MHD_add_response_header (Repone,"nResult","1");
			int nRet = MHD_queue_response(connection,MHD_HTTP_OK,Repone);
			MHD_destroy_response(Repone) ;
			return nRet;
		}
		return MHD_YES ;
	}

	return MHD_NO ;
}

int main ()
{
	struct MHD_Daemon *daemon;

	daemon = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY|MHD_USE_DEBUG, PORT, NULL, NULL,&answer_to_connection, NULL,MHD_OPTION_END);
	if (NULL == daemon)
		return 1;

	getchar ();

	MHD_stop_daemon (daemon);

	return 0;
}