/*
 *   Copyright (C) 2008 Laurent Montel <montel@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "preferencesdlg.h"
#include <QHBoxLayout>
#include <klocale.h>
#include <sonnet/configwidget.h>

PreferencesDialog::PreferencesDialog( QWidget *parent )
    : KPageDialog( parent )
{
    setFaceType( List );
    setButtons( Ok | Cancel );
    setDefaultButton( Ok );

    m_pageSpellChecking = new SpellCheckingPage( this );
    KPageWidgetItem *page = new KPageWidgetItem( m_pageSpellChecking , i18n( "spell" ) );
    page->setHeader( i18n( "Spell checking Options" ) );
    addPage(page);

    connect( this, SIGNAL( okClicked() ), this, SLOT( slotSave() ) );
}

void PreferencesDialog::slotSave()
{
    m_pageSpellChecking->saveOptions();
}

SpellCheckingPage::SpellCheckingPage( QWidget *parent )
    : QWidget( parent )
{
    QHBoxLayout *lay = new QHBoxLayout( this );
    m_confPage = new Sonnet::ConfigWidget(&( *KGlobal::config() ), this );
    lay->addWidget( m_confPage );
    setLayout( lay );
}

void SpellCheckingPage::saveOptions()
{
    m_confPage->save();
}

#include "preferencesdlg.moc"
