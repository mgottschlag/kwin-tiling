/*
 *   Copyright (C) 2000 Matthias Elter <elter@kde.org>
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
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <qlayout.h>
#include <qgroupbox.h>
#include <qlabel.h>

#include <klocale.h>
#include <kstddirs.h>
#include <kglobal.h>
#include <kdialog.h>
#include <klineedit.h>
#include <kicondialog.h>
#include <kdesktopfile.h>

#include "advancedtab.h"
#include "advancedtab.moc"
#include "khotkeys.h"

AdvancedTab::AdvancedTab( QWidget *parent, const char *name )
  : QWidget(parent, name)
{
    QGridLayout *layout = new QGridLayout(this, 2, 1,
					  KDialog::marginHint(),
					  KDialog::spacingHint());
    QGroupBox *general_group = new QGroupBox(this);
    layout->addWidget( general_group, 0, 0 );
     // dummy widget in order to make it look a bit better
    layout->addWidget( new QWidget(this), 1, 0 );
    layout->setRowStretch( 1, 4 );
    QGridLayout *grid = new QGridLayout(general_group, 3, 1,
					KDialog::marginHint(),
					KDialog::spacingHint());

    grid->addWidget(new QLabel(i18n("Current key"), general_group), 0, 0);
    _keyEdit = new KLineEdit(general_group);
    _keyEdit->setReadOnly( true );
    _keyEdit->setText( "" );
    QPushButton* _keyButton = new QPushButton( i18n( "Change" ),
        general_group );
    connect( _keyButton, SIGNAL( clicked()), this, SLOT( keyButtonPressed()));
    grid->addWidget(_keyEdit, 0, 1);
    grid->addWidget(_keyButton, 0, 2 );
    if( !KHotKeys::present())
        setEnabled( false ); // disable the whole tab if no KHotKeys found
    _khotkeysNeedsSave = false;
}

void AdvancedTab::setDesktopFile(const QString& desktopFile)
{
    _desktopFile = desktopFile;
    _khotkeysNeedsSave = false;

    // is desktopFile a .desktop file?
    if( desktopFile.find(".desktop") > 0 )
        {
        if( KHotKeys::present())
            {
            setEnabled( true );
            _keyEdit->setText( KHotKeys::getMenuEntryShortcut(
                _desktopFile ));
            }
        }
    else
        setEnabled( false ); // not a menu entry - no shortcut

    // KDesktopFile can also handle relative pathes, so we don't have
    // to make sure it's absolute.
    KDesktopFile df(desktopFile);
}

void AdvancedTab::apply( bool /*desktopFileNeedsSave*/ )
{
    if( KHotKeys::present() && _khotkeysNeedsSave )
        KHotKeys::changeMenuEntryShortcut( _desktopFile, _keyEdit->text());
    _khotkeysNeedsSave = false;
}

void AdvancedTab::reset()
{
    if(_desktopFile != "")
	setDesktopFile(_desktopFile);
    _khotkeysNeedsSave = false;
}

void AdvancedTab::keyButtonPressed()
{
    if( !KHotKeys::present())
        return;
    QString new_shortcut = KHotKeys::editMenuEntryShortcut( _desktopFile,
        _keyEdit->text(), false );
    if( new_shortcut == _keyEdit->text())
        return;
    _keyEdit->setText( new_shortcut );
    emit changed( false );
    _khotkeysNeedsSave = true;
}
