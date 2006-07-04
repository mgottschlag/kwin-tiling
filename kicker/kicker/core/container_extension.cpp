/*****************************************************************

Copyright (c) 2004-2005 Aaron J. Seigo <aseigo@kde.org>
Copyright (c) 2000-2001 the kicker authors. See file AUTHORS.

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
#include <stdlib.h>
#include <math.h>

#include <QCursor>
#include <QFile>
#include <QLayout>
#include <QMovie>
#include <QPainter>
#include <QTimer>
#include <QToolTip>

#include <QMenuItem>
#include <QPaintEvent>
#include <QGridLayout>
#include <QCloseEvent>
#include <QEvent>
#include <QMenu>
#include <QMouseEvent>

#include <kconfig.h>
#include <kdebug.h>
#include <kdesktopfile.h>
#include <kglobal.h>
#include <kicker.h>
#include <kstandarddirs.h>
#include <kwin.h>
#include <klocale.h>
#include <kglobalsettings.h>
#include <kapplication.h>
#include <netwm.h>
#include <fixx11h.h>
#include <kwinmodule.h>
#include <kmenu.h>
#include <QX11Info>
#include <kauthorized.h>

#include "container_base.h"
#include "extensionmanager.h"
#include "extensionop_mnu.h"
#include "extensionSettings.h"
#include "hidebutton.h"
#include "kicker.h"
#include "kickerSettings.h"
#include "kickertip.h"
#include "pluginmanager.h"
#include "unhidetrigger.h"
#include "userrectsel.h"

using namespace Qt;

#include "container_extension.h"

/* 1 is the initial speed, hide_show_animation is the top speed. */
#define PANEL_SPEED(x, c) (int)((1.0-2.0*fabs((x)-(c)/2.0)/c)*m_extension->settings()->hideAnimationSpeed()+1.0)

ExtensionContainer::ExtensionContainer(const AppletInfo& info,
                                       const QString& extensionId,
                                       QWidget *parent)
  : QFrame(parent, Qt::WStyle_Customize | Qt::WStyle_NoBorder),
    m_hideMode(ManualHide),
    m_unhideTriggeredAt(Plasma::NoEdge),
    _autoHidden(false),
    _userHidden(Unhidden),
    _block_user_input(false),
    _is_lmb_down(false),
    _in_autohide(false),
    _id(extensionId),
    _opMnu(0),
    _info(info),
    _ltHB(0),
    _rbHB(0),
    m_extension(0),
    m_maintainFocus(0),
    m_panelOrder(ExtensionManager::self()->nextPanelOrder())
{
    setObjectName( "ExtensionContainer" );
    // now actually try to load the extension
    m_extension = PluginManager::self()->loadExtension(info, this);
    init();
}

ExtensionContainer::ExtensionContainer(KPanelExtension* extension,
                                       const AppletInfo& info,
                                       const QString& extensionId,
                                       QWidget *parent)
  : QFrame(parent, Qt::WStyle_Customize | Qt::WStyle_NoBorder),
    m_hideMode(ManualHide),
    m_unhideTriggeredAt(Plasma::NoEdge),
    _autoHidden(false),
    _userHidden(Unhidden),
    _block_user_input(false),
    _is_lmb_down(false),
    _in_autohide(false),
    _id(extensionId),
    _opMnu(0),
    _info(info),
    _ltHB(0),
    _rbHB(0),
    m_extension(extension),
    m_maintainFocus(0),
    m_panelOrder(ExtensionManager::self()->nextPanelOrder())
{
    setObjectName("ExtensionContainer");
    m_extension->setParent(this);
    m_extension->setGeometry(0, 0, width(), height());
    init();
}

void ExtensionContainer::init()
{
    // panels live in the dock
    KWin::setType(winId(), NET::Dock);
    KWin::setState(winId(), NET::Sticky);
    KWin::setOnAllDesktops(winId(), true);

    connect(Kicker::self()->kwinModule(), SIGNAL(strutChanged()), this, SLOT(strutChanged()));
    connect(Kicker::self()->kwinModule(), SIGNAL(currentDesktopChanged(int)),
            this, SLOT( currentDesktopChanged(int)));

    setFrameStyle(NoFrame);
    setLineWidth(0);

    connect(UnhideTrigger::self(), SIGNAL(triggerUnhide(Plasma::ScreenEdge,int)),
            this, SLOT(unhideTriggered(Plasma::ScreenEdge,int)));

    _popupWidgetFilter = new PopupWidgetFilter( this );
    connect(_popupWidgetFilter, SIGNAL(popupWidgetHiding()), SLOT(maybeStartAutoHideTimer()));

    // layout
    _layout = new QGridLayout(this);
    _layout->setSizeConstraint(QLayout::FreeResize);
    _layout->setRowStretch(1,10);
    _layout->setColumnStretch(1,10);

    // instantiate the autohide timer
    _autohideTimer = new QTimer(this);
    connect(_autohideTimer, SIGNAL(timeout()), SLOT(autoHideTimeout()));

    // instantiate the updateLayout event compressor timer
    _updateLayoutTimer = new QTimer(this);
    _updateLayoutTimer->setSingleShot(true);
    connect(_updateLayoutTimer, SIGNAL(timeout()), SLOT(actuallyUpdateLayout()));

    installEventFilter(this); // for mouse event handling

    // if we were hidden when kicker quit, let's start out hidden as well!
    KConfig *config = KGlobal::config();
    config->setGroup(extensionId());
    int tmp = config->readEntry("UserHidden", int(Unhidden));
    if (tmp > Unhidden && tmp <= RightBottom)
    {
        _userHidden = static_cast<UserHidden>(tmp);
    }

    if (m_extension) {
        m_extension->setPosition(ExtensionManager::self()->initialPanelPosition(m_extension->preferredPosition()));

        connect(m_extension, SIGNAL(updateLayout()), SLOT(updateLayout()));
        connect(m_extension, SIGNAL(maintainFocus(bool)), SLOT(maintainFocus(bool)));
        _layout->addWidget(m_extension, 1, 1);
    }
}

ExtensionContainer::~ExtensionContainer()
{
}

QSize ExtensionContainer::sizeHint(Plasma::Position p, const QSize &maxSize)
{
    if (!m_extension)
        return QSize();

    int width = 0;
    int height = 0;
    ExtensionSettings* s = m_extension->settings();
    if (p == Plasma::Top || p == Plasma::Bottom)
    {
        if (needsBorder())
        {
            width += s->hideButtonSize();
        }

        if (s->showRightHideButton())
        {
            width += s->hideButtonSize();
        }

        // don't forget we might have a border!
        width += _layout->columnMinimumWidth(0) + _layout->columnMinimumWidth(2);
    }
    else
    {
        if (needsBorder())
        {
            height += s->hideButtonSize();
        }

        if (s->showRightHideButton())
        {
            height += s->hideButtonSize();
        }

        // don't forget we might have a border!
        height += _layout->rowMinimumHeight(0) + _layout->rowMinimumHeight(2);
    }

    QSize size(width, height);
    size = size.boundedTo(maxSize);

    size = m_extension->sizeHint(p, maxSize - size) + size;

    return size.boundedTo(maxSize);
}

