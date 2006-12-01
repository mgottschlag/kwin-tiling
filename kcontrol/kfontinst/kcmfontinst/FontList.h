#ifndef __FONT_LIST_H__
#define __FONT_LIST_H__

/*
 * KFontInst - KDE Font Installer
 *
 * (c) 2003-2006 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <kurl.h>
#include <kfileitem.h>
#include <kio/job.h>
#include <kdirlister.h>
#include <QList>
#include <QSet>
#include <QHash>
#include <QTreeView>
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QSortFilterProxyModel>
#include "Misc.h"

class KFileItem;
class KFileItemList;
class QMenu;
class QPixmap;
class QMimeData;
class QTimer;

#define KFI_FONT_DRAG_MIME "kfontinst/fontlist"

namespace KFI
{

class CFontItem;
class CFamilyItem;
class CGroupListItem;

enum EColumns
{
    COL_FONT,
    COL_STATUS,
    COL_PREVIEW,

    NUM_COLS
};

class CFontList : public QAbstractItemModel
{
    Q_OBJECT

    public:

    static const int constDefaultPreviewSize=25;

    static QStringList compact(const QStringList &files);
    static void        setPreviewSize(int s)         { theirPreviewSize=s; }
    static int         previewSize()                 { return theirPreviewSize; }

    CFontList(QWidget *parent = 0);
    ~CFontList();

    QVariant        data(const QModelIndex &index, int role) const;
    Qt::ItemFlags   flags(const QModelIndex &index) const;
    Qt::DropActions supportedDropActions() const;
    QMimeData *     mimeData(const QModelIndexList &indexes) const;
    QStringList     mimeTypes() const;
    QVariant        headerData(int section, Qt::Orientation orientation,
                               int role = Qt::DisplayRole) const;
    QModelIndex     index(int row, int column,
                          const QModelIndex &parent = QModelIndex()) const;
    QModelIndex     parent(const QModelIndex &index) const;
    int             rowCount(const QModelIndex &parent = QModelIndex()) const;
    int             columnCount(const QModelIndex &parent = QModelIndex()) const;
    void            setUrl(const KUrl &url);
    void            rescan();
    bool            active() const                    { return !itsDirLister->isFinished(); }
    int             row(const CFamilyItem *fam) const { return itsFamilies.indexOf((CFamilyItem *)fam); }
    void            forceNewPreviews();
    const QList<CFamilyItem *> & families() const { return itsFamilies; }
    QModelIndex     createIndex(int row, int column, void *data = 0) const
                        { return QAbstractItemModel::createIndex(row, column, data); }
    bool            hasFont(const Misc::TFont &font);

    Q_SIGNALS:

    void            finished();

    private Q_SLOTS:

    void            listingCompleted();
    void            listingCanceled();
    void            newItems(const KFileItemList &items);
    void            clearItems();
    void            deleteItem(KFileItem *item);
    void            refreshItems(const KFileItemList &);

    private:

    void            addItem(const KFileItem *item);
    CFamilyItem *   findFamily(const QString &familyName, bool create=false);
    CFontItem *     findFont(const KFileItem *item);

    private:

    QList<CFamilyItem *>                  itsFamilies;
    QHash<const KFileItem *, CFontItem *> itsFonts;   // Use for quick searching...
    KDirLister                            *itsDirLister;
    static int                            theirPreviewSize;
};

class CFontModelItem
{
    public:

    CFontModelItem(CFontModelItem *p) : itsParent(p) { }
    virtual ~CFontModelItem()                        { }

    CFontModelItem * parent() const                  { return itsParent; }
    bool             isFamily() const                { return NULL==itsParent; }
    bool             isFont() const                  { return NULL!=itsParent; }
    virtual int      rowNumber() const = 0;

    protected:

    CFontModelItem *itsParent;
};

class CFamilyItem : public CFontModelItem
{
    public:

    enum EStatus
    {
        ENABLED,
        PARTIAL,
        DISABLED
    };

    CFamilyItem(CFontList &p, const QString &n);
    virtual ~CFamilyItem();

    bool operator==(const CFamilyItem &other) const       { return itsName==other.itsName; }

    const QString &      name() const                     { return itsName; }
    const QString &      icon() const                     { return itsIcon; }
    QList<CFontItem *> & fonts()                          { return itsFonts; }
    void                 addFont(CFontItem *font);
    void                 removeFont(CFontItem *font);
    bool                 updateStatus();
    bool                 updateRegularFont(CFontItem *font);
    CFontItem *          findFont(const KFileItem *i);
    int                  rowNumber() const                { return itsParent.row(this); }
    int                  row(const CFontItem *font) const { return itsFonts.indexOf((CFontItem *)font); }
    EStatus              status() const                   { return itsStatus; }
    CFontItem *          regularFont()                    { return itsRegularFont; }

    private:

    QString            itsName,
                       itsIcon;
    QList<CFontItem *> itsFonts;
    EStatus            itsStatus;
    CFontItem          *itsRegularFont;  // 'RegularFont' is font nearest to 'Regular' style, and used for previews.
    CFontList          &itsParent;
};

//
// NOTE: CFontItem has 2 StyleInfo entries.
//          itsStyleInfo is the real one, and comes from the io-slave - this is mainly used to
//              ensure correct previews.
//          itsDisplayStyleInfo is created from itsName and is used for sorting, etc.
class CFontItem : public CFontModelItem
{
    public:

    CFontItem(CFontModelItem *p, const KFileItem *item, const QString &style=QString());
    virtual ~CFontItem() { }

    const QString &       name() const             { return itsName; }
    QString               mimetype() const         { return itsItem->mimetype(); }
    bool                  isEnabled() const        { return itsEnabled; }
    bool                  isHidden() const         { return !itsEnabled; }
    void                  updateStatus()           { itsEnabled=!Misc::isHidden(url()); }
    const KUrl &          url() const              { return itsItem->url(); }
    const KIO::UDSEntry & entry() const            { return itsItem->entry(); }
    const KFileItem *     item() const             { return itsItem; }
    bool                  isBitmap() const         { return itsBitmap; }
    const QString &       fileName() const         { return itsFileName; }
    const QString &       style() const            { return itsStyle; }
    unsigned long         styleInfo() const        { return itsStyleInfo; }
    unsigned long         displayStyleInfo() const { return itsDisplayStyleInfo; }
    int                   index() const            { return itsIndex; }
    const QString &       family()                 { return (static_cast<CFamilyItem *>(parent()))->name(); }
    const QPixmap *       pixmap(bool force=false);
    void                  clearPixmap()            { itsPixmap=NULL; }
    int                   rowNumber() const        { return (static_cast<CFamilyItem *>(parent()))->row(this); }

    private:

    const KFileItem *itsItem;
    QString         itsName,
                    itsFileName,
                    itsStyle;
    int             itsIndex;
    QPixmap         *itsPixmap;
    unsigned long   itsStyleInfo,
                    itsDisplayStyleInfo;
    bool            itsBitmap,
                    itsEnabled;
};

class CFontListSortFilterProxy : public QSortFilterProxyModel
{
    Q_OBJECT

    public:

    CFontListSortFilterProxy(QObject *parent, QAbstractItemModel *model);
    virtual ~CFontListSortFilterProxy() { }

    QVariant data(const QModelIndex &idx, int role) const;
    bool     acceptFont(CFontItem *fnt, bool checkStyleText) const;
    bool     acceptFamily(CFamilyItem *fam) const;
    bool     filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
    bool     lessThan(const QModelIndex &left, const QModelIndex &right) const;
    void     setFilterGroup(CGroupListItem *grp);
    void     setFilterText(const QString &text);
    void     setMgtMode(bool on);
    bool     mgtMode() const            { return itsMgtMode; }

    private Q_SLOTS:

    void     timeout();

    Q_SIGNALS:

    void     refresh();

    private:

    bool           itsMgtMode;
    CGroupListItem *itsGroup;
    QString        itsFilterText;
    QTimer         *itsTimer;
};

class CFontListView : public QTreeView
{
    Q_OBJECT

    public:

    CFontListView(QWidget *parent, CFontList *model);
    virtual ~CFontListView() { }

    void            readConfig(KConfig &cfg);
    void            writeConfig(KConfig &cfg);
    QModelIndexList selectedItems() const  { return selectedIndexes(); }
    void            getFonts(KUrl::List &urls, QStringList &files, QSet<Misc::TFont> *fonts,
                             bool selected, bool getEnabled=true, bool getDisabled=true);
    void            getPrintableFonts(QSet<Misc::TFont> &items, bool selected);
    void            setFilterGroup(CGroupListItem *grp);
    void            stats(int &enabled, int &disabled, int &partial);
    void            selectedStatus(bool &enabled, bool &disabled);
    QModelIndexList allFonts();
    void            setMgtMode(bool on);
    void            selectFirstFont();

    Q_SIGNALS:

    void            del();
    void            print();
    void            enable();
    void            disable();
    void            fontsDropped(const QSet<KUrl> &);
    void            itemSelected(const QModelIndex &, bool en, bool dis);
    void            refresh();
    void            reload();

    public Q_SLOTS:

    void            refreshFilter();
    void            filterText(const QString &text);

    private Q_SLOTS:

    void            setSortColumn(int col);
    void            selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void            itemCollapsed(const QModelIndex &index);

    private:

    QModelIndexList allIndexes();
    void            startDrag(Qt::DropActions supportedActions);
    void            dragEnterEvent(QDragEnterEvent *event);
    void            dropEvent(QDropEvent *event);
    void            contextMenuEvent(QContextMenuEvent *ev);

    private:

    CFontListSortFilterProxy *itsProxy;
    CFontList                *itsModel;
    QMenu                    *itsStdMenu,
                             *itsMgtMenu;
    QAction                  *itsDeleteAct,
                             *itsEnableAct,
                             *itsDisableAct,
                             *itsPrintAct;
    bool                     itsAllowDrops;
};

}

#endif
