#ifndef __KFILE_FONT_VIEW_H__
#define __KFILE_FONT_VIEW_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CKFileFontView
// Author        : Craig Drummond
// Project       : K Font Installer
// Creation Date : 31/05/2003
// Version       : $Revision$ $Date$
//
////////////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2003
////////////////////////////////////////////////////////////////////////////////

//
// NOTE: HEAVILY copied from kfiledetailview.cpp...
//
//   Copyright (C) 1997 Stephan Kulow <coolo@kde.org>
//                 2000, 2001 Carste

class KFileItem;
class QWidget;
class QKeyEvent;

#include <klistview.h>
#include <kmimetyperesolver.h>

#include "kfileview.h"

/**
 * An item for the listiew, that has a reference to its corresponding
 * @ref KFileItem.
 */
class CFontListViewItem : public KListViewItem
{
    public:

    CFontListViewItem(QListView *parent, const QString &text, const QPixmap &icon, KFileItem *fi)
	: KListViewItem(parent, text),
          itsInf(fi)
    {
        setPixmap(0, icon);
        setText(0, text);
    }

    CFontListViewItem(QListView *parent, KFileItem *fi)
        : KListViewItem(parent),
          itsInf(fi)
    {
        init();
    }

    CFontListViewItem(QListView *parent, const QString &text, const QPixmap &icon, KFileItem *fi, QListViewItem *after)
	: KListViewItem(parent, after),
          itsInf(fi)
    {
        setPixmap(0, icon);
        setText(0, text);
    }

    ~CFontListViewItem() { itsInf->removeExtraData(listView()); }

    /**
     * @returns the corresponding KFileItem
     */
    KFileItem *fileInfo() const { return itsInf; }

    virtual QString key( int /*column*/, bool /*ascending*/ ) const { return itsKey; }

    void setKey( const QString& key ) { itsKey = key; }

    QRect rect() const
    {
        QRect r = listView()->itemRect(this);

        return QRect(listView()->viewportToContents(r.topLeft()), QSize(r.width(), r.height()));
    }

    void init();

    private:

    KFileItem *itsInf;
    QString   itsKey;

    class CFontListViewItemPrivate;

    CFontListViewItemPrivate *d;
};

/**
 * A list-view capable of showing @ref KFileItem'. Used in the filedialog
 * for example. Most of the documentation is in @ref KFileView class.
 *
 * @see KDirOperator
 * @see KCombiView
 * @see KFileIconView
 */
class CKFileFontView : public KListView, public KFileView
{
    Q_OBJECT

    public:

    CKFileFontView(QWidget *parent, const char *name);
    virtual ~CKFileFontView();

    virtual QWidget *   widget() { return this; }
    virtual void        clearView();
    virtual void        setAutoUpdate(bool) {} // ### unused. remove in KDE4
    virtual void        setSelectionMode( KFile::SelectionMode sm );
    virtual void        updateView(bool b);
    virtual void        updateView(const KFileItem *i);
    virtual void        removeItem(const KFileItem *i);
    virtual void        listingCompleted();
    virtual void        setSelected(const KFileItem *i, bool b);
    virtual bool        isSelected(const KFileItem *i) const;
    virtual void        clearSelection();
    virtual void        selectAll();
    virtual void        invertSelection();
    virtual void        setCurrentItem( const KFileItem *i);
    virtual KFileItem * currentFileItem() const;
    virtual KFileItem * firstFileItem() const;
    virtual KFileItem * nextItem(const KFileItem *i) const;
    virtual KFileItem * prevItem(const KFileItem *i) const;
    virtual void        insertItem( KFileItem *i);

    // implemented to get noticed about sorting changes (for sortingIndicator)
    virtual void        setSorting(QDir::SortSpec s);

    void                ensureItemVisible(const KFileItem *i);

    // for KMimeTypeResolver
    void                mimeTypeDeterminationFinished();
    void                determineIcon(CFontListViewItem *item);
    QScrollView *       scrollWidget() const { return (QScrollView*) this; }

    protected:

    virtual void        keyPressEvent(QKeyEvent *e);

    int itsSortingCol;

    protected slots:

    void                slotSelectionChanged();

    private slots:

    void                slotSortingChanged(int c);
    void                selected(QListViewItem *item);
    void                slotActivate(QListViewItem *item);
    void                highlighted(QListViewItem *item);
    void                slotActivateMenu(QListViewItem *item, const QPoint& pos);

    private:

    virtual void        insertItem(QListViewItem *i)          { KListView::insertItem(i); }
    virtual void        setSorting(int i, bool b)             { KListView::setSorting(i, b); }
    virtual void        setSelected(QListViewItem *i, bool b) { KListView::setSelected(i, b); }

    inline CFontListViewItem * viewItem( const KFileItem *item ) const
    {
        return item ? (CFontListViewItem *) item->extraData(this) : NULL;
    }

    void                setSortingKey( CFontListViewItem *item, const KFileItem *i);

    bool                                                itsBlockSortingSignal;
    KMimeTypeResolver<CFontListViewItem,CKFileFontView> *itsResolver;

    protected:

    virtual void virtual_hook(int id, void *data);

    private:

    class CKFileFontViewPrivate;

    CKFileFontViewPrivate *d;
};

#endif
