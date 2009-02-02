#ifndef CONDITION_TYPE_MENU_H
#define CONDITION_TYPE_MENU_H
/* Copyright (C) 2009 Michael Jansen <kde@michael-jansen.biz>

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


#include <QtGui/QMenu>


/**
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
class ConditionTypeMenu : public QMenu
    {
    Q_OBJECT

public:

    /**
     * Default constructor
     */
    ConditionTypeMenu(QWidget *parent = NULL);

    /**
     * Destructor
     */
    virtual ~ConditionTypeMenu();

    enum ConditionType{
            ACTIVE_WINDOW,
            EXISTING_WINDOW,
            AND,
            OR,
            NOT};

};


#endif /* #ifndef CONDITION_TYPE_MENU_H */