static bool isnetwm12_below()
{
  NETRootInfo info( QX11Info::display(), NET::Supported );
  return info.supportedProperties()[ NETRootInfo::STATES ] & NET::KeepBelow;
}

void ExtensionContainer::readConfig()
{
    ExtensionSettings* s = m_extension->settings();
    if (s)
    {
        s->readConfig();

        if (s->autoHidePanel())
        {
            m_hideMode = AutomaticHide;
        }
        else if (s->backgroundHide())
        {
            m_hideMode = BackgroundHide;
        }
        else
        {
            m_hideMode = ManualHide;
        }

        positionChange(position());
        alignmentChange(alignment());
        setSize(static_cast<Plasma::Size>(s->size()),
                s->customSize());
    }

    if (m_hideMode != AutomaticHide)
    {
        autoHide(false);
    }

    static bool netwm12 = isnetwm12_below();
    if (netwm12) // new netwm1.2 compliant way
    {
        if (m_hideMode == BackgroundHide)
        {
            KWin::setState( winId(), NET::KeepBelow );
            UnhideTrigger::self()->setEnabled( true );
        }
        else
        {
            KWin::clearState( winId(), NET::KeepBelow );
        }
    }
    else if (m_hideMode == BackgroundHide)
    {
        // old way
        KWin::clearState( winId(), NET::StaysOnTop );
        UnhideTrigger::self()->setEnabled( true );
    }
    else
    {
        // the other old way
        KWin::setState( winId(), NET::StaysOnTop );
    }

    actuallyUpdateLayout();
    maybeStartAutoHideTimer();
}

void ExtensionContainer::writeConfig()
{
//    kDebug(1210) << "ExtensionContainer::writeConfig()" << endl;
    KConfig *config = KGlobal::config();
    config->setGroup(extensionId());

    config->writePathEntry("ConfigFile", _info.configFile());
    config->writePathEntry("DesktopFile", _info.desktopFile());
    config->writeEntry("UserHidden", int(userHidden()));

    if(m_extension)
        m_extension->settings()->writeConfig();
}

void ExtensionContainer::showPanelMenu( const QPoint& globalPos )
{
    if (!KAuthorized::authorizeKAction("kicker_rmb"))
    {
        return;
    }

    if (m_extension && m_extension->customMenu())
    {
        // use the extenion's own custom menu
        Kicker::self()->setInsertionPoint(globalPos);
        m_extension->customMenu()->exec(globalPos);
        Kicker::self()->setInsertionPoint(QPoint());
        return;
    }

    if (!_opMnu)
    {
        KDesktopFile f(KGlobal::dirs()->findResource("extensions", _info.desktopFile()));
        _opMnu = new PanelExtensionOpMenu(f.readName(),
                                          m_extension ? m_extension->actions() : 0,
                                          this);
    }

    QMenu *menu = Plasma::reduceMenu(_opMnu);

    Kicker::self()->setInsertionPoint(globalPos);

    switch (static_cast<QMenuItem*>(menu->exec(globalPos))->id())
    {
        case PanelExtensionOpMenu::Remove:
            emit removeme(this);
            break;
        case PanelExtensionOpMenu::About:
            about();
            break;
        case PanelExtensionOpMenu::Help:
            help();
            break;
        case PanelExtensionOpMenu::Preferences:
            preferences();
            break;
        case PanelExtensionOpMenu::ReportBug:
            reportBug();
            break;
        default:
            break;
    }
    Kicker::self()->setInsertionPoint(QPoint());
}

void ExtensionContainer::about()
{
    if (!m_extension)
    {
        return;
    }

    m_extension->action(Plasma::About);
}

void ExtensionContainer::help()
{
    if (!m_extension)
    {
        return;
    }

    m_extension->action(Plasma::Help);
}

void ExtensionContainer::preferences()
{
    if (!m_extension)
    {
        return;
    }

    m_extension->action(Plasma::Preferences);
}

void ExtensionContainer::reportBug()
{
    if (!m_extension)
    {
        return;
    }

    m_extension->action(Plasma::ReportBug);
}

void ExtensionContainer::removeSessionConfigFile()
{
    if (_info.configFile().isEmpty() || _info.isUniqueApplet())
    {
        return;
    }

    if (QFile::exists(KStandardDirs::locate("config", _info.configFile())))
    {
        QFile::remove(KStandardDirs::locate("config", _info.configFile()));
    }
}

void ExtensionContainer::moveMe()
{
    int screen = xineramaScreen();
    if (screen < 0)
    {
        screen = kapp->desktop()->screenNumber(this);
    }

    if (screen < 0)
    {
        // we aren't on any screen? um. ok.
        return;
    }

    stopAutoHideTimer();

    QApplication::syncX();
    UserRectSel::RectList rects;
    rects.resize(QApplication::desktop()->numScreens() * 4 * 3);

    Plasma::Position  positions[]  = { Plasma::Left,
                                       Plasma::Right,
                                       Plasma::Top,
                                       Plasma::Bottom };
    Plasma::Alignment alignments[] = { Plasma::LeftTop,
                                       Plasma::Center,
                                       Plasma::RightBottom };

    for (int s = 0; s < QApplication::desktop()->numScreens(); s++)
    {
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                // FIXME:
                // asking for initial geometry here passes bogus heightForWidth
                // and widthForHeight requests to applets and buttons. if they
                // need to make layout adjustments or need to calculate based
                // on other parameters this can lead to Bad Things(tm)
                //
                // we need to find a way to do this that doesn't result in
                // sizeHint's getting called on the extension =/
                //
                // or else we need to change the semantics for applets so that
                // they don't get their "you're changing position" signals through
                // heightForWidth/widhtForHeight
                rects.append(UserRectSel::PanelStrut(initialGeometry(positions[i],
                                                                     alignments[j], s),
                                                     s, positions[i], alignments[j]));
            }
        }
    }

    UserRectSel::PanelStrut newStrut = UserRectSel::select(rects, rect().center());
    arrange(newStrut.m_pos, newStrut.m_alignment, newStrut.m_screen);

    _is_lmb_down = false;

    // sometimes the HB's are not reset correctly
    if (_ltHB)
    {
        _ltHB->setDown(false);
    }

    if (_rbHB)
    {
        _rbHB->setDown(false);
    }

    maybeStartAutoHideTimer();
}

