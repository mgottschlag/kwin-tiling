/*
 *   Copyright 2010 by Chani Armitage <chani@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library/Lesser General Public License
 *   version 2, or (at your option) any later version, as published by the
 *   Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library/Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef ACTIVITYBAR_H
#define ACTIVITYBAR_H

//gonna need a top bar with filtery things, and a bottom bar with the scrollything.

namespace Plasma
{

class PLASMAGENERICSHELL_EXPORT ActivityBar : public QGraphicsWidget
{
    Q_OBJECT
public:
    ActivityBar(Qt::Orientation orientation, QGraphicsItem *parent=0);
    ActivityBar(QGraphicsItem *parent=0);

};
} // namespace Plasma

#endif
