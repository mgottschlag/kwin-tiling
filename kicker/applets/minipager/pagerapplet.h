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

#ifndef __MINIPAGER_H
#define __MINIPAGER_H

#include <QWheelEvent>
#include <QGridLayout>
#include <QResizeEvent>
#include <QByteArray>
#include <QPaintEvent>
#include <QHash>

#include <kpanelapplet.h>
#include <kwm.h>

#include "pagerbutton.h"
#include "pagersettings.h"

class QButtonGroup;
class QGridLayout;
class QTimer;

class K3Process;
class KShadowEngine;
class OrgKdeKdesktopBackgroundInterface;
class KSelectionOwner;

class PagerSettings;

class KMiniPager : public KPanelApplet
{
    Q_OBJECT

public:
    KMiniPager(const QString& configFile, Plasma::Type t = Plasma::Normal, int actions = 0,
               QWidget *parent = 0, const char *name = 0);

    virtual ~KMiniPager();

    int widthForHeight(int height) const;
    int heightForWidth(int width) const;

    KWM::WindowInfo* info( WId win );
    KShadowEngine* shadowEngine();

    void setActive( WId active ) { m_activeWindow = active; }
    WId activeWindow() { return m_activeWindow; }

    enum ConfigOptions { LaunchExtPager = 96, WindowThumbnails,
                         WindowIcons, ConfigureDesktops, RenameDesktop };
    int labelType() const { return m_settings->labelType(); }

    int bgType() const { return m_settings->backgroundType(); }

    bool desktopPreview() const { return m_settings->preview(); }
    bool windowIcons() const { return m_settings->icons(); }

    Qt::Orientation orientation() const { return KPanelApplet::orientation(); }
    Plasma::Position popupDirection() { return KPanelApplet::popupDirection(); }

    void emitRequestFocus(){ emit requestFocus(); }

    QPoint clickPos;

public Q_SLOTS:
    void slotSetDesktop(int desktop);
    void slotSetDesktopCount(int count);
    void slotButtonSelected(int desk );
    void slotActiveWindowChanged( WId win );
    void slotWindowAdded( WId );
    void slotWindowRemoved( WId );
    void slotWindowChanged( WId, unsigned int );
    void slotShowMenu( const QPoint&, int );
    void slotDesktopNamesChanged();
    void slotBackgroundChanged( int );

    void refresh();

protected:
    void drawButtons();
    void startDrag( const QPoint &point );

    void updateDesktopLayout(int,int,int);
    void paintEvent(QPaintEvent *ev);
    void resizeEvent(QResizeEvent*);
    void wheelEvent( QWheelEvent* e );
    void showKPager(bool toggleShow);

protected Q_SLOTS:
    void showPager();
    void applicationRegistered(const QByteArray &appName);
    void aboutToShowContextMenu();
    void contextMenuActivated(int);

private:
    QList<KMiniPagerButton*> m_desktops;
    int m_curDesk;
    int m_rmbDesk;

    QHash< int, KWM::WindowInfo* > m_windows;
    WId m_activeWindow;

    QButtonGroup *m_group;

    QGridLayout *m_layout;
    int desktopLayoutOrientation;
    int desktopLayoutX;
    int desktopLayoutY;
    KSelectionOwner* m_desktopLayoutOwner;

    KShadowEngine* m_shadowEngine;

    OrgKdeKdesktopBackgroundInterface* m_backgroundInterface;

    QMenu *m_contextMenu;
    PagerSettings *m_settings;
};

#endif