void ExtensionContainer::updateLayout()
{
    /*
       m_extension == 0 can happen for example if the constructor of a panel
       extension calls adjustSize(), resulting in a sendPostedEvents on the parent (us) and
       therefore this call. Happens with ksim for example. One can argue about ksim here, but
       kicker shouldn't crash in any case.
     */
    if (!m_extension || _updateLayoutTimer->isActive())
    {
        return;
    }

    // don't update our layout more than once every half a second...
    if (_in_autohide)
    {
        // ... unless we are autohiding
        _updateLayoutTimer->start(0);
    }
    else
    {
        _updateLayoutTimer->start(500);
    }
}

void ExtensionContainer::actuallyUpdateLayout()
{
//    kDebug(1210) << "PanelContainer::updateLayout()" << endl;
    resetLayout();
    updateWindowManager();
}

void ExtensionContainer::enableMouseOverEffects()
{
    KickerTip::enableTipping(true);
    QPoint globalPos = QCursor::pos();
    QPoint localPos = mapFromGlobal(globalPos);
    QWidget* child = childAt(localPos);

    if (child)
    {
        QMouseEvent* e = new QMouseEvent(QEvent::Enter, localPos, globalPos, Qt::NoButton, Qt::NoButton, Qt::NoModifier);
        qApp->sendEvent(child, e);
    }
}

bool ExtensionContainer::shouldUnhideForTrigger(Plasma::ScreenEdge t) const
{
    int loc = m_extension ? m_extension->settings()->unhideLocation() : t;

    if (loc == t)
    {
        return true;
    }

    if (loc == Plasma::BottomEdge)
    {
        return t == Plasma::BottomLeftEdge ||
               t == Plasma::BottomRightEdge;
    }
    else if (loc == Plasma::TopEdge)
    {
        return t == Plasma::TopLeftEdge ||
               t == Plasma::TopRightEdge;
    }
    else if (loc == Plasma::LeftEdge)
    {
        return t == Plasma::TopLeftEdge ||
               t == Plasma::BottomLeftEdge;
    }
    else if (loc == Plasma::RightEdge)
    {
        return t == Plasma::TopRightEdge ||
               t == Plasma::BottomRightEdge;
    }

    return false;
}

void ExtensionContainer::unhideTriggered(Plasma::ScreenEdge tr, int XineramaScreen)
{
    if (!m_extension) return;

    ExtensionSettings* s = m_extension->settings();
    if (m_hideMode == ManualHide)
    {
        return;
    }
    else if (tr == Plasma::NoEdge)
    {
        if (s->unhideLocation() != Plasma::NoEdge && _autoHidden)
        {
            UnhideTrigger::self()->setEnabled(false);
        }

        m_unhideTriggeredAt = Plasma::NoEdge;
        return;
    }

    if (xineramaScreen() != Plasma::XineramaAllScreens &&
        XineramaScreen != xineramaScreen())
    {
        if (s->unhideLocation() != Plasma::NoEdge)
        {
            m_unhideTriggeredAt = tr;
        }
        return;
    }

    // here we handle the case where the user has defined WHERE
    // the pannel can be popped up from.
    if (s->unhideLocation() != Plasma::NoEdge)
    {
        if (_autoHidden)
        {
            UnhideTrigger::self()->setEnabled(true);
        }

        m_unhideTriggeredAt = tr;
        if (shouldUnhideForTrigger(tr))
        {
            UnhideTrigger::self()->triggerAccepted(tr, XineramaScreen);

            if (m_hideMode == BackgroundHide)
            {
                KWin::raiseWindow(winId());
            }
            else if (_autoHidden)
            {
                autoHide(false);
                maybeStartAutoHideTimer();
            }
        }

        return;
    }

    m_unhideTriggeredAt = Plasma::NoEdge;

    // Otherwise hide mode is automatic. The code below is slightly
    // complex so as to keep the same behavior as it has always had:
    // only unhide when the cursor position is within the widget geometry.
    // We can't just do geometry().contains(QCursor::pos()) because
    // now we hide the panel completely off screen.

    int x = QCursor::pos().x();
    int y = QCursor::pos().y();
    int t = geometry().top();
    int b = geometry().bottom();
    int r = geometry().right();
    int l = geometry().left();
    if (((tr == Plasma::TopEdge ||
          tr == Plasma::TopLeftEdge ||
          tr == Plasma::TopRightEdge) &&
         position() == Plasma::Top && x >= l && x <= r) ||
        ((tr == Plasma::LeftEdge ||
          tr == Plasma::TopLeftEdge ||
          tr == Plasma::BottomLeftEdge) &&
         position() == Plasma::Left && y >= t && y <= b) ||
        ((tr == Plasma::BottomEdge ||
          tr == Plasma::BottomLeftEdge ||
          tr == Plasma::BottomRightEdge) &&
         position() == Plasma::Bottom && x >= l && x <= r ) ||
        ((tr == Plasma::RightEdge ||
          tr == Plasma::TopRightEdge ||
          tr == Plasma::BottomRightEdge) &&
         position() == Plasma::Right && y >= t && y <= b ))
    {
        UnhideTrigger::self()->triggerAccepted(tr, XineramaScreen);

        if (_autoHidden)
        {
            autoHide(false);
            maybeStartAutoHideTimer();
        }
    }
}

void ExtensionContainer::autoHideTimeout()
{
//    kDebug(1210) << "PanelContainer::autoHideTimeout() " << name() << endl;
    // Hack: If there is a popup open, don't autohide until it closes.
    QWidget* popup = QApplication::activePopupWidget();
    ExtensionSettings* s = m_extension->settings();
    if (popup)
    {

    //    kDebug(1210) << "popup detected" << endl;

        // Remove it first in case it was already installed.
        // Does nothing if it wasn't installed.
        popup->removeEventFilter( _popupWidgetFilter );

        // We will get a signal from the filter after the
        // popup is hidden. At that point, maybeStartAutoHideTimer()
        // will get called again.
        popup->installEventFilter( _popupWidgetFilter );

        // Stop the timer.
        stopAutoHideTimer();
        return;
    }

    if (m_hideMode != AutomaticHide ||
        _autoHidden ||
        _userHidden ||
        m_maintainFocus > 0)
    {
        return;
    }

    QRect r = geometry();
    QPoint p = QCursor::pos();
    if (!r.contains(p) &&
        (s->unhideLocation() == Plasma::NoEdge ||
         !shouldUnhideForTrigger(m_unhideTriggeredAt)))
    {
        stopAutoHideTimer();
        autoHide(true);
        UnhideTrigger::self()->resetTriggerThrottle();
    }
}

