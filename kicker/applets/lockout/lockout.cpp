/*****************************************************************

Copyright (c) 2001 Carsten Pfeiffer <pfeiffer@kde.org>
              2001 Matthias Elter   <elter@kde.org>
              2001 Martijn Klingens <mklingens@yahoo.com>

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
#include <QPainter>
#include <QPixmap>
#include <QMenu>
#include <QToolButton>
#include <QStyle>
#include <QToolTip>
#include <QX11Info>
//Added by qt3to4:
#include <QMouseEvent>
#include <QEvent>
#include <QBoxLayout>


#include <kapplication.h>
#include <kglobalsettings.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <krun.h>
#include <kdebug.h>
#include <kworkspace.h>

#include <stdlib.h>
#include <kauthorized.h>
#include "lockout.h"

extern "C"
{
    KDE_EXPORT KPanelApplet* init(QWidget *parent, const QString& configFile)
    {
        KGlobal::locale()->insertCatalog("lockout");
        return new Lockout(configFile, parent);
    }
}

Lockout::Lockout( const QString& configFile, QWidget *parent )
    : KPanelApplet( configFile, Plasma::Normal, 0, parent ), bTransparent( false )
{
    KConfig *conf = config();
    conf->setGroup("lockout");

    setFrameStyle(Panel | Sunken);
    setBackgroundOrigin( AncestorOrigin );

    if ( orientation() == Qt::Horizontal )
        layout = new QBoxLayout( QBoxLayout::TopToBottom, this );
    else
        layout = new QBoxLayout( QBoxLayout::LeftToRight, this );

    layout->setAutoAdd( true );
    layout->setMargin( 0 );
    layout->setSpacing( 0 );

    lockButton = new QToolButton( this );
    lockButton->setObjectName( "lock" );
    logoutButton = new QToolButton( this );
    logoutButton->setObjectName( "logout" );

    lockButton->setBackgroundRole( QPalette::NoRole );
    logoutButton->setBackgroundRole( QPalette::NoRole );
    lockButton->setForegroundRole( QPalette::NoRole );
    logoutButton->setForegroundRole( QPalette::NoRole );

    lockButton->setToolTip( i18n("Lock the session") );
    logoutButton->setToolTip( i18n("Log out") );

    lockButton->setPixmap( SmallIcon( "lock" ));
    logoutButton->setPixmap( SmallIcon( "exit" ));

    lockButton->setMinimumSize(lockButton->iconSize());
    logoutButton->setMinimumSize(logoutButton->iconSize());

    bTransparent = conf->readEntry( "Transparent", QVariant(bTransparent )).toBool();

    lockButton->setAutoRaise( bTransparent );
    logoutButton->setAutoRaise( bTransparent );

    connect( lockButton, SIGNAL( clicked() ), SLOT( lock() ));
    connect( logoutButton, SIGNAL( clicked() ), SLOT( logout() ));

    lockButton->installEventFilter( this );
    logoutButton->installEventFilter( this );

    if (!KAuthorized::authorizeKAction("lock_screen"))
       lockButton->hide();

    if (!KAuthorized::authorizeKAction("logout"))
       logoutButton->hide();

    lockButton->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
    logoutButton->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));


    connect( KGlobalSettings::self(), SIGNAL( iconChanged(int) ), SLOT( slotIconChanged() ));
}

Lockout::~Lockout()
{
    KGlobal::locale()->removeCatalog("lockout");
}

// the -2 is due to kicker allowing us a width/height of 42 and the buttons
// having a size of 44. So we rather cut those 2 pixels instead of changing
// direction and wasting a lot of space.
void Lockout::checkLayout( int height ) const
{
    QSize s = minimumSizeHint();
    QBoxLayout::Direction direction = layout->direction();

    if ( direction == QBoxLayout::LeftToRight &&
         ( ( orientation() == Qt::Vertical   && s.width() - 2 >= height ) ||
           ( orientation() == Qt::Horizontal && s.width() - 2 < height ) ) ) {
        layout->setDirection( QBoxLayout::TopToBottom );
    }
    else if ( direction == QBoxLayout::TopToBottom &&
              ( ( orientation() == Qt::Vertical   && s.height() - 2 < height ) ||
                ( orientation() == Qt::Horizontal && s.height() - 2 >= height ) ) ) {
        layout->setDirection( QBoxLayout::LeftToRight );
    }
}

int Lockout::widthForHeight( int height ) const
{
    checkLayout( height );
    return sizeHint().width();
}

int Lockout::heightForWidth( int width ) const
{
    checkLayout( width );
    return sizeHint().height();
}

void Lockout::lock()
{
    QString appname("kdesktop");

    QX11Info info;
    int kicker_screen_number = info.screen();
    if (kicker_screen_number)
    {
        appname.sprintf("kdesktop-screen-%d", kicker_screen_number);
    }
#warning "kde4: port it"
#if 0
    kapp->dcopClient()->send(appname.toLatin1(), "KScreensaverIface", "lock()", QByteArray());
#endif
}

void Lockout::logout()
{
    KWorkSpace::requestShutDown();
}

void Lockout::mousePressEvent(QMouseEvent* e)
{
    propagateMouseEvent(e);
}

void Lockout::mouseReleaseEvent(QMouseEvent* e)
{
    propagateMouseEvent(e);
}

void Lockout::mouseDoubleClickEvent(QMouseEvent* e)
{
    propagateMouseEvent(e);
}

void Lockout::mouseMoveEvent(QMouseEvent* e)
{
    propagateMouseEvent(e);
}

void Lockout::propagateMouseEvent(QMouseEvent* e)
{
    if ( !isTopLevel()  ) {
        QMouseEvent me(e->type(), mapTo( topLevelWidget(), e->pos() ),
                       e->globalPos(), e->button(), e->state() );
        QApplication::sendEvent( topLevelWidget(), &me );
    }
}

bool Lockout::eventFilter( QObject *o, QEvent *e )
{
    if (!KAuthorized::authorizeKAction("kicker_rmb"))
        return false;     // Process event normally:

    if( e->type() == QEvent::MouseButtonPress )
    {
        KConfig *conf = config();
        conf->setGroup("lockout");

        QMouseEvent *me = static_cast<QMouseEvent *>( e );
        if( me->button() == Qt::RightButton )
        {
            if( o == lockButton )
            {
                QMenu *popup = new QMenu();

                popup->insertItem( SmallIcon( "lock" ), i18n("Lock Session"),
                                   this, SLOT( lock() ) );
                popup->addSeparator();
                popup->insertItem( i18n( "&Transparent" ), 100 );
                popup->insertItem( SmallIcon( "configure" ),
                                   i18n( "&Configure Screen Saver..." ),
                                   this, SLOT( slotLockPrefs() ) );

                popup->setItemChecked( 100, bTransparent );
                popup->connectItem(100, this, SLOT( slotTransparent() ) );
                if (conf->entryIsImmutable( "Transparent" ))
                    popup->setItemEnabled( 100, false );
                popup->exec( me->globalPos() );
                delete popup;

                return true;
            }
            else if ( o == logoutButton )
            {
                QMenu *popup = new QMenu();

                popup->insertItem( SmallIcon( "exit" ), i18n("&Log Out..."),
                                   this, SLOT( logout() ) );
                popup->addSeparator();
                popup->insertItem( i18n( "&Transparent" ), 100 );
                popup->insertItem( SmallIcon( "configure" ),
                                   i18n( "&Configure Session Manager..." ),
                                   this, SLOT( slotLogoutPrefs() ) );

                popup->setItemChecked( 100, bTransparent );
                popup->connectItem(100, this, SLOT( slotTransparent() ) );
                if (conf->entryIsImmutable( "Transparent" ))
                    popup->setItemEnabled( 100, false );
                popup->exec( me->globalPos() );
                delete popup;

                return true;
            } // if o
        } // if me->button
    } // if e->type

    // Process event normally:
    return false;
}

void Lockout::slotLockPrefs()
{
    // Run the screensaver settings
    KRun::run( "kcmshell screensaver", KUrl::List() );
}

void Lockout::slotTransparent()
{
    bTransparent = !bTransparent;
    lockButton->setAutoRaise( bTransparent );
    logoutButton->setAutoRaise( bTransparent );

    KConfig* conf = config();
    conf->setGroup("lockout");
    conf->writeEntry( "Transparent", bTransparent );
    conf->sync();
}

void Lockout::slotLogoutPrefs()
{
    // Run the logout settings.
    KRun::run( "kcmshell kcmsmserver", KUrl::List() );
}

void Lockout::slotIconChanged()
{
    lockButton->setPixmap( SmallIcon( "lock" ));
    logoutButton->setPixmap( SmallIcon( "exit" ));
}

#include "lockout.moc"
