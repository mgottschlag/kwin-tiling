////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CDiskFontListWidget
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 21/04/2001
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

#include "DiskFontListWidget.h"
#include "KfiGlobal.h"
#include "Config.h"
#include "FontEngine.h"
#include "Misc.h"
#include "ErrorDialog.h"
#include "AfmCreator.h"
#include "Ttf.h"
#include "XConfig.h"
#include <klocale.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <stdlib.h>

CDiskFontListWidget::CDiskFontListWidget(QWidget *parent, const char *)
                   : CFontListWidget(parent, false, false, i18n("Install From:"), i18n("&Install"), i18n("Cha&nge Folder"),
                                     QString(getenv("HOME"))+"/", i18n("Home Directory"), "folder_home",
                                     "/", i18n("Root Directory"), "folder"),
                                     itsDestDir(QString::null)
{
    connect(itsButton1, SIGNAL(clicked()), SLOT(install()));
    connect(itsButton2, SIGNAL(clicked()), SLOT(changeDirectory()));
}

void CDiskFontListWidget::setDestDir(const QString &dir)
{
    if(itsAdvancedMode)
    {
        int numFonts=getNumSelectedFonts(),
            numDirs=getNumSelectedDirs();

        itsDestDir=dir;

        if(numFonts)
            itsButton1->setDisabled(itsDestDir.length()<=0 || CKfiGlobal::cfg().getFontsDir()==itsDestDir ? true : false);
        else
            if(numDirs)
            {
                CListViewItem *cur=getFirstSelectedItem();

                itsButton1->setDisabled(!CKfiGlobal::xcfg().ok() || !CKfiGlobal::xcfg().writable() || itsDestDir.length()<=0 ||
                                        NULL==cur || NULL==cur->parent() ? true : false);
            }
            else
                itsButton1->setDisabled(true);
    }
}

void CDiskFontListWidget::selectionChanged()
{
    CFontListWidget::selectionChanged();

    CListViewItem *cur=getFirstSelectedItem();
    bool          enable=false;

    if(cur)
        if(!itsAdvancedMode)
        {
            enable=true;

            while(NULL!=cur && enable)
            {
                if(cur->isSelected())
                {
                    if(CListViewItem::FONT==cur->getType())
                        if(CFontEngine::isATtf(cur->fullName()))
                        {
                            if(!CMisc::dWritable(CKfiGlobal::cfg().getFontsDir()+CKfiGlobal::cfg().getTTSubDir()))
                                enable=false;
                        }
                        else
                            if(!CMisc::dWritable(CKfiGlobal::cfg().getFontsDir()+CKfiGlobal::cfg().getT1SubDir()))
                                enable=false;
                }

                cur=(CListViewItem *)(cur->itemBelow());
            }
        }
        else
        {
            int numFonts=getNumSelectedFonts(),
                numDirs=getNumSelectedDirs();

            if(numFonts)
                enable=itsDestDir.length() && CMisc::dWritable(itsDestDir) && itsDestDir!=CKfiGlobal::cfg().getFontsDir() ? true : false;
            else
                if(numDirs)
                    enable=CKfiGlobal::xcfg().ok() && CKfiGlobal::xcfg().writable() && itsAdvancedMode &&
                           NULL!=cur->parent() && 
                           cur->getType()==CListViewItem::DIR && itsDestDir.length() && CMisc::dWritable(itsDestDir) ?
                               true : false;
        }

    itsButton1->setEnabled(enable);
}

void CDiskFontListWidget::fontMoved(const QString &font, const QString &, const QString &to)
{
    if(itsAdvancedMode || to==itsBasicData.dir)
        addFont(to, font);
}

void CDiskFontListWidget::dirMoved(const QString &top, const QString &sub)
{
    if(itsAdvancedMode)
        addSubDir(top, sub);
}

void CDiskFontListWidget::changeDirectory()
{
    QString dir=KFileDialog::getExistingDirectory(itsBasicData.dir, this, "Select Folder To Install From");
 
    if(QString::null!=dir && dir!=itsBasicData.dir)
    {
        itsBasicData.dir=dir;
        scan();
    }
}

void CDiskFontListWidget::rescan(bool advancedMode)
{
    if(advancedMode!=itsAdvancedMode)
    {
        itsAdvancedMode=advancedMode;
        scan();
    }
}

