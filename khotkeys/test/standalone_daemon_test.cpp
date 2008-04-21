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

#include "standalone_daemon_test.h"

#include "daemon/daemon.h"
#include "daemon/stand_alone.h"

#include <QtDBus/QtDBus>

#include <qtest_kde.h>


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


void Test::testStandAloneDaemonController()
    {
    // The sleeps are necessary. If not present the isRunning call will fail.
    sleep(1);
    QVERIFY(!KHotKeys::StandAloneDaemon::isRunning());
    // 1st round
    QVERIFY(KHotKeys::StandAloneDaemon::start());
    sleep(1);
    QVERIFY(KHotKeys::StandAloneDaemon::isRunning());
    QVERIFY(KHotKeys::StandAloneDaemon::stop());
    sleep(1);
    QVERIFY(!KHotKeys::StandAloneDaemon::isRunning());
    // 2nd round
    QVERIFY(KHotKeys::StandAloneDaemon::start());
    sleep(1);
    QVERIFY(KHotKeys::StandAloneDaemon::isRunning());
    QVERIFY(KHotKeys::StandAloneDaemon::stop());
    sleep(1);
    QVERIFY(!KHotKeys::StandAloneDaemon::isRunning());
    // 3nd round
    QVERIFY(KHotKeys::StandAloneDaemon::start());
    sleep(1);
    QVERIFY(KHotKeys::StandAloneDaemon::isRunning());
    // Ensure kded is still there
    QVERIFY(QDBusInterface( "org.kde.kded", "/kded","org.kde.kded" ).isValid());
    QVERIFY(KHotKeys::StandAloneDaemon::stop());
    }

QTEST_MAIN(Test);
#include "moc_standalone_daemon_test.cpp"
