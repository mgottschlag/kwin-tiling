////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CInstalledFontListWidget
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 20/04/2001
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

#include "InstalledFontListWidget.h"
#include "KfiGlobal.h"
#include "Config.h"
#include "ErrorDialog.h"
#include "Misc.h"
#include "StarOfficeConfig.h"
#include "Ttf.h"
#include "AfmCreator.h"
#include "FontEngine.h"
#include "XConfig.h"
#include "KfiCmModule.h"
#include <qpushbutton.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qpopupmenu.h>
#include <qwhatsthis.h>
#include <qregexp.h>
#include <qvalidator.h>
#include <qdir.h>
#include <klineeditdlg.h>
#include "XftConfig.h"

CInstalledFontListWidget::CInstalledFontListWidget(QWidget *parent, const char *)
                        : CFontListWidget(parent, true, true, i18n("Install To:"), i18n("Rem&ove"), i18n("&Apply..."),
                          CKfiGlobal::cfg().getFontsDir(),
                          CKfiGlobal::cfg().getFontsDir(), i18n("X11 Fonts Directory"), "fonts")
{
    connect(itsButton1, SIGNAL(clicked()), SLOT(uninstall()));
    connect(itsButton2, SIGNAL(clicked()), SLOT(configure()));
    connect(itsList, SIGNAL(rightButtonClicked(QListViewItem *, const QPoint &, int)), SLOT(popupMenu(QListViewItem *, const QPoint &, int)));

    setCfgButtonState(CKfiGlobal::cfg().getModifiedDirs().count()>0 || CKfiGlobal::cfg().firstTime() ? true : false);

    QWhatsThis::add(itsButton2, i18n("Installing and uninstalling the fonts only\n"
                                     "copies them to / removes them from the X\n"
                                     "fonts folder. In order to make X, Ghostscript,\n"
                                     "and StarOffice aware of the changes you\n"
                                     "must \"configure\" your system - this will\n"
                                     "create the neccessary configuration files\n"
                                     "needed by the various programs."));

    itsFontsPopup=new QPopupMenu(this);
    itsFixTtfPsNamesME=itsFontsPopup->insertItem(i18n("Fix TTF postscript names..."), this, SLOT(fixTtfPsNames()));

    itsDirsPopup=new QPopupMenu(this);
    itsCreateDirME=itsDirsPopup->insertItem(i18n("Create new sub-folder..."), this, SLOT(createDir()));
    itsDeleteDirME=itsDirsPopup->insertItem(i18n("Delete folder..."), this, SLOT(deleteDir())); 
    itsDirsPopup->insertSeparator();
    itsEnableDirME=itsDirsPopup->insertItem(i18n("Add to X font path"), this, SLOT(toggleDir()));
    itsDisableDirME=itsDirsPopup->insertItem(i18n("Remove from X font path"), this, SLOT(toggleDir()));
    itsDirsPopup->insertSeparator();
    itsTouchME=itsDirsPopup->insertItem(i18n("\"Touch\" folder"), this, SLOT(touchDir()));
    itsDirsPopup->insertSeparator();
    itsSetUnscaledME=itsDirsPopup->insertItem(i18n("Set unscaled"), this, SLOT(toggleUnscaled()));
    itsSetScaledME=itsDirsPopup->insertItem(i18n("Set scaled"), this, SLOT(toggleUnscaled()));
}

QString CInstalledFontListWidget::currentDir()
{
    CFontListWidget::CListViewItem *current=(CFontListWidget::CListViewItem *)itsList->currentItem();

    if(current)
        return current->dir();
    else
        return CKfiGlobal::cfg().getFontsDir();
}

void CInstalledFontListWidget::rescan(bool advancedMode, const QString &dir1)
{
    if(advancedMode!=itsAdvancedMode ||
       (!itsAdvancedMode && dir1!=itsBasicData.dir) ||
       (itsAdvancedMode && dir1!=itsAdvancedData.dir1))
    {
        itsAdvancedMode=advancedMode;
        itsBasicData.dir=dir1;
        itsAdvancedData.dir1=dir1;
        scan();
    }
}

