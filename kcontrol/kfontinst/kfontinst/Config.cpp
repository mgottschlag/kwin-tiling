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

#ifdef HAVE_XFT
#include "xftint.h"
#endif

static const QString constDefaultFontsDir              ("/usr/X11R6/lib/X11/fonts/");
static const QString constDefaultTTSubDir              ("TrueType/");
static const QString constDefaultT1SubDir              ("Type1/");
static const QString constDefaultXConfigFile           ("/etc/X11/XF86Config");
static const QString constDefaultEncodingsDir          ("/usr/X11R6/lib/X11/fonts/encodings/");
static const QString constDefaultGhostscriptDir        ("/usr/share/ghostscript/");
static const QString constDefaultGhostscriptFile       ("Fontmap"); 
static const QString constDefaultUninstallDir          ("/tmp/");
static const QString constDefaultSODir                 ("/opt/office52/");
static const QString constDefaultSOPpd                 ("SGENPRT.PS");
static const QString constNonRootDefaultFontsDir       ("share/fonts/");   // $KDEHOME+...
static const QString constNonRootDefaultXConfigFile    ("share/fonts/fontpaths"); // $KDEHOME+...
static const QString constNonRootDefaultGhostscriptDir ("share/fonts/");   // $KDEHOME+...
#ifdef HAVE_XFT
static const QString constDefaultXftConfigFile         (XFT_DEFAULT_PATH);
static const QString constNonRootDefaultXftConfigFile  ("/.xftconfig"); // $HOME+...
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

const QString CConfig::constNotFound ("<Not Found>");

CConfig::CConfig()
       : KConfig("kfontinstrc")
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

        defaultFontsDir=kapp->dirs()->localkdedir();
        defaultFontsDir+=constNonRootDefaultFontsDir;
        defaultXConfigFile=kapp->dirs()->localkdedir();
        defaultXConfigFile+=constNonRootDefaultXConfigFile;
#ifdef HAVE_XFT
        defaultXftConfigFile=home;
        defaultXftConfigFile+=constNonRootDefaultXftConfigFile;
#endif
        defaultGhostscriptFile=kapp->dirs()->localkdedir();
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
        if(!CMisc::dExists(QString(defaultFontsDir)+constDefaultTTSubDir))
            CMisc::createDir(QString(defaultFontsDir)+constDefaultTTSubDir);

        if(!CMisc::dExists(QString(defaultFontsDir)+constDefaultT1SubDir))
            CMisc::createDir(QString(defaultFontsDir)+constDefaultT1SubDir);
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

        for(t=0; top[t]!=QString::null; ++t)
            for(s=0; constEncodingsSubDirs[s]!=QString::null; ++s)
                if(CMisc::dExists(top[t]+constEncodingsSubDirs[s]))
                {
                    itsEncodingsDir=top[t]+constEncodingsSubDirs[s];
                    found=true;
                }

        if(!found)
            itsEncodingsDir=itsFontsDir;
    }

    if(CMisc::root())
        if(QString::null==(itsGhostscriptFile=getFile(itsGhostscriptFile, constGhostscriptDirs, constGhostscriptFiles)))
        {
            itsGhostscriptFile=constNotFound;
            itsDoGhostscript=false;
        }

    if(QString::null==(itsSODir=getDir(itsSODir, constSODirs)))
        itsSODir="/";

    // Can only configure StarOffice if AFM generation has been selected...
    if(itsSOConfigure)
        itsDoAfm=true;
        
    // Restore KConfig group...
    setGroup(origGroup);
}

void CConfig::configured()
{
    KConfigGroupSaver cfgSaver(this, "Misc");

    itsConfigured=true; 
    writeEntry("Configured", itsConfigured);
}

void CConfig::setAdvancedMode(bool b)
{
    KConfigGroupSaver cfgSaver(this, "Appearance");
 
    writeEntry("AdvancedMode", b);
    itsAdvancedMode=b;
}

void CConfig::setFontListsOrientation(Qt::Orientation o)
{
    KConfigGroupSaver cfgSaver(this, "Appearance");
 
    writeEntry("FontListsOrientation", o);
    itsFontListsOrientation=o;
}

void CConfig::setUseCustomPreviewStr(bool b)
{
    KConfigGroupSaver cfgSaver(this, "Appearance");
 
    writeEntry("UseCustomPreviewStr", b);
    itsUseCustomPreviewStr=b;
}

void CConfig::setCustomPreviewStr(const QString &s)
{
    KConfigGroupSaver cfgSaver(this, "Appearance");
 
    writeEntry("CustomPreviewStr", s);
    itsCustomPreviewStr=s;
}

void CConfig::setFontsDir(const QString &s)
{
    KConfigGroupSaver cfgSaver(this, "FoldersAndFiles");	
 
    writeEntry("FontsDir", s);
    itsFontsDir=s; 
}

void CConfig::setTTSubDir(const QString &s)
{
    KConfigGroupSaver cfgSaver(this, "FoldersAndFiles");
 
    writeEntry("TTSubDir", s);
    itsTTSubDir=s; 
}

void CConfig::setT1SubDir(const QString &s)
{
    KConfigGroupSaver cfgSaver(this, "FoldersAndFiles");
 
    writeEntry("T1SubDir", s);
    itsT1SubDir=s; 
}

void CConfig::setXConfigFile(const QString &s)
{
    KConfigGroupSaver cfgSaver(this, "FoldersAndFiles");
 
    writeEntry("XConfigFile", s);
    itsXConfigFile=s;
}

