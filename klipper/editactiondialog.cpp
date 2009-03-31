/*
   This file is part of the KDE project
   Copyright (C) 2009 by Dmitry Suzdalev <dimsuz@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "editactiondialog.h"

#include <KDebug>

#include "urlgrabber.h"

#include "ui_editactiondialog.h"

EditActionDialog::EditActionDialog(QWidget* parent)
    : KDialog(parent)
{
    setCaption(i18n("Action Properties"));
    setButtons(KDialog::Ok | KDialog::Cancel);

    QWidget* dlgWidget = new QWidget(this);
    m_ui = new Ui::EditActionDialog;
    m_ui->setupUi(dlgWidget);

    m_ui->pbAddCommand->setIcon(KIcon("list-add"));
    m_ui->pbRemoveCommand->setIcon(KIcon("list-remove"));

    m_ui->twCommandList->header()->resizeSection( 0, 170 );

    setMainWidget(dlgWidget);

    connect(m_ui->twCommandList, SIGNAL(itemSelectionChanged()), SLOT(onSelectionChanged()));
    connect(m_ui->twCommandList, SIGNAL(itemChanged(QTreeWidgetItem*, int)),
            SLOT(onItemChanged(QTreeWidgetItem*, int)));

    connect(m_ui->pbAddCommand, SIGNAL( clicked() ), SLOT( onAddCommand() ) );
    connect(m_ui->pbRemoveCommand, SIGNAL( clicked() ), SLOT( onRemoveCommand() ) );

    // update Remove button
    onSelectionChanged();
}

EditActionDialog::~EditActionDialog()
{
    delete m_ui;
}

void EditActionDialog::setAction(ClipAction* act)
{
    m_action = act;

    updateWidgets();
}

void EditActionDialog::updateWidgets()
{
    if (!m_action) {
        kDebug() << "no action to edit was set";
        return;
    }

    m_ui->twCommandList->clear();

    m_ui->leRegExp->setText(m_action->regExp());
    m_ui->leDescription->setText(m_action->description());

    foreach( const ClipCommand& cmd, m_action->commands() ) {
        QTreeWidgetItem* item = new QTreeWidgetItem;
        item->setFlags( item->flags() | Qt::ItemIsEditable );

        item->setText( 0, cmd.command );
        QString iconName = cmd.pixmap.isEmpty() ? "system-run" : cmd.pixmap;
        item->setIcon( 0, KIcon( iconName ) );
        item->setData( 0, Qt::UserRole, iconName ); // store icon name too
        item->setText( 1, cmd.description );
        m_ui->twCommandList->addTopLevelItem( item );
    }

    // update Remove button
    onSelectionChanged();
}

void EditActionDialog::saveAction()
{
    if (!m_action) {
        kDebug() << "no action to edit was set";
        return;
    }

    m_action->setRegExp( m_ui->leRegExp->text() );
    m_action->setDescription( m_ui->leDescription->text() );

    m_action->clearCommands();

    int cmdCount = m_ui->twCommandList->topLevelItemCount();
    for ( int i=0; i<cmdCount; ++i ) {
        QTreeWidgetItem* item = m_ui->twCommandList->topLevelItem( i );
        // we store icon name in Qt::UserRole in first column
        // (see onItemChanged())
        QString iconName = item->data( 0, Qt::UserRole ).toString();
        m_action->addCommand( item->text( 0 ), item->text( 1 ), true, iconName );
    }
}

void EditActionDialog::slotButtonClicked( int button )
{
    if ( button == KDialog::Ok ) {
        saveAction();
    }

    KDialog::slotButtonClicked( button );
}

void EditActionDialog::onAddCommand()
{
    QTreeWidgetItem *item = new QTreeWidgetItem;
    item->setFlags( item->flags() | Qt::ItemIsEditable );
    item->setText( 0, i18n( "new command" ) );
    item->setIcon( 0, KIcon( "system-run" ) );
    item->setText( 1, i18n( "Command Description" ) );

    m_ui->twCommandList->addTopLevelItem( item );
    m_ui->twCommandList->editItem( item );
}

void EditActionDialog::onRemoveCommand()
{
    QTreeWidgetItem* curItem = m_ui->twCommandList->currentItem();
    delete curItem;
}

void EditActionDialog::onItemChanged( QTreeWidgetItem* item, int column )
{
    if ( column == 0 ) {
        // let's try to update icon of the item according to command
        QString command = item->text( 0 );
        if ( command.contains( ' ' ) )
            // get first word
            command = command.section( ' ', 0, 0 );

        QPixmap iconPix = KIconLoader::global()->loadIcon(
                                         command, KIconLoader::Small, 0,
                                         KIconLoader::DefaultState,
                                         QStringList(), 0, true /* canReturnNull */ );

        // block signals to prevent infinite recursion when setIcon will trigger itemChanged again
        m_ui->twCommandList->blockSignals( true );

        if ( !iconPix.isNull() ) {
            item->setIcon( 0, KIcon( command ) );
            // let's save icon name in data field (if we found something that is not "system-run")
            item->setData( 0, Qt::UserRole, command ); // command is actually the icon name here :)
        } else {
            item->setIcon( 0, KIcon( "system-run" ) );
        }

        m_ui->twCommandList->blockSignals( false );
    }
}

void EditActionDialog::onSelectionChanged()
{
    m_ui->pbRemoveCommand->setEnabled( !m_ui->twCommandList->selectedItems().isEmpty() );
}

#include "editactiondialog.moc"
