#ifndef __KFI_CONFIG_H__
#define __KFI_CONFIG_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CKfiConfig
// Author        : Craig Drummond
// Project       : K Font Installer
// Creation Date : 05/03/2003
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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2003
////////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qstring.h>
#include <qnamespace.h>
#include <qstringlist.h>
#include <kconfig.h>

class CKfiConfig : public KConfig
{
    public:

    CKfiConfig(bool all=false, bool checkDirs=true, bool checkX=false);
    virtual ~CKfiConfig()                    { }

    const QStringList & getRealTopDirs(const QString &f=QString::null);

    const QStringList & getUserFontsDirs() { return itsUserFontsDirs; }
    const QStringList & getSysFontsDirs()  { return itsSysFontsDirs; }
    const QString &   getSysXConfigFile()  { return itsSysXfs ? itsSysXfsConfigFile : itsSysXConfigFile; }
    const QString &   getUserXConfigFile() { return itsUserXConfigFile; }
#ifndef HAVE_FONT_ENC
    const QString &   getEncodingsDir()    { return itsEncodingsDir; }
#endif
    bool              getSysXfs()          { return itsSysXfs; }
    const QString &   getSysTTSubDir( )    { return itsSysTTSubDir; }
    const QString &   getSysT1SubDir( )    { return itsSysT1SubDir; }
    const QString &   getFontmapDir()      { return itsFontmapDir; }       // Where top-level Fontmap file is placed
    const QString &   getGhostscriptFile() { return itsGhostscriptFile; }  // GS Fontmap file - this will .runlibfile our Fontmap

    void              storeSysXConfigFileTs();

    private:

    void checkAndModifyXConfigFile();

    private:

    QStringList itsUserFontsDirs,
                itsSysFontsDirs;
    QString     itsSysXConfigFile,
                itsSysXfsConfigFile,
                itsUserXConfigFile,
#ifndef HAVE_FONT_ENC
                itsEncodingsDir,
#endif
                itsSysTTSubDir,
                itsSysT1SubDir,
                itsFontmapDir,
                itsGhostscriptFile;
    bool        itsSysXfs;
};

#endif
