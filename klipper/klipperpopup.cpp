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
#include <klineedit.h>

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

namespace {
    static const int TOP_HISTORY_ITEM_INDEX = 2;
}

// #define DEBUG_EVENTS__

#ifdef DEBUG_EVENTS__
kdbgstream& operator<<( kdbgstream& stream,  const QKeyEvent& e ) {
    stream << "(QKeyEvent(text=" << e.text() << ",key=" << e.key() << ( e.isAccepted()?",accepted":",ignored)" ) << ",count=" << e.count();
    if ( e.state() & Qt::AltButton ) {
        stream << ",ALT";
    }
    if ( e.state() & Qt::ControlButton ) {
        stream << ",CTRL";
    }
    if ( e.state() & Qt::MetaButton ) {
        stream << ",META";
    }
    if ( e.state() & Qt::ShiftButton ) {
        stream << ",SHIFT";
    }
    if ( e.isAutoRepeat() ) {
        stream << ",AUTOREPEAT";
    }
    stream << ")";

    return stream;
}
#endif

/**
 * Exactly the same as KLineEdit, except that ALL key events are swallowed.
 *
 * We need this to avoid infinite loop when sending events to the search widget
 */
class KLineEditBlackKey : public KLineEdit {
public:
    KLineEditBlackKey(const QString& string, QWidget* parent, const char* name )
        : KLineEdit( string, parent, name )
        {}

    KLineEditBlackKey( QWidget* parent, const char* name )
        : KLineEdit( parent, name )
        {}

    ~KLineEditBlackKey() {
    }
protected:
    virtual void keyPressEvent( QKeyEvent* e ) {
        KLineEdit::keyPressEvent( e );
        e->accept();

    }

};

KlipperPopup::KlipperPopup( History* history, QWidget* parent, const char* name )
    : KPopupMenu( parent, name ),
      m_dirty( true ),
      QSempty( i18n( "<empty clipboard>" ) ),
      QSnomatch( i18n( "<no matches>" ) ),
      m_history( history ),
      helpmenu( new KHelpMenu( this,  KlipperWidget::aboutData(), false ) ),
      m_popupProxy( 0 ),
      m_filterWidget( 0 ),
      m_filterWidgetId( 10 ),
      n_history_items( 0 )
{
    m_popupProxy = new PopupProxy( this, "popup_proxy", calcItemsPerMenu() );

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
    if ( m_filterWidget ) {
        if ( !m_filterWidget->text().isEmpty() ) {
            m_dirty = true;
            m_filterWidget->clear();
            setItemVisible( m_filterWidgetId,  false );
            m_filterWidget->hide();
        }
    }
    ensureClean();

}

void KlipperPopup::ensureClean() {
    // If the history is unchanged since last menu build, the is no reason
    // to rebuild it,
    if ( m_dirty ) {
        rebuild();
    }

}

void KlipperPopup::buildFromScratch() {
    m_filterWidget = new KLineEditBlackKey( this, "Klipper filter widget" );
    insertTitle( SmallIcon( "klipper" ), i18n("Klipper - Clipboard Tool"));
    m_filterWidgetId = insertItem( m_filterWidget, m_filterWidgetId, 1 );
    m_filterWidget->setFocusPolicy( QWidget::NoFocus );
    setItemVisible( m_filterWidgetId,  false );
    m_filterWidget->hide();
    QString lastGroup;

    // Bit of a hack here. It would be better of KHelpMenu could be an action.
    //    Insert Help-menu at the butttom of the "default" group.
    QString group;
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

void KlipperPopup::rebuild( const QString& filter ) {

    bool from_scratch = ( count() == 0 );
    if ( from_scratch ) {
        buildFromScratch();
    } else {
        for ( int i=0; i<n_history_items; i++ ) {
            removeItemAt( TOP_HISTORY_ITEM_INDEX );
        }
    }

//    m_filterWidget->setText( filter );
    QRegExp filterexp( filter );
    if ( filterexp.isValid() ) {
        m_filterWidget->setPaletteForegroundColor( paletteForegroundColor() );
    } else {
        m_filterWidget->setPaletteForegroundColor( QColor( "red" ) );
    }
    n_history_items = m_popupProxy->buildParent( TOP_HISTORY_ITEM_INDEX, filterexp );

    if ( n_history_items == 0 ) {
        if ( m_history->empty() ) {
            insertItem( QSempty, -1, TOP_HISTORY_ITEM_INDEX  );
        } else {
            insertItem( QSnomatch, -1, TOP_HISTORY_ITEM_INDEX );
        }
        n_history_items++;
    }

    m_dirty = false;

}

void KlipperPopup::plugAction( KAction* action ) {
    m_actions.append( action );
}




/* virtual */
void KlipperPopup::keyPressEvent( QKeyEvent* e ) {
    // If alt-something is pressed, select a shortcut
    // from the menu. Do this by sending a keyPress
    // without the alt-modifier to the superobject.
    if ( e->state() & Qt::AltButton ) {
        QKeyEvent ke( QEvent::KeyPress,
                      e->key(),
                      e->ascii(),
                      e->state() ^ Qt::AltButton,
                      e->text(),
                      e->isAutoRepeat(),
                      e->count() );
        KPopupMenu::keyPressEvent( &ke );
#ifdef DEBUG_EVENTS__
        kdDebug() << "Passing this event to ancestor (KPopupMenu): " << e "->" << ke << endl;
#endif
        if ( ke.isAccepted() ) {
            e->accept();
            return;
        } else {
            e->ignore();
        }
    }

    // Otherwise, send most events to the search
    // widget, except a few used for navigation:
    // These go to the superobject.
    switch( e->key() ) {
    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_Right:
    case Qt::Key_Left:
    case Qt::Key_Tab:
    case Qt::Key_Backtab:
    case Qt::Key_Escape:
    case Qt::Key_Return:
    case Qt::Key_Enter:
    {
#ifdef DEBUG_EVENTS__
        kdDebug() << "Passing this event to ancestor (KPopupMenu): " << e << endl;
#endif
        KPopupMenu::keyPressEvent( e );
        if ( isItemActive( m_filterWidgetId ) ) {
            setActiveItem( TOP_HISTORY_ITEM_INDEX );
        }
        break;
    }
    default:
    {
#ifdef DEBUG_EVENTS__
        kdDebug() << "Passing this event down to child (KLineEdit): " << e << endl;
#endif
	QString lastString = m_filterWidget->text();
        QApplication::sendEvent( m_filterWidget, e );
        if ( m_filterWidget->text().isEmpty() ) {
            if ( isItemVisible( m_filterWidgetId ) )
            {
                setItemVisible( m_filterWidgetId, false );
                m_filterWidget->hide();
            }
        }
        else if ( !isItemVisible( m_filterWidgetId ) )
        {
            setItemVisible( m_filterWidgetId, true );
            m_filterWidget->show();

        }
	if ( m_filterWidget->text() != lastString) {
            slotHistoryChanged();
            rebuild( m_filterWidget->text() );
	}
        break;

    } //default:
    } //case

}



#include "klipperpopup.moc"
