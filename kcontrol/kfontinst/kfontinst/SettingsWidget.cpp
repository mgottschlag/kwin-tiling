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
#include <klineedit.h>
#include <ctype.h>
#include <stdlib.h>
#include <fstream>
#include <qgroupbox.h>

void CSettingsWidget::reset()
{
    if(CMisc::root())
    {
        itsFontsDirReq->setShowLocalProtocol(false);
        itsFontsDirReq->setURL(CKfiGlobal::cfg().getFontsDir());
        itsFontsDirReq->lineEdit()->setReadOnly(true);

        itsEncodingsDirReq->setShowLocalProtocol(false);
        itsEncodingsDirReq->setURL(CKfiGlobal::cfg().getEncodingsDir());
        itsEncodingsDirReq->lineEdit()->setReadOnly(true);

        itsGhostscriptFileReq->setShowLocalProtocol(false);
        itsGhostscriptFileReq->setURL(CKfiGlobal::cfg().getGhostscriptFile());
        itsGhostscriptFileReq->lineEdit()->setReadOnly(true);
        itsGhostscriptCheck->setChecked(CKfiGlobal::cfg().getDoGhostscript());
        itsGhostscriptFileReq->setEnabled(CKfiGlobal::cfg().getDoGhostscript());

        itsXConfigFileReq->setShowLocalProtocol(false);
        itsXConfigFileReq->setURL(CKfiGlobal::cfg().getXConfigFile());
        itsXConfigFileReq->lineEdit()->setReadOnly(true);

        itsCupsDirReq->setShowLocalProtocol(false);
        itsCupsDirReq->setURL(CKfiGlobal::cfg().getCupsDir());
        itsCupsDirReq->lineEdit()->setReadOnly(true);
        itsCupsCheck->setChecked(CKfiGlobal::cfg().getDoCups());
        itsCupsDirReq->setEnabled(CKfiGlobal::cfg().getDoCups());
        setupSubDirCombos();
        itsXRefreshCombo->setCurrentItem(CKfiGlobal::cfg().getXRefreshCmd());
        itsRestartXfsCommand->setEnabled(CConfig::XREFRESH_CUSTOM==CKfiGlobal::cfg().getXRefreshCmd());
        itsRestartXfsCommand->setText(CKfiGlobal::cfg().getCustomXRefreshCmd());
    }
    else
    {
        itsPrintingGroupBox->hide();
        itsXSettingsGroupBox->hide();
    }

    itsCheck->setChecked(CKfiGlobal::cfg().getSOConfigure());
    itsStarOfficeDirReq->setEnabled(CKfiGlobal::cfg().getSOConfigure());
    itsPpdCombo->setEnabled(CKfiGlobal::cfg().getSOConfigure());

    itsStarOfficeDirReq->setShowLocalProtocol(false);
    itsStarOfficeDirReq->setURL(CKfiGlobal::cfg().getSODir());
    itsStarOfficeDirReq->lineEdit()->setReadOnly(true);

    setupPpdCombo();

    itsGenAfmsCheck->setChecked(CKfiGlobal::cfg().getDoAfm());
    itsT1AfmCheck->setChecked(CKfiGlobal::cfg().getDoT1Afms());
    itsTtAfmCheck->setChecked(CKfiGlobal::cfg().getDoTtAfms());
    scanEncodings();
}

void CSettingsWidget::encodingsDirButtonPressed()
{
    KFileDialog *dlg = itsEncodingsDirReq->fileDialog();
    dlg->setMode(KFile::Directory | KFile::LocalOnly);
    dlg->setCaption(i18n("Select Encodings Folder"));
}

void CSettingsWidget::encodingsDirChanged(const QString& dir)
{
    CKfiGlobal::cfg().setEncodingsDir(dir);
    CKfiGlobal::enc().clear();
    CKfiGlobal::enc().addDir(dir);
    scanEncodings();
    emit madeChanges();
}

void CSettingsWidget::gsFontmapButtonPressed()
{
    KFileDialog *dlg = itsGhostscriptFileReq->fileDialog();
    dlg->setMode(KFile::File | KFile::LocalOnly);
    dlg->setCaption(i18n("Select Ghostscript \"Fontmap\""));
    dlg->setFilter("Fontmap");
}

void CSettingsWidget::gsFontmapChanged(const QString &file)
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

void CSettingsWidget::cupsButtonPressed()
{
    KFileDialog *dlg = itsCupsDirReq->fileDialog();
    dlg->setMode(KFile::Directory | KFile::LocalOnly);
    dlg->setCaption(i18n("Select CUPS Folder"));
}

void CSettingsWidget::cupsChanged(const QString &dir)
{
    CKfiGlobal::cfg().setCupsDir(dir);
    emit madeChanges();
}

void CSettingsWidget::xDirButtonPressed()
{
    KFileDialog *dlg = itsFontsDirReq->fileDialog();
    dlg->setMode(KFile::Directory | KFile::LocalOnly);
    dlg->setCaption(i18n("Select Fonts Folder"));
}

void CSettingsWidget::xDirChanged(const QString &dir)
{
    CKfiGlobal::cfg().setFontsDir(dir);
    setupSubDirCombos();
    emit madeChanges();
}

void CSettingsWidget::xConfigButtonPressed()
{
    KFileDialog *dlg = itsXConfigFileReq->fileDialog();
    dlg->setMode(KFile::File | KFile::LocalOnly);
    dlg->setCaption(i18n("Select X config file"));
}

void CSettingsWidget::xConfigChanged(const QString &file)
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

void CSettingsWidget::dirButtonPressed()
{
    KFileDialog *dlg = itsStarOfficeDirReq->fileDialog();
    dlg->setMode(KFile::Directory | KFile::LocalOnly);
    dlg->setCaption(i18n("Select StarOffice Folder"));
}

void CSettingsWidget::dirChanged(const QString &dir)
{
    CKfiGlobal::cfg().setSODir(dir);
    setupPpdCombo();
    emit madeChanges();
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
    itsGhostscriptFileReq->setURL(f);
    CKfiGlobal::cfg().setGhostscriptFile(f);
}

void CSettingsWidget::setXConfigFile(const QString &f)
{
    itsXConfigFileReq->setURL(f);
    CKfiGlobal::cfg().setXConfigFile(f);
    CKfiGlobal::xcfg().readConfig();
}


void CSettingsWidget::configureSelected(bool on)
{
    itsStarOfficeDirReq->setEnabled(on);
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

void CSettingsWidget::xRefreshSelected(int val)
{
    if(val!=(int)CKfiGlobal::cfg().getXRefreshCmd())
    {
        CKfiGlobal::cfg().setXRefreshCmd((CConfig::EXFontListRefresh)val);
        emit madeChanges();
        itsRestartXfsCommand->setEnabled(CConfig::XREFRESH_CUSTOM==CKfiGlobal::cfg().getXRefreshCmd());
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