void CInstalledFontListWidget::configure()
{
    if(CKfiGlobal::cfg().getModifiedDirs().count()<=0 && !CKfiGlobal::xcfg().madeChanges()
#ifdef HAVE_XFT
                                                      && !CKfiGlobal::xft().madeChanges()
#endif
                                                      && !CKfiGlobal::cfg().firstTime())
        KMessageBox::information(this, i18n("No changes have been made."));
    else
        if(KMessageBox::questionYesNo(this, i18n("Apply system changes now?\n"
                                                 "\n"
                                                 "This will create various required configuration files.\n"
                                                 "You do not need to do this after each install/uninstall -\n"
                                                 "it is only necessary after you have finished."),
                                            i18n("Configure System"))==KMessageBox::Yes)
            emit configureSystem();
}

void CInstalledFontListWidget::uninstall()
{
    bool dir=getNumSelectedDirs() ? true : false;
    int  res;

    if((res=KMessageBox::warningYesNoCancel(this, (dir ? i18n("Uninstall selected folder (and ALL of its contents)?") :
                                                         i18n("Uninstall selected fonts?") ) +
                                                  i18n("\n\nPlease select uninstall method :"
                                                       "\n(\"Move\" will move the selected items to \"%1\")").arg(CKfiGlobal::cfg().getUninstallDir()),
                                                  i18n("Remove"), i18n("&Move"), i18n("&Delete")))!=KMessageBox::Cancel)
    {
        int           failures=0,
                      successes=0;
        CListViewItem *item=(CListViewItem *)itsList->firstChild(),
                      *firstSel=getFirstSelectedItem();

        CKfiGlobal::errorDialog().clear();

        progressInit(i18n("Uninstalling:"), dir ? NULL!=firstSel ? CMisc::countFonts(firstSel->fullName())+1 : 0 : getNumSelectedFonts());

        while(item!=NULL)
        {
            if(item->isSelected())
            {
                EStatus status;
                QString parentDir=dir ? NULL!=item->parent() ? ((CListViewItem *)item->parent())->dir()
                                                             : CKfiGlobal::cfg().getFontsDir()
                                      : QString::null;


                if(SUCCESS==(status=dir ?
                                     uninstallDir(parentDir, item->text(0), res==KMessageBox::No) :
                                     uninstall(item->dir(), QString::null, item->text(0), true, res==KMessageBox::No)))
                {
                    CListViewItem *tmp=(CListViewItem *)item->itemBelow();

                    if(dir)
                    {
                        CKfiGlobal::cfg().removeModifiedDir(item->fullName());
                        CKfiGlobal::xcfg().removePath(item->fullName());
                    }
                    else
                        CKfiGlobal::cfg().addModifiedDir(item->dir());
                    delete item;
                    item=tmp;
                    successes++;
                }
                else
                {
                    CKfiGlobal::errorDialog().add(item->text(0), statusToStr(status));
                    failures++;
                    item=(CListViewItem *)item->itemBelow();
                }
            }
            else
                item=(CListViewItem *)item->itemBelow();
        }

        progressStop();

        if(failures)
            CKfiGlobal::errorDialog().open(i18n("The following items could not be uninstalled:"));
        else
            itsButton1->setEnabled(false);

        if(successes)
            enableCfgButton();
    }
}

