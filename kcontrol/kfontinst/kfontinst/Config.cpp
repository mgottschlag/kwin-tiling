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

#include "Config.h"
#include "Encodings.h"
#include "Misc.h"
#include <qfont.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <kconfig.h>
#include <kapp.h>
#include <kstddirs.h>
#include <stdlib.h>
#include <unistd.h>
#include <kglobal.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#ifdef HAVE_XFT
#include "xftint.h"
#endif

static const QCString constDefaultFontsDir              ("/usr/X11R6/lib/X11/fonts/");
static const QCString constDefaultTTSubDir              ("TrueType/");
static const QCString constDefaultT1SubDir              ("Type1/");
static const QCString constDefaultXConfigFile           ("/etc/X11/XF86Config");
static const QCString constDefaultEncodingsDir          ("/usr/X11R6/lib/X11/fonts/encodings/");
static const QCString constDefaultGhostscriptDir        ("/usr/share/ghostscript/");
static const QCString constDefaultGhostscriptFile       ("Fontmap"); 
static const QCString constDefaultCupsDir               ("/usr/share/cups/");
static const QCString constDefaultUninstallDir          ("/tmp/");
static const QCString constDefaultSODir                 ("/opt/office52/");
static const QCString constDefaultSOPpd                 ("SGENPRT.PS");
static const QCString constNonRootDefaultFontsDir       ("share/fonts/");   // $KDEHOME+...
static const QCString constNonRootDefaultXConfigFile    ("share/fonts/fontpaths"); // $KDEHOME+...
static const QCString constNonRootDefaultGhostscriptDir ("share/fonts/");   // $KDEHOME+...
#ifdef HAVE_XFT
static const QCString constDefaultXftConfigFile         (XFT_DEFAULT_PATH);
static const QCString constNonRootDefaultXftConfigFile  ("/.xftconfig"); // $HOME+...
#endif

static QString getDir(const QString &entry, const QString *posibilities, const QString &base=QString::null)
{
    if(CMisc::dExists(base+entry))
        return entry;
    else
    {
        int d;

        for(d=0; QString::null!=posibilities[d]; ++d)
            if(CMisc::dExists(base+posibilities[d]))
                break;

        return QString::null==posibilities[d] ? QString::null : base+posibilities[d];
    }
}

static QString getFirstSubDir(const QString &parent)
{
    QString sub(QString::null);
    QDir    dir(parent);

    if(dir.isReadable())
    {
        const QFileInfoList *files=dir.entryInfoList();
 
        if(files)
        {
            QFileInfoListIterator it(*files);
            QFileInfo             *fInfo;
 
            for(; NULL!=(fInfo=it.current()); ++it)
                if("."!=fInfo->fileName() && ".."!=fInfo->fileName())
                    if(fInfo->isDir())
                    {
                        sub=fInfo->fileName()+"/";
                        break;
                    }
        }
    }

    return sub;
}

static const QString & getFile(const QString &entry, const QString *posibilities)
{
    if(CMisc::fExists(entry))
        return entry;
    else
    {
        int f;

        for(f=0; QString::null!=posibilities[f]; ++f)
            if(CMisc::fExists(posibilities[f]))
                break;

        return posibilities[f];
    }
}

static QString locateFile(const QString &dir, const QString *files, int level=0)
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
                            if(QString::null!=(str=locateFile(fInfo->filePath()+"/", files, level+1)))
                                return str;
                        }
                        else
                        { 
                            int f;

                            for(f=0; QString::null!=files[f]; ++f)
                                if(fInfo->fileName()==files[f])
                                    return fInfo->filePath();
                        }
            }
        }
    }

    return QString::null;
}

static QString getFile(const QString &entry, const QString *dirs, const QString *files)
{
    if(CMisc::fExists(entry))
        return entry;
    else
    {
        int     d;
        QString str;

        for(d=0; QString::null!=dirs[d]; ++d)
            if(QString::null!=(str=locateFile(dirs[d], files)))
                return str;

        return QString::null;
    }
}

