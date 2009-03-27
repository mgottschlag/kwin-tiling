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


#ifndef KEPHALAPP_H
#define KEPHALAPP_H

#include <QApplication>

#include <kephal/screens.h>


class KephalApp : public QApplication
{
    Q_OBJECT
    public:
        KephalApp(int & argc, char ** argv);
        ~KephalApp();
        
    private:
        bool m_listen;
        QString m_arg;
        QString m_exec;
        
        void init(int & argc, char ** argv);
        void query();
        
    private Q_SLOTS:
        void run();
        void printHelp();
        void unknownArg();
        
        void screenMoved(Kephal::Screen * s, QPoint o, QPoint n);
        void screenResized(Kephal::Screen * s, QSize o, QSize n);
        void screenRemoved(int s);
        void screenAdded(Kephal::Screen * s);
};


#endif //KEPHALAPP_H

