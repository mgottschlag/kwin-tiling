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

#include "hotkeys_widget_iface.h"


HotkeysWidgetIFace::HotkeysWidgetIFace( QWidget *parent )
        : QWidget(parent)
         ,_changedSignals(new QSignalMapper(this))
    {
    // Listen to the signal mapper, but not yet
    connect(
        _changedSignals, SIGNAL(mapped(QString)),
        this, SLOT(slotChanged(QString)) );
    _changedSignals->blockSignals(true);
    }


HotkeysWidgetIFace::~HotkeysWidgetIFace()
    {}


void HotkeysWidgetIFace::copyFromObject()
    {
    _changedSignals->blockSignals(true);
    doCopyFromObject();
    _changedSignals->blockSignals(false);
    }

void HotkeysWidgetIFace::copyToObject()
    {
    doCopyToObject();
    }


void HotkeysWidgetIFace::slotChanged( const QString & /* what */ )
    {
    emit changed(isChanged());
    }




#include "moc_hotkeys_widget_iface.cpp"
