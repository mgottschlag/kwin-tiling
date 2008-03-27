/*
   Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

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
#ifndef ACTIONGROUPWIDGET_H
#define ACTIONGROUPWIDGET_H


#include "hotkeys_widget_base.h"

/**
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
class ActionGroupWidget : public HotkeysWidgetBase
    {
    Q_OBJECT

    typedef HotkeysWidgetBase Base;

public:

    /**
     * Default constructor
     */
    ActionGroupWidget( QWidget *parent = 0 );

    /**
     * Destructor
     */
    virtual ~ActionGroupWidget();

    /**
     * The associated action.
     */
    KHotKeys::ActionDataBase *data()
        {
        return static_cast<KHotKeys::ActionDataBase*>( _data );
        }

    const KHotKeys::ActionDataBase *data() const
        {
        return static_cast<const KHotKeys::ActionDataBase*>( _data );
        }

    void setActionData( KHotKeys::ActionDataGroup *group );

private:

    void doCopyFromObject();
    void doCopyToObject();

};

#endif /* #ifndef ACTIONGROUPWIDGET_HPP */

