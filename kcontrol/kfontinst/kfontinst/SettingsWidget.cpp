////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CSettingsWidget
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

#include "SettingsWidget.h"
#include "KfiGlobal.h"
#include "Config.h"
#include "Misc.h"
#include "XConfig.h"
#include "Encodings.h"
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <qframe.h>
#include <qsizepolicy.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <ctype.h>
#include <stdlib.h>
#include <fstream>

void CSettingsWidget::reset()
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

    if(CKfiGlobal::cfg().getSOConfigure())
    {
        itsCheck->setChecked(true);
        itsDirButton->setEnabled(true);
        itsPpdCombo->setEnabled(true);
    }
    else
    {
        itsCheck->setChecked(false);
        itsDirButton->setEnabled(false);
        itsPpdCombo->setEnabled(false);
    }
    itsDirText->setText(CKfiGlobal::cfg().getSODir());

    itsDirButton->setPixmap(KGlobal::iconLoader()->loadIcon("fileopen", KIcon::Small));
    setupPpdCombo();

    itsGenAfmsCheck->setChecked(CKfiGlobal::cfg().getDoAfm());
    itsT1AfmCheck->setChecked(CKfiGlobal::cfg().getDoT1Afms());
    itsTtAfmCheck->setChecked(CKfiGlobal::cfg().getDoTtAfms());
    switch(CKfiGlobal::cfg().getXRefreshCmd())
    {
        default:
        case CConfig::XREFRESH_XSET_FP_REHASH:
            itsXsetRadio->setChecked(true);
            break;
        case CConfig::XREFRESH_XFS_RESTART:
            itsXfsRadio->setChecked(true);
            break;
        case CConfig::XREFRESH_CUSTOM:
            itsCustomRadio->setChecked(true);
    }
    itsRestartXfsCommand->setText(CKfiGlobal::cfg().getCustomXRefreshCmd());
    scanEncodings();
}

void CSettingsWidget::encodingsDirButtonPressed()
{
    QString dir=KFileDialog::getExistingDirectory(i18n(CConfig::constNotFound.utf8())==itsEncodingsDirText->text() ? QString::null : itsEncodingsDirText->text(),
                                                  this, i18n("Select Encodings Folder"));

    if(QString::null!=dir && dir!=itsEncodingsDirText->text())
    {
        itsEncodingsDirText->setText(dir);
        CKfiGlobal::cfg().setEncodingsDir(dir);
        CKfiGlobal::enc().clear();
        CKfiGlobal::enc().addDir(dir);
        scanEncodings();
        emit madeChanges();
    }
}

void CSettingsWidget::gsFontmapButtonPressed()
{
    QString file=KFileDialog::getSaveFileName(i18n(CConfig::constNotFound.utf8())==itsGhostscriptFileText->text() ? QString::null : itsGhostscriptFileText->text(),
                                              "Fontmap*", this, i18n("Select Ghostscript \"Fontmap\""));

    if(QString::null!=file && file!=itsGhostscriptFileText->text())
    {
        bool ok=false;

        if(!(CMisc::fExists(file)))
            if(CMisc::dWritable(CMisc::getDir(file)))
                ok=KMessageBox::questionYesNo(this, i18n("File does not exist.\n"
                                                         "Create new file?"), i18n("File error"))==KMessageBox::Yes ? true : false;
            else
                KMessageBox::error(this, i18n("File does not exist "
                                              "and folder is not writeable."), i18n("File error"));
        else
            ok=true;

        if(ok)
        {
            setGhostscriptFile(file);
            emit madeChanges();
        }
    }
}

void CSettingsWidget::cupsButtonPressed()
{
    QString dir=KFileDialog::getExistingDirectory(i18n(CConfig::constNotFound.utf8())==itsCupsDirText->text() ? QString::null : itsCupsDirText->text(),
                                                  this, i18n("Select CUPS Folder"));

    if(QString::null!=dir && dir!=itsCupsDirText->text())
    {
        itsCupsDirText->setText(dir);
        CKfiGlobal::cfg().setCupsDir(dir);
    }
}

void CSettingsWidget::xDirButtonPressed()
{
    QString dir=KFileDialog::getExistingDirectory(i18n(CConfig::constNotFound.utf8())==itsFontsDirText->text() ? QString::null : itsFontsDirText->text(),
                                                  this, i18n("Select Fonts Folder"));

    if(QString::null!=dir && dir!=itsFontsDirText->text())
    {
        itsFontsDirText->setText(dir);
        CKfiGlobal::cfg().setFontsDir(dir);
        setupSubDirCombos();
        emit madeChanges();
    }
}