void ExtensionContainer::hideLeft()
{
    animatedHide(true);
}

void ExtensionContainer::hideRight()
{
    animatedHide(false);
}

void ExtensionContainer::autoHide(bool hide)
{
//   kDebug(1210) << "PanelContainer::autoHide( " << hide << " )" << endl;

    if (_in_autohide || hide == _autoHidden)
    {
        return;
    }

    //    kDebug(1210) << "entering autohide for real" << endl;

    blockUserInput(true);

    QPoint oldpos = pos();
    QRect newextent = initialGeometry( position(), alignment(), xineramaScreen(), hide, Unhidden );
    QPoint newpos = newextent.topLeft();

    if (hide)
    {
        /* bail out if we are unable to hide */

        for (int s=0; s <  QApplication::desktop()->numScreens(); s++)
        {
            /* don't let it intersect with any screen in the hidden position
             * that it doesn't intesect in the shown position. Should prevent
             * panels from hiding by sliding onto other screens, while still
             * letting them show reveal buttons onscreen */
            QRect desktopGeom = QApplication::desktop()->screenGeometry(s);
            if (desktopGeom.intersects(newextent) &&
                !desktopGeom.intersects(geometry()))
            {
                blockUserInput( false );
                return;
            }
        }
    }

    _in_autohide = true;
    _autoHidden = hide;
    UnhideTrigger::self()->setEnabled(_autoHidden);
    KickerTip::enableTipping(false);

    if (hide)
    {
        // So we don't cover other panels
        lower();
    }
    else
    {
        // So we aren't covered by other panels
        raise();
    }

    if (m_extension->settings()->hideAnimation())
     {
        if (position() == Plasma::Left || position() == Plasma::Right)
        {
            for (int i = 0; i < abs(newpos.x() - oldpos.x());
                 i += PANEL_SPEED(i,abs(newpos.x() - oldpos.x())))
            {
                if (newpos.x() > oldpos.x())
                {
                    move(oldpos.x() + i, newpos.y());
                }
                else
                {
                    move(oldpos.x() - i, newpos.y());
                }

                qApp->syncX();
                qApp->processEvents();
            }
        }
        else
        {
            for (int i = 0; i < abs(newpos.y() - oldpos.y());
                    i += PANEL_SPEED(i,abs(newpos.y() - oldpos.y())))
            {
                if (newpos.y() > oldpos.y())
                {
                    move(newpos.x(), oldpos.y() + i);
                }
                else
                {
                    move(newpos.x(), oldpos.y() - i);
                }

                qApp->syncX();
                qApp->processEvents();
            }
        }
    }

    blockUserInput(false);

    updateLayout();

    // Sometimes tooltips don't get hidden
    //### KDE4: hopefully not needed any more QToolTip::hide();

    _in_autohide = false;

    QTimer::singleShot(100, this, SLOT(enableMouseOverEffects()));
}

void ExtensionContainer::animatedHide(bool left)
{
//    kDebug(1210) << "PanelContainer::animatedHide()" << endl;
    KickerTip::enableTipping(false);
    blockUserInput(true);

    UserHidden newState;
    if (_userHidden != Unhidden)
    {
        newState = Unhidden;
    }
    else if (left)
    {
        newState = LeftTop;
    }
    else
    {
        newState = RightBottom;
    }

    QPoint oldpos = pos();
    QRect newextent = initialGeometry(position(), alignment(), xineramaScreen(), false, newState);
    QPoint newpos(newextent.topLeft());

    if (newState != Unhidden)
    {
        /* bail out if we are unable to hide */
        for(int s=0; s <  QApplication::desktop()->numScreens(); s++)
        {
            /* don't let it intersect with any screen in the hidden position
             * that it doesn't intesect in the shown position. Should prevent
             * panels from hiding by sliding onto other screens, while still
            * letting them show reveal buttons onscreen */
            if (QApplication::desktop()->screenGeometry(s).intersects(newextent) &&
                !QApplication::desktop()->screenGeometry(s).intersects(geometry()))
            {
                blockUserInput(false);
                QTimer::singleShot(100, this, SLOT(enableMouseOverEffects()));
                return;
            }
        }

        _userHidden = newState;

        // So we don't cover the mac-style menubar
        lower();
    }

    if (m_extension->settings()->hideAnimation())
    {
        if (position() == Plasma::Left || position() == Plasma::Right)
        {
            for (int i = 0; i < abs(newpos.y() - oldpos.y());
                 i += PANEL_SPEED(i, abs(newpos.y() - oldpos.y())))
            {
                if (newpos.y() > oldpos.y())
                {
                    move(newpos.x(), oldpos.y() + i);
                }
                else
                {
                    move(newpos.x(), oldpos.y() - i);
                }
                qApp->syncX();
                qApp->processEvents();
            }
        }
        else
        {
            for (int i = 0; i < abs(newpos.x() - oldpos.x());
                 i += PANEL_SPEED(i, abs(newpos.x() - oldpos.x())))
            {
                if (newpos.x() > oldpos.x())
                {
                    move(oldpos.x() + i, newpos.y());
                }
                else
                {
                    move(oldpos.x() - i, newpos.y());
                }
                qApp->syncX();
                qApp->processEvents();
            }
        }
    }

    blockUserInput( false );

    _userHidden = newState;

    actuallyUpdateLayout();
    qApp->syncX();
    qApp->processEvents();

    // save our hidden status so that when kicker starts up again
    // we'll come back in the same state
    KConfig *config = KGlobal::config();
    config->setGroup(extensionId());
    config->writeEntry("UserHidden", int(userHidden()));

    QTimer::singleShot(100, this, SLOT(enableMouseOverEffects()));
}

bool ExtensionContainer::reserveStrut() const
{
    return !m_extension || m_extension->reserveStrut();
}

Plasma::Alignment ExtensionContainer::alignment() const
{
    // KConfigXT really needs to get support for vars that are enums that
    // are defined in other classes
    return static_cast<Plasma::Alignment>(m_extension->settings()->alignment());
}

