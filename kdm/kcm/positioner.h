/*
    Copyright (C) 2006 Oswald Buddenhagen <ossi@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef POSITIONER_H
#define POSITIONER_H

#include "kworkspace/screenpreviewwidget.h"

class QFrame;
class QLabel;

class Positioner : public ScreenPreviewWidget {
    Q_OBJECT

  public:
    Positioner(QWidget *parent);
    void setPosition(int x, int y);
    int x() const { return m_x; }
    int y() const { return m_y; }

  Q_SIGNALS:
    void positionChanged();

  protected:
    virtual void resizeEvent(QResizeEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void focusInEvent(QFocusEvent *event);
    virtual void focusOutEvent(QFocusEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual int heightForWidth(int w) const;

  private:
    void updateHandle();

    int m_x, m_y;
    QPoint m_delta;
    QFrame *m_frame;
    QWidget *m_screen;
    QFrame *m_dlg;
    QLabel *m_ptr;
    QPixmap m_anchor;

};

#endif // POSITIONER_H
