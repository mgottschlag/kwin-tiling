////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CSysConfigurer
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 03/05/2001
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

#include "SysConfigurer.h"
#include "Misc.h"
#include "KfiGlobal.h"
#include "Config.h"
#include "FontEngine.h"
#include "XConfig.h"
#include <stdlib.h>
#include <qwidget.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <qdir.h>
#include <qstringlist.h>
#include <qfileinfo.h>
#include "kxftconfig.h"

static const char * constFinishedStr = "Finished";

CSysConfigurer::CSysConfigurer(QWidget *parent)
              : QObject(parent, NULL),
                itsParent(parent)
{
    connect(&itsAfm, SIGNAL(step(const QString &)), SLOT(step(const QString &)));
    connect(&CKfiGlobal::xcfg(), SIGNAL(step(const QString &)), SLOT(step(const QString &)));
    connect(&itsFm, SIGNAL(step(const QString &)), SLOT(step(const QString &)));
    connect(&itsSo, SIGNAL(step(const QString &)), SLOT(step(const QString &)));
}

static void getTTandT1Dirs(const QString &dir, QStringList &list, int level=0)
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

                if(CMisc::dContainsTTorT1Fonts(dir))
                    list.append(dir);
 
                for(; NULL!=(fInfo=it.current()); ++it)
                    if("."!=fInfo->fileName() && ".."!=fInfo->fileName())
                        if(fInfo->isDir())
                            getTTandT1Dirs(fInfo->filePath()+"/", list, level+1);
            }
        }
    }
}

void CSysConfigurer::getTTandT1Dirs(QStringList &list)
{
    bool xConfig;
 
    if((xConfig=CKfiGlobal::xcfg().ok()))
        xConfig=CKfiGlobal::xcfg().getTTandT1Dirs(list);
 
    if(!xConfig)  // If can't get list from XConfig file - then look in ALL directories in the X fonts dir...
        ::getTTandT1Dirs(CKfiGlobal::cfg().getFontsDir(), list);
}

