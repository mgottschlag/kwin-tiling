/*
 *   Copyright 2008 Marco Martin <notmart@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef POSITIONINGRULER_H
#define POSITIONINGRULER_H 

#include <QWidget>

#include <plasma/plasma.h>



namespace Plasma
{
    class Containment;
}

class PositioningRuler : public QWidget
{
Q_OBJECT
public:
    PositioningRuler(QWidget* parent = 0);
    ~PositioningRuler();

    QSize sizeHint() const;

    void setLocation(const Plasma::Location &loc);
    Plasma::Location location() const;

    void setAlignment(const Qt::Alignment &align);
    Qt::Alignment alignment() const;

    void setOffset(int newOffset);
    int offset() const;

    void setMaxLength(int newMax);
    int maxLength() const;

    void setMinLength(int newMin);
    int minLength() const;

    void setAvailableLength(int newAvailable);
    int availableLength() const;

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *event);

Q_SIGNALS:
     void rulersMoved(int offset, int minLength, int maxLength);

private:
    class Private;
    Private *d;
};


#endif // multiple inclusion guard

