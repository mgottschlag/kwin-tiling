/*
 * Copyright (c) 2000 Malte Starostik <malte@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>

#include <kapplication.h>
#include <klocale.h>
#include <kglobal.h>
#include <kcharsets.h>
#include <klineedit.h>
#include <kcombobox.h>
#include <kmessagebox.h>

#include "searchproviderdlg_ui.h"
#include "searchproviderdlg.h"
#include "searchprovider.h"

SearchProviderDialog::SearchProviderDialog(SearchProvider *provider,
                                           QWidget *parent, const char *name)
                     :KDialogBase(parent, name, true, QString::null, Ok|Cancel),
                      m_provider(provider)
{
    m_dlg = new SearchProviderDlgUI (this);
    setMainWidget(m_dlg);

    enableButtonSeparator(true);

    QString whatsThis = i18n("Enter the human readable name of the search provider here.");
    QWhatsThis::add(m_dlg->lbName, whatsThis);
    QWhatsThis::add(m_dlg->leName, whatsThis);

    whatsThis = i18n("Enter the URI that is used to do a search on the search "
                     "engine here.\nThe whole text to be searched for can be "
                     "specified as \\{@} or \\{0}.\nRecommended is \\{@}, since "
                     "it removes all query variables (name=value) from the "
                     "resulting string whereas \\{0} will be substituted with the "
                     "unmodified query string.\nYou can use \\{1} ... \\{n} to "
                     "specify certain words from the query and \\{name} to "
                     "specify a value given by 'name=value' in the user query.\n"
                     "In addition it is possible to specify multiple references "
                     "(names, numbers and strings) at once (\\{name1,name2,...,\""
                     "string\"}).\nThe first matching value (from the left) will "
                     "be used as substitution value for the resulting URI.\n"
                     "A quoted string can be used as default value if nothing "
                     "matches from the left of the reference list.");
    QWhatsThis::add(m_dlg->lbQuery, whatsThis);
    QWhatsThis::add(m_dlg->leQuery, whatsThis);
    m_dlg->leQuery->setMinimumWidth(kapp->fontMetrics().width('x') * 40);

    whatsThis = i18n("The shortcuts entered here can be used as a pseudo-URI "
                     "scheme in KDE. For example, the shortcut <em>av</em> can "
                     "be used as in <em>av</em>:<em>my search</em>.");
    QWhatsThis::add(m_dlg->lbShortcut, whatsThis);
    QWhatsThis::add(m_dlg->leShortcut, whatsThis);

    whatsThis = i18n("Select the character set that will be used to encode "
                     "your search query.");
    QWhatsThis::add(m_dlg->lbCharset, whatsThis);
    QWhatsThis::add(m_dlg->cbCharset, whatsThis);

    connect(m_dlg->leName, SIGNAL(textChanged(const QString &)), SLOT(slotChanged()));
    connect(m_dlg->leQuery, SIGNAL(textChanged(const QString &)), SLOT(slotChanged()));
    connect(m_dlg->leShortcut, SIGNAL(textChanged(const QString &)), SLOT(slotChanged()));

    // Data init
    QStringList charsets = KGlobal::charsets()->availableEncodingNames();
    charsets.prepend(i18n("Default"));
    m_dlg->cbCharset->insertStringList(charsets);

    if (m_provider)
    {
        setPlainCaption(i18n("Modify Search Provider"));
        m_dlg->leName->setText(m_provider->name());
        m_dlg->leQuery->setText(m_provider->query());
        m_dlg->leShortcut->setText(m_provider->keys().join(","));
        m_dlg->cbCharset->setCurrentItem(m_provider->charset().isEmpty() ? 0 : charsets.findIndex(m_provider->charset()));
        m_dlg->leName->setEnabled(false);
        m_dlg->leQuery->setFocus();
    }
    else
    {
        setPlainCaption(i18n("New Search Provider"));
        m_dlg->leName->setFocus();
        enableButton(Ok, false);
    }
}

void SearchProviderDialog::slotChanged()
{
    enableButton(Ok, !(m_dlg->leName->text().isEmpty()
                       || m_dlg->leShortcut->text().isEmpty()
                       || m_dlg->leQuery->text().isEmpty()));
}

void SearchProviderDialog::slotOk()
{
    if ((m_dlg->leQuery->text().find("\\{") == -1)
        && KMessageBox::warningContinueCancel(0,
            i18n("The URI does not contain a \\{...} placeholder for the user query.\n"
                 "This means that the same page is always going to be visited, "
                 "regardless of what the user types..."),
            QString::null, i18n("Keep It")) == KMessageBox::Cancel)
        return;

    if (!m_provider)
        m_provider = new SearchProvider;
    m_provider->setName(m_dlg->leName->text().stripWhiteSpace());
    m_provider->setQuery(m_dlg->leQuery->text().stripWhiteSpace());
    m_provider->setKeys(QStringList::split(",", m_dlg->leShortcut->text().stripWhiteSpace()));
    m_provider->setCharset(m_dlg->cbCharset->currentItem() ? m_dlg->cbCharset->currentText() : QString::null);
    KDialog::accept();
}

#include "searchproviderdlg.moc"
