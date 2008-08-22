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
#include "../screens/screens.h"


#include <QDebug>
#include <QDBusConnection>


using namespace kephal;

int main(int argc, char *argv[])
{
    KephalApp app(argc, argv);

    return app.exec();
}


KephalApp::KephalApp(int & argc, char ** argv)
    : QApplication(argc, argv)
{
    qDebug() << "kephal starting up";
    init();
}

KephalApp::~KephalApp()
{
}

void KephalApp::init() {
    qDebug() << "size of screen 0:" << Screens::instance()->screens()[0]->size();
}


