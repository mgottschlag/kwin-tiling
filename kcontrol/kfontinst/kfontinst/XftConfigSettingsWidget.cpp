////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CXftConfigSettingsWidget
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 28/05/2001
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
///////////////////////////////////////////////////////////////////////////////

#include "XftConfigSettingsWidget.h"

#ifdef HAVE_XFT
#include "KfiGlobal.h"
#include "Config.h"
#include "Misc.h"
#include "SysConfigurer.h"
#include <kglobal.h>
#include <kiconloader.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <qvalidator.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qtimer.h>
#include <kapp.h>
#include "XftConfigRules.h"

CXftConfigSettingsWidget::CXftConfigSettingsWidget(QWidget *parent, const char *name)
                        : CXftConfigSettingsWidgetData(parent, name),
                          itsRulesDialog(NULL)
{
    bool wizard=(NULL!=name && NULL!=strstr(name, "Wizard")) ? true : false,
         xftMadeChanges=CKfiGlobal::xft().madeChanges();

    if(wizard)
    {
        itsAdvancedButton->hide();
        itsSaveButton->hide();
    }

    itsFromText->setValidator(new QDoubleValidator(itsFromText));
    itsToText->setValidator(new QDoubleValidator(itsToText));
    itsConfigFileButton->setPixmap(KGlobal::iconLoader()->loadIcon("fileopen", KIcon::Small));
    setWidgets();
    if(!wizard)
        itsSaveButton->setEnabled(xftMadeChanges);
}

void CXftConfigSettingsWidget::fileButtonPressed()
{
    QString file=KFileDialog::getSaveFileName(i18n(CConfig::constNotFound.utf8())==itsConfigFileText->text() ? QString::null : itsConfigFileText->text(),
                                              "XftConfig .xftconfig", this, i18n("Select Anti-Alias configuration file"));

    if(QString::null!=file && file!=itsConfigFileText->text())
    {
        bool ok=false;

        if(!(CMisc::fExists(file)))
        {
            if(CMisc::dWritable(CMisc::getDir(file)))
            {
                ok=KMessageBox::questionYesNo(this, i18n("File does not exist.\n"
                                                         "Create new file?"), i18n("File error"))==KMessageBox::Yes ? true : false;
                if(ok)
                    CKfiGlobal::xft().newFile();
            }
            else
                KMessageBox::error(this, i18n("File does not exist "
                                              "and directory is not writeable."), i18n("File error"));
        }
        else
        {
            ok=CKfiGlobal::xft().read(file);

            if(!ok)
            {
                KMessageBox::error(this, i18n("Parse error, "
                                              "incorrect file?"), i18n("File error"));
                CKfiGlobal::xft().read(CKfiGlobal::cfg().getXftConfigFile());
            }
            else
                if(!CMisc::fWritable(file))
                    KMessageBox::information(this, i18n("File is not writeable!\n"
                                                        "Modifications will not be possible."));
        }

        if(ok)
        {
            CKfiGlobal::cfg().setXftConfigFile(file);
            emit madeChanges();
            setWidgets();
        }
    }
}

void CXftConfigSettingsWidget::excludeRangeChecked(bool on)
{
    bool   changed=false;
    double from,
           to;

    itsFromText->setEnabled(on && itsExcludeRangeCheck->isChecked());
    itsToText->setEnabled(on && itsExcludeRangeCheck->isChecked());

    if(on)
    {
        if(!CKfiGlobal::xft().getExcludeRange(from, to) || from!=itsFromText->text().toDouble() || to!=itsToText->text().toDouble())
        {
            CKfiGlobal::xft().setExcludeRange(itsFromText->text().toDouble(), itsToText->text().toDouble());
            changed=true;
        }
    }
    else
        if(CKfiGlobal::xft().getExcludeRange(from, to))
        {
            CKfiGlobal::xft().removeExcludeRange();
            changed=true;
        }

    if(changed)
    {
        itsSaveButton->setEnabled(true);
        emit madeChanges();
    }
}

