#ifndef __CONFIG_H__
#define __CONFIG_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CConfig
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 29/04/2001
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qstring.h>
#include <qnamespace.h>
#include <qstringlist.h>
#include <kconfig.h>

class CConfig : public KConfig
{
    public:

    enum EXFontListRefresh
    {
        XREFRESH_XSET_FP_REHASH = 0,
        XREFRESH_XFS_RESTART,
        XREFRESH_CUSTOM
    };

    enum EListWidget
    {
        DISK      =0,
        INSTALLED =1
    };

    struct TAdvanced
    {
        QStringList dirs;
        QString     topItem;
    };

    CConfig();
    virtual ~CConfig();

    bool              firstTime()                   { return !itsConfigured; }
    void              configured();
    bool              getAdvancedMode()             { return itsAdvancedMode; }
    Qt::Orientation   getFontListsOrientation()     { return itsFontListsOrientation; }
    bool              getUseCustomPreviewStr()      { return itsUseCustomPreviewStr; }
    const QString &   getCustomPreviewStr()         { return itsCustomPreviewStr; }
 
    const QString &   getFontsDir()                 { return itsFontsDir; }
    const QString &   getTTSubDir()                 { return itsTTSubDir; }
    const QString &   getT1SubDir()                 { return itsT1SubDir; }
    const QString &   getXConfigFile()              { return itsXConfigFile; }
#ifdef HAVE_XFT
    const QString &   getXftConfigFile()            { return itsXftConfigFile; }
#endif
    const QString &   getEncodingsDir()             { return itsEncodingsDir; }
    const QString &   getGhostscriptFile()          { return itsGhostscriptFile; }
    bool              getDoGhostscript()            { return itsDoGhostscript; }
    const QString &   getCupsDir()                  { return itsCupsDir; }
    bool              getDoCups()                   { return itsDoCups; }

    bool              getFixTtfPsNamesUponInstall() { return itsFixTtfPsNamesUponInstall; }
    const QString &   getUninstallDir()             { return itsUninstallDir; }
    const QString &   getInstallDir()               { return itsInstallDir; }

    const QStringList & getAdvancedDirs(EListWidget w) { return itsAdvanced[w].dirs; } 
    const QString &   getAdvancedTopItem(EListWidget w) { return itsAdvanced[w].topItem; }
    bool              getSOConfigure()              { return itsSOConfigure; }
    const QString &   getSODir()                    { return itsSODir; }
    const QString &   getSOPpd()                    { return itsSOPpd; }

    bool              getExclusiveEncoding()        { return itsExclusiveEncoding; }
    const QString &   getEncoding()                 { return itsEncoding; }
    bool              getDoAfm()                    { return itsDoAfm; }
    bool              getDoTtAfms()                 { return itsDoTtAfms; }
    bool              getDoT1Afms()                 { return itsDoT1Afms; }
    bool              getOverwriteAfms()            { return itsOverwriteAfms; }
    const QString &   getAfmEncoding()              { return itsAfmEncoding; }
    EXFontListRefresh getXRefreshCmd()              { return itsXRefreshCmd; }
    const QString &   getCustomXRefreshCmd()        { return itsCustomXRefreshCmd; }

    const QStringList getModifiedDirs()             { return itsModifiedDirs; }

    void setAdvancedMode(bool b);
    void setFontListsOrientation(Qt::Orientation o);
    void setUseCustomPreviewStr(bool b);
    void setCustomPreviewStr(const QString &s);
 
    void setFontsDir(const QString &s);
    void setTTSubDir(const QString &s);
    void setT1SubDir(const QString &s);
    void setXConfigFile(const QString &s);
#ifdef HAVE_XFT
    void setXftConfigFile(const QString &s);
#endif
    void setEncodingsDir(const QString &s);
    void setGhostscriptFile(const QString &s);
    void setDoGhostscript(bool b);
    void setCupsDir(const QString &s);
    void setDoCups(bool b);
 
    void setFixTtfPsNamesUponInstall(bool b);
    void setUninstallDir(const QString &s);
    void setInstallDir(const QString &s);

    void addAdvancedDir(EListWidget w, const QString &d);
    void removeAdvancedDir(EListWidget w, const QString &d);
    void setAdvancedTopItem(EListWidget w, const QString &s);
 
    void setSOConfigure(bool b);
    void setSODir(const QString &s);
    void setSOPpd(const QString &s);
 
    void setExclusiveEncoding(bool b);
    void setEncoding(const QString &s);
    void setDoAfm(bool b);
    void setDoTtAfms(bool b);
    void setDoT1Afms(bool b); 
    void setOverwriteAfms(bool b);
    void setAfmEncoding(const QString &s);
    void setXRefreshCmd(EXFontListRefresh cmd);
    void setCustomXRefreshCmd(const QString &s);

    void addModifiedDir(const QString &d);
    void removeModifiedDir(const QString &d);
    void clearModifiedDirs();

    public:

    static const QString constFontsDirs[];
    static const QString constTTSubDirs[];
    static const QString constT1SubDirs[];
    static const QString constXConfigFiles[];
    static const QString constXfsConfigFiles[];
#ifdef HAVE_XFT
    static const QString constXftConfigFiles[];
#endif
    static const QString constEncodingsSubDirs[];  // sub dirs of <X11>, /usr/share, /usr/local/share, etc...
    static const QString constGhostscriptDirs[];   // Excludes version number
    static const QString constGhostscriptFiles[];
    static const QString constCupsDirs[];
    static const QString constSODirs[];
    static const QString constNotFound;

    private:

    void write(const QString &sect, const QString &key, const QString &value);
    void write(const QString &sect, const QString &key, const QStringList &value);
    void write(const QString &sect, const QString &key, bool value);
    void write(const QString &sect, const QString &key, int value);

    private:

    QString           itsCustomPreviewStr,
                      itsFontsDir,
                      itsTTSubDir,
                      itsT1SubDir,
                      itsXConfigFile,
#ifdef HAVE_XFT
                      itsXftConfigFile,
#endif
                      itsEncodingsDir,
                      itsGhostscriptFile,
                      itsCupsDir,
                      itsUninstallDir,
                      itsInstallDir,
                      itsSODir,
                      itsSOPpd,
                      itsEncoding,
                      itsAfmEncoding,
                      itsCustomXRefreshCmd;
    bool              itsDoGhostscript,
                      itsDoCups,
                      itsAdvancedMode,
                      itsUseCustomPreviewStr,
                      itsFixTtfPsNamesUponInstall,
                      itsSOConfigure,
                      itsExclusiveEncoding,
                      itsDoAfm,
                      itsDoTtAfms,
                      itsDoT1Afms,
                      itsOverwriteAfms,
                      itsConfigured,
                      itsAutoSync;
    EXFontListRefresh itsXRefreshCmd;
    Qt::Orientation   itsFontListsOrientation;
    QStringList       itsModifiedDirs;
    TAdvanced         itsAdvanced[2];
};

#endif
