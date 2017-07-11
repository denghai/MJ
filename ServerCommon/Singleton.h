#pragma once
template<class T >
class CSingleton
{
protected:
	CSingleton(){ }
public:
	virtual ~CSingleton(){}
	static T* getInstance()
	{
		if ( s_Instance == nullptr )
		{
			s_Instance = new T ;
		}
		return s_Instance ;
	}
	static void destroyInstance()
	{
		delete s_Instance ;
		s_Instance = nullptr ;
	}
protected:
	static T* s_Instance ;
};

template<class T >
T* CSingleton<T>::s_Instance = nullptr ;