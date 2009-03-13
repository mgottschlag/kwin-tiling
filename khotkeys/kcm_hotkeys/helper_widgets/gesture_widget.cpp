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

#include "gesture_widget.h"
#include "helper_widgets/edit_gesture_dialog.h"


GestureWidget::GestureWidget(QWidget *parent)
    :   QWidget(parent)
    {
    ui.setupUi(this);

    connect(ui.edit_button, SIGNAL(clicked(bool)),
            SLOT(edit()));
    }


GestureWidget::~GestureWidget()
    {
    }


void GestureWidget::edit()
    {
    EditGestureDialog dia(ui.gesture->pointData());
    switch (dia.exec())
        {
        case KDialog::Accepted:
            setPointData(dia.pointData(), true);
            break;

        case KDialog::Rejected:
            break;

        default:
            Q_ASSERT(false);
        }
    }




KHotKeys::StrokePoints GestureWidget::pointData() const
    {
    return ui.gesture->pointData();
    }


void GestureWidget::setPointData(const KHotKeys::StrokePoints &data , bool emitSignal)
    {
    ui.gesture->setPointData(data);
    if(emitSignal)
        emit changed();
    }

#include "moc_gesture_widget.cpp"
