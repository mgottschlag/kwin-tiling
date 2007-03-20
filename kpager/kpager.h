/**************************************************************************

    kpager.h  - KPager's main window
    Copyright (C) 2000  Antonio Larrosa Jimenez <larrosa@kde.org>
			Matthias Ettrich
			Matthias Elter

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

    Send comments and bug fixes to larrosa@kde.org

***************************************************************************/
#ifndef __KPAGER_H
#define __KPAGER_H

#include <QWidget>
#include <q3intdict.h>
//Added by qt3to4:
#include <QShowEvent>
#include <QFrame>
#include <QGridLayout>
#include <QMenu>
#include <kmainwindow.h>
#include <kwin.h>

class KPager;
class QMenu;
class OrgKdeKdesktopBackgroundInterface;

class KPagerMainWindow : public KMainWindow
{
    Q_OBJECT

public:
    KPagerMainWindow(QWidget *parent=0, const char *name=0);
    ~KPagerMainWindow();

public Q_SLOTS:
	// DBus interface
    virtual void showAt(int x, int y);
    virtual void toggleShow(int x, int y);

public Q_SLOTS:
    virtual void reallyClose();

protected:
    bool queryClose();
    void showEvent(QShowEvent *ev);

    KPager *m_pPager;
    class QTimer *timeout;
    bool m_reallyClose;
};

class KPager : public QFrame
{
    Q_OBJECT

    friend class KPagerMainWindow;

public:
    KPager(KPagerMainWindow *parent=0, const char *name=0);
    ~KPager();

    class KWinModule *kwin() const { return m_winmodule; }
    void updateLayout();

    void redrawDesktops();

    void showPopupMenu( WId wid, QPoint pos);

    KWin::WindowInfo* info( WId win );

    QSize sizeHint() const;

    enum LayoutTypes { Classical=0, Horizontal, Vertical };

public Q_SLOTS:
    void configureDialog();

    void slotActiveWindowChanged( WId win );
    void slotWindowAdded( WId );
    void slotWindowRemoved( WId );
    void slotWindowChanged( WId, unsigned int );
    void slotStackingOrderChanged();
    void slotDesktopNamesChanged();
    void slotNumberOfDesktopsChanged(int ndesktops);
    void slotCurrentDesktopChanged(int);

    void slotGrabWindows();

protected Q_SLOTS:
    void slotBackgroundChanged(int);
    void clientPopupAboutToShow();
    void clientPopupActivated(QAction*);
    void desktopPopupAboutToShow();
    void sendToDesktop(QAction*);

protected:
    enum WindowOperation {
        MaximizeOp = 100,
        IconifyOp,
        StickyOp,
        CloseOp
    };

protected:
    KWinModule *m_winmodule;
    QList<class Desktop *> m_desktops;

    Q3IntDict<KWin::WindowInfo> m_windows;
    WId m_activeWin;

    const QString lWidth();
    const QString lHeight();

    LayoutTypes m_layoutType;

    class QGridLayout *m_layout;
    KMenu *m_mnu;
    QAction *m_mnu_title;
    QAction *m_iconify_action;
    QAction *m_maximize_action;
    QAction *m_sticky_action;
    QMenu *m_smnu, *m_dmnu;
    QAction *m_quit_action;
    QAction *m_prefs_action;
    KWin::WindowInfo m_winfo;

    QTimer *m_grabWinTimer;
    int     m_currentDesktop;
    OrgKdeKdesktopBackgroundInterface* m_backgroundInterface;

public:
    static const LayoutTypes c_defLayout;
};

#endif
