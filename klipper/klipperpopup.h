/* -------------------------------------------------------------

klipperpopup.h (part of Klipper - Cut & paste history for KDE)

(C) by Andrew Stanley-Jones
(C) Esben Mose Hansen <kde@mosehansen.dk>

Generated with the KDE Application Generator

Licensed under the GNU GPL Version 2

------------------------------------------------------------- */


#ifndef _KLIPPERPOPUP_H_
#define _KLIPPERPOPUP_H_

#include <kpopupmenu.h>
#include <qptrlist.h>
#include <qstring.h>

class History;
class KlipperWidget;
class KHelpMenu;
class KAction;
class PopupProxy;
class KLineEdit;

/**
 * Default view of clipboard history.
 *
 */
class KlipperPopup : public KPopupMenu
{
    Q_OBJECT

public:
    KlipperPopup( History* history, QWidget* parent=0, const char* name=0 );
    ~KlipperPopup();
    void plugAction( KAction* action );

    /**
     * Normally, the popupmenu is only rebuilt just before showing.
     * If you need the pixel-size or similar of the this menu, call
     * this beforehand.
     */
    void ensureClean();

    History* history() { return m_history; }
    const History* history() const { return m_history; }

public slots:
    void slotHistoryChanged() { m_dirty = true; }
    void slotAboutToShow();

private:
    void rebuild( const QString& filter = QString::null );
    void buildFromScratch();
    int calcItemsPerMenu();

    void insertSearchFilter();
    void removeSearchFilter();

protected:
     virtual void keyPressEvent( QKeyEvent* e );

private:
    bool m_dirty : 1; // true if menu contents needs to be rebuild.

    /**
     * Contains the string shown if the menu is empty.
     */
    QString QSempty;

    /**
     * Contains the string shown if the search string has no
     * matches and the menu is not empty.
     */
    QString QSnomatch;

    /**
     * The "document" (clipboard history)
     */
    History* m_history;

    /**
     * The help menu
     */
    KHelpMenu* helpmenu;

    /**
     * (unowned) actions to plug into the primary popup menu
     */
    QPtrList<KAction> m_actions;

    /**
     * Proxy helper object used to track history items
     */
    PopupProxy* m_popupProxy;

    /**
     * search filter widget
     */
    KLineEdit* m_filterWidget;

    /**
     * id of search widget, for convenience
     */
    int m_filterWidgetId;

    /**
     * Number of history items currently in menu
     */
    int n_history_items;

signals:
    void clearHistory();
    void configure();
    void quit();

};

#endif