CInstalledFontListWidget::EStatus CInstalledFontListWidget::uninstall(const QString &dir, const QString &sub, const QString &file, bool deleteAfm, bool uninstIsDel)
{
    EStatus status=PERMISSION_DENIED;

    progressShow(itsAdvancedMode ? dir+sub+file : file);

    if(uninstIsDel)
        status=CMisc::removeFile(dir+sub+file) ? SUCCESS : PERMISSION_DENIED;
    else
        status=CMisc::moveFile(dir+sub+file, CKfiGlobal::cfg().getUninstallDir()+sub) ? SUCCESS : PERMISSION_DENIED;

    if(SUCCESS==status && deleteAfm)
    {
        if(!uninstIsDel)
            emit fontMoved(file, dir+sub, CKfiGlobal::cfg().getUninstallDir()+sub);

        if(CMisc::fExists(CMisc::afmName(dir+sub+file)))
            uninstall(dir, sub, CMisc::afmName(file), false, uninstIsDel);
        CStarOfficeConfig::removeAfm(dir+sub+file);
    }
 
    return status;
}

CInstalledFontListWidget::EStatus CInstalledFontListWidget::uninstallDir(const QString &top, const QString &sub, bool uninstIsDel)
{
    EStatus status=PERMISSION_DENIED;

    progressShow(sub);

    if(CMisc::dHasSubDirs(top+sub+"/"))
        status=HAS_SUB_DIRS;
    else
        if(!CMisc::dWritable(top+sub))
            status=PERMISSION_DENIED;
        else
        {
            bool created=false;

            status=uninstIsDel || 
                   CMisc::dExists(CKfiGlobal::cfg().getUninstallDir()+sub) ||
                   (created=CMisc::createDir(CKfiGlobal::cfg().getUninstallDir()+sub)) ? 
                       SUCCESS : COULD_NOT_CREATE_DIR;

            if(created)
                emit dirMoved(top, sub);

            if(SUCCESS==status) 
            {
                QDir dir(top+sub+"/");
 
                if(dir.isReadable())
                {
                    const QFileInfoList *files=dir.entryInfoList();

                    if(files)
                    {
                        QFileInfoListIterator it(*files);
                        QFileInfo             *fInfo;

                        for(; NULL!=(fInfo=it.current()) && SUCCESS==status; ++it)
                            if("."!=fInfo->fileName() && ".."!=fInfo->fileName())
                                if(!fInfo->isDir())
                                    if(CFontEngine::isAFont(fInfo->fileName()))
                                        status=uninstall(top, sub+"/", fInfo->fileName(), true, uninstIsDel);
                                    else
                                        if(uninstIsDel)
                                            CMisc::removeFile(top+sub+"/"+fInfo->fileName());
                                        else
                                            CMisc::moveFile(top+sub+"/"+fInfo->fileName(), CKfiGlobal::cfg().getUninstallDir()+sub);
                    }
                }

                if(SUCCESS==status)
                    status=CMisc::removeDir(top+sub) ? SUCCESS : COULD_NOT_DELETE_DIR;
            }
        }
 
    return status;
}

void CInstalledFontListWidget::addFont(const QString &path, const QString &file)
{
    CFontListWidget::addFont(path, file);
    CKfiGlobal::cfg().addModifiedDir(path);
    enableCfgButton();
}

void CInstalledFontListWidget::addSubDir(const QString &top, const QString &sub)
{
    CFontListWidget::addSubDir(top, sub);
    CKfiGlobal::cfg().addModifiedDir(top+sub+"/");
    CKfiGlobal::xcfg().addPath(top+sub+"/");
    enableCfgButton();
}

