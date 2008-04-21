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

#include "daemon_test.h"

#include "daemon/daemon.h"
#include "daemon/kded_module.h"
#include "daemon/stand_alone.h"

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


void Test::testDaemonController()
    {
    // Check that isRunning works with no daemon running.
    QVERIFY(!KHotKeys::Daemon::isRunning());

    // Start the kdedmodule
    QVERIFY(KHotKeys::KdedModuleDaemon::start());
    QVERIFY(KHotKeys::KdedModuleDaemon::isRunning());

    // Check that isRunning catches the kded module
    QVERIFY(KHotKeys::Daemon::isRunning());

    // Ensure that the standalone daemon stops the kded module when starting
    QVERIFY(KHotKeys::StandAloneDaemon::start());
    QVERIFY(!KHotKeys::KdedModuleDaemon::isRunning());

    // Check that isRunning catches the standalone daemon
    QVERIFY(KHotKeys::Daemon::isRunning());

    // Ensure that the kdedmodule daemon stops the standalone daemon when
    // starting
    QVERIFY(KHotKeys::KdedModuleDaemon::start());
    QVERIFY(!KHotKeys::StandAloneDaemon::isRunning());

    // Check that the stop method stops the standalone daemon
    QVERIFY(KHotKeys::Daemon::stop());
    QVERIFY(!KHotKeys::Daemon::isRunning());

    // Check that the stop method stops the kded module daemon
    QVERIFY(KHotKeys::StandAloneDaemon::start());
    QVERIFY(KHotKeys::Daemon::stop());
    QVERIFY(!KHotKeys::Daemon::isRunning());

    // TODO: reread_configuration
    }


QTEST_MAIN(Test);
#include "moc_daemon_test.cpp"
