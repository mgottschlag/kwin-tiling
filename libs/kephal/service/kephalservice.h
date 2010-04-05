/*
 *   Copyright 2008 Aike J Sommer <dev@aikesommer.name>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
 *   or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef KEPHALSERVICE_H
#define KEPHALSERVICE_H

#include <QList>
#include <QWidget>

class QTimer;

namespace Kephal { class Output;
    class XRandROutputs;
}

class X11EventFilter : public QWidget
{
Q_OBJECT
public:
    X11EventFilter(Kephal::XRandROutputs * outputs);
    virtual ~X11EventFilter();

protected:
#ifdef Q_WS_X11
    bool x11Event(XEvent * event);
#endif
private:
    Kephal::XRandROutputs * m_outputs;
};


class KephalService : public QObject
{
Q_OBJECT
public:
    KephalService(QObject * parent);
    virtual ~KephalService();
private Q_SLOTS:
    void outputDisconnected(Kephal::Output * output);
    void outputConnected(Kephal::Output * output);
    void poll();
    void pollingActivated();
    void pollingDeactivated();

private:
    void init();
    void activateConfiguration();

    bool m_noXRandR;
    Kephal::XRandROutputs * m_outputs;
    QTimer * m_pollTimer;
    X11EventFilter * m_eventFilter;
};

#endif // KEPHALSERVICE_H