void CInstalledFontListWidget::popupMenu(QListViewItem *item, const QPoint &point, int)
{
    if(itsAdvancedMode && NULL!=item)
    {
        if(!item->isSelected())
            itsList->setSelected(item, true);

        if(((CListViewItem *)item)->getType()==CListViewItem::FONT)
        {
            int numTT,
                numT1;

            getNumSelected(numTT, numT1);
            if(numTT)
            {
                itsFontsPopup->setItemEnabled(itsFixTtfPsNamesME, numTT ? true : false);
                itsFontsPopup->popup(point);
            }
        }
        else
            if(CKfiGlobal::xcfg().ok() && CKfiGlobal::xcfg().writable())
            {
                bool showMenu=false,
                     fontsDir=((CListViewItem *)item)->fullName()==CKfiGlobal::cfg().getFontsDir();

                if(NULL==item->parent())
                {
                    itsDirsPopup->setItemEnabled(itsEnableDirME, false);
                    itsDirsPopup->setItemEnabled(itsDisableDirME, false);
                    itsDirsPopup->setItemEnabled(itsSetScaledME, false);
                    itsDirsPopup->setItemEnabled(itsSetUnscaledME, false);
                    itsDirsPopup->setItemEnabled(itsTouchME, false);
                }
                else
                {
                    showMenu=true;

                    if(CKfiGlobal::xcfg().inPath(((CListViewItem *)item)->fullName()))
                    {
                        bool unscaled=CKfiGlobal::xcfg().isUnscaled(((CListViewItem *)item)->fullName());

                        itsDirsPopup->setItemEnabled(itsEnableDirME, false);
                        itsDirsPopup->setItemEnabled(itsDisableDirME, true);

                        if(CKfiGlobal::xcfg().custom())
                        {
                            itsDirsPopup->setItemEnabled(itsSetScaledME, false);
                            itsDirsPopup->setItemEnabled(itsSetUnscaledME, false);
                        }
                        else
                        {
                            itsDirsPopup->setItemEnabled(itsSetScaledME, unscaled);
                            itsDirsPopup->setItemEnabled(itsSetUnscaledME, !unscaled);
                        }
                    }
                    else
                    {
                        itsDirsPopup->setItemEnabled(itsEnableDirME, true);
                        itsDirsPopup->setItemEnabled(itsDisableDirME, false);
                        itsDirsPopup->setItemEnabled(itsSetScaledME, false);
                        itsDirsPopup->setItemEnabled(itsSetUnscaledME, false);
                    }

                    itsDirsPopup->setItemEnabled(itsTouchME, !fontsDir);
                }

                if(CMisc::dWritable(((CListViewItem *)item)->fullName()))
                {
                    itsDirsPopup->setItemEnabled(itsCreateDirME, true);
                    itsDirsPopup->setItemEnabled(itsDeleteDirME, !fontsDir);
                    showMenu=true;
                }
                else
                {
                    itsDirsPopup->setItemEnabled(itsCreateDirME, false);
                    itsDirsPopup->setItemEnabled(itsDeleteDirME, false);
                }

                if(showMenu)
                    itsDirsPopup->popup(point);
            }
    }
}

void CInstalledFontListWidget::fixTtfPsNames()
{
    if(KMessageBox::questionYesNo(this, i18n("This will *permanently* alter the TrueType font file(s),"
                                             "\nand this cannot be undone."
                                             "\n"
                                             "\nAre you sure?"), i18n("Fix TTF postscript names"))==KMessageBox::Yes)
    {
        CListViewItem *item=(CListViewItem *)itsList->firstChild();
        int           failures=0,
                      numTT,
                      numT1;

        getNumSelected(numTT, numT1); 
 
        CKfiGlobal::errorDialog().clear();
        progressInit(i18n("Fixing:"), numTT); 

        while(NULL!=item)
        {
            if(item->isSelected() && item->getType()==CListViewItem::FONT && CFontEngine::isATtf(item->text(0)))
            {
                CTtf::EStatus status;

                progressShow(item->fullName());
                if(CTtf::SUCCESS!=(status=CKfiGlobal::ttf().fixPsNames(item->fullName())) && CTtf::NO_REMAP_GLYPHS!=status)
                {
                    CKfiGlobal::errorDialog().add(item->text(0), CTtf::toString(status));
                    failures++;
                }
            }
            item=(CListViewItem *)(item->itemBelow());
        }

        progressStop(); 
        if(failures)
            CKfiGlobal::errorDialog().open(i18n("The following files could not be modified:"));
    }
}

