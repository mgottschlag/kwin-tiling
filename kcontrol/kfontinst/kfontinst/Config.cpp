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
#include <kapplication.h>
#include <kstandarddirs.h>
#include <stdlib.h>
#include <unistd.h>
#include <kglobal.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <fstream>

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

const QString CConfig::constNotFound(I18N_NOOP("<Not Found>"));

void CConfig::load()
{
    QCString defaultFontsDir,
             defaultXConfigFile,
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
        defaultGhostscriptFile=constDefaultGhostscriptDir+"5.50/"+constDefaultGhostscriptFile;
    }
    else
    {
        defaultFontsDir=kapp->dirs()->localkdedir().local8Bit();
        defaultFontsDir+=constNonRootDefaultFontsDir;
        defaultXConfigFile=kapp->dirs()->localkdedir().local8Bit();
        defaultXConfigFile+=constNonRootDefaultXConfigFile;
        defaultGhostscriptFile=kapp->dirs()->localkdedir().local8Bit();
        defaultGhostscriptFile+=constNonRootDefaultGhostscriptDir;
        defaultGhostscriptFile+=constDefaultGhostscriptFile;
    }

    setGroup("Misc");
    itsConfigured=readBoolEntry("Configured");

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
    setGroup("InstallUninstall");
    val=readEntry("InstallDir");
    itsInstallDir=val.length()>0 && CMisc::dExists(val) ? val : (QString(getenv("HOME"))+"/");

    setGroup("StarOffice");
    itsSOConfigure=readBoolEntry("SOConfigure");
    val=readEntry("SODir");
    itsSODir=val.length()>0 ? val : constDefaultSODir;
    val=readEntry("SOPpd");
    itsSOPpd=val.length()>0 ? val : constDefaultSOPpd;

    setGroup("SystemConfiguration");
    itsDoAfm=readBoolEntry("DoAfm");
    itsDoTtAfms=readBoolEntry("DoTtAfms", true);
    itsDoT1Afms=readBoolEntry("DoT1Afms", true);
    itsAfmEncoding=readEntry("AfmEncoding", "iso8859-1"); // QFont::encodingName(QFont::charSetForLocale()));
    intVal=readNumEntry("XRefreshCmd", XREFRESH_XSET_FP_REHASH);
    itsXRefreshCmd=intVal>=0 && intVal <=XREFRESH_CUSTOM ? (EXFontListRefresh)intVal : XREFRESH_XSET_FP_REHASH;
    itsCustomXRefreshCmd=readEntry("CustomXRefreshCmd");
    if(QString::null==itsCustomXRefreshCmd && itsXRefreshCmd==XREFRESH_CUSTOM)
        itsXRefreshCmd=XREFRESH_XSET_FP_REHASH;
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
        itsFontsDir=i18n(constNotFound.utf8());

    //
    // Check for TrueType and Type1 sub-folders, and create if either doesn't exist
    if(!CMisc::root() && !itsConfigured && QString(defaultFontsDir)==itsFontsDir)
    {
        if(!CMisc::dExists(QString(defaultFontsDir)+QString(constDefaultTTSubDir)))
        {
            CMisc::createDir(QString(defaultFontsDir)+QString(constDefaultTTSubDir));
            addModifiedDir(QString(defaultFontsDir)+QString(constDefaultTTSubDir));
        }

        if(!CMisc::dExists(QString(defaultFontsDir)+QString(constDefaultT1SubDir)))
        {
            CMisc::createDir(QString(defaultFontsDir)+QString(constDefaultT1SubDir));
            addModifiedDir(QString(defaultFontsDir)+QString(constDefaultT1SubDir));
        }
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
                itsXConfigFile=i18n(constNotFound.utf8());

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
            itsGhostscriptFile=i18n(constNotFound.utf8());
            itsDoGhostscript=false;
        }

        if(QString::null==(itsCupsDir=getDir(itsCupsDir, constCupsDirs)))
        {
            itsCupsDir=i18n(constNotFound.utf8());
            itsDoCups=false;
        }

        if(!itsConfigured)
        {
            checkAndModifyFontmapFile();
            checkAndModifyXConfigFile();
        }
    }

    if(QString::null==(itsSODir=getDir(itsSODir, constSODirs)))
        itsSODir="/";

    // Can only configure StarOffice if AFM generation has been selected...
    if(itsSOConfigure)
        itsDoAfm=true;

    // Restore KConfig group...
    setGroup(origGroup);
}

void CConfig::save()
{
    QString origGroup=group();

    setGroup("Misc");
    writeEntry("Configured", itsConfigured);

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

    setGroup("InstallUninstall");
    writeEntry("InstallDir", itsInstallDir);

    setGroup("StarOffice");
    writeEntry("SOConfigure", itsSOConfigure);
    writeEntry("SODir", itsSODir);
    writeEntry("SOPpd", itsSOPpd);

    setGroup("SystemConfiguration");
    writeEntry("DoAfm", itsDoAfm);
    writeEntry("DoTtAfms", itsDoTtAfms);
    writeEntry("DoT1Afms", itsDoT1Afms);
    writeEntry("AfmEncoding", itsAfmEncoding);
    writeEntry("XRefreshCmd", itsXRefreshCmd);
    writeEntry("CustomXRefreshCmd", itsCustomXRefreshCmd);

    // Restore KConfig group...
    setGroup(origGroup);
    sync();
}

