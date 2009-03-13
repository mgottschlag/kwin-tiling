#ifndef EDIT_GESTURE_DIALOG_H
#define EDIT_GESTURE_DIALOG_H
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

#include "helper_widgets/gesture_recorder.h"
#include "triggers/gestures.h"


#include <KDE/KDialog>

/**
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
/**
 * This dialog is what pops up after clicking the edit button. It contains
 * a GestureRecorder to enable the input of a new gesture.
 */
class EditGestureDialog : public KDialog
    {
    Q_OBJECT

public:

    /**
     * Default constructor
     */
    EditGestureDialog(const KHotKeys::StrokePoints &pointData, QWidget *parent=NULL);

    /**
     * Destructor
     */
    virtual ~EditGestureDialog();

    KHotKeys::StrokePoints pointData() const;

private Q_SLOTS:

    void recorded(const KHotKeys::StrokePoints &data);

private:

    GestureRecorder _recorder;
    KHotKeys::StrokePoints _pointData;

    };


#endif /* #ifndef EDIT_GESTURE_DIALOG_H */
