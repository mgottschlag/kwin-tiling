/*****************************************************************

Copyright (c) 1996-2000 the kicker authors. See file AUTHORS.

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

#ifndef __applethandle_h__
#define __applethandle_h__

#include <QWidget>
#include <QPushButton>

#include "utils.h"

#include "container_applet.h"

class QBoxLayout;
class QTimer;
class AppletHandleDrag;
class AppletHandleButton;

class AppletHandle : public QWidget
{
    Q_OBJECT
    public:
        AppletHandle(AppletContainer* parent);

        void resetLayout();
        void setFadeOutHandle(bool);

        bool eventFilter (QObject *, QEvent *);

        int widthForHeight( int h ) const;
        int heightForWidth( int w ) const;

        void setPopupDirection(Plasma::Position);
        Plasma::Position popupDirection() const
        {
            return m_popupDirection;
        }

        Qt::Orientation orientation() const
        {
            return m_applet->orientation();
        }

        bool onMenuButton(const QPoint& point) const;

    Q_SIGNALS:
        void moveApplet( const QPoint& moveOffset );
        void showAppletMenu();

    public Q_SLOTS:
        void toggleMenuButtonOff();

    protected Q_SLOTS:
        void menuButtonPressed();
        void checkHandleHover();

    private:
        QPixmap xpmPixmap(const char* const xpm[], const char* key);

        AppletContainer* m_applet;
        QBoxLayout* m_layout;
        AppletHandleDrag* m_dragBar;
        AppletHandleButton* m_menuButton;
        bool m_drawHandle;
        Plasma::Position m_popupDirection;
        QTimer* m_handleHoverTimer;
};

class AppletHandleDrag : public QWidget
{
    Q_OBJECT
    public:
        AppletHandleDrag(AppletHandle* parent);

        QSize minimumSizeHint() const;
        QSize minimumSize() const { return minimumSizeHint(); }
        QSize sizeHint() const { return minimumSize(); }
        QSizePolicy sizePolicy() const;

    protected:
        void paintEvent(QPaintEvent *);
        const AppletHandle* m_parent;
};

class AppletHandleButton : public QPushButton
{
    Q_OBJECT
    public:
        AppletHandleButton(AppletHandle *parent);
        ~AppletHandleButton();
        QSize minimumSizeHint() const;
        QSize minimumSize() const { return minimumSizeHint(); }
        QSize sizeHint() const { return minimumSize(); }
        QSizePolicy sizePolicy() const;

        void setPixmap(const QPixmap&);
        const QPixmap& pixmap();

    protected:
        void paintEvent(QPaintEvent*);
        void enterEvent(QEvent*);
        void leaveEvent(QEvent*);

    private:
        QPixmap m_pixmap;
        bool m_moveMouse;
        const AppletHandle* m_parent;
};

#endif
