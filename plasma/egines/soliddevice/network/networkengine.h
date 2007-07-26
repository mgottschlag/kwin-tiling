/*
 *   Copyright (C) 2007 Percy Leonhardt <percy@eris23.de>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#ifndef NETWORKENGINE_H
#define NETWORKENGINE_H

#include "plasma/dataengine.h"

class QTimer;
class Plasma::DataContainer;

class NetworkEngine : public Plasma::DataEngine
{
    Q_OBJECT

public:
    NetworkEngine( QObject* parent, const QStringList& args );
    ~NetworkEngine();

protected:
    bool sourceRequested(const QString &name);

protected slots:
    void updateNetworkData();

private:
    bool readNumberFromFile( const QString &fileName, unsigned int &value );
    bool readStringFromFile( const QString &fileName, QString &string );
    void updateWirelessData( const QString &ifName, Plasma::DataContainer *source );
    void updateInterfaceData( const QString &ifName, Plasma::DataContainer *source );

    QTimer* m_timer;
};

K_EXPORT_PLASMA_DATAENGINE(network, NetworkEngine)

#endif
