/*****************************************************************

Copyright (c) 2005 Marc Cramdal
Copyright (c) 2005 Aaron Seigo <aseigo@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#ifndef _appletwidget_h_
#define _appletwidget_h_

#include <QPoint>

#include <QList>
#include <QMouseEvent>
#include <QEvent>

#include <klocale.h>
#include <kdialogbase.h>

#include "appletinfo.h"
#include "appletitem.h"

class AppletWidget: public AppletItem
{
    Q_OBJECT

    public:
        typedef QList<AppletWidget*> List;

        AppletWidget(const AppletInfo& info, bool odd, QWidget *parent);
        const AppletInfo& info() const { return m_appletInfo; }
        virtual bool eventFilter(QObject* watched, QEvent* e);

        void setSelected(bool selected);
        void setOdd(bool odd);
        bool odd() { return m_odd; }

    Q_SIGNALS:
        void clicked(AppletWidget*);
        void doubleClicked(AppletWidget*);

    protected:
        void keyPressEvent(QKeyEvent *e);
        void mousePressEvent(QMouseEvent *e);
        void mouseMoveEvent(QMouseEvent *e);
        void mouseReleaseEvent(QMouseEvent *e);
        void mouseDoubleClickEvent(QMouseEvent *e);
        void focusInEvent(QFocusEvent* e);
        void focusOutEvent(QFocusEvent* e);

    private:
        AppletInfo m_appletInfo;
        bool m_odd;
        bool m_selected;
        QPoint m_dragStart;
};

#endif

