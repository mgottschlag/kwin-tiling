////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CConfig
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

#include "Config.h"
#include "Misc.h"
#include <qdir.h>
#include <qfile.h>
#include <kapplication.h>
#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>
#include <klocale.h>
#include <fstream>

#define KCONFIG_GROUP "KFontinst"

static const QString constDefaultSysFontsDir        ("/usr/X11R6/lib/X11/fonts/");
static const QString constDefaultSysTTSubDir        ("TrueType/");
static const QString constDefaultSysT1SubDir        ("Type1/");
static const QString constDefaultXConfigFile        ("/etc/X11/XF86Config-4");
static const QString constDefaultXfsConfigFile      ("/etc/X11/fs/config");
#ifndef HAVE_FONT_ENC
static const QString constDefaultEncodingsDir       ("/usr/X11R6/lib/X11/fonts/encodings/");
#endif
static const QString constDefaultGhostscriptDir     ("/usr/share/ghostscript/");
static const QString constNonRootDefaultXConfigFile ("fontpaths");

static QString getDir(const QString &entry, const QString *posibilities, const QString &base=QString::null)
{
    if(CMisc::dExists(base+entry))
        return entry;
    else
    {
        int d;

        for(d=0; !posibilities[d].isNull(); ++d)
            if(CMisc::dExists(base+posibilities[d]))
                break;

        return posibilities[d];
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

        for(f=0; !posibilities[f].isNull(); ++f)
            if(CMisc::fExists(posibilities[f]))
                break;

        return posibilities[f];
    }
}

static QString locateFile(const QString &dir, const QString file, int level=0)
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
                            if(!(str=locateFile(fInfo->filePath()+"/", file, level+1)).isNull())
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

static QString locateFile(const QString &file, const QString *dirs)
{
    int     d;
    QString str;

    for(d=0; !dirs[d].isNull(); ++d)
        if(!(str=locateFile(dirs[d], file)).isNull())
            return str;

    return QString::null;
}

static const QString constSysFontsDirs[]=
{
    constDefaultSysFontsDir,
    "/usr/lib/X11/fonts/",
    "/usr/openwin/lib/X11/fonts/",
    QString::null
};

static const QString constTTSubDirs[]=
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
    QString::null
};

static const QString constT1SubDirs[]=
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
    QString::null
};

#ifndef HAVE_FONT_ENC
static const QString constEncodingsSubDirs[]=
{
    "encodings/",
    "Encodings/",
    "enc/",
    QString::null
};
#endif

static const QString constXConfigFiles[]=
{
    constDefaultXConfigFile,
    "/etc/XF86Config-4",
    "/usr/X11R6/etc/X11/XF86Config-4",
    "/usr/X11R6/lib/X11/XF86Config-4",
    "/etc/X11/XF86Config",
    "/usr/X11R6/etc/X11/XF86Config",
    "/etc/X11/XF86Config",
    "/etc/XF86Config",
    "/usr/X11R6/lib/X11/XF86Config",
    QString::null
};

static const QString constXfsConfigFiles[]=
{
    constDefaultXfsConfigFile,
    "/usr/openwin/lib/X11/fonts/fontserver.cfg",
    QString::null
};

static const QString constGhostscriptDirs[]=
{
    constDefaultGhostscriptDir,
    "/usr/local/share/ghostscript/",
    QString::null
};

