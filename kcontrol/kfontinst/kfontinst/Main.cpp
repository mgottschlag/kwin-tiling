////////////////////////////////////////////////////////////////////////////////
//
// File Name     : Main.cpp
// Author        : Craig Drummond
// Project       : K Font Installer
// Creation Date : 20/03/2003
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

#include "Global.h"
#include "Misc.h"
#include "Config.h"
#include "Fontmap.h"
#include "XConfig.h"
#include "kxftconfig.h"
#include <kinstance.h>
#include <qstring.h>
#include <qstringlist.h>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef HAVE_FONTCONFIG
#define XFT_CACHE_CMD "fc-cache"
#else
#define XFT_CACHE_CMD "xftcache"
#endif

static int kfi_rmdir(const char *dir)
{
    QString ds(CMisc::dirSyntax(dir));

    CMisc::removeAssociatedFiles(dir, true);

    int rv=rmdir(dir);

    if(0==rv)
    {
        KInstance kinst("kfontinst");

        CGlobal::create(false, true);

        CGlobal::xcfg().addPath(CGlobal::cfg().getUserFontsDir());
        CGlobal::xcfg().removePath(ds);
        CGlobal::xft().addDir(CGlobal::cfg().getUserFontsDir());
        CGlobal::xft().removeDir(ds);
        rv=CGlobal::xcfg().writeConfig() && CGlobal::xft().apply() ? 0 : -4;

#ifdef HAVE_FONTCONFIG
        CMisc::doCmd(XFT_CACHE_CMD, CMisc::xDirSyntax(CGlobal::cfg().getUserFontsDir()));
#else
        CMisc::doCmd(XFT_CACHE_CMD, CMisc::xDirSyntax(ds));
#endif

        if(CMisc::root())
            CGlobal::cfg().storeSysXConfigFileTs();

        CGlobal::destroy();
    }
    
    return rv;
}

static int kfi_mkdir(const char *dir, bool make=true)
{
    QString ds(CMisc::dirSyntax(dir));
    int     rv=!make || CMisc::createDir(ds) ? 0 : -2;

    if(0==rv)
    {
        KInstance kinst("kfontinst");

        CGlobal::create(false, true);

        chmod(dir, CMisc::DIR_PERMS);
        CGlobal::xcfg().addPath(CGlobal::cfg().getUserFontsDir());
        CGlobal::xcfg().addPath(ds);

#ifdef HAVE_FONTCONFIG
        CXConfig::configureDir(ds);
#else
        QStringList symFamilies;
        CXConfig::configureDir(ds, symFamilies);  // symFamilies will be empty -> new dir!
#endif

        CFontmap::createLocal(ds);
        CGlobal::xft().addDir(CGlobal::cfg().getUserFontsDir());
        CGlobal::xft().addDir(ds);
        rv=CGlobal::xcfg().writeConfig() && CGlobal::xft().apply() ? 0 : -4;

#ifdef HAVE_FONTCONFIG
        CMisc::doCmd(XFT_CACHE_CMD, CMisc::xDirSyntax(CGlobal::cfg().getUserFontsDir()));
#else
        CMisc::doCmd(XFT_CACHE_CMD, CMisc::xDirSyntax(ds));
#endif 

        if(CMisc::root())
            CGlobal::cfg().storeSysXConfigFileTs();
        CMisc::setTimeStamps(ds);

        CGlobal::destroy();
    }
    
    return rv;
}

static int kfi_cfgdir(const char *dir)
{
    KInstance kinst("kfontinst");

    CGlobal::create(true, false);
    
    QString     ds(CMisc::dirSyntax(dir));
#ifdef HAVE_FONTCONFIG
    int         rv=CXConfig::configureDir(ds) ? 0 : -2;
#else
    QStringList x11SymFamilies; 
    int         rv=CXConfig::configureDir(ds, x11SymFamilies) ? 0 : -2;
#endif

    CFontmap::createLocal(ds);

    if(0==rv)
    {
#ifndef HAVE_FONTCONFIG
        QStringList           xftSymFamilies=CGlobal::xft().getSymbolFamilies();
        QStringList::Iterator it;
        bool                  saveXft=false;  

        for(it=x11SymFamilies.begin(); it!=x11SymFamilies.end(); ++it)
            if(-1==xftSymFamilies.findIndex(*it))
            {
                CGlobal::xft().addSymbolFamily(*it);
                saveXft=true;
            }
        if(saveXft)
            CGlobal::xft().apply();
#endif

#ifdef HAVE_FONTCONFIG
        CMisc::doCmd(XFT_CACHE_CMD, CMisc::xDirSyntax(CGlobal::cfg().getUserFontsDir()));
#else
        CMisc::doCmd(XFT_CACHE_CMD, CMisc::xDirSyntax(ds));
#endif 
    }
    CMisc::setTimeStamps(ds);

    CGlobal::destroy();

    return rv;
}

