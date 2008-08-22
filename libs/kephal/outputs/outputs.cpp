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


#include <QDebug>

#include "outputs.h"
#include "screens/screens.h"
#include "configurations/configurations.h"

#ifdef OUTPUTS_FACTORY
void OUTPUTS_FACTORY();
#endif


namespace kephal {

    Outputs * Outputs::instance() {
#ifdef OUTPUTS_FACTORY
        if (Outputs::m_instance == 0) {
            OUTPUTS_FACTORY();
        }
#endif
        return Outputs::m_instance;
    }
    
    Outputs::Outputs(QObject * parent)
            : QObject(parent)
    {
        Outputs::m_instance = this;
    }
    
    Output * Outputs::output(const QString & id) {
        foreach (Output * o, outputs()) {
            if (o->id() == id) {
                return o;
            }
        }
        return 0;
    }
    
    Outputs * Outputs::m_instance = 0;
    
    
    
    Output::Output(QObject * parent)
            : QObject(parent)
    {
    }
    
    QRect Output::geom() {
        return QRect(position(), size());
    }
    
    Screen * Output::screen() {
        if (! isActivated()) {
            return 0;
        }
        
        foreach (Screen * screen, Screens::instance()->screens()) {
            if (screen->outputs().contains(this)) {
                return screen;
            }
        }
        return 0;
    }
    
    QList<QPoint> Output::availablePositions() {
        return Configurations::instance()->possiblePositions(this);
    }
    
    void Output::setPosition(const QPoint & position) {
        qDebug() << "Output::setPosition() called:" << position;
        Configurations::instance()->move(this, position);
    }
    
    void Output::setSize(const QSize & size) {
        qDebug() << "Output::setSize() called:" << size;
        Configurations::instance()->resize(this, size);
    }
    
}