void CInstalledFontListWidget::toggleDir()
{
    CListViewItem *item=getFirstSelectedItem();
 
    if(NULL!=item && item->getType()==CListViewItem::DIR)
    {
        if(CKfiGlobal::xcfg().inPath(((CListViewItem *)item)->fullName()))
            CKfiGlobal::xcfg().removePath(((CListViewItem *)item)->fullName());
        else
            CKfiGlobal::xcfg().addPath(((CListViewItem *)item)->fullName());

        setCfgButton();
        item->repaint();
    }
}

void CInstalledFontListWidget::touchDir()
{
    CListViewItem *item=getFirstSelectedItem();
 
    if(NULL!=item && item->getType()==CListViewItem::DIR)
    {
        CKfiGlobal::cfg().addModifiedDir(((CListViewItem *)item)->fullName());
        setCfgButton();
    }
}

void CInstalledFontListWidget::toggleUnscaled()
{
    CListViewItem *item=getFirstSelectedItem();
 
    if(NULL!=item && item->getType()==CListViewItem::DIR)
    {
        CKfiGlobal::xcfg().setUnscaled(((CListViewItem *)item)->fullName(), !CKfiGlobal::xcfg().isUnscaled(((CListViewItem *)item)->fullName()));
        setCfgButton(); 
        item->repaint();
    }
}

void CInstalledFontListWidget::setCfgButton()
{
     setCfgButtonState(CKfiGlobal::cfg().getModifiedDirs().count()>0
                          || CKfiGlobal::xcfg().madeChanges()
#ifdef HAVE_XFT
                          || CKfiGlobal::xft().madeChanges()
#endif
                          || CKfiGlobal::cfg().firstTime());
}

void CInstalledFontListWidget::selectionChanged()
{
    CFontListWidget::selectionChanged();

    CListViewItem *item=getFirstSelectedItem();
    bool          enable=false;

    if(item)
    {
        enable=true;

        while(NULL!=item && enable)
        {
            if(item->isSelected())
                if(CListViewItem::DIR==item->getType())
                    enable=item->fullName()!=CKfiGlobal::cfg().getFontsDir() && CMisc::dWritable(item->fullName());
                else
                {
                    enable=CMisc::fWritable(item->fullName());

                    if(enable)
                    {
                        QString afm=CMisc::afmName(item->fullName());

                        if(CMisc::fExists(afm))
                            enable=CMisc::fWritable(afm);
                    }
                }

            item=(CListViewItem *)(item->itemBelow());
        }
    }

    itsButton1->setEnabled(enable); 
}

void CInstalledFontListWidget::setCfgButtonState(bool state)
{
    itsButton2->setEnabled(state);
    CKfiCmModule::madeChanges(state);
}

class CCreateDirDialog : public KLineEditDlg
{
    class CValidator : public QValidator
    {
        public:

        CValidator(QWidget *widget) : QValidator(widget) {}
        virtual ~CValidator() {}

        State validate(QString &input, int &) const
        {
            if(input.contains(QRegExp("[¬|!\"£$%&*:;@'`#~<>?/\\\\]")))
                return Invalid;
            else
                return Valid;
        }
    };

    public:

    CCreateDirDialog(QWidget *parent) : KLineEditDlg(i18n("Please type name of new folder:"), "", parent)
    {
        edit->setValidator(new CValidator(edit));
    }

    virtual ~CCreateDirDialog() {}
};

void CInstalledFontListWidget::createDir()
{
    CCreateDirDialog *dlg=new CCreateDirDialog(this);

    if(dlg->exec())
    {
        QString       dir=dlg->text().stripWhiteSpace();
        CListViewItem *item=getFirstSelectedItem();

        if(CMisc::dExists(item->dir()+dir))
            KMessageBox::error(this, i18n("Folder already exists!"), i18n("Error"));
        else
            if(CMisc::createDir(item->dir()+dir))
            {
                addSubDir(item->dir(), dir);
                CKfiGlobal::xcfg().addPath(item->dir()+dir+"/");
                CKfiGlobal::cfg().addModifiedDir(item->dir()+dir+"/");
            }
            else
                KMessageBox::error(this, i18n("Failed to create folder"), i18n("Error"));
    }

    delete dlg;
}

