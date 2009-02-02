#ifndef KEYBOARD_INPUT_ACTION_WIDGET_H
#define KEYBOARD_INPUT_ACTION_WIDGET_H
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


#include "action_widget_base.h"
#include "ui_keyboard_input_action_widget.h"


/**
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
class KeyboardInputActionWidget : public ActionWidgetBase
    {
    Q_OBJECT

    typedef ActionWidgetBase Base;

public:

    /**
     * Default constructor
     */
    KeyboardInputActionWidget(
            KHotKeys::KeyboardInputAction *action,
            QWidget *parent = NULL);

    /**
     * Destructor
     */
    virtual ~KeyboardInputActionWidget();

    KHotKeys::KeyboardInputAction *action();
    const KHotKeys::KeyboardInputAction *action() const;

    virtual bool isChanged() const;

protected:

    virtual void doCopyFromObject();
    virtual void doCopyToObject();

    Ui::KeyboardInputActionWidget ui;

};


#endif /* #ifndef KEYBOARD_INPUT_ACTION_WIDGET_H */
