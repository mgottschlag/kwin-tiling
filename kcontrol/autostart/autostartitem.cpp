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
#include <KIO/NetAccess>
#include <KIO/DeleteJob>
#include <KIO/CopyJob>

#include <klocale.h>
#include <KDebug>

AutoStartItem::AutoStartItem( const QString &service, QTreeWidgetItem *parent )
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


DesktopStartItem::DesktopStartItem( const QString &service, QTreeWidgetItem *parent )
    : AutoStartItem( service, parent )
{
    setCheckState ( Autostart::COL_STATUS,Qt::Checked );
}

DesktopStartItem::~DesktopStartItem()
{
}

ScriptStartItem::ScriptStartItem( const QString &service, QTreeWidgetItem *parent )
    : AutoStartItem( service, parent )
{
    m_comboBoxStartup = new QComboBox;
    QStringList startupLst;
    startupLst<<i18n( "Startup" )<<i18n( "Shutdown" );
    m_comboBoxStartup->addItems( startupLst );
    setText( 2, i18n( "Enabled" ) );
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
    default:
        kDebug()<<" type is not defined :"<<type;
    }
}

#include "autostartitem.moc"
