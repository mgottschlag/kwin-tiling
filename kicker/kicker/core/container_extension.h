/*****************************************************************

Copyright (c) 2000 Matthias Elter <elter@kde.org>

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

#ifndef __container_extension_h__
#define __container_extension_h__

#include <QPaintEvent>
#include <QGridLayout>
#include <QCloseEvent>
#include <QEvent>
#include <QBoxLayout>
#include <QList>
#include <QMenu>
#include <QFrame>

#include <kpanelextension.h>
#include <netwm_def.h>


#include "utils.h"

#include "appletinfo.h"

class QBoxLayout;
class QGridLayout;
class QMenu;
class QTimer;
class HideButton;
class KConfig;
class KWinModule;
class PopupWidgetFilter;
class PanelExtensionOpMenu;

class ExtensionContainer : public QFrame
{
    Q_OBJECT

public:
    enum UserHidden { Unhidden, LeftTop, RightBottom };
    enum HideMode { ManualHide, AutomaticHide, BackgroundHide };
    typedef QList<ExtensionContainer*> List;

    ExtensionContainer(const AppletInfo& info,
                       const QString& extensionId,
                       QWidget *parent = 0);
    ExtensionContainer(KPanelExtension* extension,
                       const AppletInfo& info,
                       const QString& extensionId,
                       QWidget *parent = 0);
    virtual ~ExtensionContainer();

    virtual QSize sizeHint(Plasma::Position, const QSize &maxSize);

    const AppletInfo& info() const { return _info; }

    QString extensionId() const { return _id; }

    void readConfig();
    void writeConfig();

    virtual QString panelId() const { return extensionId(); }

    virtual void about();
    virtual void help();
    virtual void preferences();
    virtual void reportBug();

    void removeSessionConfigFile();

    Qt::Orientation orientation() const;
    Plasma::Position position() const;
    void setPosition(Plasma::Position p) { arrange( p, alignment(), xineramaScreen() ); }

    int xineramaScreen() const;
    void setXineramaScreen(int screen);

    void setResizeableHandle( bool resizeablehandle=true );
    void setHideButtons(bool showLeft, bool showRight);
    void setSize(Plasma::Size size, int custom);
    Plasma::Size size() const;
    int customSize() const;
    HideMode hideMode() const;
    void unhideIfHidden(int showForHowManyMS = 0);
    bool reserveStrut() const;

    Plasma::Alignment alignment() const;
    void setAlignment(Plasma::Alignment a) { arrange( position(), a, xineramaScreen() ); }

    QRect currentGeometry();
    QRect initialGeometry(Plasma::Position p, Plasma::Alignment a,
                          int XineramaScreen, bool autoHidden = false,
                          UserHidden userHidden = Unhidden);

    bool eventFilter( QObject *, QEvent * );

    int panelOrder() { return m_panelOrder; }
    void setPanelOrder(int order) { m_panelOrder = order; }

Q_SIGNALS:
    void removeme(ExtensionContainer*);

protected Q_SLOTS:
    virtual void showPanelMenu( const QPoint& pos );
    void moveMe();
    void updateLayout();
    void actuallyUpdateLayout();
    void enableMouseOverEffects();

protected:
    bool event(QEvent*);
    void closeEvent( QCloseEvent* e );
    void paintEvent(QPaintEvent*);
    void leaveEvent(QEvent*);

    void arrange(Plasma::Position p, Plasma::Alignment a, int XineramaScreen);
    bool autoHidden() const { return _autoHidden; }
    UserHidden userHidden() const { return _userHidden; }
    void resetLayout();
    bool needsBorder();

private Q_SLOTS:
    void unhideTriggered( Plasma::ScreenEdge t, int XineramaScreen );
    void autoHideTimeout();
    void hideLeft();
    void hideRight();
    void autoHide(bool hide);
    void animatedHide(bool left);
    void updateWindowManager();
    void currentDesktopChanged(int);
    void strutChanged();
    void blockUserInput( bool block );
    void maybeStartAutoHideTimer();
    void stopAutoHideTimer();
    void maintainFocus(bool);

private:
    bool shouldUnhideForTrigger(Plasma::ScreenEdge t) const;
    void init();
    QSize initialSize(Plasma::Position p, QRect workArea);
    QPoint initialLocation(Plasma::Position p, Plasma::Alignment a,
                           int XineramaScreen, const QSize &s, QRect workArea,
                           bool autohidden = false, UserHidden userHidden = Unhidden);
    void positionChange(Plasma::Position p);
    void alignmentChange(Plasma::Alignment a);
    void xineramaScreenChange(int /*XineramaScreen*/) {}
    int arrangeHideButtons();
    int setupBorderSpace();

    ExtensionContainer::HideMode m_hideMode;
    Plasma::ScreenEdge m_unhideTriggeredAt;

    // State variables
    bool             _autoHidden;
    UserHidden       _userHidden;
    bool             _block_user_input;
    QPoint           _last_lmb_press;
    bool             _is_lmb_down;
    bool             _in_autohide;

    // Misc objects
    QTimer               *_autohideTimer;
    QTimer               *_updateLayoutTimer;
    NETExtendedStrut      _strut;
    PopupWidgetFilter    *_popupWidgetFilter;

    QString               _id;
    PanelExtensionOpMenu *_opMnu;
    AppletInfo            _info;
    Plasma::Type          _type;


    // Widgets
    HideButton     *_ltHB; // Left Hide Button
    HideButton     *_rbHB; // Right Hide Button
    QGridLayout    *_layout;

    KPanelExtension *m_extension;
    int m_maintainFocus;
    int m_panelOrder;
};

class PopupWidgetFilter : public QObject
{
  Q_OBJECT

  public:
    PopupWidgetFilter( QObject *parent );
    ~PopupWidgetFilter() {}
    bool eventFilter( QObject *obj, QEvent* e );
  Q_SIGNALS:
    void popupWidgetHiding();
};

#endif

