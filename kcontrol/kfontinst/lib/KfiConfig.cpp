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

#include "KfiConfig.h"
#include "Global.h"
#include "Misc.h"
#include "XConfig.h"
#include <qdir.h>
#include <qfile.h>
#include <kapplication.h>
#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>
#include <klocale.h>
#include <fstream>

#define KCONFIG_GROUP                                "KFontinst"
#define KFI_XF86CFG                                  "XF86Config"
#define KFI_XORGCFG                                  "xorg.conf"

static const char * constDefaultSysX11FontsDir     = "/usr/X11R6/lib/X11/fonts/";
static const char * constDefaultSysTTSubDir        = "TrueType/";
static const char * constDefaultSysT1SubDir        = "Type1/";
static const char * constDefaultXConfigFile        = "/etc/X11/"KFI_XF86CFG"-4";
static const char * constDefaultXfsConfigFile      = "/etc/X11/fs/config";
#ifndef HAVE_FONT_ENC
static const char * constDefaultEncodingsDir       = "/usr/X11R6/lib/X11/fonts/encodings/";
#endif
static const char * constDefaultGhostscriptDir     = "/usr/share/ghostscript/";
static const char * constNonRootDefaultXConfigFile = "fontpaths";

static const char * getDir(const char *entry, const char **posibilities, const QString &base=QString::null)
{
    if(CMisc::dExists(base+entry))
        return entry;
    else
    {
        int d;

        for(d=0; posibilities[d]; ++d)
            if(CMisc::dExists(base+posibilities[d]))
                break;

        return posibilities[d];
    }
}

static const char * getFile(const char *entry, const char **posibilities)
{
    if(CMisc::fExists(entry))
        return entry;
    else
    {
        int f;

        for(f=0; posibilities[f]; ++f)
            if(CMisc::fExists(posibilities[f]))
                break;

        return posibilities[f];
    }
}

static QString locateFile(const char *dir, const char *file, int level=0)
{
    if(level<CMisc::MAX_SUB_DIRS)
    {
        QDir d(dir);

        if(d.isReadable())
        {
            const QFileInfoList *fList=d.entryInfoList();

            if(fList)
            {
                QFileInfoListIterator it(*fList);
                QFileInfo             *fInfo;
                QString               str;

                for(; NULL!=(fInfo=it.current()); ++it)
                    if("."!=fInfo->fileName() && ".."!=fInfo->fileName())
                        if(fInfo->isDir())
                        {
                            if(!(str=locateFile(QFile::encodeName(fInfo->filePath()+"/"), file, level+1)).isEmpty())
                                return str;
                        }
                        else
                            if(fInfo->fileName()==file)
                                return fInfo->filePath();
            }
        }
    }

    return QString::null;
}

static QString locateFile(const char *file, const char **dirs)
{
    int     d;
    QString str;

    for(d=0; dirs[d]; ++d)
        if(!(str=locateFile(dirs[d], file)).isEmpty())
            return str;

    return QString::null;
}

static const char * constSysX11FontsDirs[]=
{
    constDefaultSysX11FontsDir,
    "/usr/lib/X11/fonts/",
    "/usr/openwin/lib/X11/fonts/",
    NULL
};

static const char * constTTSubDirs[]=
{
    constDefaultSysTTSubDir,
    "truetype/",
    "Truetype/",
    "ttf/",
    "TTF/",
    "Ttf/",
    "tt",
    "TT",
    "True_Type/",
    "true_type/",
    "True_type/",
    "ttf.st/typefaces/",
    "ttf.st/",
    NULL
};

static const char * constT1SubDirs[]=
{
    constDefaultSysT1SubDir,
    "type1/",
    "T1/",
    "t1/",
    "Postscript/",
    "PSType1/",
    "pstype1/",
    "PsType1/",
    "Pstype1/",
    "type1.st/typefaces/",
    "type1.st/",
    NULL
};

#ifndef HAVE_FONT_ENC
static const char * constEncodingsSubDirs[]=
{
    "encodings/",
    "Encodings/",
    "enc/",
    NULL
};
#endif

