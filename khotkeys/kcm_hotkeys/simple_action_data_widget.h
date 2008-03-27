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
#ifndef SIMPLE_ACTION_DATA_WIDGET_H
#define SIMPLE_ACTION_DATA_WIDGET_H

#include "hotkeys_widget_base.h"

#include "ui_simple_action_data_widget.h"
#include "simple_action_data.h"

#include <QtGui/QWidget>


/**
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
class SimpleActionDataWidget : public HotkeysWidgetBase
    {
    Q_OBJECT

    typedef HotkeysWidgetBase Base;

public:

    /**
     * Default constructor
     */
    SimpleActionDataWidget( QWidget *parent = 0 );


    /**
     * Edit \a action.
     */
    void setActionData( KHotKeys::SimpleActionData *action );


    KHotKeys::SimpleActionData *data()
        {
        return static_cast<KHotKeys::SimpleActionData*>( _data );
        }

    const KHotKeys::SimpleActionData *data() const
        {
        return static_cast<const KHotKeys::SimpleActionData*>( _data );
        }


    /**
     * Destructor
     */
    virtual ~SimpleActionDataWidget();

    virtual bool isChanged() const;


protected:

    virtual void doCopyFromObject();
    virtual void doCopyToObject();

private:

    Ui::SimpleActionDataWidget ui;

    HotkeysWidgetIFace *currentTrigger;
    HotkeysWidgetIFace *currentAction;

};

#endif /* #ifndef SIMPLE_ACTION_DATA_WIDGET_H */
