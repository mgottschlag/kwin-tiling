/*****************************************************************

Copyright (c) 2005 Fred Schaettgen <kde.sch@ttgen.net>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/


#include <QComboBox>

#include <klocale.h>
#include <kdebug.h>

#include "prefs.h"
#include "configdlgbase.h"

#include "configdlg.h"
#include "configdlg.moc"

ConfigDlg::ConfigDlg(QWidget *parent, const char *name, Prefs *config,
                     int autoSize,FaceType  dialogType,
                     ButtonCodes dialogButtons) :
    KConfigDialog(parent, name, config, dialogType, dialogButtons),
    m_settings(config),
    m_autoSize(autoSize)
{
    QFrame * frame = new QFrame( this );
    setMainWidget( frame );
    m_ui = new ConfigDlgBase(frame);
    addPage(m_ui, i18n("Configure"), "config");

    m_ui->iconDim->clear();
    m_ui->iconDim->addItem(i18n("Automatic"));
    for (int n=0; n<int(m_settings->iconDimChoices().size()); ++n)
    {
        m_ui->iconDim->addItem(QString::number(
            m_settings->iconDimChoices()[n]));
    }
    connect(m_ui->iconDim, SIGNAL(textChanged(const QString&)),
            this, SLOT(updateButtons()));
    updateWidgets();
    m_oldIconDimText = m_ui->iconDim->currentText();
    updateButtons();
}

void ConfigDlg::updateSettings()
{
    kDebug() << "updateSettings" << endl;
    KConfigDialog::updateSettings();
    if (!hasChanged())
    {
        return;
    }
    m_oldIconDimText = m_ui->iconDim->currentText();
    if (m_ui->iconDim->currentText() == i18n("Automatic"))
    {
        m_settings->setIconDim(m_autoSize);
    }
    else
    {
        m_settings->setIconDim(m_ui->iconDim->currentText().toInt());
    }
    settingsChangedSlot();
}

void ConfigDlg::updateWidgets()
{
    KConfigDialog::updateWidgets();
    if (m_settings->iconDim() == m_autoSize)
    {
        m_ui->iconDim->setEditText(i18n("Automatic"));
    }
    else
    {
        m_ui->iconDim->setEditText(QString::number(m_settings->iconDim()));
    }
}

void ConfigDlg::updateWidgetsDefault()
{
    KConfigDialog::updateWidgetsDefault();
}

bool ConfigDlg::hasChanged()
{
    return m_oldIconDimText != m_ui->iconDim->currentText() ||
        KConfigDialog::hasChanged();
}
