/* -------------------------------------------------------------

   klipperpopup.cpp (part of Klipper - Cut & paste history for KDE)

   (C) 2004 Esben Mose Hansen <kde@mosehansen.dk>
   (C) by Andrew Stanley-Jones
   (C) 2000 by Carsten Pfeiffer <pfeiffer@kde.org>

   Generated with the KDE Application Generator

   Licensed under the GNU GPL Version 2

 ------------------------------------------------------------- */
#include <qstyle.h>

#include <kmessagebox.h>
#include <khelpmenu.h>
#include <kiconloader.h>

#include <klocale.h>
#include <kaction.h>
#include <kglobalsettings.h>
#include <kwin.h>
#include <kapplication.h>
#include <kglobalsettings.h>
#include <kdebug.h>

#include "klipperpopup.h"
#include "history.h"
#include "toplevel.h"
#include "popupproxy.h"


KlipperPopup::KlipperPopup( History* history, QWidget* parent, const char* name )
    : KPopupMenu( parent, name ),
      m_dirty( true ),
      QSempty( i18n("<empty clipboard>") ),
      m_history( history ),
      helpmenu( new KHelpMenu( this,  KlipperWidget::aboutData(), false ) ),
      m_popupProxy(new PopupProxy( this, "popup_proxy", calcItemsPerMenu() ) )
{

    connect( this, SIGNAL( aboutToShow() ), SLOT( slotAboutToShow() ) );
}

KlipperPopup::~KlipperPopup() {

}

int KlipperPopup::calcItemsPerMenu() {
    KWin::WindowInfo i = KWin::windowInfo( winId(), NET::WMGeometry );
    QRect g = i.geometry();
    QRect screen = KGlobalSettings::desktopGeometry(g.center());

    // Determine height of a menu item. This requires us to insert a menuitem
    // in a popupmenu; we'll just this menu and remove the item again quickly.
    int fontheight = QFontMetrics( fontMetrics()  ).height();
    int id = insertItem( "XMg" );
    QMenuItem* mi = findItem( id );
    int lineheight = style().sizeFromContents(QStyle::CT_PopupMenuItem,
                                              this,
                                              QSize( 0, fontheight ),
                                              QStyleOption(mi,10,0) ).height();
    removeItem( id );
   // Use about 75% of the screen height for items
    int itemsPerMenu = ( screen.height() / lineheight ) * 3/4;

    return itemsPerMenu;
}

void KlipperPopup::slotAboutToShow() {
    ensureClean();

}

void KlipperPopup::ensureClean() {
    // If the history is unchanged since last menu build, the is no reason
    // to rebuild it,
    if ( m_dirty ) {
        rebuild();
    }

}

void KlipperPopup::rebuild() {

    clear();
    insertTitle( SmallIcon( "klipper" ), i18n("Klipper - Clipboard Tool"));
    m_popupProxy->buildParent();

    if ( m_history->empty() ) {
        insertItem( QSempty );
    }
    QString lastGroup;
    QString group;
    // Bit of a hack here. It would be better of KHelpMenu could be an action.
    // Insert Help-menu at the butttom of the "default" group.
    QString defaultGroup( "default" );
    for ( KAction* action = m_actions.first(); action; action = m_actions.next() ) {
        group = action->group();
        if ( group != lastGroup ) {
            if ( lastGroup == defaultGroup ) {
                insertItem( SmallIconSet("help"), KStdGuiItem::help().text(), helpmenu->menu() );
            }
            insertSeparator();
        }
        lastGroup = group;
        action->plug( this,  -1 );
    }

    if ( KGlobalSettings::insertTearOffHandle() ) {
        insertTearOffHandle();
    }
    m_dirty = false;

}

void KlipperPopup::plugAction( KAction* action ) {
    m_actions.append( action );
}


#include "klipperpopup.moc"

