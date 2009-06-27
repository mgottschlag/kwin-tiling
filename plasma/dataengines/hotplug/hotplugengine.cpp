/*
 *   Copyright (C) 2007 Menard Alexis <darktears31@gmail.com>
 *
 * This program is free software you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "hotplugengine.h"

#include <KConfigGroup>
#include <KDebug>
#include <KLocale>
#include <KStandardDirs>
#include <KDesktopFile>
#include <Plasma/DataContainer>

//solid specific includes
#include <solid/devicenotifier.h>
#include <solid/device.h>
#include <solid/deviceinterface.h>



HotplugEngine::HotplugEngine(QObject* parent, const QVariantList& args)
    : Plasma::DataEngine(parent, args)
{
}

HotplugEngine::~HotplugEngine()
{

}

void HotplugEngine::init()
{
    findPredicates();
    const QString query("[ Is StorageAccess OR [ Is StorageDrive OR [ Is StorageVolume OR [ Is OpticalDrive OR [ Is PortableMediaPlayer OR [ Is SmartCardReader OR Is Camera ] ] ] ] ] ]");
    foreach (const Solid::Device &dev, Solid::Device::listFromQuery(query)) {
        onDeviceAdded(dev.udi());
    }
    m_predicates.clear();

    connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceAdded(const QString &)),
            this, SLOT(onDeviceAdded(const QString &)));
    connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceRemoved(const QString &)),
            this, SLOT(onDeviceRemoved(const QString &)));
}

void HotplugEngine::findPredicates()
{
    foreach (const QString &path, KGlobal::dirs()->findAllResources("data", "solid/actions/")) {
        KDesktopFile cfg(path);
        const QString string_predicate = cfg.desktopGroup().readEntry("X-KDE-Solid-Predicate");
        //kDebug() << path << string_predicate;
        m_predicates.insert(KUrl(path).fileName(), Solid::Predicate::fromString(string_predicate));
    }

    if (m_predicates.isEmpty()) {
        m_predicates.insert(QString(), Solid::Predicate::fromString(QString()));
    }
}

void HotplugEngine::onDeviceAdded(const QString &udi)
{
    if (m_predicates.isEmpty()) {
        findPredicates();
    }

    Solid::Device device(udi);
    QStringList interestingDesktopFiles;
    //search in all desktop configuration file if the device inserted is a correct device
    QHashIterator<QString, Solid::Predicate> it(m_predicates);
    //kDebug() << "=================" << udi;
    while (it.hasNext()) {
        it.next();
        if (it.value().matches(device)) {
            //kDebug() << "     hit" << it.key();
            interestingDesktopFiles << it.key();
        }
    }

    if (!interestingDesktopFiles.isEmpty()) {
        //kDebug() << device.product();
        //kDebug() << device.vendor();
        //kDebug() << "number of interesting desktop file : " << interestingDesktopFiles.size();
        Plasma::DataEngine::Data data;
        data.insert("added", true);
        data.insert("udi", udi);

        if (device.vendor().isEmpty()) {
            data.insert("text", device.product());
        } else {
            data.insert("text", device.vendor() + ' ' + device.product());
        }
        data.insert("icon", device.icon());
        setData(udi, "icon", device.icon());
        data.insert("predicateFiles", interestingDesktopFiles);

        setData(udi, data);
        //kDebug() << "add hardware solid : " << udi;
    }
}

void HotplugEngine::onDeviceRemoved(const QString &udi)
{
    removeSource(udi);

    //kDebug() << "remove hardware solid : " << udi;

    scheduleSourcesUpdated();
}

#include "hotplugengine.moc"
