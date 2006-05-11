/*****************************************************************

Copyright (c) 2001 Matthias Elter <elter@kde.org>

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

#include <QLayout>
//Added by qt3to4:
#include <QHBoxLayout>

#include <kapplication.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <kauthorized.h>

#include "utils.h"

#include "taskbar.h"

#include "taskbarapplet.h"
#include "taskbarapplet.moc"

extern "C"
{
    KDE_EXPORT KPanelApplet* init( QWidget *parent, const QString& configFile )
    {
        // FIXME: what about two taskbars? perhaps this should be inserted just once
        KGlobal::locale()->insertCatalog( "ktaskbarapplet" );
        int options = 0;
        if (KAuthorized::authorizeControlModule("kde-kcmtaskbar.desktop"))
           options = Plasma::Preferences;
        TaskbarApplet *taskbar = new TaskbarApplet( configFile, Plasma::Stretch,
                                                    options, parent, "ktaskbarapplet" );
	return taskbar;
    }
}

TaskbarApplet::TaskbarApplet( const QString& configFile, Plasma::Type type, int actions,
                             QWidget *parent, const char *name )
    : KPanelApplet( configFile, type, actions, parent, name )
{
    QHBoxLayout* layout = new QHBoxLayout( this );
    layout->setMargin(0);
    container = new TaskBar( this );
    connect(container, SIGNAL(containerCountChanged()), this, SIGNAL(updateLayout()));
    layout->addWidget( container, 1 );
    container->popupDirectionChange(popupDirection());
}

TaskbarApplet::~TaskbarApplet()
{
    // FIXME: what about TWO taskbars?
    KGlobal::locale()->removeCatalog( "ktaskbarapplet" );
}

int TaskbarApplet::widthForHeight(int h) const
{
    // FIXME KDE4: when either TaskBarContainer or Applet smartens us
    //             simplify this
    Plasma::Position d = orientation() == Qt::Horizontal ?
                                  Plasma::Top :
                                  Plasma::Left;
    return container->sizeHint(d, QSize(200, h)).width();
}

int TaskbarApplet::heightForWidth(int w) const
{
    // FIXME KDE4: when either TaskBarContainer or Applet smartens us
    //             simplify this
    Plasma::Position d = orientation() == Qt::Horizontal ?
                                  Plasma::Top :
                                  Plasma::Left;
    return container->sizeHint(d, QSize(w, 200)).height();
}

void TaskbarApplet::preferences()
{
    container->preferences();
}

void TaskbarApplet::orientationChange( Qt::Orientation o )
{
    container->orientationChange( o );
}

void TaskbarApplet::popupDirectionChange( Plasma::Position d )
{
    container->popupDirectionChange( d );
}