const QString CConfig::constFontsDirs[]=
{
    constDefaultFontsDir,
    "/usr/lib/X11/fonts/",
    "/usr/openwin/lib/X11/fonts/",
    QString::null 
};
const QString CConfig::constTTSubDirs[]=
{
    constDefaultTTSubDir, 
    "truetype/", 
    "ttf/", 
    "TTF/", 
    "Ttf/", 
    "tt",
    "TT",
    "True_Type/", 
    "true_type/", 
    QString::null 
};
const QString CConfig::constT1SubDirs[]=
{
    constDefaultT1SubDir, 
    "type1/", 
    "T1/", 
    "t1/", 
    "Postscript/", 
    "PSType1/", 
    "pstype1/", 
    QString::null 
};
const QString CConfig::constEncodingsSubDirs[]=
{ 
    "encodings/",
    "Encodings/",
    "enc/",
    QString::null
};
const QString CConfig::constGhostscriptDirs[]=
{
    constDefaultGhostscriptDir,
    "/usr/local/share/ghostscript/",
    QString::null
};
const QString CConfig::constCupsDirs[]=
{
    constDefaultCupsDir,
    QString::null
};
const QString CConfig::constGhostscriptFiles[]=
{ 
    constDefaultGhostscriptFile, 
    "Fontmap.GS",
    QString::null
};
const QString CConfig::constSODirs[]=
{
    constDefaultSODir,
    "/opt/office51/",
    "/usr/local/office52/",
    "/usr/local/office51/",
    "/usr/local/StarOffice/",
    QString::null
};
const QString CConfig::constXConfigFiles[]=
{
    constDefaultXConfigFile,
    "/usr/X11R6/etc/X11/XF86Config",
    "/etc/X11/XF86Config-4",
    "/etc/XF86Config",
    "/usr/X11R6/lib/X11/XF86Config",
    QString::null
};
const QString CConfig::constXfsConfigFiles[]=
{
    "/etc/X11/fs/config",
    "/usr/openwin/lib/X11/fonts/fontserver.cfg",
    QString::null
};
#ifdef HAVE_XFT
const QString CConfig::constXftConfigFiles[]=
{
    constDefaultXftConfigFile,
    "/etc/X11/XftConfig",
    QString::null
};
#endif

const QString CConfig::constNotFound(i18n("<Not Found>"));