void CSysConfigurer::go()
{
    unsigned int totalSteps=0,
                 totalfonts=0,
                 totalTtFonts=0,
                 totalT1Fonts=0,
                 d;
    QStringList  symbolFamilies;

    if(CKfiGlobal::cfg().getModifiedDirs().count())
    {
        bool xConfig=CKfiGlobal::xcfg().ok();

        // Count number of fonts...
        for(d=0; d<CKfiGlobal::cfg().getModifiedDirs().count(); d++)
        {
            QDir dir(CKfiGlobal::cfg().getModifiedDirs()[d]);
 
            if(dir.isReadable())
            {
                const QFileInfoList *files=dir.entryInfoList();
                bool                hasFonts=false;
 
                if(files)
                {
                    QFileInfoListIterator it(*files);
                    QFileInfo             *fInfo;
 
                    for(; NULL!=(fInfo=it.current()); ++it)
                        if("."!=fInfo->fileName() && ".."!=fInfo->fileName())
                        {
                            if(CFontEngine::isATtf(fInfo->fileName().local8Bit()))
                                totalTtFonts++;

                            if(CFontEngine::isAType1(fInfo->fileName().local8Bit()))
                                totalT1Fonts++;

                            if(CFontEngine::isAFont(fInfo->fileName().local8Bit()))
                            {
                                totalfonts++;
                                hasFonts=true;
                            }
                        }
                }

                if(xConfig && hasFonts)  // Ensure XConfig file has dir listed...
                    CKfiGlobal::xcfg().addPath(CKfiGlobal::cfg().getModifiedDirs()[d]);
            }
        }

        totalSteps= (CKfiGlobal::cfg().getModifiedDirs().count()*2) + totalfonts; // For XConfig stuff (initial message, one-per-font, encodings.dir)

        if(CKfiGlobal::cfg().getDoGhostscript())                                  // Fontmap (initial message, one-per-font, Writing...)
        {
            totalSteps+=(CKfiGlobal::cfg().getModifiedDirs().count()*2) + totalTtFonts + totalT1Fonts;
            if(CMisc::root() && CKfiGlobal::cfg().getDoCups())
                totalSteps++;
        }

        if(CKfiGlobal::cfg().getDoAfm())                                          // Initial message, one-per-font...
        {
            totalSteps+=CKfiGlobal::cfg().getModifiedDirs().count();
            if(CKfiGlobal::cfg().getDoTtAfms())
                totalSteps+=totalTtFonts;
            if(CKfiGlobal::cfg().getDoT1Afms())
                totalSteps+=totalT1Fonts;
        }

        if(CKfiGlobal::cfg().getSOConfigure())                                    // Initial message, one-per-font, xprinter.prolog...
            totalSteps+=(CKfiGlobal::cfg().getModifiedDirs().count()*2) + totalTtFonts + totalT1Fonts;

    }

    if(CKfiGlobal::xcfg().madeChanges())
        totalSteps++;

    if(CKfiGlobal::cfg().getXRefreshCmd()!=CConfig::XREFRESH_CUSTOM ||
       (CKfiGlobal::cfg().getXRefreshCmd()==CConfig::XREFRESH_CUSTOM && CKfiGlobal::cfg().getCustomXRefreshCmd().length()>0))
        totalSteps++;

    totalSteps++; // For Xft...

    emit initProgress(i18n("Configuring System:"), totalSteps);

    for(d=0; d<CKfiGlobal::cfg().getModifiedDirs().count(); d++)
    {
        status(i18n("Configuring X (%1)...").arg(CMisc::shortName(CKfiGlobal::cfg().getModifiedDirs()[d])));
        if(!CKfiGlobal::xcfg().go(CKfiGlobal::cfg().getModifiedDirs()[d], symbolFamilies))
        {
            status(i18n("Could not configure X (%1)").arg(CMisc::shortName(CKfiGlobal::cfg().getModifiedDirs()[d])), QString::null, true);
            return;
        }
    }

    if(CKfiGlobal::xcfg().madeChanges())
    {
        status(i18n("Saving X configuration file..."));
        if(!CKfiGlobal::xcfg().writeConfig())
        {
            status(i18n("Could not save X configuration file"), i18n("File permissions?"), true);
            return;
        }
    }

    QStringList           dirs;
    KXftConfig            xft(KXftConfig::Dirs|KXftConfig::SymbolFamilies, CMisc::root());
    QStringList::Iterator it;

    getTTandT1Dirs(dirs);

    status(i18n("Saving XRender configuration file..."));

    xft.clearDirs();
    for(it=dirs.begin(); it!=dirs.end(); ++it)
        xft.addDir(*it);
    for(it=symbolFamilies.begin(); it!=symbolFamilies.end(); ++it)
        xft.addSymbolFamily(*it);

    if(!xft.apply())
    {
        status(i18n("Could not save XRender configuration file"), i18n("File permissions?"), true);
        return;
    }

    if(CKfiGlobal::cfg().getDoGhostscript())
    {
        for(d=0; d<CKfiGlobal::cfg().getModifiedDirs().count(); d++)
        {
            status(i18n("Creating Fontmap (%1)...").arg(CMisc::shortName(CKfiGlobal::cfg().getModifiedDirs()[d])));
            if(!itsFm.go(CKfiGlobal::cfg().getModifiedDirs()[d]))
            {
                status(i18n("Could not configure Fontmap (%1)").arg(CMisc::shortName(CKfiGlobal::cfg().getModifiedDirs()[d])), i18n("File permissions?"), true);
                return;
            }
        }

        if(CMisc::root() && CKfiGlobal::cfg().getDoCups())
        {
            const QString constCupsFontmap("pstoraster/Fontmap");

            QString   cupsFontmap=CKfiGlobal::cfg().getCupsDir()+constCupsFontmap;
            QFileInfo info(cupsFontmap);

            status(i18n("Configuring CUPS..."));
            if(cupsFontmap!=CKfiGlobal::cfg().getGhostscriptFile() && info.readLink()!=CKfiGlobal::cfg().getGhostscriptFile())
            {
                QString constExt(".bak");
                bool    moved=!CMisc::fExists(cupsFontmap+constExt) ?
                                  CMisc::moveFile(cupsFontmap, cupsFontmap+constExt) :
                                  false;

                if(moved)
                    if(!CMisc::linkFile(CKfiGlobal::cfg().getGhostscriptFile(), cupsFontmap))
                    {
                        status(i18n("Could not configure CUPS"), i18n("File permissions?"), true);
                        CMisc::moveFile(cupsFontmap+constExt, cupsFontmap);
                        return;
                    }
            }
        }
    }

    if(CKfiGlobal::cfg().getDoAfm())
        for(d=0; d<CKfiGlobal::cfg().getModifiedDirs().count(); d++)
        {
            CAfmCreator::EStatus st;

            status(i18n("Creating AFMs (%1)...").arg(CMisc::shortName(CKfiGlobal::cfg().getModifiedDirs()[d])));
            if((st=itsAfm.go(CKfiGlobal::cfg().getModifiedDirs()[d]))!=CAfmCreator::SUCCESS)
            {
                status(i18n("Could not create AFM files (%1)").arg(CMisc::shortName(CKfiGlobal::cfg().getModifiedDirs()[d])), CAfmCreator::statusToStr(st), true);
                return;
            }
        }

    if(CKfiGlobal::cfg().getSOConfigure())
        for(d=0; d<CKfiGlobal::cfg().getModifiedDirs().count(); d++)
        {
            CStarOfficeConfig::EStatus st;

            status(i18n("Configuring StarOffice (%1)...").arg(CMisc::shortName(CKfiGlobal::cfg().getModifiedDirs()[d])));
            if((st=itsSo.go(CKfiGlobal::cfg().getModifiedDirs()[d]))!=CStarOfficeConfig::SUCCESS)
            {
                status(i18n("Could not configure StarOffice (%1)").arg(CMisc::shortName(CKfiGlobal::cfg().getModifiedDirs()[d])), CStarOfficeConfig::statusToStr(st), true);
                return;
            }
        }

        status(constFinishedStr);
}

