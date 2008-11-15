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


#ifndef KEPHALD_H
#define KEPHALD_H

#include <QDebug>
#include <QDBusConnection>
#include <QApplication>
#include <QTimer>

#ifdef Q_WS_X11
#include "outputs/xrandr/xrandroutputs.h"
#endif

namespace Kephal {
    class Output;
    class XRandROutputs;
}

using namespace Kephal;


class KephalD : public QApplication
{
    Q_OBJECT
    public:
        KephalD(int & argc, char ** argv);
        ~KephalD();
#ifdef Q_WS_X11
        virtual bool x11EventFilter(XEvent * e);
#endif
    private Q_SLOTS:
        void outputDisconnected(Kephal::Output * output);
        void outputConnected(Kephal::Output * output);
        void poll();
        void pollingActivated();
        void pollingDeactivated();
        
    private:
        void init();
        void parseArgs(int & argc, char ** argv);
        void activateConfiguration();
        
        bool m_noXRandR;
        XRandROutputs * m_outputs;
        QTimer * m_pollTimer;
};


#endif //KEPHALD_H

