/***************************************************************************
 *   Copyright 2008 by Davide Bettio <davide.bettio@kdemail.net>           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include <QGraphicsLinearLayout>

#include <KDebug>

#include <plasma/svg.h>
#include <plasma/theme.h>

#include "calendar.h"

CalendarTest::CalendarTest(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
    m_sizedirty(true)
{
    resize(330, 240);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
}

void CalendarTest::init()
{
    cwdg = new Plasma::Calendar(this);
    cwdg->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QGraphicsLinearLayout *m_layout = new QGraphicsLinearLayout(Qt::Vertical, this);
    m_layout->addItem(cwdg);
    m_layout->setAlignment(cwdg, Qt::AlignHCenter);
}

CalendarTest::~CalendarTest()
{

}

Qt::Orientations CalendarTest::expandingDirections() const
{
    return 0;
}

QSizeF CalendarTest::contentSizeHint() const
{
    QSizeF sizeHint = geometry().size();

    switch (formFactor()) {
        case Plasma::Vertical:
            sizeHint.setHeight(sizeHint.width());
            break;

        case Plasma::Horizontal:
            sizeHint.setWidth(sizeHint.height() / 4);
            break;

        default:
            sizeHint.setWidth(sizeHint.height() / 4);
            break;
    }

    return sizeHint;
}

void CalendarTest::configAccepted()
{
    update();
}

void CalendarTest::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint || constraints & Plasma::SizeConstraint) {
        m_sizedirty = true;
    }
}

#include "calendar.moc"