void ExtensionContainer::updateWindowManager()
{
    NETExtendedStrut strut;

    if (reserveStrut())
    {
        //    kDebug(1210) << "PanelContainer::updateWindowManager()" << endl;
        // Set the relevant properties on the window.
        int w = 0;
        int h = 0;

        QRect geom = initialGeometry(position(), alignment(), xineramaScreen());
        QRect virtRect(QApplication::desktop()->geometry());
        QRect screenRect(QApplication::desktop()->screenGeometry(xineramaScreen()));

        if (m_hideMode == ManualHide && !userHidden())
        {
            w = width();
            h = height();
        }

        switch (position())
        {
            case Plasma::Top:
                strut.top_width = geom.y() + h;
                strut.top_start = x();
                strut.top_end = x() + width() - 1;
                break;

            case Plasma::Bottom:
                // also claim the non-visible part at the bottom
                strut.bottom_width = (virtRect.bottom() - geom.bottom()) + h;
                strut.bottom_start = x();
                strut.bottom_end = x() + width() - 1;
                break;

            case Plasma::Right:
                strut.right_width = (virtRect.right() - geom.right()) + w;
                strut.right_start = y();
                strut.right_end = y() + height() - 1;
                break;

            case Plasma::Left:
                strut.left_width = geom.x() + w;
                strut.left_start = y();
                strut.left_end = y() + height() - 1;
                break;

            case Plasma::Floating:
                // should never be reached, anyways
                break;
        }
    }

    if (strut.left_width != _strut.left_width ||
        strut.left_start != _strut.left_start ||
        strut.left_end != _strut.left_end ||
        strut.right_width != _strut.right_width ||
        strut.right_start != _strut.right_start ||
        strut.right_end != _strut.right_end ||
        strut.top_width != _strut.top_width ||
        strut.top_start != _strut.top_start ||
        strut.top_end != _strut.top_end ||
        strut.bottom_width != _strut.bottom_width ||
        strut.bottom_start != _strut.bottom_start ||
        strut.bottom_end != _strut.bottom_end)
    {
        /*kDebug(1210) << " === Panel sets new strut for pos " << position() << " ===" << endl;

       kDebug(1210) << "strut for " << winId() << ": " << endl <<
            "\tleft  : " << strut.left_width << " " << strut.left_start << " " << strut.left_end << endl <<
            "\tright : " << strut.right_width << " " << strut.right_start << " " << strut.right_end << endl <<
            "\ttop   : " << strut.top_width << " " << strut.top_start << " " << strut.top_end << endl <<
            "\tbottom: " << strut.bottom_width << " " << strut.bottom_start << " " << strut.bottom_end << endl; */
        _strut = strut;

        KWin::setExtendedStrut(winId(),
            strut.left_width, strut.left_start, strut.left_end,
            strut.right_width, strut.right_start, strut.right_end,
            strut.top_width, strut.top_start, strut.top_end,
            strut.bottom_width, strut.bottom_start, strut.bottom_end);
        KWin::setStrut(winId(), strut.left_width, strut.right_width, strut.top_width, strut.bottom_width);
    }
    /*else
    {
        kDebug(1210) << "Panel strut did NOT change!" << endl;
    }*/
}

void ExtensionContainer::currentDesktopChanged(int)
{
    //    kDebug(1210) << "PanelContainer::currentDesktopChanged" << endl;
    if (m_extension->settings()->autoHideSwitch())
    {
        if (m_hideMode == AutomaticHide)
        {
            autoHide(false);
        }
        else if (m_hideMode == BackgroundHide)
        {
            KWin::raiseWindow(winId());
        }
    }

    // For some reason we don't always get leave events when the user
    // changes desktops and moves the cursor out of the panel at the
    // same time. Maybe always calling this will help.
    maybeStartAutoHideTimer();
}

void ExtensionContainer::strutChanged()
{
    //kDebug(1210) << "PanelContainer::strutChanged()" << endl;
    QRect ig = currentGeometry();

    if (ig != geometry())
    {
        setGeometry(ig);
        updateLayout();
    }
}

void ExtensionContainer::blockUserInput( bool block )
{
    if (block == _block_user_input)
    {
        return;
    }

    // If we don't want any user input to be possible we should catch mouse
    // events and such. Therefore we install an eventfilter and let the
    // eventfilter discard those events.
    if ( block )
    {
        qApp->installEventFilter( this );
    }
    else
    {
        qApp->removeEventFilter( this );
    }

    _block_user_input = block;
}

void ExtensionContainer::maybeStartAutoHideTimer()
{
    if (m_hideMode == AutomaticHide &&
        !_autoHidden &&
        !_userHidden)
    {
        // kDebug(1210) << "starting auto hide timer for " << name() << endl;
        ExtensionSettings* s = m_extension->settings();
        if (s->autoHideDelay() == 0)
        {
            _autohideTimer->start(250);
        }
        else
        {
            _autohideTimer->start(s->autoHideDelay() * 1000);
        }
    }
}

void ExtensionContainer::stopAutoHideTimer()
{
    if (_autohideTimer->isActive())
    {
        //kDebug(1210) << "stopping auto hide timer for " << name() << endl;
        _autohideTimer->stop();
    }
}

void ExtensionContainer::maintainFocus(bool maintain)
{
    if (maintain)
    {
        ++m_maintainFocus;

        if (_autoHidden)
        {
            autoHide(false);
        }
        else if (_userHidden == LeftTop)
        {
            animatedHide(true);
        }
        else if (_userHidden == RightBottom)
        {
            animatedHide(false);
        }
    }
    else if (m_maintainFocus > 0)
    {
        --m_maintainFocus;
    }
}

int ExtensionContainer::arrangeHideButtons()
{
    bool layoutEnabled = _layout->isEnabled();

    if (layoutEnabled)
    {
        _layout->setEnabled(false);
    }

    if (orientation() == Vertical)
    {
        int maxWidth = width();

        if (needsBorder())
        {
            --maxWidth;
        }

        if (_ltHB)
        {
            _ltHB->setMaximumWidth(maxWidth);
            _ltHB->setMaximumHeight(14);
            _layout->removeWidget(_ltHB);
            _layout->addWidget(_ltHB, 0, 1, Qt::AlignBottom | Qt::AlignLeft);
        }

        if (_rbHB)
        {
            _rbHB->setMaximumWidth(maxWidth);
            _rbHB->setMaximumHeight(14);
            _layout->removeWidget(_rbHB);
            _layout->addWidget(_rbHB, 2, 1);
        }
    }
    else
    {
        int maxHeight = height();

        if (needsBorder())
        {
            --maxHeight;
        }

        Qt::Alignment vertAlignment = (position() == Plasma::Top) ? Qt::AlignTop : AlignmentFlag(0);
        Qt::Alignment leftAlignment = Qt::AlignRight;

        if (_ltHB)
        {
            _ltHB->setMaximumHeight(maxHeight);
            _ltHB->setMaximumWidth(14);
            _layout->removeWidget(_ltHB);
            if (kapp->layoutDirection() == Qt::RightToLeft)
            {
                _layout->addWidget(_ltHB, 1, 2, vertAlignment);
            }
            else
            {
                _layout->addWidget(_ltHB, 1, 0, leftAlignment | vertAlignment);
            }
        }

        if (_rbHB)
        {
            _rbHB->setMaximumHeight(maxHeight);
            _rbHB->setMaximumWidth(14);
            _layout->removeWidget(_rbHB);
            if (kapp->layoutDirection() == Qt::RightToLeft)
            {
                _layout->addWidget(_rbHB, 1, 0, leftAlignment | vertAlignment);
            }
            else
            {
                _layout->addWidget(_rbHB, 1, 2, vertAlignment);
            }
        }
    }

    int layoutOffset = setupBorderSpace();
    if (layoutEnabled)
    {
        _layout->setEnabled(true);
    }

    return layoutOffset;
}

