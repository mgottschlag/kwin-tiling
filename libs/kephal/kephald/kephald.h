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

#include <QList>
#include <kdedmodule.h>

class QTimer;
class QVariant;

namespace kephal {
    class Output;
    class XRandROutputs;
}

using namespace kephal;


class KephalD : public KDEDModule
{
    Q_OBJECT
    public:
        KephalD(QObject* parent, const QList<QVariant>&);
        ~KephalD();
        
    private Q_SLOTS:
        void outputDisconnected(kephal::Output * output);
        void outputConnected(kephal::Output * output);
        void poll();
        void pollingActivated();
        void pollingDeactivated();
        
    private:
        void init();
        void activateConfiguration();
        
        bool m_noXRandR;
        XRandROutputs * m_outputs;
        QTimer * m_pollTimer;
};


#endif //KEPHALD_H