CConfig::CConfig(bool all, bool checkDirs, bool checkX)
       : KConfig("kfontinstrc")
{
    QString defaultUserFontsDir,
            defaultSysFontsDir=constDefaultSysFontsDir,
            defaultSysXConfigFile,
            defaultUserXConfigFile,
            origUserFontsDir,
            origSysFontsDir,
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
            origSysXfs=false;
    int     origSysXConfigFileTs=0,
            sysXConfigFileTs=0;

    KConfigGroupSaver saver(this, KCONFIG_GROUP);

    if(CMisc::root())
        defaultUserFontsDir=constDefaultSysFontsDir;
    else
    {
        const char *hm=getenv("HOME");

        if(!hm)
            hm="/";

        home=hm;

        defaultUserFontsDir=QFile::encodeName(CMisc::dirSyntax(home+"/.fonts/"));
    }
    if(all)
    {
        defaultUserXConfigFile=defaultUserFontsDir+constNonRootDefaultXConfigFile;
        defaultSysXConfigFile=constDefaultXConfigFile;
    }

    configured=readBoolEntry("Configured", false);
    itsUserFontsDir=CMisc::dirSyntax(readPathEntry("UserFontsDir", defaultUserFontsDir));
    itsSysFontsDir=CMisc::dirSyntax(readPathEntry("SysFontsDir", defaultSysFontsDir));

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
        itsFontmapDir=CMisc::dirSyntax(readPathEntry("FontmapDir", CMisc::root() ? "/etc/fonts" : getUserFontsDir()));

        origConfigured=configured;
        origUserFontsDir=itsUserFontsDir;
        origSysFontsDir=itsSysFontsDir;
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

        if(CMisc::root())
        {
            itsGhostscriptFile=readPathEntry("GhostscriptFile", QString::null);
            origGhostscriptFile=itsGhostscriptFile;
        }
    }

    // Now check the read in data...

    //
    // First of all, if this is the first time KFontinst has been run as non-root, check for the existance of
    // $HOME/.fonts - if it doesn't exist then try to create
    if(!CMisc::root() && !CMisc::dExists(itsUserFontsDir))
        CMisc::createDir(itsUserFontsDir);

    if(!configured && !CMisc::dExists(itsFontmapDir))
        CMisc::createDir(itsFontmapDir);

    if(checkDirs)
        if((itsSysFontsDir=getDir(itsSysFontsDir, constSysFontsDirs)).isNull())
            itsSysFontsDir=defaultSysFontsDir;

    if(all)
    {
        //
        // Try to determine location for X and xfs config files...
        // ...note on sime systems (Solaris and HP-UX) only the xfs file will be found
        itsSysXConfigFile=getFile(itsSysXConfigFile, constXConfigFiles);
        itsSysXfsConfigFile=getFile(itsSysXfsConfigFile, constXfsConfigFiles);

        // If found xfs, but not X - then assume that xfs is being used...
        if(!itsSysXfsConfigFile.isNull() && itsSysXConfigFile.isNull())
            itsSysXfs=true;
        else
            itsSysXfs=false;
        //
        // Check to see whether user has switched from X to xfs, or vice versa...
        if(configured && checkX && CMisc::fExists(itsSysXConfigFile))
        {
            int ts=CMisc::getTimeStamp(itsSysXConfigFile);

            //
            // The X config file is where the system will specify whether xfs is being used or not,
            // so check if the timestamp of this has changed. If so, then set xfs to false - so that
            // a re-check of the X config file will be performed.
            if(sysXConfigFileTs!=ts)
                itsSysXfs=false;
            else   // OK, file has not been modified, therefore no reason to force a check
                checkX=false;
        }

        if(!configured || checkDirs)
        {
            if(CMisc::root())
            {
                if(itsGhostscriptFile.isNull() || !CMisc::fExists(itsGhostscriptFile))
                    itsGhostscriptFile=locateFile("Fontmap", constGhostscriptDirs);
            }
            else
            {
                itsSysTTSubDir=getDir(itsSysTTSubDir, constTTSubDirs, itsSysFontsDir);
                itsSysT1SubDir=getDir(itsSysT1SubDir, constT1SubDirs, itsSysFontsDir);

                if(itsSysTTSubDir.isNull() || itsSysT1SubDir.isNull())
                    if(itsSysTTSubDir.isNull() && !itsSysT1SubDir.isNull())
                        itsSysTTSubDir=itsSysT1SubDir;
                    else
                        if(!itsSysTTSubDir.isNull() && itsSysT1SubDir.isNull())
                            itsSysT1SubDir=itsSysTTSubDir;
                        else // Bugger, they're both NULL!!
                            itsSysT1SubDir=itsSysTTSubDir=getFirstSubDir(itsSysFontsDir);
            }

#ifndef HAVE_FONT_ENC
            if(!CMisc::dExists(itsEncodingsDir))
            {
                QString top[]={ itsSysFontsDir, "/usr/share/", "/usr/local/share/", QString::null };
                bool    found=false;
                int     t,
                        s;

                for(t=0; !top[t].isNull() && !found; ++t)
                    for(s=0; !constEncodingsSubDirs[s].isNull() && !found; ++s)
                        if(CMisc::dExists(top[t]+constEncodingsSubDirs[s]))
                        {
                            itsEncodingsDir=top[t]+constEncodingsSubDirs[s];
                            found=true;
                        }

                if(!found)
                    itsEncodingsDir=itsSysFontsDir;
            }
#endif
        }

        if(!configured || checkX)
            checkAndModifyXConfigFile();

        configured=true;

        if(CMisc::root())
        {
            itsUserFontsDir=itsSysFontsDir;  // For root, these are *always* the same
            if(origGhostscriptFile!=itsGhostscriptFile)
                writeEntry("GhostscriptFile", itsGhostscriptFile);
        }

        if(origConfigured!=configured)
            writeEntry("Configured", configured);
        if(origUserFontsDir!=itsUserFontsDir)
            writePathEntry("UserFontsDir", itsUserFontsDir);
        if(origSysFontsDir!=itsSysFontsDir)
            writePathEntry("SysFontsDir", itsSysFontsDir);
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

    // Restore KConfig group...
    sync();
}

void CConfig::checkAndModifyXConfigFile()
{
    //
    // Check if XF86Config has been selected by CConfig, and if so, have a look to see wether it has
    // 'unix/<hostname>:<port>' as the fontpath - if so then look for the fontserver 'config' file instead...
    if(!itsSysXfs)
    {
        int slashPos=itsSysXConfigFile.findRev('/');

        if(slashPos!=-1)
        {
            QString file=itsSysXConfigFile.mid(slashPos+1);

            if(file.find("XF86Config")!=-1)
            {
                std::ifstream f(QFile::encodeName(itsSysXConfigFile));

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
                        for(int i=0; !constXfsConfigFiles[i].isNull(); ++i)
                            if(CMisc::fExists(constXfsConfigFiles[i]))
                            {
                                itsSysXfsConfigFile=constXfsConfigFiles[i];
                                itsSysXfs=true;
                                break;
                            }
                }
            }
        }
    }
}

const QString & CConfig::getRealTopDir(const QString &f)
{
    return CMisc::root() || CMisc::getSect(f)==i18n("System")
               ? itsSysFontsDir
               : itsUserFontsDir;
}

void CConfig::storeSysXConfigFileTs()
{
    int ts=CMisc::getTimeStamp(itsSysXConfigFile);

    if(0!=ts)
    {
        KConfigGroupSaver saver(this, KCONFIG_GROUP);

        writeEntry("SysXConfigFileTs", ts);
    }
}