int ExtensionContainer::setupBorderSpace()
{
    _layout->setRowMinimumHeight(0, 0);
    _layout->setRowMinimumHeight(2, 0);
    _layout->setColumnMinimumWidth(0, 0);
    _layout->setColumnMinimumWidth(2, 0);

    if (!needsBorder())
    {
        return 0;
    }

    int layoutOffset = 0;
    QRect r = QApplication::desktop()->screenGeometry(xineramaScreen());
    QRect h = geometry();

    if (orientation() == Vertical)
    {
        if (h.top() > 0)
        {
            int topHeight = (_ltHB && _ltHB->isVisibleTo(this)) ? _ltHB->height() + 1 : 1;
            _layout->setRowMinimumHeight(0, topHeight);
            ++layoutOffset;
        }

        if (h.bottom() < r.bottom())
        {
            int bottomHeight = (_rbHB && _rbHB->isVisibleTo(this)) ? _rbHB->height() + 1 : 1;
            _layout->setRowMinimumHeight(1, bottomHeight);
            ++layoutOffset;
        }
    }
    else
    {
        if (h.left() > 0)
        {
            int leftWidth = (_ltHB && _ltHB->isVisibleTo(this)) ? _ltHB->width() + 1 : 1;
            _layout->setColumnMinimumWidth(0, leftWidth);
            ++layoutOffset;
        }

        if (h.right() < r.right())
        {
            int rightWidth = (_rbHB && _rbHB->isVisibleTo(this)) ? _rbHB->width() + 1 : 1;
            _layout->setColumnMinimumWidth(1, rightWidth);
            ++layoutOffset;
        }
    }

    switch (position())
    {
        case Plasma::Left:
            _layout->setColumnMinimumWidth(2, 1);
            break;

        case Plasma::Right:
            _layout->setColumnMinimumWidth(0, 1);
            break;

        case Plasma::Top:
            _layout->setRowMinimumHeight(2, 1);
            break;

        case Plasma::Bottom:
        default:
            _layout->setRowMinimumHeight(0, 1);
            break;
    }

    return layoutOffset;
}

void ExtensionContainer::positionChange(Plasma::Position p)
{
    arrangeHideButtons();

    m_extension->setPosition(p);

    update();
}

void ExtensionContainer::paintEvent(QPaintEvent *e)
{
    QFrame::paintEvent(e);

    if (needsBorder())
    {
        // draw border
        QPainter p(this);
        p.setPen(palette().color(QPalette::Active, QPalette::Mid));
        p.drawRect(0, 0, width(), height());
    }
}

void ExtensionContainer::leaveEvent(QEvent*)
{
    maybeStartAutoHideTimer();
}

void ExtensionContainer::alignmentChange(Plasma::Alignment a)
{
    if (!m_extension)
    {
        return;
    }

    m_extension->setAlignment(a);
}

void ExtensionContainer::setSize(Plasma::Size size, int custom)
{
    if (!m_extension)
    {
        return;
    }

    m_extension->setSize(size, custom);
}

Plasma::Size ExtensionContainer::size() const
{
    // KConfigXT really needs to get support for vars that are enums that
    // are defined in other classes
    return static_cast<Plasma::Size>(m_extension->settings()->size());
}

int ExtensionContainer::customSize() const
{
    return m_extension->settings()->customSize();
}

ExtensionContainer::HideMode ExtensionContainer::hideMode() const
{
    return m_hideMode;
}

void ExtensionContainer::unhideIfHidden(int showForAtLeastHowManyMS)
{
    if (_autoHidden)
    {
        autoHide(false);
        QTimer::singleShot(showForAtLeastHowManyMS,
                           this, SLOT(maybeStartAutoHideTimer()));
        return;
    }

    if (_userHidden == LeftTop)
    {
        animatedHide(true);
    }
    else if (_userHidden == RightBottom)
    {
        animatedHide(false);
    }
}

void ExtensionContainer::setHideButtons(bool showLeft, bool showRight)
{
    ExtensionSettings* s = m_extension->settings();
    if (s->showLeftHideButton() == showLeft &&
        s->showRightHideButton() == showRight)
    {
        return;
    }

    s->setShowLeftHideButton(showLeft);
    s->setShowRightHideButton(showRight);
    resetLayout();
}

bool ExtensionContainer::event(QEvent* e)
{
    // Update the layout when we receive a LayoutHint. This way we can adjust
    // to changes of the layout of the main widget.
    if (e->type() == QEvent::LayoutHint)
    {
        updateLayout();
    }

    return QFrame::event(e);
}

void ExtensionContainer::closeEvent(QCloseEvent* e)
{
    // Prevent being closed via Alt-F4
    e->ignore();
}

void ExtensionContainer::arrange(Plasma::Position p,
                                 Plasma::Alignment a,
                                 int XineramaScreen)
{
    ExtensionSettings* s = m_extension->settings();
    if (p == s->position() &&
        a == s->alignment() &&
        XineramaScreen == xineramaScreen())
    {
        return;
    }

    bool positionChanged = p != s->position();
    if (positionChanged)
    {
        s->setPosition(p);
    }
    else if (!needsBorder())
    {
        // this ensures that the layout gets rejigged
        // even if position doesn't change
        _layout->setRowMinimumHeight(0, 0);
        _layout->setRowMinimumHeight(2, 0);
        _layout->setColumnMinimumWidth(0, 0);
        _layout->setColumnMinimumWidth(2, 0);
    }

    if (a != s->alignment())
    {
        s->setAlignment(a);
        setAlignment(a);
    }

    if (XineramaScreen != xineramaScreen())
    {
        s->setXineramaScreen(XineramaScreen);
        xineramaScreenChange(XineramaScreen);
    }

    actuallyUpdateLayout();
    if (positionChanged)
    {
        positionChange(p);
    }
    writeConfig();
}

Qt::Orientation ExtensionContainer::orientation() const
{
    if (position() == Plasma::Top || position() == Plasma::Bottom)
    {
        return Horizontal;
    }
    else
    {
        return Vertical;
    }
}

