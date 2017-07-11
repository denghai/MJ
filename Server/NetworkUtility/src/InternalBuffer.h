#pragma once
#include <cstdio>  
#include <cstdlib>  
#include <cstring> 
#include "NetworkDefine.h"
class CInternalBuffer  
{  
public:  
	enum { header_length = 4 };  
	enum { max_body_length = _MSG_BUF_LEN };  

	CInternalBuffer()  
		: body_length_(0)  
	{  
	}  

	bool setData(const char* pData , size_t nLen )
	{
		if ( nLen + header_length > max_body_length )
		{
			printf("body data too big \n ") ;
			return false ;
		}
		bodyLength(nLen) ;
		memcpy(body(),pData,nLen);
		encodeHeader();
		return true ;
	}

	const char* data() const  
	{  
		return data_;  
	}  

	char* data()  
	{  
		return data_;  
	}  

	size_t length() const  
	{  
		return header_length + body_length_;  
	}  

	const char* body() const  
	{  
		return data_ + header_length;  
	}  

	char* body()  
	{  
		return data_ + header_length;  
	}  

	size_t bodyLength() const  
	{  
		return body_length_;  
	}  

	void bodyLength(size_t new_length)  
	{  
		body_length_ = new_length;  
		if (body_length_ > max_body_length)  
			body_length_ = max_body_length;  
	}  

	bool decodeHeader()//将报头的4字节字符串转换成数字  
	{  
		char header[header_length] = {0};  
		memcpy(header, data_, header_length);
		unsigned short* pLen = (unsigned short*)header;
		body_length_ = (*pLen) - 3;
		if ( body_length_ % 16 != header[2] || body_length_ % 6 != header[3] )  // check error ;
		{
			printf("decodeHeader error, body len = %d , key = %d , key2 = %d\n",body_length_,header[2],header[3]);
			return false ;
		}

		if (body_length_ > max_body_length - header_length )  
		{  
			body_length_ = 0;  
			printf("receive msg too big , so kick out it \n") ;
			return false;  
		}  
		return true;  
	}  

	void encodeHeader()//把数据部分大小编码成字符串  
	{  
		char header[header_length] = {0};  
		unsigned short* nFakeLen = (unsigned short*)header ;
		*nFakeLen = body_length_ + 3 ;
		header[2] = (body_length_) % 16 ;
		header[3] = (body_length_) % 6 ;
		memcpy(data_, header, header_length);  
		//printf("decodeHeader error, body len = %d , key = %d , key2 = %d\n",body_length_,header[2],header[3]);
	}  

private:  
	char data_[header_length + max_body_length];  
	size_t body_length_;  
};  