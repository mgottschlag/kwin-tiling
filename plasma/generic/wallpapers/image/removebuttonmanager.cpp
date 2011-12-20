/***************************************************************************
 *   Copyright (C) 2008 by Peter Penz <peter.penz@gmx.at>                  *
 *   Copyright (C) 2010 by Davide Bettio <davide.bettio@kdemail.net>       *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#include "removebuttonmanager.h"

#include "removebutton.h"
#include <kiconeffect.h>

#include <QAbstractButton>
#include <QAbstractItemView>
#include <QAbstractProxyModel>
#include <QApplication>
#include <QModelIndex>
#include <QPainter>
#include <QPaintEvent>
#include <QRect>
#include <QTimeLine>

#include "backgroundlistmodel.h"
#include <Plasma/Package>

RemoveButtonManager::RemoveButtonManager(QAbstractItemView* parent, QStringList *list) :
    QObject(parent),
    m_view(parent),
    m_removeButton(0),
    m_connected(false)
{
    m_removableWallpapers = list;
  
    parent->setMouseTracking(true);
  
    connect(parent, SIGNAL(entered(QModelIndex)),
            this, SLOT(slotEntered(QModelIndex)));
    connect(parent, SIGNAL(viewportEntered()),
            this, SLOT(slotViewportEntered()));
    m_removeButton = new RemoveButton(m_view->viewport());
    m_removeButton->hide();
    connect(m_removeButton, SIGNAL(clicked(bool)),
            this, SLOT(removeButtonClicked()));
}

RemoveButtonManager::~RemoveButtonManager()
{
}

void RemoveButtonManager::slotEntered(const QModelIndex& index)
{
    m_removeButton->hide();
    
    BackgroundListModel *model = static_cast<BackgroundListModel *>(m_view->model());
    m_removeButton->setItemName(model->package(index.row())->filePath("preferred"));

    if (m_removableWallpapers->indexOf(m_removeButton->itemName()) < 0){
      return;
    }

    if (!m_connected) {
        connect(m_view->model(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
                this, SLOT(slotRowsRemoved(QModelIndex,int,int)));
        connect(m_view->selectionModel(),
                SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                this, SLOT(slotSelectionChanged(QItemSelection,QItemSelection)));
        m_connected = true;
    }

    // increase the size of the removeButton for large items
    const int height = m_view->iconSize().height();
    if (height >= KIconLoader::SizeEnormous) {
        m_removeButton->resize(KIconLoader::SizeMedium, KIconLoader::SizeMedium);
    } else if (height >= KIconLoader::SizeLarge) {
        m_removeButton->resize(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium);
    } else {
        m_removeButton->resize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
    }

    const QRect rect = m_view->visualRect(index);
    int x = rect.left();
    int y = rect.top();

    m_removeButton->move(QPoint(x, y));
    m_removeButton->show();
}

void RemoveButtonManager::slotViewportEntered()
{
    m_removeButton->hide();
}

void RemoveButtonManager::slotRowsRemoved(const QModelIndex& parent, int start, int end)
{
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);
    m_removeButton->hide();
}

void RemoveButtonManager::removeButtonClicked()
{
    RemoveButton *removeButton = static_cast<RemoveButton *>(sender());
    emit removeClicked(removeButton->itemName());
}

#include "removebuttonmanager.moc"
