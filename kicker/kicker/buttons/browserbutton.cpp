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

#include <QTimer>
#include <QToolTip>
#include <q3dragobject.h>
//Added by qt3to4:
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>

#include <kconfig.h>
#include <klocale.h>
#include <konq_operations.h>
#include <kfileitem.h>

#include "kicker.h"
#include "browser_mnu.h"
#include "browser_dlg.h"
#include "utils.h"

#include "browserbutton.h"
#include "browserbutton.moc"

BrowserButton::BrowserButton( const QString& icon, const QString& startDir, QWidget* parent )
    : PanelPopupButton( parent, "BrowserButton" )
    , topMenu( 0 )
{
    initialize( icon, startDir );
}

BrowserButton::BrowserButton( const KConfigGroup& config, QWidget* parent )
    : PanelPopupButton( parent, "BrowserButton" )
    , topMenu( 0 )
{
    initialize( config.readEntry("Icon", "kdisknav"), config.readPathEntry("Path") );
}

BrowserButton::~BrowserButton()
{
    delete topMenu;
}

void BrowserButton::initialize( const QString& icon, const QString& path )
{
    _icon = icon;

    // Don't parent to this, so that the tear of menu is not always-on-top.
    topMenu = new PanelBrowserMenu( path );
    setPopup(topMenu);

    _menuTimer = new QTimer( this );
    _menuTimer->setSingleShot(true);
    connect( _menuTimer, SIGNAL(timeout()), SLOT(slotDelayedPopup()) );

    this->setToolTip( i18n("Browse: %1", path));
    setTitle( path );
    setIcon ( _icon );
}

void BrowserButton::saveConfig( KConfigGroup& config ) const
{
    config.writeEntry("Icon", _icon);
    config.writePathEntry("Path", topMenu->path());
}

void BrowserButton::dragEnterEvent( QDragEnterEvent *ev )
{
    if ((ev->source() != this) && KUrl::List::canDecode(ev->mimeData()))
    {
        _menuTimer->start(500);
        ev->accept();
    }
    else
    {
        ev->ignore();
    }
    PanelButton::dragEnterEvent(ev);
}

void BrowserButton::dragLeaveEvent( QDragLeaveEvent *ev )
{
   _menuTimer->stop();
   PanelButton::dragLeaveEvent(ev);
}

void BrowserButton::dropEvent( QDropEvent *ev )
{
    KUrl path ( topMenu->path() );
    _menuTimer->stop();
    KFileItem item( path, QString::fromLatin1( "inode/directory" ), KFileItem::Unknown );
    KonqOperations::doDrop( &item, path, ev, this );
    PanelButton::dropEvent(ev);
}

void BrowserButton::initPopup()
{
    topMenu->initialize();
}

void BrowserButton::slotDelayedPopup()
{
    topMenu->initialize();
    topMenu->popup(Plasma::popupPosition(popupDirection(), topMenu, this));
    setDown(false);
}

void BrowserButton::properties()
{
    PanelBrowserDialog dlg( topMenu->path(), _icon, this );

    if( dlg.exec() == QDialog::Accepted ){
	_icon = dlg.icon();
	QString path = dlg.path();

	if ( path != topMenu->path() ) {
	    delete topMenu;
	    topMenu = new PanelBrowserMenu( path, this );
	    setPopup( topMenu );
	    setTitle( path );
	}
	setIcon( _icon );
	emit requestSave();
    }
}

void BrowserButton::startDrag()
{
    KUrl url(topMenu->path());
    emit dragme(KUrl::List(url), labelIcon());
}

