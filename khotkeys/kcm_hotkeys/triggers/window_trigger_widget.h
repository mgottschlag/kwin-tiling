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
#ifndef WINDOW_TRIGGER_WIDGET_H
#define WINDOW_TRIGGER_WIDGET_H

#include "trigger_widget_base.h"

#include "ui_window_trigger_widget.h"

class QKeySequence;
class WindowDefinitionListWidget;

/**
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
class WindowTriggerWidget : public TriggerWidgetBase
    {
    Q_OBJECT

    typedef TriggerWidgetBase Base;

public:

    /**
     * Default constructor
     */
    WindowTriggerWidget( KHotKeys::WindowTrigger *trigger, QWidget *parent = 0 );

    /**
     * Destructor
     */
    virtual ~WindowTriggerWidget();

    KHotKeys::WindowTrigger *trigger();
    const KHotKeys::WindowTrigger *trigger() const;


    virtual bool isChanged() const;

private Q_SLOTS:

    void slotWindowSelectionChanged(bool) const;

private:

    virtual void doCopyFromObject();
    virtual void doCopyToObject();

    Ui::WindowTriggerWidget window_trigger_ui;

    WindowDefinitionListWidget *_windowdef_widget;

};

#endif /* #ifndef WINDOW_TRIGGER_WIDGET_H */

