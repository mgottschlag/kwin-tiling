#include <kdesktopfile.h>
#include <klocale.h>

#include "saverconfig.h"

SaverConfig::SaverConfig()
{
}

bool SaverConfig::read(const QString &file)
{
    KDesktopFile config(file, true);
    if( !config.tryExec())
      return false;
    mExec = config.readPathEntry("Exec");
    mName = config.readEntry("Name");
    mCategory = i18n("Screen saver category", // Must be same in Makefile.am
                     config.readEntry("X-KDE-Category").toUtf8());

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
