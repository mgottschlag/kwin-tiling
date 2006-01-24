/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2000, 2001 David Faure <faure@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License version 2 as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef kdiconview_h
#define kdiconview_h

#include <konq_iconviewwidget.h>
#include <kaction.h>
#include <kfileitem.h>
#include <kdirnotify.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QShowEvent>
#include <QList>
#include <QWheelEvent>
#include <QDropEvent>

class KDirLister;
class KonqSettings;
class KSimpleConfig;
class KAccel;
class KShadowEngine;
class KDesktopShadowSettings;

/**
 * This class is KDesktop's icon view.
 * The icon view is a child widget of the KDesktop widget.
 *
 * Added shadow capability by Laur Ivan (C) 2003
 * The shadow is supported by the new KFileIVIDesktop objects
 * which replace KFileIVI objects.
 */
class KDIconView : public KonqIconViewWidget, public KDirNotify
{
    Q_OBJECT

public:
    KDIconView( QWidget *parent, const char* name = 0L );
    ~KDIconView();

    virtual void initConfig( bool init );
    void configureMedia();
    /**
     * Start listing
     */
    void start();

    KActionCollection *actionCollection() { return &m_actionCollection; }

    enum SortCriterion {
        NameCaseSensitive = 0, NameCaseInsensitive, Size, Type, Date };

    void rearrangeIcons( SortCriterion sc, bool bSortDirectoriesFirst);

    /**
     * Re-arrange the desktop icons without confirmation.
     */
    void rearrangeIcons();

    void lineupIcons(Q3IconView::Arrangement);

    void setAutoAlign( bool b );

    void refreshIcons();
    QStringList selectedURLs();

    /**
     * Save the icon positions
     */
    void saveIconPositions();

    /**
     * Check if the URL to the desktop has changed
     */
    void recheckDesktopURL();

    /**
     * Called when the work area has changed
     */
    void updateWorkArea( const QRect &wr );

    /**
     * Reimplemented from KonqIconViewWidget (for image drops)
     */
    virtual void setWallpaper(const KUrl &url) { emit newWallpaper( url ); }
    void setLastIconPosition( const QPoint & );

    static KUrl desktopURL();

    /// KDirNotify interface, for trash:/
    virtual void FilesAdded( const KUrl & directory );
    virtual void FilesRemoved( const KUrl::List & fileList );
    virtual void FilesChanged( const KUrl::List & ) {}

protected Q_SLOTS:
    // slots connected to the icon view
    void slotReturnPressed( Q3IconViewItem *item );
    void slotExecuted( Q3IconViewItem *item );
    void slotMouseButtonPressed(int _button, Q3IconViewItem* _item, const QPoint& _global);
    void slotMouseButtonClickedKDesktop(int _button, Q3IconViewItem* _item, const QPoint& _global);
    void slotContextMenuRequested(Q3IconViewItem* _item, const QPoint& _global);
    void slotEnableAction( const char * name, bool enabled );
    void slotAboutToCreate(const QPoint &pos, const QList<KIO::CopyInfo> &files);

    void slotItemRenamed(Q3IconViewItem*, const QString &name);

    // slots connected to the directory lister
    void slotClear();
    void slotStarted( const KUrl& url );
    void slotCompleted();
    void slotNewItems( const KFileItemList& );
    void slotDeleteItem( KFileItem * );
    void slotRefreshItems( const KFileItemList& );

    // slots connected to the popupmenu (actions)
    void slotCut();
    void slotCopy();
    void slotTrashActivated( KAction::ActivationReason reason, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers );
    void slotDelete();
    void slotPopupPasteTo();
    void slotProperties();

    void slotClipboardDataChanged();

    void slotNewMenuActivated();

    // For communication with KDesktop
Q_SIGNALS:
    void colorDropEvent( QDropEvent *e );
    void imageDropEvent( QDropEvent *e );
    void newWallpaper( const KUrl & );
    void iconMoved();
    void wheelRolled( int delta );

public Q_SLOTS:
    /**
     * Lineup the desktop icons.
     */
    void lineupIcons();
    void slotPaste(); // for krootwm
protected:
    void createActions();
    void setupSortKeys();
    void initDotDirectories();

    bool makeFriendlyText( KFileIVI *fileIVI );
    static QString stripDesktopExtension( const QString & text );
    bool isDesktopFile( KFileItem * _item ) const;
    bool isFreePosition( const Q3IconViewItem *item ) const;
    bool isFreePosition( const Q3IconViewItem *item, const QRect& rect ) const;
    void moveToFreePosition(Q3IconViewItem *item );

    bool deleteGlobalDesktopFiles();

    static void renameDesktopFile(const QString &path, const QString &name);

    void popupMenu( const QPoint &_global, const KFileItemList& _items );
    virtual void showEvent( QShowEvent *e );
    virtual void contentsDropEvent( QDropEvent *e );
    virtual void viewportWheelEvent( QWheelEvent * );
    virtual void contentsMousePressEvent( QMouseEvent *e );
    virtual void mousePressEvent( QMouseEvent *e );
    virtual void wheelEvent( QWheelEvent* e );

private:
    void refreshTrashIcon();

private Q_SLOTS:
    void desktopResized();

private:
    /** Our action collection, parent of all our actions */
    KActionCollection m_actionCollection;

    /** KAccel object, to make the actions shortcuts work */
    KAccel *m_accel;

    bool m_bNeedRepaint;
    bool m_bNeedSave;
    bool m_autoAlign;

    /** true if even one icon has an icon-position entry in the .directory */
    bool m_hasExistingPos;

    /** whether the user may move/edit/remove desktop icons */
    bool m_bEditableDesktopIcons;

    /** Show dot files ? */
    bool m_bShowDot;

    /** Vertical or Horizontal align of icons on desktop */
    bool m_bVertAlign;

    /** The directory lister - created only in start() */
    KDirLister* m_dirLister;

    /** The list of urls to be merged into the desktop, in addition to desktopURL */
    KUrl::List m_mergeDirs;

    /** The list of dirs to be merged into the desktop, in addition to desktopURL **/
    QStringList m_desktopDirs;

    /** The desktop's .directory, used for storing icon positions */
    KSimpleConfig *m_dotDirectory;

    /** Position of last deleted icon - used when renaming a file */
    QPoint m_lastDeletedIconPos;

    /** Sorting */
    SortCriterion m_eSortCriterion;
    bool m_bSortDirectoriesFirst;
    QStringList m_itemsAlwaysFirst;

    /**
     * The shadow object
     */
    KShadowEngine *m_shadowEngine;

    /** Position where to move the next item.
     * It is set to the KRootWm position when "new file" is chosen. */
    QPoint m_nextItemPos;

    /** Position where the last drop occurred */
    QPoint m_dropPos;

    /** Position for the last dropped item */
    QPoint m_lastDropPos;

    /** URL of the items which is being RMB'ed - when only one */
    KUrl m_popupURL;

    /** media list management */
    bool m_enableMedia;
    QStringList m_excludedMedia;
};

#endif
