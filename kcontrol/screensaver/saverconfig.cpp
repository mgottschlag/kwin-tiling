#include <kdesktopfile.h>
#include <klocale.h>

#include "saverconfig.h"

SaverConfig::SaverConfig()
{
}

bool SaverConfig::read(const QString &file)
{
    KDesktopFile config(file, true);
#if 0    
    if( !config.tryExec())
      return false;
#endif    
    mExec = config.readPathEntry("Exec");
    mName = config.readEntry("Name");
    QString categoryName = config.readEntry("X-KDE-Category");
    if(categoryName.isEmpty())
	mCategory = categoryName;
    else	    
        mCategory = i18nc("Screen saver category", // Must be same in CMakeFiles.txt
                     categoryName.toUtf8());

    if (config.hasActionGroup("Setup"))
    {
      config.setActionGroup("Setup");
      mSetup = config.readPathEntry("Exec");
    }

    if (config.hasActionGroup("InWindow"))
    {
      config.setActionGroup("InWindow");
      mSaver = config.readPathEntry("Exec");
    }

    int indx = file.lastIndexOf('/');
    if (indx >= 0) {
        mFile = file.mid(indx+1);
    }

    return !mSaver.isEmpty();
}
