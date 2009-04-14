/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2009 Marco Martin <notmart@gmail.com>                   * *                                                                         *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef DBUSSYSTEMTRAYTASK_H
#define DBUSSYSTEMTRAYTASK_H

#include "../../core/task.h"

namespace SystemTray
{

class DBusSystemTrayTaskPrivate;

class DBusSystemTrayTask : public Task
{
    Q_OBJECT

    Q_ENUMS(ItemStatus)
    Q_ENUMS(ItemCategory)

    friend class DBusSystemTrayProtocol;

public:
    //FIXME: those enums have to be redefined until the library won't be in kdelibs
    enum ItemStatus {
        Passive = 1,
        Active = 2,
        NeedsAttention = 3
    };

    enum ItemCategory {
        ApplicationStatus = 1,
        Communications = 2,
        SystemServices = 3,
        Hardware = 4
    };

    DBusSystemTrayTask(const QString &service);
    ~DBusSystemTrayTask();

    QGraphicsWidget* createWidget(Plasma::Applet *host);
    bool isValid() const;
    ItemCategory category() const;
    virtual bool isEmbeddable() const;
    virtual QString name() const;
    virtual QString typeId() const;
    virtual QIcon icon() const;

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:
    DBusSystemTrayTaskPrivate *const d;
    friend class DBusSystemTrayTaskPrivate;

    Q_PRIVATE_SLOT(d, void iconDestroyed(QObject *obj))
    Q_PRIVATE_SLOT(d, void refresh())
    Q_PRIVATE_SLOT(d, void syncIcon())
    Q_PRIVATE_SLOT(d, void syncAttentionIcon())
    Q_PRIVATE_SLOT(d, void syncToolTip())
    Q_PRIVATE_SLOT(d, void syncStatus(QString status))
    Q_PRIVATE_SLOT(d, void updateMovieFrame())
    Q_PRIVATE_SLOT(d, void blinkAttention())
};

}


#endif
