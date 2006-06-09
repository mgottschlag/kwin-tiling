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

#ifndef __k_mnu_h__
#define __k_mnu_h__

#include <dcopobject.h>
#include <QPixmap>

#include <QMouseEvent>
#include <QResizeEvent>
#include <QMenu>
#include <QPaintEvent>
#include <QHash>

#include "service_mnu.h"

class KBookmarkMenu;
class KActionCollection;
class KBookmarkOwner;
class Panel;

class PanelKMenu : public PanelServiceMenu, public DCOPObject
{
    Q_OBJECT
    K_DCOP

k_dcop:
    void slotServiceStartedByStorageId(QString starter, QString desktopPath);

public:
    PanelKMenu();
    ~PanelKMenu();

    virtual QSize sizeHint() const;
    virtual void setMinimumSize(const QSize &);
    virtual void setMaximumSize(const QSize &);
    virtual void setMinimumSize(int, int);
    virtual void setMaximumSize(int, int);
    void clearRecentMenuItems();

public Q_SLOTS:
    virtual void initialize();

    //### KDE4: workaround for Qt bug, remove later
    virtual void resize(int width, int height);

protected Q_SLOTS:
    void slotLock();
    void slotLogout();
    void slotPopulateSessions();
    void slotSessionActivated( int );
    void slotSaveSession();
    void slotRunCommand();
    void slotEditUserContact();
    void paletteChanged();
    virtual void configChanged();
    void updateRecent();

protected:
    QRect sideImageRect();
    QMouseEvent translateMouseEvent(QMouseEvent* e);
    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    bool loadSidePixmap();
    void doNewSession(bool lock);
    void createRecentMenuItems();
    virtual void clearSubmenus();

private:
    QMenu                 *sessionsMenu;
    QPixmap                     sidePixmap;
    QPixmap                     sideTilePixmap;
    int                         client_id;
    bool                        delay_init;
    KBookmarkMenu              *bookmarkMenu;
    KActionCollection          *actionCollection;
    KBookmarkOwner             *bookmarkOwner;
    PopupMenuList               dynamicSubMenus;
};

#endif
