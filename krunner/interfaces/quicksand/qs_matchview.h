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

#ifndef QS_MATCHVIEW_H
#define QS_MATCHVIEW_H

#include <QList>
#include <QWidget>

class QFocusEvent;
class QKeyPressEvent;
class QResizeEvent;

namespace QuickSand {

    class MatchItem;

    /**
     * @class QsMatchView
     * @short A class to visualize a set of items
     *
     * QsMatchView is composed of a title label, an item count label,
     * a visualization box, and a popup completion box. The visualization
     * box has three modes: scrolling icon, selected item, and text mode.
     * The scrolling icon mode visualizes all items as a single horizontal
     * row of icons. Scrolling is controlled by either the completion box
     * or the view itself.
     */
    class QsMatchView : public QWidget
    {
        Q_OBJECT
        public:
            QsMatchView(QWidget *parent = 0);
            ~QsMatchView();

            /**
             * Removes all items from the scene
             */
            void clear(bool deleteItems = false);
            /**
             * Removes items from the scene and places default find message
             */
            void reset();
            /**
             * Shows the loading animation
             */
            void showLoading();
            /**
             * Sets the list of items to be displayed on screen
             * @param items The list of items to display
             * @param popup Display the popup completion box
             * @param append Append items to the current list instead of replacing it
             */
            void setItems(const QList<MatchItem*> &items, bool popup = true, bool append = false);

            /**
             * Sets the item count text on the upper right hand corner
             * Defaults to "items"
             */
             void setCountingActions(bool actions);

        public slots:
            /**
             * Sets the title text on the upper left hand corner of the widget
             */
            void setTitle(const QString &title);
            /**
             * Display the popup
             */
            void showPopup();
        private slots:
            /**
             * Switches between Icon Parade and Selected Item modes
             */
            void toggleView();
            void scrollToItem(int index);
            /**
             * Uses whole view to display selected item and hides all other matches
             */
            void showSelected();
            /**
             * Shows a scrolling list of icons for each match
             */
            void showList();
            /**
             * Paste the content of the clipboard to the lineedit
             */
            void pasteClipboard();
        signals:
            /**
             * Emitted when the user presser enter
             */
            void itemActivated(MatchItem *item);
            /**
             * Emitted when the user changes selection through either the completion
             * box or the scrolling icon view
             */
            void selectionChanged(MatchItem *item);
            /**
             * Emitted when the internal query string changes
             */
            void textChanged(const QString &text);
        private:
            /**
             * Sets the text in the bottom portion of the widget
             * @param text Text to display
             * @param color Color of the rectangle behind the text
             */
            void setDescriptionText(const QString &text, const QColor &color);
            /**
             * Convenience method. Calls setDescriptionText(text, Plasma::Theme.color())
             */
            void setDescriptionText(const QString &text);

            /**
             * Removes match items from scene but does not delete them
             */
            void clearItems();

            /**
             * Selects item at the specified index
             */
            void selectItem(int index);
            /**
             * Highlights item and centers item when in scrolling icon mode
             */
            void focusItem(int index);
            void scrollLeft();
            void scrollRight();

            void setItemCount(int items);

            void resizeEvent(QResizeEvent *e);
            void focusInEvent(QFocusEvent *event);
            void focusOutEvent(QFocusEvent *event);
            void keyPressEvent(QKeyEvent *event);

            /**
             * If there is already an animation and a new one should start, then the old
             * will be finished
             */
            void finishAnimation();

            class Private;
            Private* const d;
    };
}

#endif
