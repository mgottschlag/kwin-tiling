/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#ifndef RUNNER_H
#define RUNNER_H

#include <QObject>
#include <QList>

#include <kdemacros.h>

class KDE_EXPORT Runner : public QObject
{
    Q_OBJECT

    public:
        typedef QList<Runner*> List;

        explicit Runner(QObject* parent = 0);
        virtual ~Runner();

        virtual bool accepts(const QString& term) = 0;

        virtual bool hasOptions() = 0;
        virtual QWidget* options() = 0;

        virtual bool exec(const QString& command) = 0;

    signals:
        void matches();
};

#define K_EXPORT_KRUNNER_RUNNER( libname, classname )                       \
    K_EXPORT_COMPONENT_FACTORY(                                             \
        krunner_##libname,                                               \
        KGenericFactory<classname>("krunner_" #libname) )

#endif
