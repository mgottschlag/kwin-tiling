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

#include "edit_gesture_dialog.h"

#include <KDE/KLocale>

#include <QtGui/QLabel>
#include <QtGui/QLayout>


EditGestureDialog::EditGestureDialog(const KHotKeys::StrokePoints &pointData, QWidget *parent)
    :   KDialog(parent)
        ,_recorder(this)
        ,_pointData(pointData)
    {
    setCaption(i18n("Edit Gesture"));
    setButtons(KDialog::Ok | KDialog::Cancel);

    QString message(i18n(
                    "Draw the gesture you would like to record below. Press "
                    "and hold the left mouse button while drawing, and release "
                    "when you have finished."));

    QLabel *label = new QLabel(message, this);
    label->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    layout->addWidget(&_recorder);

    QWidget *w = new QWidget;
    w->setLayout(layout);

    setMainWidget(w);

    connect(&_recorder, SIGNAL(recorded(KHotKeys::StrokePoints)),
            SLOT(recorded(KHotKeys::StrokePoints)));
    }


EditGestureDialog::~EditGestureDialog()
    {}


KHotKeys::StrokePoints EditGestureDialog::pointData() const
    {
    return _pointData;
    }


void EditGestureDialog::recorded(const KHotKeys::StrokePoints &data)
    {
    _pointData=data;
    accept();
    }


#include "moc_edit_gesture_dialog.cpp"
