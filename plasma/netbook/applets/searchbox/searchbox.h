/*
 *   Copyright 2009 by Artur Duque de Souza <morpheuz@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2,
 *   or (at your option) any later version.
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

#ifndef SEARCHBOX_APPLET_H
#define SEARCHBOX_APPLET_H

#include <Plasma/PopupApplet>


namespace Plasma
{
    class IconWidget;
    class LineEdit;
}

class QTimer;

class SearchBox: public Plasma::PopupApplet
{
    Q_OBJECT

public:
    SearchBox(QObject *parent, const QVariantList &args);
    ~SearchBox();
    void init();

    QGraphicsWidget *graphicsWidget();

public slots:
    void delayedQuery();
    void query();

protected:
    void popupEvent(bool shown);
    void focusInEvent(QFocusEvent *event);
    bool eventFilter(QObject* watched, QEvent *event);
    void focusEditor();

private:
    QGraphicsWidget *m_widget;
    Plasma::LineEdit *m_search;
    Plasma::IconWidget *m_closeIcon;
    QTimer *m_searchTimer;
};

K_EXPORT_PLASMA_APPLET(searchbox, SearchBox)

#endif
