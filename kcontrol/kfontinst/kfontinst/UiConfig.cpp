////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CUiConfig
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 19/06/2002
// Version       : $Revision$ $Date$
//
////////////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2001
////////////////////////////////////////////////////////////////////////////////

#include "UiConfig.h"
#include <kcmdlineargs.h>
#include "Misc.h"

static void checkSize(QStringList &list, bool space=false)
{
    static const unsigned int constMaxItems=30;

    unsigned int max=space ? constMaxItems-1 : constMaxItems;

    while(list.count()>max)
        list.remove(list.begin());
}

CUiConfig::CUiConfig()
         : KConfig("kcmfontinstuirc")
{
    QString origGroup=group();
    int     intVal;

    //
    // If this module is being run as root from a non-root started kcontrol (i.e. via kcmshell), then
    // need to save Config changes as they are changed - as when the module is unloaded it is
    // simply killed, and the destructors don't get a chance to run.
    const char *appName=KCmdLineArgs::appName();
    itsAutoSync=CMisc::root() && (NULL==appName || strcmp("kcontrol", appName));

    setGroup("KcmFontinst");
    itsOpenInstDirs=readListEntry("OpenInstDirs");
    itsInstTopItem=readEntry("InstTopItem");
    itsOpenFsDirs=readListEntry("OpenFsDirs");
    itsFsTopItem=readEntry("FsTopItem");
    intVal=readNumEntry("Mode", BASIC);
    itsMode=intVal>=BASIC && intVal <=ADVANCED_PLUS_FS ? (EMode)intVal : BASIC;

    // Restore KConfig group...
    setGroup(origGroup);
}

CUiConfig::~CUiConfig()
{
    QString origGroup=group();

    setGroup("KcmFontinst");
    checkSize(itsOpenInstDirs);
    writeEntry("OpenInstDirs", itsOpenInstDirs);
    writeEntry("InstTopItem", itsInstTopItem);
    checkSize(itsOpenFsDirs);
    writeEntry("OpenFsDirs", itsOpenFsDirs);
    writeEntry("FsTopItem", itsFsTopItem);
    writeEntry("Mode", (int)itsMode);

    // Restore KConfig group...
    setGroup(origGroup);
}

void CUiConfig::setMode(EMode m)
{
    if(m!=itsMode)
    {
        itsMode=m;
        write("Mode", (int)itsMode);
    }
}

void CUiConfig::addOpenInstDir(const QString &d)
{
    storeInList(itsOpenInstDirs, d);
    write("OpenInstDirs", itsOpenInstDirs);
}

void CUiConfig::removeOpenInstDir(const QString &d)
{
    if(-1!=itsOpenInstDirs.findIndex(d))
    {
        itsOpenInstDirs.remove(d);
        write("OpenInstDirs", itsOpenInstDirs);
    }
}

void CUiConfig::setInstTopItem(const QString &s)
{
    itsInstTopItem=s;
    write("InstTopItem", s);
}

void CUiConfig::addOpenFsDir(const QString &d)
{
    storeInList(itsOpenFsDirs, d);
    write("OpenFsDirs", itsOpenFsDirs);
}

void CUiConfig::removeOpenFsDir(const QString &d)
{
    if(-1!=itsOpenFsDirs.findIndex(d))
    {
        itsOpenFsDirs.remove(d);
        write("OpenFsDirs", itsOpenFsDirs);
    }
}

void CUiConfig::setFsTopItem(const QString &s)
{
    itsFsTopItem=s;
    write("FsTopItem", s);
}

void CUiConfig::write(const QString &key, const QStringList &value)
{
    if(itsAutoSync)
    {
        KConfigGroupSaver cfgSaver(this, "KcmFontinst");

        writeEntry(key, value);
        sync();
    }
}

void CUiConfig::write(const QString &key, const QString &value)
{
    if(itsAutoSync)
    {
        KConfigGroupSaver cfgSaver(this, "KcmFontinst");

        writeEntry(key, value);
        sync();
    }
}

void CUiConfig::write(const QString &key, int value)
{
    if(itsAutoSync)
    {
        KConfigGroupSaver cfgSaver(this, "KcmFontinst");

        writeEntry(key, value);
        sync();
    }
}

void CUiConfig::storeInList(QStringList &list, const QString &s)
{
    unsigned int idx=list.findIndex(s);

    if(!list.count() || list.count()-1!=idx)
    {
        if(-1!=idx)
            list.remove(s);

        if(itsAutoSync)
            checkSize(list, true);
        list.append(s);
    }
}
