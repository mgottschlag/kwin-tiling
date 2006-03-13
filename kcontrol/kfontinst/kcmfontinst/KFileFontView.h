//Added by qt3to4:
#include <QPixmap>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QKeyEvent>
#include <QDropEvent>
#include <QDragEnterEvent>
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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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

#include <k3listview.h>
#include <kmimetyperesolver.h>

#include "kfileview.h"

/**
 * An item for the listiew, that has a reference to its corresponding
 * @ref KFileItem.
 */
class CFontListViewItem : public K3ListViewItem
{
    public:

    CFontListViewItem(Q3ListView *parent, const QString &text, const QPixmap &icon, KFileItem *fi)
	: K3ListViewItem(parent, text),
          itsInf(fi)
    {
        setPixmap(0, icon);
        setText(0, text);
    }

    CFontListViewItem(Q3ListView *parent, KFileItem *fi)
        : K3ListViewItem(parent),
          itsInf(fi)
    {
        init();
    }

    CFontListViewItem(Q3ListView *parent, const QString &text, const QPixmap &icon, KFileItem *fi, Q3ListViewItem *after)
	: K3ListViewItem(parent, after),
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
class CKFileFontView : public K3ListView, public KFileView
{
    Q_OBJECT

    public:

    CKFileFontView(QWidget *parent);
    virtual ~CKFileFontView();

    virtual QWidget *   widget() { return this; }
    virtual void        clearView();
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

    void                readConfig(KConfig *kc, const QString &group);
    void                writeConfig(KConfig *kc, const QString &group);

    // implemented to get noticed about sorting changes (for sortingIndicator)
    virtual void        setSorting(QDir::SortFlags s);
    void                ensureItemVisible(const KFileItem *i);

    // for KMimeTypeResolver
    void                mimeTypeDeterminationFinished();
    void                determineIcon(CFontListViewItem *item);
    Q3ScrollView *       scrollWidget() const { return (Q3ScrollView*) this; }

    Q_SIGNALS:
    // The user dropped something.
    // fileItem points to the item dropped on or can be 0 if the
    // user dropped on empty space.
    void                dropped(QDropEvent *event, KFileItem *fileItem);
    // The user dropped the URLs urls.
    // url points to the item dropped on or can be empty if the
    // user dropped on empty space.
    void                dropped(QDropEvent *event, const KUrl::List &urls, const KUrl &url);

    protected:

    virtual void        keyPressEvent(QKeyEvent *e);
    // DND support
    Q3DragObject *       dragObject();
    void                contentsDragEnterEvent(QDragEnterEvent *e);
    void                contentsDragMoveEvent(QDragMoveEvent *e);
    void                contentsDragLeaveEvent(QDragLeaveEvent *e);
    void                contentsDropEvent(QDropEvent *e);
    bool                acceptDrag(QDropEvent *e) const;

    int itsSortingCol;

    protected Q_SLOTS:

    void                slotSelectionChanged();

    private Q_SLOTS:

    void                slotSortingChanged(int c);
    void                selected(Q3ListViewItem *item);
    void                slotActivate(Q3ListViewItem *item);
    void                highlighted(Q3ListViewItem *item);
    void                slotActivateMenu(Q3ListViewItem *item, const QPoint& pos);
    void                slotAutoOpen();

    private:

    virtual void        insertItem(Q3ListViewItem *i)          { K3ListView::insertItem(i); }
    virtual void        setSorting(int i, bool b)             { K3ListView::setSorting(i, b); }
    virtual void        setSelected(Q3ListViewItem *i, bool b) { K3ListView::setSelected(i, b); }

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
