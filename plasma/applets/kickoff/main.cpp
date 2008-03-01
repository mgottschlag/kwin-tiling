/*  
    Copyright 2007 Robert Knight <robertknight@gmail.com>

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

// KDE
#include <KAboutData>
#include <KApplication>
#include <KCmdLineArgs>

// Local
#include "ui/launcher.h"

#define KICKOFF_VERSION "1.9.3"

int main(int argc,char** argv)
{
    KAboutData about("kickoff",0,ki18n("Kickoff"),KICKOFF_VERSION,
                     ki18n("Application Launcher"),
                     KAboutData::License_GPL_V2);
    KCmdLineArgs::init(argc,argv,&about);

    KApplication app;

    Kickoff::Launcher *launcher = new Kickoff::Launcher((QObject*)0);
    launcher->setWindowTitle("Kickoff KDE 4 Test Application");
    // ensure launcher gets deleted when the app closes so that favorites, 
    // recent applications etc. are saved
    launcher->setAttribute(Qt::WA_DeleteOnClose);
    launcher->show();

    return app.exec();
}

