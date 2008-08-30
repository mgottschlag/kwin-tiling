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


using namespace kephal;

int main(int argc, char *argv[])
{
    KephalApp app(argc, argv);

    return app.exec();
}


KephalApp::KephalApp(int & argc, char ** argv)
    : QApplication(argc, argv)
{
    //qDebug() << "kephal starting up";
    init();
}

KephalApp::~KephalApp()
{
}

void KephalApp::init() {

    QTimer::singleShot(0, this, SLOT(run()));
}

void KephalApp::run() {
    query();
    exit(0);
}

void KephalApp::query() {
    QTextStream cout(stdout);
    cout << "Screens:\n";
    foreach (Screen * screen, Screens::instance()->screens()) {
        cout << "  Screen " << screen->id() << ":\n";
        cout << "    Size: " << screen->size().width() << "x" << screen->size().height() << "\n";
        cout << "    Position: (" << screen->position().x() << "," << screen->position().y() << ")\n";
        
        foreach (Output * output, screen->outputs()) {
            cout << "    Output: " << output->id() << "\n";
        }
    }
}


