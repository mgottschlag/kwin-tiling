/*
    Copyright (c) 2006 Volker Krause <volker.krause@rwth-aachen.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kdedmodule_daemon_test.h"

#include "daemon/daemon.h"
#include "daemon/kded_module.h"

#include <qtest_kde.h>

#include <QtDBus/QtDBus>


void Test::initTestCase()
    {
    daemonActive = KHotKeys::Daemon::isRunning();
    if (daemonActive)
        {
        qWarning() << "Shutting down a running daemon";
        KHotKeys::Daemon::stop();
        }
    }


void Test::cleanupTestCase()
    {
    if (daemonActive)
        {
        qWarning() << "Restarting the stopped daemon";
        KHotKeys::Daemon::start();
        }
    }


void Test::testLoading()
    {
    // Check loading/unloading. We have to do it more than once. When
    // something is amiss kded will crash after reloading it.
    QVERIFY(!KHotKeys::KdedModuleDaemon::isRunning());
    // 1st round
    QVERIFY(KHotKeys::KdedModuleDaemon::start());
    QVERIFY(KHotKeys::KdedModuleDaemon::isRunning());
    QVERIFY(KHotKeys::KdedModuleDaemon::stop());
    QVERIFY(!KHotKeys::KdedModuleDaemon::isRunning());
    // 2nd round
    QVERIFY(KHotKeys::KdedModuleDaemon::start());
    QVERIFY(KHotKeys::KdedModuleDaemon::isRunning());
    QVERIFY(KHotKeys::KdedModuleDaemon::stop());
    QVERIFY(!KHotKeys::KdedModuleDaemon::isRunning());
    // 3nd round
    QVERIFY(KHotKeys::KdedModuleDaemon::start());
    QVERIFY(KHotKeys::KdedModuleDaemon::isRunning());
    // Ensure kded is still there
    QVERIFY(QDBusInterface( "org.kde.kded", "/kded","org.kde.kded" ).isValid());
    QVERIFY(KHotKeys::KdedModuleDaemon::stop());
    }

QTEST_MAIN(Test)
#include "moc_kdedmodule_daemon_test.cpp"
