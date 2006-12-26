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

#ifndef __container_applet_h__
#define __container_applet_h__

#include <kpanelapplet.h>
#include <kwin.h>

#include "appletinfo.h"
#include "container_base.h"
#include <QMenu>
#include <QByteArray>
#include <QBoxLayout>

class KHBox;
class QBoxLayout;
class KConfig;

class AppletHandle;

class AppletContainer : public BaseContainer
{
    Q_OBJECT

public:
    AppletContainer(const AppletInfo& info, QMenu* opMenu, bool isImmutable = false, QWidget* parent = 0);

    Plasma::Type type() const { return _type; }
    const AppletInfo& info() const { return _info; }
    const QMenu* appletsOwnMenu() const;
    bool isStretch() const { return type() ==  Plasma::Stretch; }
    void resetLayout();

    virtual void configure();
    virtual void about();
    virtual void help();
    virtual void preferences();
    virtual void reportBug();
    virtual bool isValid() const { return _valid; }
    virtual QString appletType() const { return "Applet"; }
    virtual QString icon() const { return _info.icon(); }
    virtual QString visibleName() const { return _info.name(); }

    int widthForHeight(int height) const;
    int heightForWidth(int width)  const;

    void setWidthForHeightHint(int w) { _widthForHeightHint = w; }
    void setHeightForWidthHint(int h) { _heightForWidthHint = h; }

Q_SIGNALS:
    void updateLayout();

public Q_SLOTS:
    virtual void slotRemoved(KConfig* config);
    virtual void setPopupDirection(Plasma::Position d);
    virtual void setOrientation(Qt::Orientation o);
    virtual void setImmutable(bool immutable);
    void moveApplet( const QPoint& moveOffset );
    void showAppletMenu();
    void slotReconfigure();
    void activateWindow();

protected:
    virtual void doLoadConfiguration( KConfigGroup& );
    virtual void doSaveConfiguration( KConfigGroup&, bool layoutOnly ) const;
    virtual void alignmentChange(Plasma::Alignment a);

    virtual QMenu* createOpMenu();

    AppletInfo         _info;
    AppletHandle      *_handle;
    KHBox             *_appletframe;
    QBoxLayout        *_layout;
    Plasma::Type       _type;
    int                _widthForHeightHint;
    int                _heightForWidthHint;
    QString            _deskFile, _configFile;
    bool               _firstuse;
    QByteArray         _id;
    KPanelApplet *     _applet;
    bool               _valid;

protected Q_SLOTS:
    void slotRemoveApplet();
    void slotUpdateLayout();
    void signalToBeRemoved();
    void slotDelayedDestruct();
    void focusRequested(bool);
};

#endif