void CConfig::setSOConfigure(bool b)
{
    itsSOConfigure=b;
    if(b)
        setDoAfm(true);
}

void CConfig::setDoAfm(bool b)
{
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
    itsDoTtAfms=b;
    if(!b && !itsDoTtAfms)
        setDoAfm(false);
    else
        if(b && !itsDoAfm)
            setDoAfm(true);
}

void CConfig::setDoT1Afms(bool b)
{
    itsDoT1Afms=b;
    if(!b && !itsDoT1Afms)
        setDoAfm(false);
    else
        if(b && !itsDoAfm)
            setDoAfm(true);
}

void CConfig::addModifiedDir(const QString &d)
{
    QString ds(CMisc::dirSyntax(d));

    if(-1==itsModifiedDirs.findIndex(ds))
        itsModifiedDirs.append(ds);
}

void CConfig::removeModifiedDir(const QString &d)
{
    QString ds(CMisc::dirSyntax(d));

    if(-1!=itsModifiedDirs.findIndex(ds))
        itsModifiedDirs.remove(ds);
}

void CConfig::checkAndModifyFontmapFile()
{
    //
    // Check if "Fontmap" has been selected by CConfig, and if so, have a look at its contents to see
    // whether it says '(Fontmap.GS) .runlibfile' - if so then use 'Fontmap.GS' instead...
    if(i18n(constNotFound.utf8())!=itsGhostscriptFile)
    {
        int slashPos=itsGhostscriptFile.findRev('/');

        if(slashPos!=-1)
        {
            QString file=itsGhostscriptFile.mid(slashPos+1);

            if("Fontmap"==file)
            {
                ifstream f(itsGhostscriptFile.local8Bit());

                if(f)
                {
                    const int constMaxLineLen=1024;

                    char line[constMaxLineLen];
                    bool useGS=false;

                    do
                    {
                        f.getline(line, constMaxLineLen);

                        if(f.good())
                        {
                            line[constMaxLineLen-1]='\0';

                            if(strstr(line, "Fontmap.GS")!=NULL && strstr(line, ".runlibfile")!=NULL)
                                useGS=true;
                        }
                    }
                    while(!f.eof() && !useGS);

                    f.close();

                    if(useGS)
                    {
                        QString f(CMisc::getDir(itsGhostscriptFile)+"Fontmap.GS");
                        itsGhostscriptFile=f;
                    }
                }
            }
        }
    }
}

void CConfig::checkAndModifyXConfigFile()
{
    //
    // Check if XF86Config has been selected by CConfig, and if so, have a look to see wether it has
    // 'unix/<hostname>:<port>' as the fontpath - if so then look for the fontserver 'config' file instead...
    if(i18n(constNotFound.utf8())!=itsXConfigFile)
    {
        int slashPos=itsXConfigFile.findRev('/');

        if(slashPos!=-1)
        {
            QString file=itsXConfigFile.mid(slashPos+1);

            if(file.find("XF86Config")!=-1)
            {
                ifstream f(itsXConfigFile.local8Bit());

                if(f)
                {
                    const int constMaxLineLen=1024;

                    char line[constMaxLineLen],
                         str1[constMaxLineLen],
                         str2[constMaxLineLen];
                    bool inFiles=false,
                         useXfs=false;

                    do
                    {

                        f.getline(line, constMaxLineLen);

                        if(f.good())
                        {
                            line[constMaxLineLen-1]='\0';

                            if('#'!=line[0] && sscanf(line, "%s %s", str1, str2)==2)
                            {
                                if(!inFiles)
                                {
                                    if(strcmp(str1, "Section")==0 && strcmp(str2, "\"Files\"")==0)
                                        inFiles=true;
                                }
                                else
                                    if(strcmp(str1, "FontPath")==0 && str2[0]=='\"')
                                    {
                                        unsigned int s2len=strlen(str2);

                                        if(s2len>8 && str2[s2len-1]=='\"' && &str2[1]==strstr(&str2[1], "unix/") && strchr(&str2[1], ':')!=NULL)
                                            useXfs=true;
                                    }
                            }
                            else
                                if(inFiles && sscanf(line, "%s", str1)==1 && strcmp(str1, "EndSection")==0)
                                    break;
                        }
                    }
                    while(!f.eof() && !useXfs);

                    f.close();

                    if(useXfs)
                    {
                        int i;

                        for(i=0; QString::null!=CConfig::constXfsConfigFiles[i]; ++i)
                            if(CMisc::fExists(CConfig::constXfsConfigFiles[i]))
                            {
                                itsXRefreshCmd=XREFRESH_XFS_RESTART;
                                itsXConfigFile=constXfsConfigFiles[i];
                                break;
                            }
                    }
                }
            }
        }
    }
}