void CXftConfigSettingsWidget::setWidgets()
{
    double to,
           from;
    bool   fExists      = CMisc::fExists(CKfiGlobal::cfg().getXftConfigFile()),
           enableConfig = ( (fExists && CMisc::fWritable(CKfiGlobal::cfg().getXftConfigFile())) ||
                            (!fExists && CMisc::dWritable(CMisc::getDir(CKfiGlobal::cfg().getXftConfigFile()))));

    itsUseSubPixelHintingCheck->setEnabled(enableConfig);
    itsAdvancedButton->setEnabled(fExists);
    itsFromText->setEnabled(enableConfig && itsExcludeRangeCheck->isChecked());
    itsToText->setEnabled(enableConfig && itsExcludeRangeCheck->isChecked());

    itsConfigFileText->setText(CKfiGlobal::cfg().getXftConfigFile());

    if(CKfiGlobal::xft().getExcludeRange(from, to))
    {
        QString str;

        itsFromText->setText(str.setNum(from));
        itsToText->setText(str.setNum(to));
        itsExcludeRangeCheck->setChecked(true);
    }
    else
        itsExcludeRangeCheck->setChecked(false);

    itsExcludeRangeCheck->setEnabled(enableConfig);

    itsUseSubPixelHintingCheck->setChecked(CKfiGlobal::xft().getUseSubPixelHinting());
}

void CXftConfigSettingsWidget::fromChanged(const QString &str)
{
    if(itsExcludeRangeCheck->isChecked())
    {
        double from,
               to;

        if(!CKfiGlobal::xft().getExcludeRange(from, to) || from!=str.toDouble())
        {
            CKfiGlobal::xft().setExcludeRange(str.toDouble(), itsToText->text().toDouble());
            itsSaveButton->setEnabled(true);
            emit madeChanges();
        }
    }
}

void CXftConfigSettingsWidget::toChanged(const QString &str)
{
    if(itsExcludeRangeCheck->isChecked())
    {
        double from,
               to;

        if(!CKfiGlobal::xft().getExcludeRange(from, to) || to!=str.toDouble())
        {
            CKfiGlobal::xft().setExcludeRange(itsFromText->text().toDouble(), str.toDouble());
            itsSaveButton->setEnabled(true);
            emit madeChanges();
        }
    }
}

void CXftConfigSettingsWidget::useSubPixelChecked(bool on)
{
    bool set=CKfiGlobal::xft().getUseSubPixelHinting();

    if((on && !set) || (!on && set))
    {
        CKfiGlobal::xft().setUseSubPixelHinting(on);
        itsSaveButton->setEnabled(true);
        emit madeChanges();
    }
}

void CXftConfigSettingsWidget::advancedButtonPressed()
{
    if(NULL==itsRulesDialog)
        itsRulesDialog = new CXftConfigRules(this);

    if(itsRulesDialog->display())
    {
        CKfiGlobal::xft().setEntries(itsRulesDialog->getList());
        CKfiGlobal::xft().setIncludes(itsRulesDialog->getIncludes());
        CKfiGlobal::xft().setIncludeIfs(itsRulesDialog->getIncludeIfs());
        setWidgets();
        itsSaveButton->setEnabled(true);
        emit madeChanges();
    }
}

void CXftConfigSettingsWidget::saveButtonPressed()
{
    QStringList dirs;

    CSysConfigurer::getTTandT1Dirs(dirs);
    if(CKfiGlobal::xft().save(CKfiGlobal::cfg().getXftConfigFile(), dirs))
    {
        itsSaveButton->setEnabled(false);
        emit savedChanges();
    }
    else
        KMessageBox::error(this, i18n("Could not save file"), i18n("Error"));
}

#endif

#include "XftConfigSettingsWidget.moc"