Plasma::Position ExtensionContainer::position() const
{
    // KConfigXT really needs to get support for vars that are enums that
    // are defined in other classes
    return static_cast<Plasma::Position>(m_extension->settings()->position());
}

void ExtensionContainer::resetLayout()
{
    QRect g = initialGeometry(position(), alignment(), xineramaScreen(),
                              autoHidden(), userHidden());
    ExtensionSettings* s = m_extension->settings();

    // Disable the layout while we rearrange the panel.
    // Necessary because the children may be
    // relayouted with the wrong size.

    _layout->setEnabled(false);

    setGeometry(g);

    // layout
    bool haveToArrangeButtons = false;
    bool showLeftHideButton = s->showLeftHideButton() || userHidden() == RightBottom;
    bool showRightHideButton = s->showRightHideButton() || userHidden() == LeftTop;

    // left/top hide button
    if (showLeftHideButton)
    {
        if (!_ltHB)
        {
            _ltHB = new HideButton(this);
            _ltHB->installEventFilter(this);
            _ltHB->setEnabled(true);
            connect(_ltHB, SIGNAL(clicked()), this, SLOT(hideLeft()));
            haveToArrangeButtons = true;
        }

        if (orientation() == Horizontal)
        {
            _ltHB->setArrowType(Qt::LeftArrow);
            _ltHB->setFixedSize(s->hideButtonSize(), height());
        }
        else
        {
            _ltHB->setArrowType(Qt::UpArrow);
            _ltHB->setFixedSize(width(), s->hideButtonSize());
        }

        _ltHB->show();
    }
    else if (_ltHB)
    {
        _ltHB->hide();
    }

    // right/bottom hide button
    if (showRightHideButton)
    {
        if (!_rbHB)
        {
            // right/bottom hide button
            _rbHB = new HideButton(this);
            _rbHB->installEventFilter(this);
            _rbHB->setEnabled(true);
            connect(_rbHB, SIGNAL(clicked()), this, SLOT(hideRight()));
            haveToArrangeButtons = true;
        }

        if ( orientation() == Horizontal)
        {
            _rbHB->setArrowType(Qt::RightArrow);
            _rbHB->setFixedSize(s->hideButtonSize(), height());
        }
        else
        {
            _rbHB->setArrowType(Qt::DownArrow);
            _rbHB->setFixedSize(width(), s->hideButtonSize());
        }

        _rbHB->show();
    }
    else if (_rbHB)
    {
        _rbHB->hide();
    }

    if (_ltHB)
    {
        if (userHidden())
        {
            _ltHB->setToolTip( i18n("Show panel"));
        }
        else
        {
            _ltHB->setToolTip( i18n("Hide panel"));
        }
    }

    if (_rbHB)
    {
        if (userHidden())
        {
            _rbHB->setToolTip( i18n("Show panel"));
        }
        else
        {
            _rbHB->setToolTip( i18n("Hide panel"));
        }
    }

    updateGeometry();
    Q_ASSERT(m_extension); // the check further below seems outdated (COOLO)
    int endBorderWidth = haveToArrangeButtons ? arrangeHideButtons() : setupBorderSpace();

    if (orientation() == Horizontal)
    {
        if (m_extension)
        {
            int maxWidth = width() - endBorderWidth;

            if (showLeftHideButton)
            {
                maxWidth -= _ltHB->width();
            }

            if (showRightHideButton)
            {
                maxWidth -= _rbHB->width();
            }

            m_extension->setMaximumWidth(maxWidth);

            if (needsBorder())
            {
                m_extension->setFixedHeight(height() - 1);
            }
            else
            {
                m_extension->setFixedHeight(height());
            }
        }
    }
    else if (m_extension)
    {
        int maxHeight = height() - endBorderWidth;

        if (showLeftHideButton)
        {
            maxHeight -= _ltHB->height();
        }

        if (showRightHideButton)
        {
            maxHeight -= _rbHB->height();
        }

        m_extension->setMaximumHeight(maxHeight);

        if (needsBorder())
        {
            m_extension->setFixedWidth(width() - 1);
        }
        else
        {
            m_extension->setFixedWidth(width());
        }
    }

    _layout->setEnabled(true);
}

bool ExtensionContainer::needsBorder()
{
    // FIXME: we may wish to actually have this return false sometimes once
    //        we have trans, etc
    return true;
}

QSize ExtensionContainer::initialSize(Plasma::Position p, QRect workArea)
{
    /*kDebug(1210) << "initialSize() Work Area: (" << workArea.topLeft().x() <<
        ", " << workArea.topLeft().y() << ") to (" << workArea.bottomRight().x() <<
        ", " << workArea.bottomRight().y() << ")" << endl;*/

    QSize hint = sizeHint(p, workArea.size()).boundedTo(workArea.size());
    ExtensionSettings* s = m_extension->settings();
    int width = 0;
    int height = 0;

    if (p == Plasma::Left || p == Plasma::Right)
    {
        width = hint.width();
        height = (workArea.height() * s->sizePercentage()) / 100;

        if (s->expandSize())
        {
            height = qMax(height, hint.height());
        }
    }
    else
    {
        width = (workArea.width() * s->sizePercentage()) / 100;
        height = hint.height();

        if (s->expandSize())
        {
            width = qMax( width, hint.width() );
        }
    }

    return QSize(width, height);
}