void CDiskFontListWidget::install()
{
    bool    dir=getNumSelectedDirs() ? true : false;
    QString q(dir ? i18n("Copy slected folder (and ALL of its contents) to:\n") : i18n("Copy selected fonts to:\n"));

    q+=i18n("X11 fonts folder");
    if(itsAdvancedMode)
        q+="/"+CMisc::shortName(itsDestDir);

    q+=i18n("\n\nAre you sure?");

    if(KMessageBox::questionYesNo(this, q, i18n("Install"))==KMessageBox::Yes)
    {
        int           failures=0,
                      successes=0;
        CListViewItem *item=(CListViewItem *)itsList->firstChild(),
                      *firstSel=getFirstSelectedItem();
 
        CKfiGlobal::errorDialog().clear();

        progressInit(i18n("Installing:"), dir ? NULL!=firstSel ? CMisc::countFonts(firstSel->fullName())+1 : 0 :  getNumSelectedFonts());

        while(item!=NULL)
        {
            if(item->isSelected())
            {
                QString destDir;
                EStatus status;

                if(itsAdvancedMode)
                    destDir=itsDestDir;
                else
                {
                    destDir=CKfiGlobal::cfg().getFontsDir();
                    if(CFontEngine::isATtf(item->text(0)))
                        destDir+=CKfiGlobal::cfg().getTTSubDir();
                    else
                        if(CFontEngine::isAType1(item->text(0)))
                            destDir+=CKfiGlobal::cfg().getT1SubDir();
                }

                if(SUCCESS==(status=dir ? installDir(item->dir(), destDir, item->text(0))
                                        : install(item->dir(), destDir, item->text(0)) ) )
                {
                    item->setSelected(false);
                    itsList->repaintItem(item);
                    successes++;
                    if(dir)
                        emit installDir(destDir, item->text(0));
                    else
                        emit installFont(destDir, item->text(0));
                }
                else
                {
                    CKfiGlobal::errorDialog().add(item->text(0), statusToStr(status));
                    failures++;
                }
            }
            item=(CListViewItem *)item->itemBelow();
        }

        progressStop();
  
        if(failures)
            CKfiGlobal::errorDialog().open(i18n("The following items could not be installed:"));
        else
            itsButton1->setEnabled(false);
    }
}

CDiskFontListWidget::EStatus CDiskFontListWidget::install(const QString &sourceDir, const QString &destDir, const QString &fname)
{
    EStatus status=PERMISSION_DENIED;

    progressShow(itsAdvancedMode ? sourceDir+fname : fname);

    // Check to make sure font is not already installed...
    if(CMisc::fExists(destDir+fname))
        status=ALREADY_INSTALLED;
    else
    {
        status=CMisc::copyFile(sourceDir+fname, destDir) ? SUCCESS : PERMISSION_DENIED;
        if(SUCCESS==status)
        {
            QString afm=CMisc::afmName(fname);

            if(CMisc::fExists(sourceDir+afm))
                CMisc::copyFile(sourceDir+afm, destDir);
 
            if(CFontEngine::isATtf(fname) && CKfiGlobal::cfg().getFixTtfPsNamesUponInstall())
                CKfiGlobal::ttf().fixPsNames(destDir+fname);
        }
    }
 
    return status;
}

CDiskFontListWidget::EStatus CDiskFontListWidget::installDir(const QString &sourceDir, const QString &destDir, const QString &sub)
{
    EStatus status=PERMISSION_DENIED;

    progressShow(sub);
 
    // Check to make sure font is not already installed...
    if(CMisc::dExists(destDir+sub))
        status=ALREADY_INSTALLED;
    else
    {
        if(CMisc::dHasSubDirs(sourceDir))
            status=HAS_SUB_DIRS;
        else
        {
            status=CMisc::createDir(destDir+sub) ? SUCCESS : COULD_NOT_CREATE_DIR;

            if(SUCCESS==status)
            {
                QDir dir(sourceDir);

                if(dir.isReadable())
                {
                    const QFileInfoList *files=dir.entryInfoList();
 
                    if(files)
                    {
                        QFileInfoListIterator it(*files);
                        QFileInfo             *fInfo;
 
                        for(; NULL!=(fInfo=it.current()) && SUCCESS==status; ++it)
                            if("."!=fInfo->fileName() && ".."!=fInfo->fileName())
                                if(!fInfo->isDir() && CFontEngine::isAFont(fInfo->fileName()))
                                    status=install(sourceDir, destDir+sub+"/", fInfo->fileName());
                    }
                }
            }
        }
    }
 
    return status;
}

QString CDiskFontListWidget::statusToStr(EStatus status)
{
    switch(status)
    {
        case SUCCESS:
            return i18n("Success");
        case PERMISSION_DENIED:
            return i18n("Permission denied?");
        case ALREADY_INSTALLED:
            return i18n("Already installed");
        case HAS_SUB_DIRS:
            return i18n("Contains sub-folders");
        case COULD_NOT_CREATE_DIR:
            return i18n("Could not create folder");
        default:
            return i18n("Unknown");
    }
}
#include "DiskFontListWidget.moc"