// AFMs are now displayed => user must ask to delete...
//static int kfi_rmfont(const char *font)
//{
//    if(0==unlink(font))
//    {
//         CMisc::removeAssociatedFiles(font);
//        return 0;
//    }
//    return -2;
//}
#define kfi_rmfont(font) unlink(font)

static int kfi_install(const char *from, const char *to)
{
    if(CMisc::doCmd("cp", "-f", from, to))
    {
        chmod(to, CMisc::FILE_PERMS);
        return 0;
    }
    return -2;
}

static int kfi_rename(const char *from, const char *to)
{
    bool isDir=CMisc::dExists(from);
    int  rv=CMisc::doCmd("mv", "-f", from, to) ? 0 : -2;

    if(0==rv && isDir)
    {
        QString   toDs(CMisc::dirSyntax(to)),
                  fromDs(CMisc::dirSyntax(from));
        KInstance kinst("kfontinst");
        
        CGlobal::create(false, true);

        CGlobal::xcfg().addPath(CGlobal::cfg().getUserFontsDir());
        CGlobal::xft().addDir(CGlobal::cfg().getUserFontsDir());

        if(!CMisc::hidden(to, true))
        {
            CGlobal::xcfg().addPath(toDs);

#ifdef HAVE_FONTCONFIG
            CXConfig::configureDir(toDs);
#else
            QStringList symFamilies;
            CXConfig::configureDir(toDs, symFamilies);  // symFamilies will be empty -> new dir!
#endif
            CGlobal::xft().addDir(toDs);
        }
        if(!CMisc::hidden(from, true))
        {
            CGlobal::xcfg().removePath(fromDs);
            CGlobal::xft().removeDir(fromDs);
        }

        int slash=fromDs.findRev('/');

        if(-1!=slash)
            slash=fromDs.findRev('/', slash-1);

        rv=CGlobal::xcfg().writeConfig() && CGlobal::xft().apply() ? 0 : -4;

#ifdef HAVE_FONTCONFIG
        CMisc::doCmd(XFT_CACHE_CMD, CMisc::xDirSyntax(CGlobal::cfg().getUserFontsDir()));
#else
        if(-1!=slash)
            CMisc::doCmd(XFT_CACHE_CMD, CMisc::xDirSyntax(fromDs.left(slash+1)));
        CMisc::doCmd(XFT_CACHE_CMD, CMisc::xDirSyntax(toDs));
#endif

        if(CMisc::root())
            CGlobal::cfg().storeSysXConfigFileTs();
        CGlobal::destroy();
    }

    return rv;
}

int main(int argc, char *argv[])
{
    int rv=0;

    switch(argc)
    {
        case 2:
            if(0==CMisc::stricmp(argv[1], "refresh"))
            {
                CGlobal::sysXcfg().refreshPaths();
                CFontmap::createTopLevel();
            }
            else if(0==CMisc::stricmp(argv[1], "createfontmap"))
                CFontmap::createTopLevel();
            else
                rv=-1;
            break;
        case 3:
            if(0==CMisc::stricmp(argv[1], "mkdir"))
                rv=kfi_mkdir(argv[2]);
            if(0==CMisc::stricmp(argv[1], "adddir"))
                rv=kfi_mkdir(argv[2], false);
            else if(0==CMisc::stricmp(argv[1], "rmdir"))
                rv=kfi_rmdir(argv[2]);
            else if(0==CMisc::stricmp(argv[1], "cfgdir"))
                rv=kfi_cfgdir(argv[2]);
            else if(0==CMisc::stricmp(argv[1], "rmfont"))
                rv=kfi_rmfont(argv[2]);
            else
                rv=-1;
            break;
        case 4:
            if(0==CMisc::stricmp(argv[1], "install"))
                rv=kfi_install(argv[2], argv[3]);
            else if(0==CMisc::stricmp(argv[1], "rename"))
                rv=kfi_rename(argv[2], argv[3]);
            else
                rv=-1;
            break;
        default:
            rv=-1;
    }

    if(-1==rv)
        std::cerr << "Usage: " << argv[0] << "<command> <dir/font> [<to>]\n"
                  << "       " << argv[0] << "refresh\n"
                  << "       " << argv[0] << "createfontmap\n"
                  << "           Commands:\n"
                  << "               mkdir     Create directory, and add to config files\n"
                  << "               rmdir     Remove directory, and remove from config files\n"
                  << "               adddir    Add a directory to config files\n"
                  << "               cfgdir    Configure directory, i.e. create fonts.dir, etc.\n"
                  << "               rmfont    Remove a font, and associated files.\n"
                  << "               rename    Rename a font or directory.\n"
                  << "               install   Add a font to <to>.\n";

    return rv;
}

