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
#include <QTimer>

#include <QTextStream>

#include <iostream>


using namespace Kephal;

int main(int argc, char *argv[])
{
    KephalApp app(argc, argv);
    return app.exec();
}


KephalApp::KephalApp(int & argc, char ** argv)
    : QApplication(argc, argv), m_listen(false)
{
    init(argc, argv);
}

KephalApp::~KephalApp()
{
}

void KephalApp::init(int & argc, char ** argv) {
    for (int i = 1; i < argc; ++i) {
        QString arg = argv[i];
        
        if (arg == "--listen" || arg == "-l") {
            m_listen = true;
        } else if (arg == "--help" || arg == "-?") {
            QTimer::singleShot(0, this, SLOT(printHelp()));
            return;
        } else {
            m_exec = argv[0];
            m_arg = arg;
            QTimer::singleShot(0, this, SLOT(unknownArg()));
            return;
        }
    }
    
    QTimer::singleShot(0, this, SLOT(run()));
}

void KephalApp::unknownArg() {
    QTextStream cerr(stderr);
    
    cerr << "Unknown argument: " << m_arg << "\n";
    cerr << "Try: " << m_exec << " --help\n";
    exit(1);
}

void KephalApp::printHelp() {
    QTextStream cout(stdout);
    cout << "Usage:\n";
    cout << "    --help      Display this message\n";
    cout << "    --listen    Listen for Kephal-signals\n";
    exit(0);
}

void KephalApp::run() {
    query();
    if (! m_listen) {
        exit(0);
    }
    
    connect(Kephal::Screens::self(), SIGNAL(screenMoved(Kephal::Screen *, QPoint, QPoint)), this, SLOT(screenMoved(Kephal::Screen *, QPoint, QPoint)));
    connect(Kephal::Screens::self(), SIGNAL(screenResized(Kephal::Screen *, QSize, QSize)), this, SLOT(screenResized(Kephal::Screen *, QSize, QSize)));
    connect(Kephal::Screens::self(), SIGNAL(screenRemoved(int)), this, SLOT(screenRemoved(int)));
    connect(Kephal::Screens::self(), SIGNAL(screenAdded(Kephal::Screen *)), this, SLOT(screenAdded(Kephal::Screen *)));
}

void KephalApp::screenMoved(Kephal::Screen * s, QPoint o, QPoint n) {
    QTextStream cout(stdout);
    cout << "****************\n";
    cout << "* New message coming in:\n";
    cout << "* screenMoved: " << s->id() << " from (" << o.x() << ", " << o.y() << ") to (" << n.x() << ", " << n.y() << ")\n";
    cout << "****************\n";
}

void KephalApp::screenResized(Kephal::Screen * s, QSize o, QSize n) {
    QTextStream cout(stdout);
    cout << "****************\n";
    cout << "* New message coming in:\n";
    cout << "* screenResized: " << s->id() << " from (" << o.width() << ", " << o.height() << ") to (" << n.width() << ", " << n.height() << ")\n";
    cout << "****************\n";
}

void KephalApp::screenRemoved(int s) {
    QTextStream cout(stdout);
    cout << "****************\n";
    cout << "* New message coming in:\n";
    cout << "* screenRemoved: " << s << "\n";
    cout << "****************\n";
}

void KephalApp::screenAdded(Kephal::Screen * s) {
    QTextStream cout(stdout);
    cout << "****************\n";
    cout << "* New message coming in:\n";
    cout << "* screenAdded: " << s->id() << " at (" << s->position().x() << ", " << s->position().y() << ") with size (" << s->size().width() << ", " << s->size().height() << ")\n";
    cout << "****************\n";
}


void KephalApp::query() {
    QTextStream cout(stdout);
    cout << "Screens:\n";
    foreach (Screen * screen, Screens::self()->screens()) {
        cout << "  Screen " << screen->id() << ":\n";
        cout << "    Size: " << screen->size().width() << "x" << screen->size().height() << "\n";
        cout << "    Position: (" << screen->position().x() << "," << screen->position().y() << ")\n";
        
        foreach (Output * output, screen->outputs()) {
            cout << "    Output: " << output->id() << "\n";
        }
    }
    
    cout << "\nOutputs:\n";
    foreach (Output * output, Outputs::self()->outputs()) {
        cout << "  Output " << output->id() << ":\n";
        cout << "    Connected: " << output->isConnected() << "\n";
        
        if (! output->isConnected()) continue;
        
        cout << "    Activated: " << output->isActivated() << "\n";
        cout << "    Size: " << output->size().width() << "x" << output->size().height() << "\n";
        cout << "    Position: (" << output->position().x() << "," << output->position().y() << ")\n";
        cout << "    Vendor: " << output->vendor() << "\n";
        cout << "    PreferredSize: " << output->preferredSize().width() << "x" << output->preferredSize().height() << "\n";
        cout << "    Rotation: " << output->rotation() << "\n";
        cout << "    ReflectX: " << output->reflectX() << "\n";
        cout << "    ReflectY: " << output->reflectY() << "\n";
        cout << "    Rate: " << output->rate() << "\n";
        cout << "    Screen: " << (output->screen() ? output->screen()->id() : -1) << "\n";

        cout << "\n    Available sizes: ";
        foreach (QSize size, output->availableSizes()) {
            cout << size.width() << "x" << size.height() << ", ";
        }

        cout << "\n    Available positions: ";
        foreach (QPoint pos, output->availablePositions()) {
            cout << "(" << pos.x() << "," << pos.y() << "), ";
        }

        cout << "\n    Available rates: ";
        foreach (float rate, output->availableRates()) {
            cout << rate << ", ";
        }
        
        cout << "\n";
    }
}