CConfig::CConfig()
       : KConfig("kcmfontinstrc")
{
    QCString defaultFontsDir,
             defaultXConfigFile,
#ifdef HAVE_XFT
             defaultXftConfigFile,
#endif
             defaultGhostscriptFile;
             
    QString origGroup=group(),
            val;
    int     intVal;


    //
    // If this module is being run as root from a non-root started kcontrol (i.e. via kcmshell), then
    // need to save Config changes as they are changed - as when the module is unloaded it is
    // simply killed, and the destructors don't get a chance to run.
    const char *appName=KCmdLineArgs::appName();
    itsAutoSync=CMisc::root() && (NULL==appName || strcmp("kcontrol", appName));

    if(CMisc::root())
    {
        defaultFontsDir=constDefaultFontsDir;
        defaultXConfigFile=constDefaultXConfigFile;
#ifdef HAVE_XFT
        defaultXftConfigFile=constDefaultXftConfigFile;
#endif
        defaultGhostscriptFile=constDefaultGhostscriptDir+"5.50/"+constDefaultGhostscriptFile;
    }
    else
    {
#ifdef HAVE_XFT
        const char *home=getenv("HOME");

        if(!home)
            home="";
#endif

        defaultFontsDir=kapp->dirs()->localkdedir().local8Bit();
        defaultFontsDir+=constNonRootDefaultFontsDir;
        defaultXConfigFile=kapp->dirs()->localkdedir().local8Bit();
        defaultXConfigFile+=constNonRootDefaultXConfigFile;
#ifdef HAVE_XFT
        defaultXftConfigFile=home;
        defaultXftConfigFile+=constNonRootDefaultXftConfigFile;
#endif
        defaultGhostscriptFile=kapp->dirs()->localkdedir().local8Bit();
        defaultGhostscriptFile+=constNonRootDefaultGhostscriptDir;
        defaultGhostscriptFile+=constDefaultGhostscriptFile;
    }

    setGroup("Misc");
    itsConfigured=readBoolEntry("Configured");

    setGroup("Appearance");
    itsAdvancedMode=readBoolEntry("AdvancedMode");
    intVal=readNumEntry("FontListsOrientation", Qt::Vertical);
    itsFontListsOrientation=intVal==Qt::Horizontal ? (Qt::Orientation)intVal : Qt::Vertical;
    itsUseCustomPreviewStr=readBoolEntry("UseCustomPreviewStr");
    itsCustomPreviewStr=readEntry("CustomPreviewStr");
    if(QString::null==itsCustomPreviewStr)
        itsUseCustomPreviewStr=false;

    setGroup("FoldersAndFiles");
    val=readEntry("FontsDir");
    itsFontsDir=val.length()>0 ? val : defaultFontsDir;
    val=readEntry("TTSubDir");
    itsTTSubDir=val.length()>0 ? val : constDefaultTTSubDir;
    val=readEntry("T1SubDir");
    itsT1SubDir=val.length()>0 ? val : constDefaultT1SubDir;
    val=readEntry("XConfigFile");
    itsXConfigFile=val.length()>0 ? val : defaultXConfigFile;
    val=readEntry("EncodingsDir");
    itsEncodingsDir=val.length()>0 ? val : constDefaultEncodingsDir;
    val=readEntry("GhostscriptFile");
    itsGhostscriptFile=val.length()>0 ? val : defaultGhostscriptFile;
    itsDoGhostscript=readBoolEntry("DoGhostscript", true);
    if(CMisc::root())
    {
        val=readEntry("CupsDir");
        itsCupsDir=val.length()>0 ? val : constDefaultCupsDir;
        itsDoCups=readBoolEntry("DoCups", true);
    }
#ifdef HAVE_XFT
    setGroup("AntiAlias");
    val=readEntry("XftConfigFile");
    itsXftConfigFile=val.length()>0 ? val : defaultXftConfigFile;
#endif
    setGroup("InstallUninstall");
    itsFixTtfPsNamesUponInstall=readBoolEntry("FixTtfPsNamesUponInstall", true);
    val=readEntry("UninstallDir");
    itsUninstallDir=val.length()>0 ? val : constDefaultUninstallDir;
    val=readEntry("InstallDir");
    itsInstallDir=val.length()>0 && CMisc::dExists(val) ? val : (QString(getenv("HOME"))+"/");

    setGroup("AdvancedMode");
    itsAdvanced[DISK].dirs=readListEntry("DiskDirs");
    itsAdvanced[DISK].topItem=readEntry("DiskTopItem");
    itsAdvanced[INSTALLED].dirs=readListEntry("InstalledDirs");
    itsAdvanced[INSTALLED].topItem=readEntry("InstalledTopItem");

    setGroup("StarOffice");
    itsSOConfigure=readBoolEntry("SOConfigure");
    val=readEntry("SODir");
    itsSODir=val.length()>0 ? val : constDefaultSODir;
    val=readEntry("SOPpd");
    itsSOPpd=val.length()>0 ? val : constDefaultSOPpd;

    setGroup("SystemConfiguration");
    itsExclusiveEncoding=readBoolEntry("ExclusiveEncoding");
#if QT_VERSION < 300
    itsEncoding=readEntry("Encoding", QFont::encodingName(QFont::charSetForLocale()));
#else
    itsEncoding=readEntry("Encoding", "iso8859-1");
#endif
    itsDoAfm=readBoolEntry("DoAfm");
    itsDoTtAfms=readBoolEntry("DoTtAfms", true);
    itsDoT1Afms=readBoolEntry("DoT1Afms", true);
    itsOverwriteAfms=readBoolEntry("OverwriteAfms", false);
    itsAfmEncoding=readEntry("AfmEncoding", "iso8859-1"); // QFont::encodingName(QFont::charSetForLocale()));
    intVal=readNumEntry("XRefreshCmd", XREFRESH_XSET_FP_REHASH);
    itsXRefreshCmd=intVal>=0 && intVal <=XREFRESH_CUSTOM ? (EXFontListRefresh)intVal : XREFRESH_XSET_FP_REHASH;
    itsCustomXRefreshCmd=readEntry("CustomXRefreshCmd");
    if(QString::null==itsCustomXRefreshCmd && itsXRefreshCmd==XREFRESH_CUSTOM)
        itsXRefreshCmd=XREFRESH_XSET_FP_REHASH;
    itsModifiedDirs=readListEntry("ModifiedDirs");

    //
    // Check data read from config file is valid...
    // ...if not, try to make it sensible...

    //
    // First of all, if this is the first time KFontinst has been run as non-root, check for the existance of
    // $KDEHOME/share/fonts - if it doesn't exist then try to create
    if(!CMisc::root() && !itsConfigured && QString(defaultFontsDir)==itsFontsDir)
        if(!CMisc::dExists(itsFontsDir))
            if(CMisc::dExists(kapp->dirs()->localkdedir())) // Then $KDEHOME exists, check for share and share/fonts
            {
                if(!CMisc::dExists(kapp->dirs()->localkdedir()+"share"))
                    CMisc::createDir(kapp->dirs()->localkdedir()+"share");

                if(CMisc::dExists(kapp->dirs()->localkdedir()+"share")) // Should do now...
                    if(!CMisc::dExists(defaultFontsDir))
                        CMisc::createDir(defaultFontsDir);
            }

    if(QString::null==(itsFontsDir=getDir(itsFontsDir, constFontsDirs)))
        itsFontsDir=constNotFound;

    //
    // Check for TrueType and Type1 sub-folders, and create if either doesn't exist
    if(!CMisc::root() && !itsConfigured && QString(defaultFontsDir)==itsFontsDir)
    {
        if(!CMisc::dExists(QString(defaultFontsDir)+QString(constDefaultTTSubDir)))
            CMisc::createDir(QString(defaultFontsDir)+QString(constDefaultTTSubDir));

        if(!CMisc::dExists(QString(defaultFontsDir)+QString(constDefaultT1SubDir)))
            CMisc::createDir(QString(defaultFontsDir)+QString(constDefaultT1SubDir));
    }

    itsTTSubDir=getDir(itsTTSubDir, constTTSubDirs, itsFontsDir);
    itsT1SubDir=getDir(itsT1SubDir, constT1SubDirs, itsFontsDir);

    if(QString::null==itsTTSubDir || QString::null==itsT1SubDir) 
        if(QString::null==itsTTSubDir && QString::null!=itsT1SubDir)
            itsTTSubDir=itsT1SubDir;
        else
            if(QString::null!=itsTTSubDir && QString::null==itsT1SubDir)
                itsT1SubDir=itsTTSubDir;
            else // Bugger, they're both NULL!!
                itsT1SubDir=itsTTSubDir=getFirstSubDir(itsFontsDir);

    if(CMisc::root())
        if(QString::null==(itsXConfigFile=getFile(itsXConfigFile, constXConfigFiles)))
            if(QString::null==(itsXConfigFile=getFile(itsXConfigFile, constXfsConfigFiles)))
                itsXConfigFile=constNotFound;

#ifdef HAVE_XFT
    if(CMisc::root())
        if(QString::null==(itsXftConfigFile=getFile(itsXftConfigFile, constXftConfigFiles)))
            itsXftConfigFile=constNotFound;
#endif

    if(!CMisc::dExists(itsEncodingsDir))
    {
        QString top[]={ itsFontsDir, "/usr/share/", "/usr/local/share/", QString::null };
        bool    found=false;
        int     t,
                s;

        for(t=0; top[t]!=QString::null && !found; ++t)
            for(s=0; constEncodingsSubDirs[s]!=QString::null && !found; ++s)
                if(CMisc::dExists(top[t]+constEncodingsSubDirs[s]))
                {
                    itsEncodingsDir=top[t]+constEncodingsSubDirs[s];
                    found=true;
                }

        if(!found)
            itsEncodingsDir=itsFontsDir;
    }

    if(CMisc::root())
    {
        if(QString::null==(itsGhostscriptFile=getFile(itsGhostscriptFile, constGhostscriptDirs, constGhostscriptFiles)))
        {
            itsGhostscriptFile=constNotFound;
            itsDoGhostscript=false;
        }

        if(QString::null==(itsCupsDir=getDir(itsCupsDir, constCupsDirs)))
            itsCupsDir=constNotFound;
    }

    if(QString::null==(itsSODir=getDir(itsSODir, constSODirs)))
        itsSODir="/";

    // Can only configure StarOffice if AFM generation has been selected...
    if(itsSOConfigure)
        itsDoAfm=true;
        
    // Restore KConfig group...
    setGroup(origGroup);
}

