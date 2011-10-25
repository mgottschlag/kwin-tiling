/***************************************************************************
 *   Copyright (C) 2008 by Montel Laurent <montel@kde.org>                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

#include "autostartitem.h"
#include "autostart.h"
#include <QComboBox>
#include <QTreeWidgetItem>
#include <QTreeWidget>

#include <klocale.h>
#include <KDebug>
#include <KIO/CopyJob>

AutoStartItem::AutoStartItem( const QString &service, QTreeWidgetItem *parent, Autostart* )
    : QTreeWidgetItem( parent )
{
    m_fileName = KUrl(service);
}

AutoStartItem::~AutoStartItem()
{

}

KUrl AutoStartItem::fileName() const
{
    return m_fileName;
}

void AutoStartItem::setPath(const QString &path) {
    if (path == m_fileName.directory(KUrl::AppendTrailingSlash))
        return;
    KIO::move(m_fileName, KUrl( path + '/' + m_fileName.fileName() ));
    m_fileName = KUrl(path + m_fileName.fileName());
}


DesktopStartItem::DesktopStartItem( const QString &service, QTreeWidgetItem *parent, Autostart*autostart )
    : AutoStartItem( service, parent,autostart )
{
    setCheckState ( Autostart::COL_STATUS,Qt::Checked );
}

DesktopStartItem::~DesktopStartItem()
{
}

ScriptStartItem::ScriptStartItem( const QString &service, QTreeWidgetItem *parent, Autostart* autostart )
    : AutoStartItem( service, parent,autostart )
{
    m_comboBoxStartup = new QComboBox;
    m_comboBoxStartup->addItems( autostart->listPathName() );

    setText( 2, i18nc( "The program will be run", "Enabled" ) );
    QObject::connect( m_comboBoxStartup, SIGNAL(activated(int)),autostart,SLOT(slotChangeStartup(int)) );
    treeWidget()->setItemWidget ( this, Autostart::COL_RUN, m_comboBoxStartup );
}

ScriptStartItem::~ScriptStartItem()
{
}

void ScriptStartItem::changeStartup(ScriptStartItem::ENV type )
{
    switch( type )
    {
    case ScriptStartItem::START:
        m_comboBoxStartup->setCurrentIndex( 0 );
        break;
    case ScriptStartItem::SHUTDOWN:
        m_comboBoxStartup->setCurrentIndex( 1 );
        break;
    case ScriptStartItem::PRE_START:
        m_comboBoxStartup->setCurrentIndex( 2 );
        break;
    default:
        kDebug()<<" type is not defined :"<<type;
        break;
    }
}

