#pragma once
#include "MessageIdentifer.h"


enum  eMJCardType
{
	eCT_None,
	eCT_Wan,
	eCT_Tong,
	eCT_Tiao,
	eCT_Feng,  // 1 dong , 2 nan , 3 xi  4 bei 
	eCT_Jian, // 1 zhong , 2 fa , 3 bai 
	eCT_Hun, // °Ù´î¡¢ñ®×Ó
	eCT_Hua,
	eCT_Max,
};

enum  ePosType
{
	ePos_Self,
	ePos_Last,
	ePos_Next ,
	ePos_Oppsite,
	ePos_Any,
	ePos_Other,
	ePos_Already,
	ePos_Max,
};