QPoint ExtensionContainer::initialLocation(Plasma::Position p,
                                           Plasma::Alignment a,
                                           int XineramaScreen,
                                           const QSize &s,
                                           QRect workArea,
                                           bool autohidden,
                                           UserHidden userHidden)
{
    QRect wholeScreen;
    if (XineramaScreen == Plasma::XineramaAllScreens)
    {
        wholeScreen = QApplication::desktop()->geometry();
    }
    else
    {
        wholeScreen = QApplication::desktop()->screenGeometry(XineramaScreen);
    }

    /*kDebug(1210) << "initialLocation() Work Area: (" <<
                        workArea.topLeft().x() << ", " <<
                        area.topLeft().y() << ") to (" <<
                        workArea.bottomRight().x() << ", " <<
                        workArea.bottomRight().y() << ")" << endl;*/

    int left;
    int top;

    // If the panel is horizontal
    if (p == Plasma::Top || p == Plasma::Bottom)
    {
        // Get the X coordinate
        switch (a)
        {
            case Plasma::LeftTop:
                left = workArea.left();
            break;

            case Plasma::Center:
                left = wholeScreen.left() + ( wholeScreen.width() - s.width() ) / 2;
                if (left < workArea.left())
                {
                    left = workArea.left();
                }
            break;

            case Plasma::RightBottom:
                left = workArea.right() - s.width() + 1;
            break;

            default:
                left = workArea.left();
            break;
        }

        // Get the Y coordinate
        if (p == Plasma::Top)
        {
            top = workArea.top();
        }
        else
        {
            top = workArea.bottom() - s.height() + 1;
        }
    }
    else // vertical panel
    {
        // Get the Y coordinate
        switch (a)
        {
            case Plasma::LeftTop:
                top = workArea.top();
            break;

            case Plasma::Center:
                top = wholeScreen.top() + ( wholeScreen.height() - s.height() ) / 2;
                if (top < workArea.top())
                {
                    top = workArea.top();
                }
            break;

            case Plasma::RightBottom:
                top = workArea.bottom() - s.height() + 1;
            break;

            default:
                top = workArea.top();
        }

            // Get the X coordinate
        if (p == Plasma::Left)
        {
            left = workArea.left();
        }
        else
        {
            left = workArea.right() - s.width() + 1;
        }
    }

    ExtensionSettings* set = m_extension->settings();
    // Correct for auto hide
    if (autohidden)
    {
        switch (position())
        {
            case Plasma::Left:
                left -= s.width();
            break;

            case Plasma::Right:
                left += s.width();
            break;

            case Plasma::Top:
                top -= s.height();
            break;

            case Plasma::Bottom:
            default:
                top += s.height();
            break;
        }
        // Correct for user hide
    }
    else if (userHidden == LeftTop)
    {
        if (position() == Plasma::Left || position() == Plasma::Right)
        {
            top = workArea.top() - s.height() + set->hideButtonSize();
        }
        else
        {
            left = workArea.left() - s.width() + set->hideButtonSize();
        }
    }
    else if (userHidden == RightBottom)
    {
        if (position() == Plasma::Left || position() == Plasma::Right)
        {
            top = workArea.bottom() - set->hideButtonSize() + 1;
        }
        else
        {
            left = workArea.right() - set->hideButtonSize() + 1;
        }
    }

    return QPoint( left, top );
}

int ExtensionContainer::xineramaScreen() const
{
    // sanitize at runtime only, since many Xinerama users
    // turn it on and off and don't want kicker to lose their configs

    /* -2 means all screens, -1 primary screens, the rest are valid screen numbers */
    ExtensionSettings* s = m_extension->settings();
    if (Plasma::XineramaAllScreens <= s->xineramaScreen() &&
        s->xineramaScreen() < QApplication::desktop()->numScreens())
    {
        return s->xineramaScreen();
    }
    else
    {
        /* force invalid screen locations onto the primary screen */
        return QApplication::desktop()->primaryScreen();
    }
}

void ExtensionContainer::setXineramaScreen(int screen)
{
    if (m_extension->settings()->isImmutable("XineramaScreen"))
    {
        return;
    }

    arrange(position(),alignment(), screen);
}

QRect ExtensionContainer::currentGeometry()
{
    return initialGeometry(position(), alignment(), xineramaScreen(),
                           autoHidden(), userHidden());
}

QRect ExtensionContainer::initialGeometry(Plasma::Position p,
                                          Plasma::Alignment a,
                                          int XineramaScreen,
                                          bool autoHidden,
                                          UserHidden userHidden)
{
    //RESEARCH: is there someway to cache the results of the repeated calls to this method?

    /*kDebug(1210) << "initialGeometry() Computing geometry for " << name() <<
        " on screen " << XineramaScreen << endl;*/
    QRect workArea = ExtensionManager::self()->workArea(this, XineramaScreen);
    QSize size = initialSize(p, workArea);
    QPoint point = initialLocation(p, a, XineramaScreen,
                                   size, workArea,
                                   autoHidden, userHidden);

    //kDebug(1210) << "Size: " << size.width() << " x " << size.height() << endl;
    //kDebug(1210) << "Pos: (" << point.x() << ", " << point.y() << ")" << endl;

    return QRect(point, size);
}

bool ExtensionContainer::eventFilter( QObject*, QEvent * e)
{
    if (autoHidden())
    {
        switch ( e->type() )
        {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        case QEvent::MouseMove:
        case QEvent::KeyPress:
        case QEvent::KeyRelease:
        return true; // ignore;
        default:
        break;
        }
    }

    QEvent::Type eventType = e->type();
    if (_block_user_input)
    {
        return (eventType == QEvent::MouseButtonPress ||
                eventType == QEvent::MouseButtonRelease ||
                eventType == QEvent::MouseButtonDblClick ||
                eventType == QEvent::MouseMove ||
                eventType == QEvent::KeyPress ||
                eventType == QEvent::KeyRelease ||
                eventType == QEvent::Enter ||
                eventType == QEvent::Leave);
    }

    switch (eventType)
    {
        case QEvent::MouseButtonPress:
        {
            QMouseEvent* me = static_cast<QMouseEvent*>(e);
            if ( me->button() == Qt::LeftButton )
            {
                _last_lmb_press = me->globalPos();
                _is_lmb_down = true;
            }
            else if (me->button() == Qt::RightButton)
            {
                showPanelMenu(me->globalPos());
                return true; // don't crash!
            }
        }
        break;

        case QEvent::MouseButtonRelease:
        {
            QMouseEvent* me = static_cast<QMouseEvent*>(e);
            if ( me->button() == Qt::LeftButton )
            {
                _is_lmb_down = false;
            }
        }
        break;

        case QEvent::MouseMove:
        {
            QMouseEvent* me = (QMouseEvent*) e;
            if (_is_lmb_down &&
                ((me->buttons() & Qt::LeftButton) == Qt::LeftButton) &&
                !Kicker::self()->isImmutable() &&
                !m_extension->settings()->config()->isImmutable() &&
                !ExtensionManager::self()->isMenuBar(this))
            {
                QPoint p(me->globalPos() - _last_lmb_press);
                int x_treshold = width();
                int y_treshold = height();

                if (x_treshold > y_treshold)
                {
                    x_treshold = x_treshold / 3;
                }
                else
                {
                    y_treshold = y_treshold / 3;
                }

                if ((abs(p.x()) > x_treshold) ||
                    (abs(p.y()) > y_treshold))
                {
                    moveMe();
                    return true;
                }
            }
        }
        break;

        default:
        break;
    }

    return false;
}

PopupWidgetFilter::PopupWidgetFilter( QObject *parent )
  : QObject( parent )
{
	setObjectName( "PopupWidgetFilter" );
}

bool PopupWidgetFilter::eventFilter( QObject*, QEvent* e )
{
    if (e->type() == QEvent::Hide)
    {
        emit popupWidgetHiding();
    }
    return false;
}

#include "container_extension.moc"

