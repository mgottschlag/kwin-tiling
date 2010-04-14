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


#include "configurations.h"

#include <QDebug>
#include <QRegExp>

#ifdef CONFIGURATIONS_FACTORY
void CONFIGURATIONS_FACTORY();
#endif

#include "outputs.h"
#include "screens.h"
//#include "backend.h"


namespace Kephal {

    Configurations * Configurations::self() {
#ifdef CONFIGURATIONS_FACTORY
        if (Configurations::s_instance == 0) {
            CONFIGURATIONS_FACTORY();
        }
#endif
        return Configurations::s_instance;
    }

    Configurations::Configurations(QObject * parent)
            : QObject(parent)
    {
        Configurations::s_instance = this;
    }

    Configurations::~Configurations()
    {
        Configurations::s_instance = 0;
    }

    Configurations * Configurations::s_instance = 0;

    Configuration * Configurations::configuration(QString name)
    {
        foreach (Configuration * config, configurations()) {
            if (config->name() == name) {
                return config;
            }
        }
        return 0;
    }

    void Configurations::translateOrigin(QMap<int, QPoint> & layout)
    {
        QPoint origin;
        bool first = true;
        for (QMap<int, QPoint>::const_iterator i = layout.constBegin(); i != layout.constEnd(); ++i) {
            if (first || (i.value().x() < origin.x())) {
                origin.setX(i.value().x());
            }
            if (first || (i.value().y() < origin.y())) {
                origin.setY(i.value().y());
            }
            first = false;
        }
        translateOrigin(layout, origin);
    }

    void Configurations::translateOrigin(QMap<int, QPoint> & layout, QPoint origin) {
        for (QMap<int, QPoint>::iterator i = layout.begin(); i != layout.end(); ++i) {
            i.value() -= origin;
        }
    }

    void Configurations::translateOrigin(QMap<int, QRect> & layout) {
        QPoint origin;
        bool first = true;
        for (QMap<int, QRect>::const_iterator i = layout.constBegin(); i != layout.constEnd(); ++i) {
            if (first || (i.value().x() < origin.x())) {
                origin.setX(i.value().x());
            }
            if (first || (i.value().y() < origin.y())) {
                origin.setY(i.value().y());
            }
            first = false;
        }
        translateOrigin(layout, origin);
    }

    void Configurations::translateOrigin(QMap<int, QRect> & layout, QPoint origin) {
        QPoint offset(0, 0);
        offset -= origin;
        for (QMap<int, QRect>::iterator i = layout.begin(); i != layout.end(); ++i) {
            i.value().translate(offset);
        }
    }



    Configuration::Configuration(QObject * parent)
            : QObject(parent)
    {
    }


}

