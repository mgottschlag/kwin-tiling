////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CDirSettingsWidget
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

#include "DirSettingsWidget.h"
#include "KfiGlobal.h"
#include "Config.h"
#include "Encodings.h"
#include "XConfig.h"
#include "Misc.h"
#include <kglobal.h>
#include <kiconloader.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qcheckbox.h>

CDirSettingsWidget::CDirSettingsWidget(QWidget *parent, const char *name)
                  : CDirSettingsWidgetData(parent, name)
{
    itsFontsDirText->setText(CKfiGlobal::cfg().getFontsDir());
    itsEncodingsDirText->setText(CKfiGlobal::cfg().getEncodingsDir());
    itsGhostscriptFileText->setText(CKfiGlobal::cfg().getGhostscriptFile());
    itsGhostscriptCheck->setChecked(CKfiGlobal::cfg().getDoGhostscript());
    itsXConfigFileText->setText(CKfiGlobal::cfg().getXConfigFile());
    itsFontsDirButton->setPixmap(KGlobal::iconLoader()->loadIcon("fileopen", KIcon::Small));
    itsEncodingsDirButton->setPixmap(KGlobal::iconLoader()->loadIcon("fileopen", KIcon::Small));
    itsGhostscriptFileButton->setPixmap(KGlobal::iconLoader()->loadIcon("fileopen", KIcon::Small));
    itsXConfigFileButton->setPixmap(KGlobal::iconLoader()->loadIcon("fileopen", KIcon::Small));

    if(CMisc::root())
    {
        itsCupsDirText->setText(CKfiGlobal::cfg().getCupsDir());
        itsCupsCheck->setChecked(CKfiGlobal::cfg().getDoCups());
        itsCupsDirButton->setPixmap(KGlobal::iconLoader()->loadIcon("fileopen", KIcon::Small));
    }
    else
    {
        itsCupsDirText->hide();
        itsCupsCheck->hide();
        itsCupsDirButton->hide();
    }

    setupSubDirCombos();
}
 
void CDirSettingsWidget::encodingsDirButtonPressed()
{
    QString dir=KFileDialog::getExistingDirectory(CConfig::constNotFound==itsEncodingsDirText->text() ? QString::null : itsEncodingsDirText->text(),
                                                  this, i18n("Select Encodings Folder"));
 
    if(QString::null!=dir && dir!=itsEncodingsDirText->text())
    {
        itsEncodingsDirText->setText(dir);
        CKfiGlobal::cfg().setEncodingsDir(dir);
        CKfiGlobal::enc().clear();
        CKfiGlobal::enc().addDir(dir);
        emit encodingsDirChanged();
    }
}

void CDirSettingsWidget::gsFontmapButtonPressed()
{
    QString file=KFileDialog::getSaveFileName(CConfig::constNotFound==itsGhostscriptFileText->text() ? QString::null : itsGhostscriptFileText->text(),
                                              "Fontmap*", this, i18n("Select Ghostscript \"Fontmap\""));
 
    if(QString::null!=file && file!=itsGhostscriptFileText->text())
    {
        bool ok=false;

        if(!(CMisc::fExists(file)))
            if(CMisc::dWritable(CMisc::getDir(file)))
                ok=KMessageBox::questionYesNo(this, i18n("File does not exist.\n"
                                                         "Create new file?"), i18n("File error"))==KMessageBox::Yes ? true : false;
            else
                KMessageBox::error(this, i18n("File does not exist\n"
                                              "and directory is not writeable."), i18n("File error"));
        else
            ok=true;

        if(ok)
            setGhostscriptFile(file);
    }
}

void CDirSettingsWidget::cupsButtonPressed()
{
    QString dir=KFileDialog::getExistingDirectory(CConfig::constNotFound==itsCupsDirText->text() ? QString::null : itsCupsDirText->text(),
                                                  this, i18n("Select CUPS Folder"));

    if(QString::null!=dir && dir!=itsCupsDirText->text())
    {
        itsCupsDirText->setText(dir);
        CKfiGlobal::cfg().setCupsDir(dir);
    }
}

void CDirSettingsWidget::xDirButtonPressed()
{
    QString dir=KFileDialog::getExistingDirectory(CConfig::constNotFound==itsFontsDirText->text() ? QString::null : itsFontsDirText->text(),
                                                  this, i18n("Select Fonts Folder"));
 
    if(QString::null!=dir && dir!=itsFontsDirText->text())
    {
        itsFontsDirText->setText(dir);
        CKfiGlobal::cfg().setFontsDir(dir);
        setupSubDirCombos();
    }
}

