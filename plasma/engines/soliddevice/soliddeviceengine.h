/*
 *   Copyright (C) 2007 Christopher Blauvelt <cblauvelt@gmail.com>
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

#ifndef SOLIDENGINE_H
#define SOLIDENGINE_H

#include <QObject>
#include <QString>
#include <QList>

#include <solid/devicenotifier.h>
#include <solid/device.h>
#include <solid/processor.h>
#include <solid/block.h>
#include <solid/storageaccess.h>
#include <solid/storagedrive.h>
#include <solid/opticaldrive.h>
#include <solid/storagevolume.h>
#include <solid/opticaldisc.h>
#include <solid/camera.h>
#include <solid/portablemediaplayer.h>
#include <solid/networkinterface.h>
#include <solid/acadapter.h>
#include <solid/battery.h>
#include <solid/button.h>
#include <solid/audiointerface.h>
#include <solid/dvbinterface.h>

#include "plasma/dataengine.h"

/**
 * This class evaluates the basic expressions given in the interface.
 */
class SolidDeviceEngine : public Plasma::DataEngine
{
    Q_OBJECT

    public:
		SolidDeviceEngine( QObject* parent, const QStringList& args);
        ~SolidDeviceEngine();

    protected:
        bool sourceRequested(const QString &name);

    private:
		bool populateDeviceData(const QString &name);
        QStringList devicelist;
		QMap<QString,QString> typemap;
		QStringList typelist;

	private slots:
		void deviceAdded(const QString &udi);
		void deviceRemoved(const QString &udi);
};

K_EXPORT_PLASMA_DATAENGINE(soliddevice, SolidDeviceEngine)

#endif