static const char * constXConfigFiles[]=
{
    constDefaultXConfigFile,
    "/etc/X11/"KFI_XF86CFG,
    "/etc/"KFI_XF86CFG"-4",
    "/etc/"KFI_XF86CFG,
    "/usr/X11R6/etc/X11/"KFI_XF86CFG"-4",
    "/usr/X11R6/etc/X11/"KFI_XF86CFG,
    "/usr/X11R6/lib/X11/"KFI_XF86CFG"-4",
    "/usr/X11R6/lib/X11/"KFI_XF86CFG,

    // Support for X.org's X server...

    "/etc/X11/"KFI_XORGCFG,
    "/etc/"KFI_XORGCFG,
    "/usr/X11R6/etc/X11/"KFI_XORGCFG,
    "/usr/X11R6/lib/X11/"KFI_XORGCFG,

    NULL
};

static const char * constXfsConfigFiles[]=
{
    constDefaultXfsConfigFile,
    "/usr/openwin/lib/X11/fonts/fontserver.cfg",
    NULL
};

static const char * constGhostscriptDirs[]=
{
    constDefaultGhostscriptDir,
    "/usr/local/share/ghostscript/",
    NULL
};

static QString kdeHome()
{
    QString kHome(getenv("KDEHOME"));

    if(kHome.isEmpty())
        kHome=QDir::homeDirPath()+"/.kde/";

    return kHome;
}