CConfig::~CConfig()
{
    QString origGroup=group();

    setGroup("Misc");
    writeEntry("Configured", itsConfigured);

    setGroup("Appearance");
    writeEntry("AdvancedMode", itsAdvancedMode);
    writeEntry("FontListsOrientation", itsFontListsOrientation);
    writeEntry("UseCustomPreviewStr", itsUseCustomPreviewStr);
    writeEntry("CustomPreviewStr", itsCustomPreviewStr);

    setGroup("FoldersAndFiles");
    writeEntry("FontsDir", itsFontsDir);
    writeEntry("TTSubDir", itsTTSubDir);
    writeEntry("T1SubDir", itsT1SubDir);
    writeEntry("XConfigFile", itsXConfigFile);
    writeEntry("EncodingsDir", itsEncodingsDir);
    writeEntry("GhostscriptFile", itsGhostscriptFile);
    writeEntry("DoGhostscript", itsDoGhostscript);
    if(CMisc::root())
    {
        writeEntry("CupsDir", itsCupsDir);
        writeEntry("DoCups", itsDoCups);
    }

#ifdef HAVE_XFT
    setGroup("AntiAlias");
    writeEntry("XftConfigFile", itsXftConfigFile);
#endif

    setGroup("InstallUninstall");
    writeEntry("FixTtfPsNamesUponInstall", itsFixTtfPsNamesUponInstall);
    writeEntry("UninstallDir", itsUninstallDir);
    writeEntry("InstallDir", itsInstallDir);

    setGroup("AdvancedMode");
    writeEntry("DiskDirs", itsAdvanced[DISK].dirs);
    writeEntry("DiskTopItem", itsAdvanced[DISK].topItem);
    writeEntry("InstalledDirs", itsAdvanced[INSTALLED].dirs);
    writeEntry("InstalledTopItem", itsAdvanced[INSTALLED].topItem);

    setGroup("StarOffice");
    writeEntry("SOConfigure", itsSOConfigure);
    writeEntry("SODir", itsSODir);
    writeEntry("SOPpd", itsSOPpd);

    setGroup("SystemConfiguration");
    writeEntry("ExclusiveEncoding", itsExclusiveEncoding);
    writeEntry("Encoding", itsEncoding);
    writeEntry("DoAfm", itsDoAfm);
    writeEntry("DoTtAfms", itsDoTtAfms);
    writeEntry("DoT1Afms", itsDoT1Afms);
    writeEntry("AfmEncoding", itsAfmEncoding);
    writeEntry("OverwriteAfms", itsOverwriteAfms);
    writeEntry("XRefreshCmd", itsXRefreshCmd);
    writeEntry("CustomXRefreshCmd", itsCustomXRefreshCmd);
    writeEntry("ModifiedDirs", itsModifiedDirs);
        
    // Restore KConfig group...
    setGroup(origGroup);
}