void CDirSettingsWidget::xConfigButtonPressed()
{
    QString file=KFileDialog::getSaveFileName(CConfig::constNotFound==itsXConfigFileText->text() ? QString::null : itsXConfigFileText->text(),
                                              NULL, this, i18n("Select X config file"));

    if(QString::null!=file && file!=itsXConfigFileText->text())
    {
        bool ok=false;

        if(!(CMisc::fExists(file)))
            if(CMisc::dWritable(CMisc::getDir(file)))
                ok=KMessageBox::questionYesNo(this, i18n("File does not exist.\n"
                                                         "Create new file?"), i18n("File error"))==KMessageBox::Yes ? true : false;
            else
                KMessageBox::error(this, i18n("File does not exist\n"
                                              "and directory is not writeable."), i18n("File error"));
        else
            ok=true;

        if(ok)
        {
            setXConfigFile(file);
            if(!CKfiGlobal::xcfg().ok())
                KMessageBox::information(this, i18n("File format not recognized!\n"
                                                    "Advanced mode folder operations will not be available."));
        }
    }
}

void CDirSettingsWidget::setupSubDirCombos()
{
    itsTTCombo->clear();
    itsT1Combo->clear();

    QDir dir(CKfiGlobal::cfg().getFontsDir());

    if(dir.isReadable())
    {
        const QFileInfoList *files=dir.entryInfoList();
 
        if(files)
        {
            QFileInfoListIterator it(*files);
            QFileInfo             *fInfo;
            int                   d,
                                  sub,
                                  tt=-1,
                                  t1=-1;
 
            for(; NULL!=(fInfo=it.current()); ++it)
                if("."!=fInfo->fileName() && ".."!=fInfo->fileName())
                    if(fInfo->isDir())
                    {
                        itsTTCombo->insertItem(fInfo->fileName()+"/");
                        itsT1Combo->insertItem(fInfo->fileName()+"/");
                    }

            for(d=0; d<itsTTCombo->count() && (-1==t1 || -1==tt); ++d)
            {
                if(-1==tt)
                {
                    if(itsTTCombo->text(d)==CKfiGlobal::cfg().getTTSubDir())
                        tt=d;
                    else
                        for(sub=0; QString::null!=CConfig::constTTSubDirs[sub]; ++sub)
                            if(itsTTCombo->text(d)==CConfig::constTTSubDirs[sub])
                                tt=d;
                }
                if(-1==t1)
                {
                    if(itsT1Combo->text(d)==CKfiGlobal::cfg().getT1SubDir())
                        t1=d;
                    else
                        for(sub=0; QString::null!=CConfig::constT1SubDirs[sub]; ++sub)
                            if(itsT1Combo->text(d)==CConfig::constT1SubDirs[sub])
                                t1=d;
                }
            }
            if(-1==tt && -1!=t1)
                tt=t1;
            else
                if(-1!=tt && -1==t1)
                    t1=tt;
                else
                    if(-1==tt && -1==t1)
                        t1=tt=0;
            
            CKfiGlobal::cfg().setTTSubDir(itsTTCombo->text(tt));
            CKfiGlobal::cfg().setT1SubDir(itsT1Combo->text(t1));
            itsTTCombo->setCurrentItem(tt);
            itsT1Combo->setCurrentItem(t1);
        }
    }
}

void CDirSettingsWidget::t1SubDir(const QString &str)
{
    CKfiGlobal::cfg().setT1SubDir(str);
}

void CDirSettingsWidget::ttSubDir(const QString &str)
{
    CKfiGlobal::cfg().setTTSubDir(str);
}

void CDirSettingsWidget::ghostscriptChecked(bool on)
{
    CKfiGlobal::cfg().setDoGhostscript(on);
    if(!on)
        cupsChecked(false);
}

void CDirSettingsWidget::cupsChecked(bool on)
{
    CKfiGlobal::cfg().setDoCups(on);
    if(on)
        ghostscriptChecked(true);
}

void CDirSettingsWidget::setGhostscriptFile(const QString &f)
{
    itsGhostscriptFileText->setText(f);
    CKfiGlobal::cfg().setGhostscriptFile(f);
}
 
void CDirSettingsWidget::setXConfigFile(const QString &f)
{
    itsXConfigFileText->setText(f);
    CKfiGlobal::cfg().setXConfigFile(f);
    CKfiGlobal::xcfg().readConfig();
}

#include "DirSettingsWidget.moc"
