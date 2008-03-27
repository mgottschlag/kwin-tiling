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
#ifndef HOTKEYS_WIDGET_IFACE
#define HOTKEYS_WIDGET_IFACE

#include "libkhotkeysfwd.h"


#include <QtCore/QSignalMapper>
#include <QtGui/QWidget>


/**
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
class HotkeysWidgetIFace : public QWidget
    {
    Q_OBJECT

public:

    /**
     * Default constructor
     */
    HotkeysWidgetIFace( QWidget *parent = 0 );

    /**
     * Destructor
     */
    virtual ~HotkeysWidgetIFace();

    virtual bool isChanged() const = 0;
    void copyFromObject();
    void copyToObject();

Q_SIGNALS:

    virtual void changed(bool) const;

public Q_SLOTS:

    virtual void slotChanged(const QString &what = "Nothing" );


protected:

    QSignalMapper *_changedSignals;

    virtual void doCopyFromObject() = 0;
    virtual void doCopyToObject() = 0;

};

#endif /* #ifndef HOTKEYS_WIDGET_BASE_H */

