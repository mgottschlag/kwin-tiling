////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CSysCfgSettingsWidget
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

#include "SysCfgSettingsWidget.h"
#include "KfiGlobal.h"
#include "Config.h"
#include "Encodings.h"
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qstringlist.h>

CSysCfgSettingsWidget::CSysCfgSettingsWidget(QWidget *parent, const char *name)
                     : CSysCfgSettingsWidgetData(parent, name)
{
    itsX11EncodingCheck->setChecked(CKfiGlobal::cfg().getExclusiveEncoding());
    itsX11EncodingCombo->setEnabled(CKfiGlobal::cfg().getExclusiveEncoding());
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
 
void CSysCfgSettingsWidget::encodingSelected(bool on)
{
    itsX11EncodingCombo->setEnabled(on);
    CKfiGlobal::cfg().setExclusiveEncoding(on);
}

void CSysCfgSettingsWidget::generateAfmsSelected(bool on)
{
    CKfiGlobal::cfg().setDoAfm(on);
    itsGenAfmsCheck->setChecked(on);
    itsT1AfmCheck->setChecked(CKfiGlobal::cfg().getDoT1Afms());
    itsTtAfmCheck->setChecked(CKfiGlobal::cfg().getDoTtAfms());

    if(!on)
        emit afmGenerationDeselected();
}

void CSysCfgSettingsWidget::customXRefreshSelected(bool on)
{
    if(on)
        CKfiGlobal::cfg().setXRefreshCmd(CConfig::XREFRESH_CUSTOM);
}

void CSysCfgSettingsWidget::xfsRestartSelected(bool on)
{
    if(on)
        CKfiGlobal::cfg().setXRefreshCmd(CConfig::XREFRESH_XFS_RESTART);
}

void CSysCfgSettingsWidget::xsetFpRehashSelected(bool on)
{
    if(on)
        CKfiGlobal::cfg().setXRefreshCmd(CConfig::XREFRESH_XSET_FP_REHASH);
}

void CSysCfgSettingsWidget::customXStrChanged(const QString &str)
{
    CKfiGlobal::cfg().setCustomXRefreshCmd(str);
}

void CSysCfgSettingsWidget::encodingSelected(const QString &str)
{
    CKfiGlobal::cfg().setEncoding(str);
}

void CSysCfgSettingsWidget::afmEncodingSelected(const QString &str)
{
    CKfiGlobal::cfg().setAfmEncoding(str);
}

void CSysCfgSettingsWidget::t1AfmSelected(bool on)
{
    CKfiGlobal::cfg().setDoT1Afms(on);

    if(!on && !CKfiGlobal::cfg().getDoTtAfms())
        generateAfmsSelected(false);
}

void CSysCfgSettingsWidget::ttAfmSelected(bool on)
{
    CKfiGlobal::cfg().setDoTtAfms(on);

    if(!on && !CKfiGlobal::cfg().getDoT1Afms())
        generateAfmsSelected(false);
}

void CSysCfgSettingsWidget::overwriteAfmsSelected(bool on)
{
    CKfiGlobal::cfg().setOverwriteAfms(on);
}

void CSysCfgSettingsWidget::scanEncodings()
{
    const CEncodings::T8Bit  *enc8;
    const CEncodings::T16Bit *enc16;
    QStringList              list,
                             afmEncs;

    itsX11EncodingCombo->clear();

    for(enc8=CKfiGlobal::enc().first8Bit(); NULL!=enc8; enc8=CKfiGlobal::enc().next8Bit())
    {
        list.append(enc8->name);
        afmEncs.append(enc8->name);
    }
 
    for(enc16=CKfiGlobal::enc().first16Bit(); NULL!=enc16; enc16=CKfiGlobal::enc().next16Bit())
        list.append(enc16->name);

    list.append(CEncodings::constUnicodeStr);
    list.sort();
    itsX11EncodingCombo->insertStringList(list);
    afmEncs.sort();
    itsAfmEncodingCombo->insertStringList(afmEncs);

    int i;
 
    for(i=0; i<itsX11EncodingCombo->count(); ++i)
        if(CKfiGlobal::cfg().getEncoding()==itsX11EncodingCombo->text(i))
        {
            itsX11EncodingCombo->setCurrentItem(i);
            break;
        }

    for(i=0; i<itsAfmEncodingCombo->count(); ++i)
        if(CKfiGlobal::cfg().getAfmEncoding()==itsAfmEncodingCombo->text(i))
        {
            itsAfmEncodingCombo->setCurrentItem(i);
            break;
        }
 
    CKfiGlobal::cfg().setEncoding(itsX11EncodingCombo->currentText());    // Just in case the above didn't match the cfg entry
    CKfiGlobal::cfg().setAfmEncoding(itsAfmEncodingCombo->currentText()); // Ditto
}

void CSysCfgSettingsWidget::enableAfmGeneration()
{
    generateAfmsSelected(true);
}
#include "SysCfgSettingsWidget.moc"