#ifdef HAVE_XFT
void CConfig::setXftConfigFile(const QString &s)
{
    KConfigGroupSaver cfgSaver(this, "AntiAlias");
 
    writeEntry("XftConfigFile", s);
    itsXftConfigFile=s;
}
#endif

void CConfig::setEncodingsDir(const QString &s)
{
    KConfigGroupSaver cfgSaver(this, "FoldersAndFiles");
 
    writeEntry("EncodingsDir", s);
    itsEncodingsDir=s; 
}

void CConfig::setGhostscriptFile(const QString &s)
{
    KConfigGroupSaver cfgSaver(this, "FoldersAndFiles");
 
    writeEntry("GhostscriptFile", s);
    itsGhostscriptFile=s; 
}

void CConfig::setDoGhostscript(bool b)
{
    KConfigGroupSaver cfgSaver(this, "FoldersAndFiles");
 
    writeEntry("DoGhostscript", b);
    itsDoGhostscript=b;
}

void CConfig::setFixTtfPsNamesUponInstall(bool b)
{
    KConfigGroupSaver cfgSaver(this, "InstallUninstall");
 
    writeEntry("FixTtfPsNamesUponInstall", b);
    itsFixTtfPsNamesUponInstall=b; 
}

void CConfig::setUninstallDir(const QString &s)
{
    KConfigGroupSaver cfgSaver(this, "InstallUninstall");
 
    writeEntry("UninstallDir", s);
    itsUninstallDir=s; 
}

void CConfig::setInstallDir(const QString &s)
{
    KConfigGroupSaver cfgSaver(this, "InstallUninstall");
 
    writeEntry("InstallDir", s);
    itsInstallDir=s;
}

void CConfig::setSOConfigure(bool b)
{
    KConfigGroupSaver cfgSaver(this, "StarOffice");
 
    writeEntry("SOConfigure", b);
    itsSOConfigure=b;
    if(b)
        setDoAfm(true);
}

void CConfig::setSODir(const QString &s)
{
    KConfigGroupSaver cfgSaver(this, "StarOffice");
 
    writeEntry("SODir", s);
    itsSODir=s; 
}

void CConfig::setSOPpd(const QString &s)
{
    KConfigGroupSaver cfgSaver(this, "StarOffice");
 
    writeEntry("SOPpd", s);
    itsSOPpd=s; 
}

void CConfig::setExclusiveEncoding(bool b)
{
    KConfigGroupSaver cfgSaver(this, "SystemConfiguration");
 
    writeEntry("ExclusiveEncoding", b);
    itsExclusiveEncoding=b; 
}

void CConfig::setEncoding(const QString &s)
{
    KConfigGroupSaver cfgSaver(this, "SystemConfiguration");
 
    writeEntry("Encoding", s);
    itsEncoding=s; 
}

void CConfig::setDoAfm(bool b)
{
    KConfigGroupSaver cfgSaver(this, "SystemConfiguration");
 
    writeEntry("DoAfm", b);
    itsDoAfm=b; 
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
    KConfigGroupSaver cfgSaver(this, "SystemConfiguration");
 
    writeEntry("DoTtAfms", b);
    itsDoTtAfms=b;
    if(!b && !itsDoTtAfms)
        setDoAfm(false);
    else
        if(b && !itsDoAfm)
            setDoAfm(true);
}

void CConfig::setDoT1Afms(bool b)
{
    KConfigGroupSaver cfgSaver(this, "SystemConfiguration");
 
    writeEntry("DoT1Afms", b);
    itsDoT1Afms=b;
    if(!b && !itsDoT1Afms)
        setDoAfm(false);
    else
        if(b && !itsDoAfm)
            setDoAfm(true);
}

void CConfig::setAfmEncoding(const QString &s)
{
    KConfigGroupSaver cfgSaver(this, "SystemConfiguration");
 
    writeEntry("AfmEncoding", s);
    itsAfmEncoding=s;
}

void CConfig::setXRefreshCmd(EXFontListRefresh cmd)
{
    KConfigGroupSaver cfgSaver(this, "SystemConfiguration");
 
    writeEntry("XRefreshCmd", (int)cmd);
    itsXRefreshCmd=cmd; 
}

void CConfig::setCustomXRefreshCmd(const QString &s)
{
    KConfigGroupSaver cfgSaver(this, "SystemConfiguration");
 
    writeEntry("CustomXRefreshCmd", s);
    itsCustomXRefreshCmd=s; 
}

void CConfig::addModifiedDir(const QString &d)
{
    if(itsModifiedDirs.findIndex(d)==-1)
    {
        KConfigGroupSaver cfgSaver(this, "SystemConfiguration");
        itsModifiedDirs.append(d);
        writeEntry("ModifiedDirs", itsModifiedDirs);
    }
}

void CConfig::removeModifiedDir(const QString &d)
{
    if(itsModifiedDirs.findIndex(d)!=-1)
    {
        KConfigGroupSaver cfgSaver(this, "SystemConfiguration");
        itsModifiedDirs.remove(d);
        writeEntry("ModifiedDirs", itsModifiedDirs);
    }
}

void CConfig::clearModifiedDirs()
{
    KConfigGroupSaver cfgSaver(this, "SystemConfiguration");
    itsModifiedDirs.clear();
    writeEntry("ModifiedDirs", itsModifiedDirs);
}
