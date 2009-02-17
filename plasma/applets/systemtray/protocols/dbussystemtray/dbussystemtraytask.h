/***************************************************************************
 *   dbussystemtraytask.h                                                  *
 *                                                                         *
 *   Copyright (C) 2009 Marco MArtin <notmart@gmail.com      >             *
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

    friend class DBusSystemTrayProtocol;

public:
    DBusSystemTrayTask(const QString &service);
    ~DBusSystemTrayTask();

    QGraphicsWidget* createWidget(Plasma::Applet *host);
    bool isValid() const;
    virtual bool isEmbeddable() const;
    virtual QString name() const;
    virtual QString typeId() const;
    virtual QIcon icon() const;

private:
    DBusSystemTrayTaskPrivate *d;
};

}


#endif