void CConfig::configured()
{
    itsConfigured=true; 
    write("Misc", "Configured", itsConfigured);
}

void CConfig::setAdvancedMode(bool b)
{
    itsAdvancedMode=b;
    write("Appearance", "AdvancedMode", itsAdvancedMode);
}

void CConfig::setFontListsOrientation(Qt::Orientation o)
{
    itsFontListsOrientation=o;
    write("Appearance", "FontListsOrientation", itsFontListsOrientation);
}

void CConfig::setUseCustomPreviewStr(bool b)
{
    itsUseCustomPreviewStr=b;
    write("Appearance", "UseCustomPreviewStr", itsUseCustomPreviewStr);
}

void CConfig::setCustomPreviewStr(const QString &s)
{
    itsCustomPreviewStr=s;
    write("Appearance", "CustomPreviewStr", itsCustomPreviewStr);
}

void CConfig::setFontsDir(const QString &s)
{
    itsFontsDir=s;
    write("FoldersAndFiles", "FontsDir", itsFontsDir);
}

void CConfig::setTTSubDir(const QString &s)
{
    itsTTSubDir=s; 
    write("FoldersAndFiles", "TTSubDir", itsTTSubDir);
}

void CConfig::setT1SubDir(const QString &s)
{
    itsT1SubDir=s;
    write("FoldersAndFiles", "T1SubDir", itsT1SubDir);
}