void CSettingsWidget::xConfigButtonPressed()
{
    QString file=KFileDialog::getSaveFileName(i18n(CConfig::constNotFound.utf8())==itsXConfigFileText->text() ? QString::null : itsXConfigFileText->text(),
                                              NULL, this, i18n("Select X config file"));

    if(QString::null!=file && file!=itsXConfigFileText->text())
    {
        bool ok=false;

        if(!(CMisc::fExists(file)))
            if(CMisc::dWritable(CMisc::getDir(file)))
                ok=KMessageBox::questionYesNo(this, i18n("File does not exist.\n"
                                                         "Create new file?"), i18n("File error"))==KMessageBox::Yes ? true : false;
            else
                KMessageBox::error(this, i18n("File does not exist "
                                              "and folder is not writeable."), i18n("File error"));
        else
            ok=true;

        if(ok)
        {
            setXConfigFile(file);
            emit madeChanges();
            if(!CKfiGlobal::xcfg().ok())
                KMessageBox::information(this, i18n("File format not recognized!\n"
                                                    "Advanced mode folder operations will not be available."));
        }
    }
}

void CSettingsWidget::setupSubDirCombos()
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

void CSettingsWidget::t1SubDir(const QString &str)
{
    CKfiGlobal::cfg().setT1SubDir(str);
    emit madeChanges();
}

void CSettingsWidget::ttSubDir(const QString &str)
{
    CKfiGlobal::cfg().setTTSubDir(str);
    emit madeChanges();
}

void CSettingsWidget::ghostscriptChecked(bool on)
{
    CKfiGlobal::cfg().setDoGhostscript(on);
    if(!on)
        cupsChecked(false);
    emit madeChanges();
}

void CSettingsWidget::cupsChecked(bool on)
{
    CKfiGlobal::cfg().setDoCups(on);
    if(on)
        ghostscriptChecked(true);
    emit madeChanges();
}

void CSettingsWidget::setGhostscriptFile(const QString &f)
{
    itsGhostscriptFileText->setText(f);
    CKfiGlobal::cfg().setGhostscriptFile(f);
}

void CSettingsWidget::setXConfigFile(const QString &f)
{
    itsXConfigFileText->setText(f);
    CKfiGlobal::cfg().setXConfigFile(f);
    CKfiGlobal::xcfg().readConfig();
}

void CSettingsWidget::dirButtonPressed()
{
    QString dir=KFileDialog::getExistingDirectory(i18n(CConfig::constNotFound.utf8())==itsDirText->text() ? QString::null : itsDirText->text(),
                                                  this, i18n("Select StarOffice Folder"));

    if(QString::null!=dir && dir!=itsDirText->text())
    {
        itsDirText->setText(dir);
        CKfiGlobal::cfg().setSODir(dir);
        setupPpdCombo();
        emit madeChanges();
    }
}

void CSettingsWidget::configureSelected(bool on)
{
    itsDirButton->setEnabled(on);
    itsPpdCombo->setEnabled(on);
    itsCheck->setChecked(on);
    CKfiGlobal::cfg().setSOConfigure(on);

    if(on)
        itsGenAfmsCheck->setChecked(true);
    emit madeChanges();
}

static bool isAPpd(const char *fname)
{
    int  len=strlen(fname);

    return (len>3 && fname[len-3]=='.' && tolower(fname[len-2])=='p' && tolower(fname[len-1])=='s');
}

static const char * getName(const QString &file)
{
    std::ifstream     f(file.local8Bit());
    const char * retVal="<Unknown>";

    if(f)
    {
        const int constMaxLen=256;
        const int constMaxLines=100;  // Only look at first 100 lines, to speed things up...

        static char name[constMaxLen];

        char buffer[constMaxLen],
             *str,
             *ch;
        bool found=false;
        int  count=0;

        do
        {
            f.getline(buffer, constMaxLen);
            count++;
            if(f.good())
            {
                buffer[constMaxLen-1]='\0'; // Just to make sure...

                if((str=strstr(buffer, "*ModelName: \""))!=NULL)
                {
                    str+=strlen("*ModelName: \"");

                    if((ch=strchr(str, '\"'))!=NULL)
                    {
                        strncpy(name, str, ch-str);
                        name[ch-str]='\0';
                        retVal=name;
                        found=true;
                    }
                }
            }
            else
                break;
        }
        while(!f.eof() && !found && count<constMaxLines);
    }

    return retVal;
}

static QString removeInfo(const QString &entry)
{
    QString      copy(entry);
    int openBPos=copy.find('(');

    if(openBPos>=0)
    {
        copy.remove(0, openBPos+1);

        int closeBPos=copy.find(')');

        if(closeBPos>=0)
            copy.remove(closeBPos, 1);
    }

    return copy;
}

void CSettingsWidget::ppdSelected(const QString &str)
{
    CKfiGlobal::cfg().setSOPpd(removeInfo(str));
    emit madeChanges();
}

