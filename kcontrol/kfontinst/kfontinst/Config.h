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

    CConfig() : KConfig("kcmfontinstrc")            { load(); }
    virtual ~CConfig()                              { }

    void              load();
    void              save();
    void              reset()                       { rollback(); load(); }

    bool              firstTime()                   { return !itsConfigured; }
    void              configured()                  { itsConfigured=true; }
 
    const QString &   getFontsDir()                 { return itsFontsDir; }
    const QString &   getTTSubDir()                 { return itsTTSubDir; }
    const QString &   getT1SubDir()                 { return itsT1SubDir; }
    const QString &   getXConfigFile()              { return itsXConfigFile; }
    const QString &   getEncodingsDir()             { return itsEncodingsDir; }
    const QString &   getGhostscriptFile()          { return itsGhostscriptFile; }
    bool              getDoGhostscript()            { return itsDoGhostscript; }
    const QString &   getCupsDir()                  { return itsCupsDir; }
    bool              getDoCups()                   { return itsDoCups; }

    const QString &   getInstallDir()               { return itsInstallDir; }

    bool              getSOConfigure()              { return itsSOConfigure; }
    const QString &   getSODir()                    { return itsSODir; }
    const QString &   getSOPpd()                    { return itsSOPpd; }

    bool              getDoAfm()                    { return itsDoAfm; }
    bool              getDoTtAfms()                 { return itsDoTtAfms; }
    bool              getDoT1Afms()                 { return itsDoT1Afms; }
    const QString &   getAfmEncoding()              { return itsAfmEncoding; }
    EXFontListRefresh getXRefreshCmd()              { return itsXRefreshCmd; }
    const QString &   getCustomXRefreshCmd()        { return itsCustomXRefreshCmd; }

    const QStringList getModifiedDirs()             { return itsModifiedDirs; }

    void setFontsDir(const QString &s)              { itsFontsDir=s; }
    void setTTSubDir(const QString &s)              { itsTTSubDir=s; }
    void setT1SubDir(const QString &s)              { itsT1SubDir=s; }
    void setXConfigFile(const QString &s)           { itsXConfigFile=s; }
    void setEncodingsDir(const QString &s)          { itsEncodingsDir=s; }
    void setGhostscriptFile(const QString &s)       { itsGhostscriptFile=s; }
    void setDoGhostscript(bool b)                   { itsDoGhostscript=b; }
    void setCupsDir(const QString &s)               { itsCupsDir=s; }
    void setDoCups(bool b)                          { itsDoCups=b; }
 
    void setInstallDir(const QString &s)            { itsInstallDir=s; }

    void setSOConfigure(bool b);
    void setSODir(const QString &s)                 { itsSODir=s; }
    void setSOPpd(const QString &s)                 { itsSOPpd=s; }
 
    void setDoAfm(bool b);
    void setDoTtAfms(bool b);
    void setDoT1Afms(bool b); 
    void setAfmEncoding(const QString &s)           { itsAfmEncoding=s; }
    void setXRefreshCmd(EXFontListRefresh cmd)      { itsXRefreshCmd=cmd; }
    void setCustomXRefreshCmd(const QString &s)     { itsCustomXRefreshCmd=s; }

    void addModifiedDir(const QString &d);
    void removeModifiedDir(const QString &d);
    void clearModifiedDirs()                        { itsModifiedDirs.clear(); }

    public:

    static const QString constFontsDirs[];
    static const QString constTTSubDirs[];
    static const QString constT1SubDirs[];
    static const QString constXConfigFiles[];
    static const QString constXfsConfigFiles[];
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
    void checkAndModifyFontmapFile();
    void checkAndModifyXConfigFile();

    private:

    QString           itsFontsDir,
                      itsTTSubDir,
                      itsT1SubDir,
                      itsXConfigFile,
                      itsEncodingsDir,
                      itsGhostscriptFile,
                      itsCupsDir,
                      itsInstallDir,
                      itsSODir,
                      itsSOPpd,
                      itsAfmEncoding,
                      itsCustomXRefreshCmd;
    bool              itsDoGhostscript,
                      itsDoCups,
                      itsSOConfigure,
                      itsDoAfm,
                      itsDoTtAfms,
                      itsDoT1Afms,
                      itsConfigured;
    EXFontListRefresh itsXRefreshCmd;
    QStringList       itsModifiedDirs;
};

#endif
