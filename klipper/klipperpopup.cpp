/* -------------------------------------------------------------

   history.cpp (part of Klipper - Cut & paste history for KDE)

   (C) 2004 Esben Mose Hansen <kde@mosehansen.dk>
   (C) by Andrew Stanley-Jones
   (C) 2000 by Carsten Pfeiffer <pfeiffer@kde.org>

   Generated with the KDE Application Generator

   Licensed under the GNU GPL Version 2

 ------------------------------------------------------------- */

#include <kmessagebox.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kstringhandler.h>

#include <klocale.h>
#include <kaction.h>
#include <kglobalsettings.h>

#include "klipperpopup.h"
#include "history.h"
#include "toplevel.h"


KlipperPopup::KlipperPopup( History* history, QWidget* parent, const char* name )
    : KPopupMenu( parent, name ),
      m_dirty( true ),
      QSempty( i18n("<empty clipboard>") ),
      m_history( history ),
      helpmenu( new KHelpMenu( this,  KlipperWidget::aboutData(), false ) )
{

    connect( this, SIGNAL( aboutToShow() ), SLOT( slotAboutToShow() ) );
}

KlipperPopup::~KlipperPopup() {

}

void KlipperPopup::slotAboutToShow() {

    // If the history is unchanged since last menu build, the is no reason
    // to rebuild it,
    if ( m_dirty ) {
        rebuild();
    }

}

void KlipperPopup::ensureClean() {
    if ( m_dirty ) {
        rebuild();
    }

}

void KlipperPopup::rebuild() {

    clear();
    insertTitle( SmallIcon( "klipper" ), i18n("Klipper - Clipboard Tool"));
    int i = 0;
    for ( const HistoryItem* item = m_history->first(); item; item = m_history->next(), i++ ) {
        int id = insertItem( KStringHandler::cEmSqueeze(item->text().simplifyWhiteSpace(), fontMetrics(), 25).replace( "&", "&&" ));
        connectItem(  id, m_history,  SLOT( slotMoveToTop( int) ) );
        setItemParameter(  id,  i );
    }
    if ( i == 0 ) {
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

}

void KlipperPopup::plugAction( KAction* action ) {
    m_actions.append( action );
}


#include "klipperpopup.moc"

