/* This file is part of the KDE project
   Copyright (C) 2003-2004 Nadeem Hasan <nhasan@kde.org>
   Copyright (C) 2004-2005 Aaron J. Seigo <aseigo@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef SIMPLEBUTTON_H
#define SIMPLEBUTTON_H

#include <QAbstractButton>

#include <kdemacros.h>

class QPainter;
class QPixmap;
class QEvent;
class QResizeEvent;
class QPaintEvent;

class KDE_EXPORT SimpleButton : public QAbstractButton
{
    Q_OBJECT

    public:
        SimpleButton(QWidget *parent);
        KDE_CONSTRUCTOR_DEPRECATED SimpleButton(QWidget *parent, const char *name);
        virtual ~SimpleButton();
        KDE_DEPRECATED void setPixmap(const QPixmap &pix) { return setIcon(QIcon(pix)); }
        void setIcon(const QIcon &icon);
        void setOrientation(Qt::Orientation orientaton);
        QSize sizeHint() const;

    protected:
        void drawButton( QPainter *p );
        void drawButtonLabel( QPainter *p );
        void generateIcons();

        void enterEvent( QEvent *e );
        void leaveEvent( QEvent *e );
        void resizeEvent( QResizeEvent *e );
        void paintEvent( QPaintEvent *e );

    protected Q_SLOTS:
        void slotSettingsChanged( int category );
        void slotIconChanged( int group );

    private:
        class Private;
        Private* d;
};

#endif // HIDEBUTTON_H

// vim:ts=4:sw=4:et
