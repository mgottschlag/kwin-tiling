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
#ifndef HOTKEYS_WIDGET_BASE_H
#define HOTKEYS_WIDGET_BASE_H

#include "ui_hotkeys_widget_base.h"


#include "hotkeys_widget_iface.h"
#include "libkhotkeysfwd.h"


class HotkeysWidgetBasePrivate;


/**
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
class HotkeysWidgetBase : public HotkeysWidgetIFace
    {
    Q_OBJECT

public:

    /**
     * Default constructor
     */
    HotkeysWidgetBase( QWidget *parent = 0 );

    /**
     * Destructor
     */
    virtual ~HotkeysWidgetBase();

    /**
     * The associated action.
     */
    KHotKeys::ActionDataBase *data()
        {
        return _data;
        }

    const KHotKeys::ActionDataBase *data() const
        {
        return _data;
        }

    virtual bool isChanged() const;

    virtual QString title() const;


protected:

    /**
     * Append the QLayoutItems from QGridLayout \from to QGridLayout \to.
     */
    void mergeLayouts( QGridLayout *to, QGridLayout *from );

Q_SIGNALS:

    void changed(bool) const;

protected:

    virtual void doCopyFromObject();
    virtual void doCopyToObject();

    Ui::HotkeysWidgetBase ui;

    KHotKeys::ActionDataBase *_data;
};

#endif /* #ifndef HOTKEYS_WIDGET_BASE_H */
