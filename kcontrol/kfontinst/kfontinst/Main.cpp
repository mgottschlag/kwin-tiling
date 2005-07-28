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
// Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2003, 2004
////////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "Misc.h"
#include "FontEngine.h"
#include "Fontmap.h"
#include "XConfig.h"
#include "kxftconfig.h"
#include <fontconfig/fontconfig.h>
#include <qfile.h>
#include <stdio.h>

//
// Bug#99335 Solaris 2.6 does not have getopt.h :-(
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else
#include <unistd.h>
#endif
#include <iostream>

#define KFI_XF86CFG "XF86Config"
#define KFI_XORGCFG "xorg.conf"

static const char * getFile(const char *entry, const char **posibilities)
{
    if(KFI::Misc::fExists(entry))
        return entry;
    else
    {
        int f;

        for(f=0; posibilities[f]; ++f)
            if(KFI::Misc::fExists(posibilities[f]))
                break;

        return posibilities[f];
    }
}

static const char * constXConfigFiles[]=
{   
    "/etc/X11/"KFI_XORGCFG,
    "/etc/X11/"KFI_XORGCFG"-4",
    "/etc/"KFI_XORGCFG,
    "/usr/X11R6/etc/X11/"KFI_XORGCFG,
    "/usr/X11R6/etc/X11/"KFI_XORGCFG"-4",
    "/usr/X11R6/lib/X11/"KFI_XORGCFG,
    "/usr/X11R6/lib/X11/"KFI_XORGCFG"-4",

    "/etc/X11/"KFI_XF86CFG"-4",
    "/etc/X11/"KFI_XF86CFG,
    "/etc/"KFI_XF86CFG"-4",
    "/etc/"KFI_XF86CFG,
    "/usr/X11R6/etc/X11/"KFI_XF86CFG"-4",
    "/usr/X11R6/etc/X11/"KFI_XF86CFG,
    "/usr/X11R6/lib/X11/"KFI_XF86CFG"-4",
    "/usr/X11R6/lib/X11/"KFI_XF86CFG,

    NULL
};  

static const char * constXfsConfigFiles[]=
{
    "/etc/X11/fs/config",
    "/usr/openwin/lib/X11/fonts/fontserver.cfg",
    NULL
};

KFI::CXConfig * getXCfg(bool root)
{
    if(root)
    {
        //
        // Try to determine location for X and xfs config files...
        // ...note on some systems (Solaris and HP-UX) only the xfs file will be found
        bool          xfs=false;
        KFI::CXConfig *xcfg=NULL;
        QString       xConfigFile=getFile(QFile::encodeName(constXConfigFiles[0]), constXConfigFiles),
                      xfsConfigFile=getFile(QFile::encodeName(constXfsConfigFiles[0]), constXfsConfigFiles);
            
        // If found xfs, but not X - then assume that xfs is being used...
        if(!xfsConfigFile.isEmpty() && xConfigFile.isEmpty())
            xfs=true;
        else if(!xConfigFile.isEmpty()) // Read xConfig file to determine which one...
        {
            xcfg=new KFI::CXConfig(KFI::CXConfig::X11, xConfigFile);

            if(!xfsConfigFile.isEmpty() && xcfg->xfsInPath())
            {
                delete xcfg;
                xfs=true;
            }
        }

        // OK, if still set to X11 config, but this mentions fontconfig FPE, then delete - as we're not interested
        // anymore...
        if(xcfg && xcfg->fcInPath())
            delete xcfg;

        return xfs ? new KFI::CXConfig(KFI::CXConfig::XFS, xfsConfigFile) : xcfg;
    }

    return NULL;
}

static void usage(char *app)
{
    std::cerr << "Usage: " << app << " [OPTIONS]... [FOLDER]..." << std::endl
              << std::endl
              << "  Helper application for KDE's fonts:/ ioslave." << std::endl
              << std::endl
#ifdef HAVE_GETOPT_H
              << "  -x, --configure_x        Configure FOLDER for regular x - i.e." << std::endl
              << "                           create fonts.dir, fonts.scale and encodngs.dir" << std::endl
              << std::endl
              << "  -g, --configure_gs       Create Fontmap file. If run as root, then " << std::endl
              << "                           no paramter is required as all fonts are " << std::endl
              << "                           configured, and Fontmap placed in /etc/fonts" << std::endl
              << "                           For non-root, fonts located in FOLDER are" << std::endl
              << "                           configured, and Fontmap placed there." << std::endl
              << std::endl
              << "  -f, --add_to_fc_cfg      Add FOLDER to fontconfig config files." << std::endl
              << std::endl
              << "  -a, --add_to_x_cfg       Add FOLDER to X config files only when run as root.," << std::endl
              << std::endl
              << "  -r, --refresh_x          Refresh X." << std::endl
              << std::endl
              << "  -s, --refresh_xfs        Refresh Xfs." << std::endl
#else
              << "  -x                       Configure FOLDER for regular x - i.e." << std::endl
              << "                           create fonts.dir, fonts.scale and encodngs.dir" << std::endl
              << std::endl
              << "  -g                       Create Fontmap file. If run as root, then " << std::endl
              << "                           no paramter is required as all fonts are " << std::endl
              << "                           configured, and Fontmap placed in /etc/fonts" << std::endl
              << "                           For non-root, fonts located in FOLDER are" << std::endl
              << "                           configured, and Fontmap placed there." << std::endl
              << std::endl
              << "  -f                       Add FOLDER to fontconfig config files." << std::endl
              << std::endl
              << "  -a                       Add FOLDER to X config files only when run as root.," << std::endl
              << std::endl
              << "  -r                       Refresh X." << std::endl
              << std::endl
              << "  -s                       Refresh Xfs." << std::endl
#endif
              << std::endl
              << std::endl
              << "  (C) Craig Drummond, 2003, 2004." << std::endl
              << std::endl;

    exit(-1);
}

