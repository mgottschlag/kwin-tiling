#include <kdelnk.h>
#include "kdelnk.moc"

KKdelnk::KKdelnk( const char* pFile )
  : configFile(pFile)
{  
  parseConfigFiles();

  setDesktopGroup();
}


void KKdelnk::parseConfigFiles()
{
  QFile aFile( configFile );
  aFile.open( IO_ReadOnly );
  parseOneConfigFile( aFile, 0L );
  aFile.close();
}
