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

#include <KIcon>

#include <Plasma/BusyWidget>

#include "../core/task.h"

class QStyleOptionGraphicsItem;

namespace Plasma
{
    class Extender;
    class PopupApplet;
    class Svg;
}

namespace SystemTray
{

class Manager;

class ExtenderTaskBusyWidget : public Plasma::BusyWidget
{
    Q_OBJECT

public:
    enum State { Empty, Info, Running };

    ExtenderTaskBusyWidget(Plasma::PopupApplet *parent, const Manager *manager);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
    void setState(State state);

protected slots:
    void updateTask();

private:
    QString expanderElement() const;

    KIcon m_icon;
    State m_state;
    Plasma::Svg *m_svg;
    Plasma::PopupApplet *m_systray;
    const Manager *m_manager;
};

class ExtenderTask : public SystemTray::Task
{
    Q_OBJECT

public:
    ExtenderTask(const Manager *manager);
    virtual ~ExtenderTask();

    bool isValid() const;
    bool isEmbeddable() const;
    QString name() const;
    QString typeId() const;
    QIcon icon() const;
    void setIcon(const QString &icon);
    bool isHideable() const;

signals:
    void taskDeleted(QString typeId);

protected:
    QGraphicsWidget* createWidget(Plasma::Applet *applet);

private:
    class Private;
    Private* const d;
};

}

#endif