void CSysConfigurer::status(const QString &str, const QString &errorMsg, bool error)
{
    if(error)
    {
        emit stopProgress();

        KMessageBox::error(itsParent, str+ (QString::null==errorMsg ? QString::null : "\n("+errorMsg+")"), i18n("Error"));
    }
    else
        if(QString(constFinishedStr)!=str)
            emit progress(str);
        else
        {
            if(CKfiGlobal::cfg().getXRefreshCmd()!=CConfig::XREFRESH_CUSTOM ||
               (CKfiGlobal::cfg().getXRefreshCmd()==CConfig::XREFRESH_CUSTOM && CKfiGlobal::cfg().getCustomXRefreshCmd().length()>0))
            {
                QString cmd;
                bool    ok=false;

                emit progress(i18n("Refreshing X font list"));

                CKfiGlobal::xcfg().refreshPaths();

                switch(CKfiGlobal::cfg().getXRefreshCmd())
                {
                    default:
                    case CConfig::XREFRESH_XSET_FP_REHASH:
                        ok=CMisc::doCmd("xset", "fp", "rehash");
                        break;
                    case CConfig::XREFRESH_XFS_RESTART:
                        ok=CMisc::doCmd("/etc/rc.d/init.d/xfs", "restart");
                        break;
                    case CConfig::XREFRESH_CUSTOM:
                        ok=CMisc::doCmdStr(CKfiGlobal::cfg().getCustomXRefreshCmd());
                        break;
                }

                if(ok)
                    emit successful();
                else
                    KMessageBox::error(itsParent, i18n("There was an error using the X refresh command"), i18n("Error"));
            }
            else
                emit successful();

            emit stopProgress();
        }
}
#include "SysConfigurer.moc"