CKfiConfig::CKfiConfig(bool all, bool checkDirs, bool checkX)
          : KConfig("kfontinstrc")
{
    QString sysX11FontsDir,
            defaultSysXConfigFile,
            defaultUserXConfigFile,
            origUserFontsDir,
            origSysX11FontsDir,
            origSysXConfigFile,
            origSysXfsConfigFile,
            origUserXConfigFile,
#ifndef HAVE_FONT_ENC
            origEncodingsDir,
#endif
            origSysTTSubDir,
            origSysT1SubDir,
            origFontmapDir,
            origGhostscriptFile,
            home;
    bool    configured=false,
            origConfigured=false,
            origSysXfs=false,
            root=CMisc::root();
    int     origSysXConfigFileTs=0,
            sysXConfigFileTs=0;

    KConfigGroupSaver saver(this, KCONFIG_GROUP);

    itsSysFontsDirs.append("/usr/local/share/fonts/");
    itsSysFontsDirs.append("/usr/share/fonts/");
    
    if(!root)
    {
        itsUserFontsDirs.append(CMisc::dirSyntax(QDir::homeDirPath()+"/.fonts/"));

        QString kdeFontsDir(CMisc::dirSyntax(kdeHome()+"/share/fonts/"));

        if(CMisc::dExists(kdeFontsDir))
            itsUserFontsDirs.append(kdeFontsDir);
    }

    if(all)
    {
        defaultSysXConfigFile=constDefaultXConfigFile;
        defaultUserXConfigFile=root ? defaultSysXConfigFile : (itsUserFontsDirs.first()+constNonRootDefaultXConfigFile);
    }

    configured=readBoolEntry("Configured", false);
    sysX11FontsDir=CMisc::dirSyntax(readPathEntry("SysX11FontsDir", constDefaultSysX11FontsDir));

    if(all)
    {
        itsSysXConfigFile=readPathEntry("SysXConfigFile", defaultSysXConfigFile);
        itsSysXfsConfigFile=readPathEntry("SysXfsConfigFile", constDefaultXfsConfigFile);
        sysXConfigFileTs=readNumEntry("SysXConfigFileTs", 0);
        itsUserXConfigFile=readPathEntry("UserXConfigFile", defaultUserXConfigFile);
#ifndef HAVE_FONT_ENC
        itsEncodingsDir=CMisc::dirSyntax(readPathEntry("EncodingsDir", constDefaultEncodingsDir));
#endif
        itsSysXfs=readBoolEntry("SysXfs", false);
        itsSysTTSubDir=CMisc::dirSyntax(readEntry("SysTTSubDir", constDefaultSysTTSubDir));
        itsSysT1SubDir=CMisc::dirSyntax(readEntry("SysT1SubDir", constDefaultSysT1SubDir));
        itsFontmapDir=CMisc::dirSyntax(readPathEntry("FontmapDir", root ? "/etc/fonts" : itsUserFontsDirs.first()));

        origConfigured=configured;
        origSysX11FontsDir=sysX11FontsDir;
        origSysXConfigFile=itsSysXConfigFile;
        origSysXfsConfigFile=itsSysXfsConfigFile;
        origUserXConfigFile=itsUserXConfigFile;
#ifndef HAVE_FONT_ENC
        origEncodingsDir=itsEncodingsDir;
#endif
        origSysTTSubDir=itsSysTTSubDir;
        origSysT1SubDir=itsSysT1SubDir;
        origSysXfs=itsSysXfs;
        origSysXConfigFileTs=sysXConfigFileTs;
        origFontmapDir=itsFontmapDir;

        if(root)
        {
            itsGhostscriptFile=readPathEntry("GhostscriptFile", QString::null);
            origGhostscriptFile=itsGhostscriptFile;
        }
    }

    // Now check the read in data...
    if(!(sysX11FontsDir=getDir(QFile::encodeName(sysX11FontsDir), constSysX11FontsDirs)).isEmpty())
        itsSysFontsDirs.append(sysX11FontsDir);

    if(root) // For root, these are *always* the same
        itsUserFontsDirs=itsSysFontsDirs;
    //
    // First of all, if this is the first time KFontinst has been run, check for the existance of
    // /usr/local/share/fonts or $HOME/.fonts - if it doesn't exist then try to create
    if(!CMisc::dExists(itsUserFontsDirs.first()))
        CMisc::createDir(itsUserFontsDirs.first());

    if(!configured && !CMisc::dExists(itsFontmapDir))
        CMisc::createDir(itsFontmapDir);

    if(all)
    {
        //
        // Try to determine location for X and xfs config files...
        // ...note on some systems (Solaris and HP-UX) only the xfs file will be found
        itsSysXConfigFile=getFile(QFile::encodeName(itsSysXConfigFile), constXConfigFiles);
        itsSysXfsConfigFile=getFile(QFile::encodeName(itsSysXfsConfigFile), constXfsConfigFiles);

        // If found xfs, but not X - then assume that xfs is being used...
        if(!itsSysXfsConfigFile.isEmpty() && itsSysXConfigFile.isEmpty())
            itsSysXfs=true;

        //
        // Check to see whether user has switched from X to xfs, or vice versa...
        if(configured && checkX && CMisc::fExists(itsSysXConfigFile))
        {
            //
            // The X config file is where the system will specify whether xfs is being used or not,
            // so check if the timestamp of this has changed. If so, then set xfs to false - so that
            // a re-check of the X config file will be performed.

            if(sysXConfigFileTs==CMisc::getTimeStamp(itsSysXConfigFile)) // File has not been modified, therefore no reason to force a check
                checkX=false;
        }

        if(!configured || checkDirs)
        {
            if(root)
            {
                if(itsGhostscriptFile.isEmpty() || !CMisc::fExists(itsGhostscriptFile))
                    itsGhostscriptFile=locateFile("Fontmap", constGhostscriptDirs);
            }

            QStringList::Iterator it;
            bool                  foundTT=false,
                                  foundT1=false;

            for(it=itsSysFontsDirs.begin(); it!=itsSysFontsDirs.end() && (!foundTT || !foundT1); ++it)
            {
                if(CMisc::dExists(*it+itsSysTTSubDir))
                    foundTT=true;
                if(CMisc::dExists(*it+itsSysT1SubDir))
                    foundT1=true;
            }

            for(it=itsSysFontsDirs.begin(); it!=itsSysFontsDirs.end() && (!foundTT || !foundT1); ++it)
            {
                if(!foundTT && !(itsSysTTSubDir=getDir(constDefaultSysTTSubDir, constTTSubDirs, *it)).isEmpty())
                    foundTT=true;
                if(!foundT1 && !(itsSysT1SubDir=getDir(constDefaultSysT1SubDir, constT1SubDirs, *it)).isEmpty())
                    foundT1=true;

            }
            

#ifndef HAVE_FONT_ENC
            if(!CMisc::dExists(itsEncodingsDir))
            {
                QString top[]={ sysX11FontsDir.isEmpty() ? constDefaultSysX11FontsDir : sysX11FontsDir, "/usr/share/", "/usr/local/share/", QString::null };
                bool    found=false;
                int     t,
                        s;

                for(t=0; !top[t].isEmpty() && !found; ++t)
                    for(s=0; !constEncodingsSubDirs[s].isEmpty() && !found; ++s)
                        if(CMisc::dExists(top[t]+constEncodingsSubDirs[s]))
                        {
                            itsEncodingsDir=top[t]+constEncodingsSubDirs[s];
                            found=true;
                        }

                if(!found)
                    itsEncodingsDir=sysX11FontsDir.isEmpty() ? itsSysFontsDirs.first() : sysX11FontsDir;
            }
#endif
        }

        if(!configured || checkX)
            checkAndModifyXConfigFile();

        configured=true;

        if(root)
            if(origGhostscriptFile!=itsGhostscriptFile)
                writeEntry("GhostscriptFile", itsGhostscriptFile);

        if(origConfigured!=configured)
            writeEntry("Configured", configured);
        if(origSysX11FontsDir!=sysX11FontsDir)
            writePathEntry("SysX11FontsDir", sysX11FontsDir);

        if(all)
        {
            if(origSysTTSubDir!=itsSysTTSubDir)
                writeEntry("SysTTSubDir", itsSysTTSubDir);
            if(origSysT1SubDir!=itsSysT1SubDir)
                writeEntry("SysT1SubDir", itsSysT1SubDir);
            if(origSysXConfigFile!=itsSysXConfigFile)
                writePathEntry("SysXConfigFile", itsSysXConfigFile);
            if(origSysXfsConfigFile!=itsSysXfsConfigFile)
                writePathEntry("SysXfsConfigFile", itsSysXfsConfigFile);
            if(origSysXConfigFileTs!=sysXConfigFileTs)
                writeEntry("SysXConfigFileTs", sysXConfigFileTs);
            if(origSysXfs!=itsSysXfs)
                writeEntry("SysXfs", itsSysXfs);
            if(origUserXConfigFile!=itsUserXConfigFile)
                writePathEntry("UserXConfigFile", itsUserXConfigFile);
#ifndef HAVE_FONT_ENC
            if(origEncodingsDir!=itsEncodingsDir)
                writePathEntry("EncodingsDir", itsEncodingsDir);
#endif
            if(origFontmapDir!=itsFontmapDir)
                writePathEntry("FontmapDir", itsFontmapDir);
        }
    }

    // Restore KConfig group...
    sync();
}

