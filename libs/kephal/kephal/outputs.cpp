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


#include "outputs.h"

#include <QDebug>

#include "screens.h"
#include "configurations.h"

#ifdef OUTPUTS_FACTORY
void OUTPUTS_FACTORY();
#endif


namespace Kephal {

    Outputs * Outputs::self() {
#ifdef OUTPUTS_FACTORY
        if (Outputs::s_instance == 0) {
            OUTPUTS_FACTORY();
        }
#endif
        return Outputs::s_instance;
    }

    Outputs::Outputs(QObject * parent)
            : QObject(parent)
    {
        Outputs::s_instance = this;
    }

    Outputs::~Outputs() {
        Outputs::s_instance = 0;
    }

    Output * Outputs::output(const QString & id) {
        foreach (Output * o, outputs()) {
            if (o->id() == id) {
                return o;
            }
        }
        return 0;
    }

    Outputs * Outputs::s_instance = 0;



    Output::Output(QObject * parent)
            : QObject(parent)
    {
    }

    QRect Output::geom() const {
        return QRect(position(), size());
    }

// Outputs should not know about Screens
    Screen * Output::screen() const {
        if (! isActivated()) {
            return 0;
        }

        foreach (Screen * screen, Screens::self()->screens()) {
            Output * out = const_cast<Output*>(this);
            if (screen->outputs().contains(out)) {
                return screen;
            }
        }
        return 0;
    }
    QList<QPoint> Output::availablePositions() {
        return QList<QPoint>();
        //return Configurations::self()->possiblePositions(this);
    }

#if 0
    bool Output::move(const QPoint & position) {
        return Configurations::self()->move(this, position);
    }

    bool Output::resize(const QSize & size) {
        return Configurations::self()->resize(this, size);
    }

    bool Output::rotate(Rotation rotation) {
        return Configurations::self()->rotate(this, rotation);
    }

    bool Output::reflectX(bool reflect) {
        return Configurations::self()->reflectX(this, reflect);
    }

    bool Output::reflectY(bool reflect) {
        return Configurations::self()->reflectY(this, reflect);
    }

    bool Output::changeRate(double rate) {
        return Configurations::self()->changeRate(this, rate);
    }
#endif
}
