////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CStarOfficeSettingsWidget
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

#include "StarOfficeSettingsWidget.h"
#include "KfiGlobal.h"
#include "Config.h"
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qstringlist.h>
#include <string.h>
#include <ctype.h>
#include <fstream.h>

CStarOfficeSettingsWidget::CStarOfficeSettingsWidget(QWidget *parent, const char *name)
                         : CStarOfficeSettingsWidgetData(parent, name)
{
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
}
 
void CStarOfficeSettingsWidget::dirButtonPressed()
{
    QString dir=KFileDialog::getExistingDirectory(CConfig::constNotFound==itsDirText->text() ? QString::null : itsDirText->text(),
                                                  this, i18n("Select StarOffice Folder"));
 
    if(QString::null!=dir && dir!=itsDirText->text())
    {
        itsDirText->setText(dir);
        CKfiGlobal::cfg().setSODir(dir);
        setupPpdCombo();
    }
}

void CStarOfficeSettingsWidget::configureSelected(bool on)
{
    itsDirButton->setEnabled(on);
    itsPpdCombo->setEnabled(on);
    itsCheck->setChecked(on);
    CKfiGlobal::cfg().setSOConfigure(on);

    if(on)
        emit cfgSelected();
}

static bool isAPpd(const char *fname)
{
    int  len=strlen(fname);
 
    return (len>3 && fname[len-3]=='.' && tolower(fname[len-2])=='p' && tolower(fname[len-1])=='s');
}

static const char * getName(const QString &file)
{
    ifstream     f(file.local8Bit());
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

void CStarOfficeSettingsWidget::ppdSelected(const QString &str)
{
    CKfiGlobal::cfg().setSOPpd(removeInfo(str));
}

void CStarOfficeSettingsWidget::disable()
{
    configureSelected(false);
}

void CStarOfficeSettingsWidget::setupPpdCombo()
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
                    if(!fInfo->isDir() && isAPpd(fInfo->fileName()))
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
#include "StarOfficeSettingsWidget.moc"