void CInstalledFontListWidget::deleteDir()
{
    if(KMessageBox::questionYesNo(this, i18n("Are you sure?"), i18n("Delete folder"))==KMessageBox::Yes)
    {
        CListViewItem *item=getFirstSelectedItem();

        if(item)
        {
            if(item->getType()!=CListViewItem::DIR)
                KMessageBox::error(this, i18n("Selected item is not a folder"), i18n("Error"));
            else
                if(!CMisc::dWritable(item->fullName()))
                    KMessageBox::error(this, i18n("Folder is not writeable"), i18n("Error"));
                else
                {
                    unsigned int numItems=CMisc::getNumItems(item->fullName()); 
                    bool         hasFontsDir=CMisc::fExists(item->fullName()+"fonts.dir"),
                                 hasEncDir=CMisc::fExists(item->fullName()+"encodings.dir"),
                                 hasAlias=CMisc::fExists(item->fullName()+"fonts.alias");

                    if(hasFontsDir)
                        numItems--;
                    if(hasEncDir)
                        numItems--;

                    if(numItems)
                        KMessageBox::error(this, i18n("Folder is not empty"), i18n("Error"));
                    else
                    {
                        bool error=false;

                        if(hasFontsDir)
                            if(!CMisc::fWritable(item->fullName()+"fonts.dir") || !CMisc::removeFile(item->fullName()+"fonts.dir"))
                            {
                                KMessageBox::error(this, i18n("Cannot delete the folder's fonts.dir file"), i18n("Error"));
                                error=true;
                            }
                            else
                            {
                                CKfiGlobal::cfg().addModifiedDir(item->fullName());  // Just in case fail to delete whole folder...
                                enableCfgButton();
                            }

                        if(!error && hasAlias)
                            if(!CMisc::fWritable(item->fullName()+"fonts.alias") || !CMisc::removeFile(item->fullName()+"fonts.alias"))
                            {
                                KMessageBox::error(this, i18n("Cannot delete the folder's fonts.alias file"), i18n("Error"));
                                error=true;
                            }
                            else
                            {
                                CKfiGlobal::cfg().addModifiedDir(item->fullName());  // Just in case fail to delete whole folder...
                                enableCfgButton();
                            }

                        if(!error && hasEncDir)
                            if(!CMisc::fWritable(item->fullName()+"encodings.dir") || !CMisc::removeFile(item->fullName()+"encodings.dir"))
                            {
                                KMessageBox::error(this, i18n("Cannot delete the folder's encodings.dir file"), i18n("Error"));
                                error=true;
                            }
                            else
                            {
                                CKfiGlobal::cfg().addModifiedDir(item->fullName());  // Just in case fail to delete whole folder...
                                enableCfgButton();
                            }


                        if(!error)
                            if(0!=CMisc::getNumItems(item->fullName()))  // Hmm it contains other items... (NOTE: This case should *not* happen!)
                                KMessageBox::error(this, i18n("Folder is not empty"), i18n("Error"));
                            else
                                if(CMisc::removeDir(item->fullName()))
                                {
                                    CKfiGlobal::cfg().removeModifiedDir(item->fullName());
                                    delete item;
                                    enableCfgButton();
                                }
                                else
                                    KMessageBox::error(this, i18n("Failed to delete folder"), i18n("Error"));
                    }
                }
        }
    }
}

QString CInstalledFontListWidget::statusToStr(EStatus status)
{
    switch(status)
    {
        case SUCCESS:
            return i18n("Success");
        case PERMISSION_DENIED:
            return i18n("Permission denied?");
        case HAS_SUB_DIRS:
            return i18n("Contains sub-folders");
        case COULD_NOT_CREATE_DIR:
            return i18n("Could not create folder to uninstall to");
        case COULD_NOT_DELETE_DIR:
            return i18n("Could not delete folder");
        default:
            return i18n("Unknown");
    }
}
#include "InstalledFontListWidget.moc"
