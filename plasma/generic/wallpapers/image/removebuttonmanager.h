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

#ifndef SELECTIONMANAGER_H
#define SELECTIONMANAGER_H

#include <kfileitem.h>

#include <QObject>

class QAbstractItemView;
class QModelIndex;
class QItemSelection;
class RemoveButton;

/**
 * @brief Allows to select and deselect items for item views.
 *
 * Whenever an item is hovered by the mouse, a removeButton button is shown
 * which allows to select/deselect the current item.
 */
class RemoveButtonManager : public QObject
{
    Q_OBJECT

public:
    RemoveButtonManager(QAbstractItemView* parent, QStringList *list);
    virtual ~RemoveButtonManager();

signals:
    /** Is emitted if the selection has been changed by the removeButton button. */
    void selectionChanged();
    void removeClicked(QString item);

private slots:
    void slotEntered(const QModelIndex& index);
    void slotViewportEntered();
    void slotRowsRemoved(const QModelIndex& parent, int start, int end);
    void removeButtonClicked();

private:
    const QModelIndex indexForUrl(const KUrl& url) const;

private:
    QAbstractItemView* m_view;
    RemoveButton* m_removeButton;
    bool m_connected;
    QStringList *m_removableWallpapers;
};

#endif
