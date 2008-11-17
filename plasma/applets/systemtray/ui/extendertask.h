/***************************************************************************
 *   Copyright (C) 2008 Rob Scheepmaker <r.scheepmaker@student.utwente.nl> *
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

#ifndef EXTENDERTASK_H
#define EXTENDERTASK_H

#include "../core/task.h"


namespace Plasma
{
    class PopupApplet;
}

namespace SystemTray
{

namespace Extender
{

class Task : public SystemTray::Task
{
    Q_OBJECT

public:
    Task(Plasma::PopupApplet *systemTray);
    virtual ~Task();

    bool isValid() const;
    virtual bool isEmbeddable() const;
    virtual QString name() const;
    virtual QString typeId() const;
    virtual QIcon icon() const;
    void setIcon(const QString &icon);

signals:
    void taskDeleted(QString typeId);

protected:
    virtual QGraphicsWidget* createWidget(Plasma::Applet *applet);

private:
    class Private;
    Private* const d;
};


}
}

#endif
