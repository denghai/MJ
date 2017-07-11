#include "IConfigFile.h"
bool IConfigFile::LoadFile(const char* pfile )
{
	CConfigReader* pReader = CConfigReader::SharedConfigReader() ;
	if ( pReader->LoadFile(pfile) == false )
	{
        printf("Can't read \"%s\" file\n",pfile);
		return false ;
	}

	CReaderRow* pRow = NULL ;
	while ( (pRow = pReader->EnumNextRow()))
	{
		OnPaser(*pRow) ;
	}
	pReader->UnLoadFile() ;
	OnFinishPaseFile();
	return true ;
}
