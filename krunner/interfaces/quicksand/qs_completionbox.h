/*
 *   Copyright (C) 2007-2008 Ryan P. Bitanga <ryan.bitanga@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .
 */

#ifndef QS_COMPLETIONBOX_H
#define QS_COMPLETIONBOX_H

#include <KCompletionBox>

class QKeyEvent;

namespace QuickSand {

    class QsStatusBar;
    
    /**
     * A completion box with configurable offset
     * and a label that shows the current position.
     */
    class QsCompletionBox : public KCompletionBox
    {
    Q_OBJECT
    public:
        QsCompletionBox(QWidget *parent = 0);

        /**
         * Set the offset of the completion box
         */
//         void setOffset(int x, int y);

        QSize minimumSizeHint() const;

        QSize sizeHint() const;

        /**
         * @internal
         */
        void updateGeometries();
    public Q_SLOTS:
        void popup();

    protected Q_SLOTS:
        void slotRowsChanged(const QModelIndex & parent, int start, int end);

    protected:
        QPoint globalPositionHint() const;
//        void resizeEvent(QResizeEvent *e);

        QRect calculateGeometry() const;

    private:
        QsStatusBar *m_status;
        QPoint m_offset;
    };
}

#endif
