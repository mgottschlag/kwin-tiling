/*****************************************************************

Copyright (c) 1996-2000 the kicker authors. See file AUTHORS.

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

#include <QFileInfo>

#include <klocale.h>
#include <kiconloader.h>

#include <QCheckBox>
#include <QDir>
#include <QFileInfo>
#include <QLineEdit>

#include <kicondialog.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <kstandarddirs.h>
#include <kurlcompletion.h>
#include <kurlrequester.h>
#include <kurl.h>
#include <kicon.h>

#include <kdebug.h>

#include "exe_dlg.h"
#include "nonKDEButtonSettings.h"

PanelExeDialog::PanelExeDialog(const QString& title, const QString& description,
                               const QString &path, const QString &icon,
                               const QString &cmd, bool inTerm,
                               QWidget *parent, const char *name)
    : KDialog( parent ),
      m_icon(icon.isEmpty() ? "exec" : icon),
      m_iconChanged(false)
{
    setObjectName( name );
    setCaption(i18n("Non-KDE Application Configuration"));
    setButtons( Ok|Cancel );
    showButtonSeparator( true );

    QFileInfo fi(path);

    KVBox *vbox = new KVBox( this );
    setMainWidget( vbox );

    ui = new NonKDEButtonSettings( vbox );
    fillCompletion();

    ui->m_title->setText(title);
    ui->m_description->setText(description);
    ui->m_exec->setUrl(path);
    ui->m_commandLine->setText(cmd);
    ui->m_inTerm->setChecked(inTerm);
    ui->m_icon->setIconType(K3Icon::Panel, K3Icon::Application);

    updateIcon();

    connect(ui->m_exec, SIGNAL(urlSelected(const KUrl &)),
            this, SLOT(slotSelect(const KUrl &)));
    connect(ui->m_exec, SIGNAL(textChanged(const QString &)),
            this, SLOT(slotTextChanged(const QString &)));
    connect(ui->m_exec, SIGNAL(returnPressed()),
            this, SLOT(slotReturnPressed()));
    connect(ui->m_icon, SIGNAL(iconChanged(QString)),
            this, SLOT(slotIconChanged(QString)));

    // leave decent space for the commandline
    resize(sizeHint().width() > 300 ? sizeHint().width() : 300,
           sizeHint().height());
}

void PanelExeDialog::accept()
{
    KDialog::accept();

    // WARNING! we get delete after this, so don't do anything after it!
    emit updateSettings( this );
}

bool PanelExeDialog::useTerminal() const
{
    return ui->m_inTerm->isChecked();
}

QString PanelExeDialog::title() const
{
    return ui->m_title->text();
}

QString PanelExeDialog::description() const
{
    return ui->m_description->text();
}

QString PanelExeDialog::commandLine() const
{
    return ui->m_commandLine->text();
}

QString PanelExeDialog::iconPath() const
{
    return ui->m_icon->icon();
}

QString PanelExeDialog::command() const
{
    return ui->m_exec->url().toString();
}

void PanelExeDialog::updateIcon()
{
    if(!m_icon.isEmpty())
        ui->m_icon->setIcon(m_icon);
}

void PanelExeDialog::fillCompletion()
{
    KCompletion *comp = ui->m_exec->completionObject();
    QStringList exePaths = KStandardDirs::systemPaths();

    for (QStringList::ConstIterator it = exePaths.begin(); it != exePaths.end(); it++)
    {
        QDir d( (*it) );
        d.setFilter( QDir::Files | QDir::Executable );

        const QFileInfoList list = d.entryInfoList();

        foreach(const QFileInfo& fi, list) {
            m_partialPath2full.insert(fi.fileName(), fi.filePath(), false);
            comp->addItem(fi.fileName());
            comp->addItem(fi.filePath());
        }
    }
}

void PanelExeDialog::slotIconChanged(QString)
{
    m_iconChanged = true;
}

void PanelExeDialog::slotTextChanged(const QString &str)
{
    if (m_iconChanged)
    {
        return;
    }

    QString exeLocation = str;
    QMap<QString, QString>::iterator it = m_partialPath2full.find(str);

    if (it != m_partialPath2full.end())
        exeLocation = it.value();
    // KMimeType::pixmapForURL(KUrl( exeLocation ), 0, K3Icon::Panel, 0, K3Icon::DefaultState, &m_icon);
    QString iconName = KMimeType::iconNameForURL(KUrl(exeLocation));
    KGlobal::iconLoader()->loadIcon(iconName, K3Icon::Panel, 0, K3Icon::DefaultState, &m_icon);
    updateIcon();
}

void PanelExeDialog::slotReturnPressed()
{
    if (m_partialPath2full.contains(ui->m_exec->url().toString()))
        ui->m_exec->setUrl(m_partialPath2full[ui->m_exec->url().toString()]);
}

void PanelExeDialog::slotSelect(const KUrl& exec)
{
    if ( exec.isEmpty() )
        return;

    QFileInfo fi(exec.path());
    if (!fi.isExecutable())
    {
        if(KMessageBox::warningYesNo(0, i18n("The selected file is not executable.\n"
                                             "Do you want to select another file?"), i18n("Not Executable"), i18n("Select Other"), KStdGuiItem::cancel())
                == KMessageBox::Yes)
        {
            ui->m_exec->button()->animateClick();
        }

        return;
    }

    // KMimeType::pixmapForURL(KUrl( exec ), 0, K3Icon::Panel, 0, K3Icon::DefaultState, &m_icon);
    QString iconName = KMimeType::iconNameForURL(KUrl(exec));
    KGlobal::iconLoader()->loadIcon(iconName, K3Icon::Panel, 0, K3Icon::DefaultState, &m_icon );
    updateIcon();
}

#include "exe_dlg.moc"