void CKfiConfig::checkAndModifyXConfigFile()
{
    //
    // Check if X11 has been selected by CKfiConfig, and if so, have a look to see wether it has
    // 'unix/<hostname>:<port>' as the fontpath - if so then look for the fontserver 'config' file instead...
    if(!itsSysXConfigFile.isEmpty())
    {
        int slashPos=itsSysXConfigFile.findRev('/');

        if(slashPos!=-1)
        {
            QString file=itsSysXConfigFile.mid(slashPos+1);

            if(file.find(KFI_XF86CFG)!=-1 || file.find(KFI_XORGCFG))
                itsSysXfs=!itsSysXfsConfigFile.isEmpty() && CXConfig(CXConfig::X11, itsSysXConfigFile).xfsInPath();
        }
    }
}

const QStringList & CKfiConfig::getRealTopDirs(const QString &f)
{
    return CMisc::root() || CMisc::getSect(f)==i18n(KIO_FONTS_SYS)
               ? itsSysFontsDirs
               : itsUserFontsDirs;
}

void CKfiConfig::storeSysXConfigFileTs()
{
    if(!itsSysXfs)
    {
        int ts=CMisc::getTimeStamp(itsSysXConfigFile);

        if(0!=ts)
        {
            KConfigGroupSaver saver(this, KCONFIG_GROUP);

            writeEntry("SysXConfigFileTs", ts);
        }
    }
}