void CSettingsWidget::setupPpdCombo()
{
    itsPpdCombo->clear();

    QDir dir(CKfiGlobal::cfg().getSODir() + "xp3/ppds/");

    if(!dir.isReadable())
        dir.setPath(CKfiGlobal::cfg().getSODir() + "share/xp3/ppds/");

    if(dir.isReadable())
    {
        const QFileInfoList *files=dir.entryInfoList();

        if(files)
        {
            const unsigned int constMaxStrLen = 40;

            QFileInfoListIterator it(*files);
            QFileInfo             *fInfo;
            QStringList           list;

            for(; NULL!=(fInfo=it.current()); ++it)
                if("."!=fInfo->fileName() && ".."!=fInfo->fileName())
                    if(!fInfo->isDir() && isAPpd(fInfo->fileName().local8Bit()))
                    {
                        QString entry(getName(fInfo->filePath()));

                        if(entry.length()+fInfo->fileName().length()+3>constMaxStrLen)
                        {
                            entry.truncate(constMaxStrLen-(fInfo->fileName().length()+3+3));
                            entry+="...";
                        }
                        entry+=" (" + fInfo->fileName() + ")";
                        list.append(entry);
                    }
            list.sort();
            itsPpdCombo->insertStringList(list);
        }

        int i;

        for(i=0; i<itsPpdCombo->count(); ++i)
            if(CKfiGlobal::cfg().getSOPpd()==removeInfo(itsPpdCombo->text(i)))
            {
                itsPpdCombo->setCurrentItem(i);
                break;
            }

        CKfiGlobal::cfg().setSOPpd(removeInfo(itsPpdCombo->currentText())); // Just in case the above didn't match the cfg entry
    }
}

void CSettingsWidget::generateAfmsSelected(bool on)
{
    CKfiGlobal::cfg().setDoAfm(on);
    itsGenAfmsCheck->setChecked(on);
    itsT1AfmCheck->setChecked(CKfiGlobal::cfg().getDoT1Afms());
    itsTtAfmCheck->setChecked(CKfiGlobal::cfg().getDoTtAfms());

    if(!on)
        itsCheck->setChecked(false);
    emit madeChanges();
}

void CSettingsWidget::customXRefreshSelected(bool on)
{
    if(on)
    {
        CKfiGlobal::cfg().setXRefreshCmd(CConfig::XREFRESH_CUSTOM);
        emit madeChanges();
    }
}

void CSettingsWidget::xfsRestartSelected(bool on)
{
    if(on)
    {
        CKfiGlobal::cfg().setXRefreshCmd(CConfig::XREFRESH_XFS_RESTART);
        emit madeChanges();
    }
}

void CSettingsWidget::xsetFpRehashSelected(bool on)
{
    if(on)
    {
        CKfiGlobal::cfg().setXRefreshCmd(CConfig::XREFRESH_XSET_FP_REHASH);
        emit madeChanges();
    }
}

void CSettingsWidget::customXStrChanged(const QString &str)
{
    CKfiGlobal::cfg().setCustomXRefreshCmd(str);
    emit madeChanges();
}

void CSettingsWidget::afmEncodingSelected(const QString &str)
{
    CKfiGlobal::cfg().setAfmEncoding(str);
    emit madeChanges();
}

void CSettingsWidget::t1AfmSelected(bool on)
{
    CKfiGlobal::cfg().setDoT1Afms(on);

    if(!on && !CKfiGlobal::cfg().getDoTtAfms())
        generateAfmsSelected(false);
    emit madeChanges();
}

void CSettingsWidget::ttAfmSelected(bool on)
{
    CKfiGlobal::cfg().setDoTtAfms(on);

    if(!on && !CKfiGlobal::cfg().getDoT1Afms())
        generateAfmsSelected(false);
    emit madeChanges();
}

void CSettingsWidget::scanEncodings()
{
    const CEncodings::T8Bit  *enc8;
    const CEncodings::T16Bit *enc16;
    QStringList              list,
                             afmEncs;

    for(enc8=CKfiGlobal::enc().first8Bit(); NULL!=enc8; enc8=CKfiGlobal::enc().next8Bit())
    {
        list.append(enc8->name);
        afmEncs.append(enc8->name);
    }

    for(enc16=CKfiGlobal::enc().first16Bit(); NULL!=enc16; enc16=CKfiGlobal::enc().next16Bit())
        list.append(enc16->name);

    list.append(CEncodings::constUnicodeStr);
    list.sort();
    afmEncs.sort();
    itsAfmEncodingCombo->insertStringList(afmEncs);

    int i;

    for(i=0; i<itsAfmEncodingCombo->count(); ++i)
        if(CKfiGlobal::cfg().getAfmEncoding()==itsAfmEncodingCombo->text(i))
        {
            itsAfmEncodingCombo->setCurrentItem(i);
            break;
        }

    CKfiGlobal::cfg().setAfmEncoding(itsAfmEncodingCombo->currentText()); // Ditto
}

#include "SettingsWidget.moc"
