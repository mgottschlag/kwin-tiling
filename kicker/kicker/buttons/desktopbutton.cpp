/*****************************************************************

Copyright (c) 1996-2001 the kicker authors. See file AUTHORS.

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

#include <qtooltip.h>
#include <q3dragobject.h>
//Added by qt3to4:
#include <QDragEnterEvent>
#include <QDropEvent>

#include <klocale.h>
#include <kglobalsettings.h>
#include <konq_operations.h>
#include <kfileitem.h>

#include "showdesktop.h"
#include "kicker.h"
#include "kickerSettings.h"

#include "desktopbutton.h"
#include "desktopbutton.moc"

DesktopButton::DesktopButton( QWidget* parent )
  : PanelButton( parent, "DesktopButton" )
{
    setCheckable(true);

    this->setToolTip( i18n("Show desktop"));
    setTitle(i18n("Desktop Access"));
    setIcon("desktop");

    connect( this, SIGNAL(toggled(bool)), this, SLOT(showDesktop(bool)) );
    connect( ShowDesktop::self(), SIGNAL(desktopShown(bool)), this, SLOT(toggle(bool)) );

    setChecked( ShowDesktop::self()->desktopShowing() );
}

void DesktopButton::toggle(bool showDesktop)
{
    KickerTip::enableTipping(false);
    setChecked(showDesktop);
    KickerTip::enableTipping(true);
}

void DesktopButton::showDesktop(bool showDesktop)
{
    KickerTip::enableTipping(false);
    ShowDesktop::self()->showDesktop(showDesktop);
    KickerTip::enableTipping(true);
}

void DesktopButton::dragEnterEvent( QDragEnterEvent *ev )
{
    if ((ev->source() != this) && KUrl::List::canDecode(ev->mimeData()))
        ev->accept(rect());
    else
        ev->ignore(rect());
    PanelButton::dragEnterEvent(ev);
}

void DesktopButton::dropEvent( QDropEvent *ev )
{
    KUrl dPath (  KGlobalSettings::desktopPath() );
    KFileItem item( dPath, QString::fromLatin1( "inode/directory" ), KFileItem::Unknown );
    KonqOperations::doDrop( &item, dPath, ev, this );
    PanelButton::dropEvent(ev);
}