void CConfig::setXConfigFile(const QString &s)
{
    itsXConfigFile=s;
    write("FoldersAndFiles", "XConfigFile", itsXConfigFile);
}

#ifdef HAVE_XFT
void CConfig::setXftConfigFile(const QString &s)
{
    itsXftConfigFile=s;
    write("AntiAlias", "XftConfigFile", itsXftConfigFile);
}
#endif

void CConfig::setEncodingsDir(const QString &s)
{
    itsEncodingsDir=s; 
    write("FoldersAndFiles", "EncodingsDir", itsEncodingsDir);
}

void CConfig::setGhostscriptFile(const QString &s)
{
    itsGhostscriptFile=s; 
    write("FoldersAndFiles", "GhostscriptFile", itsGhostscriptFile);
}

void CConfig::setDoGhostscript(bool b)
{
    itsDoGhostscript=b;
    write("FoldersAndFiles", "DoGhostscript", itsDoGhostscript);
}

void CConfig::setCupsDir(const QString &s)
{
    itsCupsDir=s;
    write("FoldersAndFiles", "CupsDir", itsCupsDir);
}

void CConfig::setDoCups(bool b)
{
    itsDoCups=b;
    write("FoldersAndFiles", "DoCups", itsDoCups);
}

void CConfig::setFixTtfPsNamesUponInstall(bool b)
{
    itsFixTtfPsNamesUponInstall=b; 
    write("InstallUninstall", "FixTtfPsNamesUponInstall", itsFixTtfPsNamesUponInstall);
}

void CConfig::setUninstallDir(const QString &s)
{
    itsUninstallDir=s; 
    write("InstallUninstall", "UninstallDir", itsUninstallDir);
}

void CConfig::setInstallDir(const QString &s)
{
    itsInstallDir=s;
    write("InstallUninstall", "InstallDir", itsInstallDir);
}

void CConfig::addAdvancedDir(EListWidget w, const QString &d)
{
    if(-1==itsAdvanced[w].dirs.findIndex(d))
    {
        itsAdvanced[w].dirs.append(d);
        write("AdvancedMode", DISK==w ? "DiskDirs" : "InstalledDirs", itsAdvanced[w].dirs);
    }
}
 
void CConfig::removeAdvancedDir(EListWidget w, const QString &d)
{
    if(-1!=itsAdvanced[w].dirs.findIndex(d))
    {
        itsAdvanced[w].dirs.remove(d);
        write("AdvancedMode", DISK==w ? "DiskDirs" : "InstalledDirs", itsAdvanced[w].dirs);
    }
}

void CConfig::setAdvancedTopItem(EListWidget w, const QString &s)
{
    itsAdvanced[w].topItem=s;
    write("AdvancedMode", DISK==w ? "DiskTopItem" : "InstalledTopItem", s);
}

void CConfig::setSOConfigure(bool b)
{
    itsSOConfigure=b;
    write("StarOffice", "SOConfigure", itsSOConfigure);
    if(b)
        setDoAfm(true);
}

