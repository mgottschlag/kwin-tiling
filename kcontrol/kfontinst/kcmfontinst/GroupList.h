#ifndef __GROUP_LIST_H__
#define __GROUP_LIST_H__

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
#include <QTreeView>
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

class QDragEnterEvent;
class QDragLeaveEvent;
class QDropEvent;
class QTextStream;
class QDomElement;

namespace KFI
{

class CGroupList;
class CFontItem;

class CGroupListItem
{
    public:

    enum EType
    {
        ALL,
        PERSONAL,
        SYSTEM,
        UNCLASSIFIED,
        STANDARD
    };

    union Data
    {
        bool       validated;
        CGroupList *parent;
    };

    CGroupListItem(const QString &name);
    CGroupListItem(EType type, CGroupList *p);

    const QString & name() const              { return itsName; }
    void            setName(const QString &n) { itsName=n; }
    QSet<QString> & families()                { return itsFamilies; }
    const EType     type() const              { return itsType; }
    bool            isStandard() const        { return STANDARD==itsType; }
    bool            isAll() const             { return ALL==itsType; }
    bool            isUnclassified() const    { return UNCLASSIFIED==itsType; }
    bool            isPersonal() const        { return PERSONAL==itsType; }
    bool            isSystem() const          { return SYSTEM==itsType; }
    bool            validated() const         { return isStandard() ? itsData.validated : true; }
    void            setValidated()            { if(isStandard()) itsData.validated=true; }
    bool            highlighted() const       { return itsHighlighted; }
    void            setHighlighted(bool b)    { itsHighlighted=b; }
    bool            hasFont(const CFontItem *fnt) const;

    bool            load(QDomElement &elem);
    bool            addFamilies(QDomElement &elem);
    void            save(QTextStream &str);
    void            addFamily(const QString &family)    { itsFamilies.insert(family); }
    void            removeFamily(const QString &family) { itsFamilies.remove(family); }
    bool            hasFamily(const QString &family)    { return itsFamilies.contains(family); }

    private:

    QSet<QString> itsFamilies;
    QString       itsName;
    EType         itsType;
    Data          itsData;
    bool          itsHighlighted;
};

class CGroupList : public QAbstractItemModel
{
    Q_OBJECT

    public:

    CGroupList(QWidget *parent = 0);
    ~CGroupList();

    QVariant        data(const QModelIndex &index, int role) const;
    Qt::ItemFlags   flags(const QModelIndex &index) const;
    QVariant        headerData(int section, Qt::Orientation orientation,
                               int role = Qt::DisplayRole) const;
    QModelIndex     index(int row, int column,
                          const QModelIndex &parent = QModelIndex()) const;
    QModelIndex     parent(const QModelIndex &index) const;
    int             rowCount(const QModelIndex &parent = QModelIndex()) const;
    int             columnCount(const QModelIndex &parent = QModelIndex()) const;
    void            update(const QModelIndex &unHighlight, const QModelIndex &highlight);
    void            setSysMode(bool sys);
    void            rescan();
    void            load();
    bool            load(const QString &file);
    bool            save();
    bool            save(const QString &fileName, CGroupListItem *grp);
    void            merge(const QString &file);
    void            clear();
    QModelIndex     index(CGroupListItem::EType t);
    void            createGroup(const QString &name);
    void            renameGroup(const QModelIndex &idx, const QString &name);
    bool            removeGroup(const QModelIndex &idx);
    void            removeFamily(const QString &family);
    void            removeFromGroup(CGroupListItem *grp, const QString &family);

    CGroupListItem * group(CGroupListItem::EType t)
                        { return itsSpecialGroups[t]; }

    public Q_SLOTS:

    void            addToGroup(const QModelIndex &group, const QSet<QString> &families);
    void            removeFromGroup(const QModelIndex &group, const QSet<QString> &families);

    Q_SIGNALS:

    void            refresh();

    private:

    void            readGroupsFile();
    void            sort(int column, Qt::SortOrder order = Qt::AscendingOrder);
    Qt::DropActions supportedDropActions() const;
    QStringList     mimeTypes() const;
    CGroupListItem * find(const QString &name);
    bool            exists(const QString &name);

    private:

    QString                 itsFileName;
    time_t                  itsTimeStamp;
    bool                    itsModified;
    QWidget                 *itsParent;
    QList<CGroupListItem *> itsGroups;
    CGroupListItem          *itsSpecialGroups[4];
    Qt::SortOrder           itsSortOrder;

    friend class CGroupListItem;
};

class CGroupListView : public QTreeView
{
    Q_OBJECT

    public:

    CGroupListView(QWidget *parent, CGroupList *model);
    virtual ~CGroupListView()              { }

    QSize                 sizeHint() const { return QSize(32, 32); }

    bool                  isStandard()     { return CGroupListItem::STANDARD==getType(); }
    bool                  isUnclassified() { return CGroupListItem::UNCLASSIFIED==getType(); }
    CGroupListItem::EType getType();
    void                  controlMenu(bool del, bool en, bool dis, bool p, bool ex);

    Q_SIGNALS:

    void                  del();
    void                  print();
    void                  enable();
    void                  disable();
    void                  addFamilies(const QModelIndex &group, const QSet<QString> &);
    void                  removeFamilies(const QModelIndex &group, const QSet<QString> &);
    void                  itemSelected(const QModelIndex &);
    void                  exportGroup();
    void                  unclassifiedChanged();

    private Q_SLOTS:

    void                  selectionChanged(const QItemSelection &selected,
                                           const QItemSelection &deselected);
    void                  rename();

    private:

    void                  contextMenuEvent(QContextMenuEvent *ev);
    void                  dragEnterEvent(QDragEnterEvent *event);
    void                  dragMoveEvent(QDragMoveEvent *event);
    void                  dragLeaveEvent(QDragLeaveEvent *event);
    void                  dropEvent(QDropEvent *event);
    void                  drawHighlighter(const QModelIndex &idx);

    private:

    QMenu       *itsMenu;
    QAction     *itsDeleteAct,
                *itsEnableAct,
                *itsDisableAct,
                *itsPrintAct,
                *itsExportAct,
                *itsRenameAct;
    QModelIndex itsCurrentDropItem;
};
}

#endif