void refresh(bool refreshX, bool refreshXfs, bool root)
{
    if(refreshX)
        KFI::CXConfig::refreshPaths(false);
    if(refreshXfs && root)
        KFI::CXConfig::refreshPaths(true);
}

int main(int argc, char *argv[])
{
#ifdef HAVE_GETOPT_H
    static struct option options[]=
    {
        { "configure_x",    0, 0, 'x' },
        { "configure_gs",   0, 0, 'g' },
        { "add_to_fc_cfg",  0, 0, 'f' },
        { "add_to_x_cfg",   0, 0, 'a' },
        { "refresh_x",      0, 0, 'r' },
        { "refresh_xfs",    0, 0, 's' },
        { 0,                0, 0, 0   }
    };
#endif

    int  c=0,
         rv=0;
    bool doX=false,
         doGs=false,
         addToX=false,
         addToFc=false,
         refreshX=false,
         refreshXfs=false,
         root=KFI::Misc::root();

#ifdef HAVE_GETOPT_H
    int optIndex;
    while(-1!=(c=getopt_long(argc, argv, "xgfars", options, &optIndex)))
#else
    while(-1!=(c=getopt(argc, argv, "xgfars")))
#endif
        switch(c)
        {
            case 'x':
                doX=true;
                break;
            case 'g':
                doGs=true;
                break;
            case 'f':
                addToFc=true;
                break;
            case 'a':
                addToX=true;
                break;
            case 'r':
                refreshX=true;
                break;
            case 's':
                refreshXfs=true;
                break;
            case '?':
                usage(argv[0]);
                break;
        }

    int  left=argc-optind;
    bool folderRequired=doX || addToX || addToFc || (!root && doGs);

    if (left>1 || (0==left && folderRequired) || (!doX && !doGs && !addToX && !addToFc))
        usage(argv[0]);
    else
    {
        QString folder;

        if(folderRequired)
        {
            folder=argv[optind];
            unsigned int len=folder.length();

            // Remove quotes...
            if( (folder[0]==QChar('\'') || folder[0]==QChar('\"')) &&
                (folder[len-1]==QChar('\'') || folder[len-1]==QChar('\"')))
                folder=folder.mid(1, len-2);
            folder=KFI::Misc::dirSyntax(folder);
        }

        if(folderRequired && !KFI::Misc::dExists(folder))
        {
            std::cerr << "ERROR: " << QFile::encodeName(folder).constData() << " does not exist!" << std::endl;
            rv=-2;
        }
        else
        {
            if(!folder.isEmpty())
            {
                if(0==rv && addToFc)
                {
                    //
                    // Only add folder to fontconfig's config if its not already there...
                    FcStrList *list=FcConfigGetFontDirs(FcConfigGetCurrent());
                    FcChar8   *dir;
                    bool      found=false;

                    while((dir=FcStrListNext(list)))
                        if(0==KFI::Misc::dirSyntax((const char *)dir).find(folder))
                            found=true;

                    if(!found)
                    {
                        KXftConfig *xft=new KXftConfig(KXftConfig::Dirs, root);

                        xft->addDir(folder);
                        rv=xft->apply() ? 0 : -3;
                        delete xft;
                   }
                }

                if(0==rv && addToX && root)
                {
                    KFI::CXConfig *x=NULL;

                    if((x=getXCfg(true)))
                    {
                        x->addPath(folder);
                        rv=x->writeConfig() ? 0 : -4;
                        delete x;
                    }
                    else
                        rv=-5;
                }
            }

            if(0==rv && (doX || doGs))
            {
                KFI::CFontEngine fe;

                if(0==rv && doX)
                    rv=KFI::CXConfig::configureDir(folder, fe) ? 0 : -5;

                refresh(refreshX, refreshXfs, root);

                if(0==rv && doGs)
                    rv=KFI::Fontmap::create(root ? QString::null : folder, fe) ? 0 : -6;
            }
            else if(0==rv)
                refresh(refreshX, refreshXfs, root);
        }
    }

    return rv;
}