void CConfig::setSODir(const QString &s)
{
    itsSODir=s; 
    write("StarOffice", "SODir", itsSODir);
}

void CConfig::setSOPpd(const QString &s)
{
    itsSOPpd=s; 
    write("StarOffice", "SOPpd", itsSOPpd);
}

void CConfig::setExclusiveEncoding(bool b)
{
    itsExclusiveEncoding=b; 
    write("SystemConfiguration", "ExclusiveEncoding", itsExclusiveEncoding);
}

void CConfig::setEncoding(const QString &s)
{
    itsEncoding=s; 
    write("SystemConfiguration", "Encoding", itsEncoding);
}

void CConfig::setDoAfm(bool b)
{
    itsDoAfm=b; 
    write("SystemConfiguration", "DoAfm", itsDoAfm);
    if(b)
    {
        if(!itsDoTtAfms && !itsDoT1Afms)
        {
            setDoTtAfms(true);
            setDoT1Afms(true);
        }
    }
    else
        setSOConfigure(false);
}

void CConfig::setDoTtAfms(bool b)
{
    itsDoTtAfms=b;
    write("SystemConfiguration", "DoTtAfms", itsDoTtAfms);
    if(!b && !itsDoTtAfms)
        setDoAfm(false);
    else
        if(b && !itsDoAfm)
            setDoAfm(true);
}

void CConfig::setDoT1Afms(bool b)
{
    itsDoT1Afms=b;
    write("SystemConfiguration", "DoT1Afms", itsDoT1Afms);
    if(!b && !itsDoT1Afms)
        setDoAfm(false);
    else
        if(b && !itsDoAfm)
            setDoAfm(true);
}

void CConfig::setOverwriteAfms(bool b)
{
    itsOverwriteAfms=b;
    write("SystemConfiguration", "OverwriteAfms", itsOverwriteAfms);
}

void CConfig::setAfmEncoding(const QString &s)
{
    itsAfmEncoding=s;
    write("SystemConfiguration", "AfmEncoding", itsAfmEncoding);
}

void CConfig::setXRefreshCmd(EXFontListRefresh cmd)
{
    itsXRefreshCmd=cmd; 
    write("SystemConfiguration", "XRefreshCmd", (int)itsXRefreshCmd);
}

void CConfig::setCustomXRefreshCmd(const QString &s)
{
    itsCustomXRefreshCmd=s; 
    write("SystemConfiguration", "CustomXRefreshCmd", itsCustomXRefreshCmd);
}

void CConfig::addModifiedDir(const QString &d)
{
    if(-1==itsModifiedDirs.findIndex(d))
    {
        itsModifiedDirs.append(d);
        write("SystemConfiguration", "ModifiedDirs", itsModifiedDirs);
    }
}

void CConfig::removeModifiedDir(const QString &d)
{
    if(-1!=itsModifiedDirs.findIndex(d))
    {
        itsModifiedDirs.remove(d);
        write("SystemConfiguration", "ModifiedDirs", itsModifiedDirs);
    }
}

void CConfig::clearModifiedDirs()
{
    itsModifiedDirs.clear();
    write("SystemConfiguration", "ModifiedDirs", itsModifiedDirs);
}

void CConfig::write(const QString &sect, const QString &key, const QString &value)
{
    if(itsAutoSync)
    {
        KConfigGroupSaver cfgSaver(this, sect);

        writeEntry(key, value);
        sync();
    }
}

void CConfig::write(const QString &sect, const QString &key, const QStringList &value)
{
    if(itsAutoSync)
    {
        KConfigGroupSaver cfgSaver(this, sect);

        writeEntry(key, value);
        sync();
    }
}

void CConfig::write(const QString &sect, const QString &key, bool value)
{
    if(itsAutoSync)
    {
        KConfigGroupSaver cfgSaver(this, sect);

        writeEntry(key, value);
        sync();
    }
}

void CConfig::write(const QString &sect, const QString &key, int value)
{
    if(itsAutoSync)
    {
        KConfigGroupSaver cfgSaver(this, sect);

        writeEntry(key, value);
        sync();
    }
}
