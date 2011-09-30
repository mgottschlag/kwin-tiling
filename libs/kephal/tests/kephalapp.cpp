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


#include "kephalapp.h"
#include "kephal/screens.h"
#include "kephal/outputs.h"


#include <QDebug>
#include <QDesktopWidget>
#include <QTimer>

#include <QTextStream>

#include <KAboutData>
#include <KCmdLineArgs>

#include <iostream>

#define WITH_OUTPUTS 0

using namespace Kephal;

int main(int argc, char *argv[])
{
    KAboutData aboutData( "kephalapp", "filetypes", ki18n("KEditFileType"), "1.0",
            ki18n("KDE file type editor - simplified version for editing a single file type"),
            KAboutData::License_GPL,
            ki18n("(c) 2000, KDE developers") );

    aboutData.addAuthor(ki18n("Aike J Sommer"), ki18n("Original author"), "dev@aikesommer.name");
    aboutData.addAuthor(ki18n("Will Stephenson"), ki18n("Developer"), "wstephenson@kde.org");

    KCmdLineArgs::init( argc, argv, &aboutData );
    KCmdLineOptions options;

    options.add("listen", ki18n("keep running and report events"));
    KCmdLineArgs::addCmdLineOptions(options);
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    KephalApp app(args->isSet("listen"), argc, argv);
    return app.exec();
}


KephalApp::KephalApp(bool listen, int & argc, char ** argv)
: QApplication(argc, argv), m_listen(listen)
{
    QTimer::singleShot(0, this, SLOT(run()));
}

KephalApp::~KephalApp()
{
}


void KephalApp::run() {
    query();
    if (!m_listen) {
        QApplication::exit(0);
    }

    connect(QApplication::desktop(), SIGNAL(resized(int)), SLOT(qdwScreenResized(int)));
    connect(QApplication::desktop(), SIGNAL(screenCountChanged(int)), SLOT(qdwScreenCountChanged(int)));
    connect(QApplication::desktop(), SIGNAL(workAreaResized(int)), SLOT(qdwWorkAreaResized(int)));

    connect(Kephal::Screens::self(), SIGNAL(screenMoved(Kephal::Screen*,QPoint,QPoint)),
            this, SLOT(screenMoved(Kephal::Screen*,QPoint,QPoint)));
    connect(Kephal::Screens::self(), SIGNAL(screenResized(Kephal::Screen*,QSize,QSize)),
            this, SLOT(screenResized(Kephal::Screen*,QSize,QSize)));
    connect(Kephal::Screens::self(), SIGNAL(screenRemoved(int)),
            this, SLOT(screenRemoved(int)));
    connect(Kephal::Screens::self(), SIGNAL(screenAdded(Kephal::Screen*)),
            this, SLOT(screenAdded(Kephal::Screen*)));
}

void KephalApp::screenMoved(Kephal::Screen * s, QPoint o, QPoint n)
{
    qDebug() << "****************";
    qDebug() << "* New message coming in:";
    qDebug() << "* screenMoved: " << s->id() << " from " << o << " to " << n;
    qDebug() << "****************";
}

void KephalApp::screenResized(Kephal::Screen * s, QSize o, QSize n)
{
    qDebug() << "****************";
    qDebug() << "* New message coming in:";
    qDebug() << "* screenResized: " << s->id() << " from " << o << " to " << n;
    qDebug() << "****************";
}

void KephalApp::screenRemoved(int s)
{
    qDebug() << "****************";
    qDebug() << "* New message coming in:";
    qDebug() << "* screenRemoved: " << s;
    qDebug() << "****************";
}

void KephalApp::screenAdded(Kephal::Screen * s)
{
    qDebug() << "****************";
    qDebug() << "* New message coming in:";
    qDebug() << "* screenAdded: " << s->id() << " at " << s->position() << " with size " << s->size();
    qDebug() << "****************";
}


void KephalApp::query()
{
    qDebug() << "Screens:";
    foreach (Screen * screen, Screens::self()->screens()) {
        qDebug() << "  Screen " << screen->id();
        qDebug() << "    Size: " << screen->size();
        qDebug() << "    Position: " << screen->position();

#if WITH_OUTPUTS
        foreach (Output * output, screen->outputs()) {
            qDebug() << "    Output: " << output->id() << "\n";
        }
#endif
    }

#if WITH_OUTPUTS
    qDebug() << "\nOutputs:\n";
    foreach (Output * output, Outputs::self()->outputs()) {
        qDebug() << "  Output " << output->id() << ":\n";
        qDebug() << "    Connected: " << output->isConnected() << "\n";

        if (! output->isConnected()) continue;

        qDebug() << "    Activated: " << output->isActivated() << "\n";
        qDebug() << "    Size: " << output->size().width() << "x" << output->size().height() << "\n";
        qDebug() << "    Position: (" << output->position().x() << "," << output->position().y() << ")\n";
        qDebug() << "    Vendor: " << output->vendor() << "\n";
        qDebug() << "    PreferredSize: " << output->preferredSize().width() << "x" << output->preferredSize().height() << "\n";
        qDebug() << "    Rotation: " << output->rotation() << "\n";
        qDebug() << "    ReflectX: " << output->reflectX() << "\n";
        qDebug() << "    ReflectY: " << output->reflectY() << "\n";
        qDebug() << "    Rate: " << output->rate() << "\n";
        qDebug() << "    Screen: " << (output->screen() ? output->screen()->id() : -1) << "\n";

        qDebug() << "\n    Available sizes: ";
        foreach (const QSize &size, output->availableSizes()) {
            qDebug() << size.width() << "x" << size.height() << ", ";
        }

        qDebug() << "\n    Available positions: ";
        foreach (const QPoint &pos, output->availablePositions()) {
            qDebug() << "(" << pos.x() << "," << pos.y() << "), ";
        }

        qDebug() << "\n    Available rates: ";
        foreach (float rate, output->availableRates()) {
            qDebug() << rate << ", ";
        }

        qDebug() << "\n";
    }
#endif
}

void KephalApp::qdwScreenResized(int screen)
{
    qDebug() << "                   *****************";
    qDebug() << "                   ** QDesktopWidget:";
    qDebug() << "                   ** screenResized: " << screen;
    QRect geom = QApplication::desktop()->screenGeometry(screen);
    qDebug() << "                   ** New geometry:";
    qDebug() << "                   ** Size: " << geom.width() << "x" << geom.height();
    qDebug() << "                   ** Position: " << geom.x() << "," << geom.y();
    qDebug() << "                   ** isVirtual: " << QApplication::desktop()->isVirtualDesktop();
    qDebug() << "                   *****************";
}

void KephalApp::qdwScreenCountChanged(int newCount)
{
    qDebug() << "                   *****************";
    qDebug() << "                   ** QDesktopWidget:";
    qDebug() << "                   ** Screen Count Changed, now:" << newCount;
    qDebug() << "                   *****************";
}

void KephalApp::qdwWorkAreaResized(int screen)
{
    qDebug() << "                   *****************";
    qDebug() << "                   ** QDesktopWidget:";
    qDebug() << "                   ** workAreaResized: " << screen;
    QRect geom = QApplication::desktop()->availableGeometry(screen);
    qDebug() << "                   ** New geometry:";
    qDebug() << "                   ** Size: " << geom.width() << "x" << geom.height();
    qDebug() << "                   ** Position: (" << geom.x() << "," << geom.y();
    qDebug() << "                   ** isVirtual: " << (QApplication::desktop()->isVirtualDesktop() ? "true" : "false" );
    qDebug() << "                   *****************";
}


