/*****************************************************************

Copyright (c) 2000 Matthias Elter

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

#include "kpanelapplet.h"
#include "kpanelapplet.moc"
#include <kconfig.h>
#include <kglobal.h>
#include <QResizeEvent>

class KPanelApplet::Private
{
public:
  Private()
    : position( Plasma::Bottom ),
      alignment( Plasma::LeftTop ),
      customMenu(0),
      hasFocus(false)
      {}

  Plasma::Type type;
  Plasma::Position position;
  Plasma::Alignment alignment;
  int actions;

  const QMenu* customMenu;
  KSharedConfig::Ptr sharedConfig;
  QList<QObject*> watchedForFocus;
  bool hasFocus;
};

KPanelApplet::KPanelApplet(const QString& configFile, Plasma::Type type,
                           int actions, QWidget *parent, const char *name, Qt::WFlags f)
  : QFrame(parent, name, f),
    d(new Private())
{
  d->type = type;
  d->actions = actions;

  setFrameStyle(NoFrame);
  QPalette pal(palette());
  if(pal.active().mid() != pal.inactive().mid()){
    pal.setInactive(pal.active());
    setPalette(pal);
  }
  setBackgroundOrigin( AncestorOrigin );

  d->sharedConfig = KSharedConfig::openConfig(configFile, KGlobal::config()->isImmutable());
}

KPanelApplet::~KPanelApplet()
{
  d->watchedForFocus.clear();
  needsFocus(false);
  delete d;
}

KConfig* KPanelApplet::config() const
{
  return d->sharedConfig.data();
}

Plasma::Type KPanelApplet::type() const
{
  return d->type;
}

int KPanelApplet::actions() const
{
  return d->actions;
}

void KPanelApplet::setPosition( Plasma::Position p )
{
  if( d->position == p ) return;
  d->position = p;
  positionChange( p );
}

void KPanelApplet::setAlignment( Plasma::Alignment a )
{
  if( d->alignment == a ) return;
  d->alignment = a;
  alignmentChange( a );
}

// FIXME: Remove implementation for KDE 4
void KPanelApplet::positionChange( Plasma::Position )
{
  orientationChange( orientation() );
  QResizeEvent e( size(), size() );
  resizeEvent( &e );
  popupDirectionChange( popupDirection() );
}

// FIXME: Remove for KDE 4
Plasma::Position KPanelApplet::popupDirection()
{
    switch( d->position ) {
        case Plasma::Top:
            return Plasma::Down;
        case Plasma::Right:
            return Plasma::Left;
        case Plasma::Left:
            return Plasma::Right;
        case Plasma::Bottom:
        default:
          return Plasma::Up;
    }
}

Qt::Orientation KPanelApplet::orientation() const
{
    if( d->position == Plasma::Top || d->position == Plasma::Bottom )
    {
        return Qt::Horizontal;
    }
    else
    {
        return Qt::Vertical;
    }
}

Plasma::Position KPanelApplet::position() const
{
    return d->position;
}

Plasma::Alignment KPanelApplet::alignment() const
{
    return d->alignment;
}

void KPanelApplet::action( Plasma::Action a )
{
    if ( (a & Plasma::About) )
    {
        about();
    }
    if ( (a & Plasma::Help) )
    {
        help();
    }
    if ( (a & Plasma::Preferences) )
    {
        preferences();
    }
    if ( (a & Plasma::ReportBug) )
    {
        reportBug();
    }
}

const QMenu* KPanelApplet::customMenu() const
{
    return d->customMenu;
}

void KPanelApplet::setCustomMenu(const QMenu* menu)
{
    d->customMenu = menu;
}

void KPanelApplet::watchForFocus(QWidget* widget, bool watch)
{
    if (!widget)
    {
        return;
    }

    if (watch)
    {
        if (!d->watchedForFocus.contains(widget))
        {
            d->watchedForFocus.append(widget);
            widget->installEventFilter(this);
        }
    }
    else if (!d->watchedForFocus.contains(widget))
    {
        d->watchedForFocus.removeAll(widget);
        widget->removeEventFilter(this);
    }
}

void KPanelApplet::needsFocus(bool focus)
{
    if (focus == d->hasFocus)
    {
        return;
    }

    d->hasFocus = focus;
    emit requestFocus(focus);
}

bool KPanelApplet::eventFilter(QObject *o, QEvent * e)
{
    if (!d->watchedForFocus.contains(o))
    {
        if (e->type() == QEvent::MouseButtonRelease ||
            e->type() == QEvent::FocusIn)
        {
            needsFocus(true);
        }
        else if (e->type() == QEvent::FocusOut)
        {
            needsFocus(false);
        }
    }

    return QFrame::eventFilter(o, e);
}

KSharedConfig::Ptr KPanelApplet::sharedConfig() const
{
    return d->sharedConfig;
}

void KPanelApplet::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

