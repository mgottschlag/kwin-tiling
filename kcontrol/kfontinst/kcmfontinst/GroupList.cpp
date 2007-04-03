/*
 * KFontInst - KDE Font Installer
 *
 * (c) 2003-2007 Craig Drummond <craig@kde.org>
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

#include "GroupList.h"
#include "FontList.h"
#include <kglobal.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kiconloader.h>
#include <kde_file.h>
#include <kmessagebox.h>
#include <kinputdialog.h>
#include <ksavefile.h>
#include <kimageeffect.h>
#include <QFont>
#include <QDropEvent>
#include <QHeaderView>
#include <QMenu>
#include <QApplication>
#include <QDomElement>
#include <QTextStream>
#include <QDir>
#include <QPainter>
#include <stdlib.h>
#include <unistd.h>
#include <utime.h>
#include "FcEngine.h"
#include "Misc.h"
#include "KfiConstants.h"
#include "config.h"

namespace KFI
{

#define GROUPS_DOC "groups"
#define GROUP_TAG  "group"
#define NAME_ATTR  "name"
#define FAMILY_TAG "family"

enum EGroupColumns
{
    COL_GROUP_NAME,
    COL_GROUP_STATUS
};

CGroupListItem::CGroupListItem(const QString &name)
              : itsName(name),
                itsType(STANDARD),
                itsHighlighted(false),
                itsStatus(CFamilyItem::ENABLED)
{
    itsData.validated=false;
}

CGroupListItem::CGroupListItem(EType type, CGroupList *p)
              : itsType(type),
                itsHighlighted(false),
                itsStatus(CFamilyItem::ENABLED)
{
    switch(itsType)
    {
        case ALL:
            itsName=i18n("All Fonts");
            break;
        case PERSONAL:
            itsName=i18n("Personal Fonts");
            break;
        case SYSTEM:
            itsName=i18n("System Fonts");
            break;
        default:
            itsName=i18n("Unclassified");
    }
    itsData.parent=p;
}

bool CGroupListItem::hasFont(const CFontItem *fnt) const
{
    switch(itsType)
    {
        case STANDARD:
            return itsFamilies.contains(fnt->family());
        case PERSONAL:
            return !fnt->isSystem();
        case SYSTEM:
            return fnt->isSystem();
        case ALL:
            return true;
        case UNCLASSIFIED:
        {
            QList<CGroupListItem *>::ConstIterator it(itsData.parent->itsGroups.begin()),
                                                   end(itsData.parent->itsGroups.end());

            for(; it!=end; ++it)
                if((*it)->isStandard() && (*it)->families().contains(fnt->family()))
                    return false;
            return true;
        }
    }
    return false;
}

void CGroupListItem::updateStatus(QSet<QString> &enabled, QSet<QString> &disabled, QSet<QString> &partial)
{
    QSet<QString> families(itsFamilies);

    if(0!=families.intersect(partial).count())
        itsStatus=CFamilyItem::PARTIAL;
    else
    {
        families=itsFamilies;

        bool haveEnabled(0!=families.intersect(enabled).count());

        families=itsFamilies;

        bool haveDisabled(0!=families.intersect(disabled).count());

        if(haveEnabled && haveDisabled)
            itsStatus=CFamilyItem::PARTIAL;
        else if(haveEnabled && !haveDisabled)
            itsStatus=CFamilyItem::ENABLED;
        else
            itsStatus=CFamilyItem::DISABLED;
    }
}

bool CGroupListItem::load(QDomElement &elem)
{
    if(elem.hasAttribute(NAME_ATTR))
    {
        itsName=elem.attribute(NAME_ATTR);
        addFamilies(elem);
        return true;
    }
    return false;
}

bool CGroupListItem::addFamilies(QDomElement &elem)
{
    int b4(itsFamilies.count());

    for(QDomNode n=elem.firstChild(); !n.isNull(); n=n.nextSibling())
    {
        QDomElement ent=n.toElement();

        if(FAMILY_TAG==ent.tagName())
            itsFamilies.insert(ent.text());
    }
    return b4!=itsFamilies.count();
}

void CGroupListItem::save(QTextStream &str)
{
    str << " <"GROUP_TAG" "NAME_ATTR"=\"" << itsName << "\">" << endl;
    if(itsFamilies.count())
    {
        QSet<QString>::ConstIterator it(itsFamilies.begin()),
                                     end(itsFamilies.end());

        for(; it!=end; ++it)
            str << "  <"FAMILY_TAG">" << (*it) << "</"FAMILY_TAG">" << endl;
    }
    str << " </"GROUP_TAG">" << endl;
}

CGroupList::CGroupList(QWidget *parent)
          : QAbstractItemModel(parent),
            itsTimeStamp(0),
            itsModified(false),
            itsParent(parent),
            itsSortOrder(Qt::AscendingOrder)
{
    itsSpecialGroups[CGroupListItem::ALL]=new CGroupListItem(CGroupListItem::ALL, this);
    itsGroups.append(itsSpecialGroups[CGroupListItem::ALL]);
    if(Misc::root())
        itsSpecialGroups[CGroupListItem::PERSONAL]=
        itsSpecialGroups[CGroupListItem::SYSTEM]=NULL;
    else
    {
        itsSpecialGroups[CGroupListItem::PERSONAL]=new CGroupListItem(CGroupListItem::PERSONAL, this);
        itsGroups.append(itsSpecialGroups[CGroupListItem::PERSONAL]);
        itsSpecialGroups[CGroupListItem::SYSTEM]=new CGroupListItem(CGroupListItem::SYSTEM, this);
        itsGroups.append(itsSpecialGroups[CGroupListItem::SYSTEM]);
    }
    itsSpecialGroups[CGroupListItem::UNCLASSIFIED]=
                new CGroupListItem(CGroupListItem::UNCLASSIFIED, this);
    itsGroups.append(itsSpecialGroups[CGroupListItem::UNCLASSIFIED]);

    // Locate groups.xml file - normall will be ~/.config/fontgroups.xml
    QString path(KGlobal::dirs()->localxdgconfdir());

    if(!Misc::dExists(path))
        Misc::createDir(path);

    itsFileName=path+"/"KFI_GROUPS_FILE;

    rescan();
}

CGroupList::~CGroupList()
{
    save();
    qDeleteAll(itsGroups);
    itsGroups.clear();
}

int CGroupList::columnCount(const QModelIndex &) const
{
    return 2;
}

void CGroupList::update(const QModelIndex &unHighlight, const QModelIndex &highlight)
{
    if(unHighlight.isValid())
    {
        CGroupListItem *grp=static_cast<CGroupListItem *>(unHighlight.internalPointer());
        if(grp)
            grp->setHighlighted(false);
        emit dataChanged(unHighlight, unHighlight);
    }
    if(highlight.isValid())
    {
        CGroupListItem *grp=static_cast<CGroupListItem *>(highlight.internalPointer());
        if(grp)
            grp->setHighlighted(true);
        emit dataChanged(highlight, highlight);
    }
}

void CGroupList::updateStatus(QSet<QString> &enabled, QSet<QString> &disabled, QSet<QString> &partial)
{
    QList<CGroupListItem *>::Iterator it(itsGroups.begin()),
                                      end(itsGroups.end());

    for(; it!=end; ++it)
        if((*it)->isStandard())
            (*it)->updateStatus(enabled, disabled, partial);
    emit layoutChanged();
}

QVariant CGroupList::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    CGroupListItem *grp=static_cast<CGroupListItem *>(index.internalPointer());

    if(grp)
        switch(index.column())
        {
            case COL_GROUP_NAME:
                switch(role)
                {
                    case Qt::FontRole:
                        if(grp->isStandard())
                        {
                            if(CFamilyItem::DISABLED==grp->status())
                            {
                                QFont font;
                                font.setItalic(true);
                                return font;
                            }
                        }
                        else
                        {
                            QFont font;
                            font.setBold(true);
                            return font;
                        }
                        break;
                    case Qt::DisplayRole:
                        return grp->name();
                    case Qt::DecorationRole:
                        if(grp->highlighted())
                            return SmallIcon(Qt::LeftToRight==QApplication::layoutDirection()
                                       ? "arrow-right" : "arrow-left");
                    default:
                        break;
                }
                break;
            case COL_GROUP_STATUS:
                if(Qt::DecorationRole==role)
                    if(grp->isStandard())
                        if(grp->families().count())
                            switch(grp->status())
                            {
                                case CFamilyItem::PARTIAL:
                                    return SmallIcon("dialog-ok", 0, K3Icon::DisabledState);
                                case CFamilyItem::ENABLED:
                                    return SmallIcon("dialog-ok");
                                case CFamilyItem::DISABLED:
                                    return SmallIcon("dialog-cancel");
                            }
                        else
                            return SmallIcon("dialog-cancel");
        }
    return QVariant();
}

Qt::ItemFlags CGroupList::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled;
}

QVariant CGroupList::headerData(int section, Qt::Orientation orientation,
                                int role) const
{
    if (Qt::Horizontal==orientation)
        switch(section)
        {
            case COL_GROUP_NAME:
                if(Qt::DisplayRole==role)
                    return i18n("Group");
                else if(Qt::TextAlignmentRole==role)
                    return Qt::AlignLeft;
                break;
            case COL_GROUP_STATUS:
                if(Qt::DecorationRole==role)
                    return SmallIcon("enablefont");
        }

    return QVariant();
}

QModelIndex CGroupList::index(int row, int column, const QModelIndex &parent) const
{
    if(!parent.isValid())
    {
        CGroupListItem *grp=itsGroups.value(row);

        if(grp)
            return createIndex(row, column, grp);
    }

    return QModelIndex();
}

QModelIndex CGroupList::parent(const QModelIndex &) const
{
    return QModelIndex();
}

int CGroupList::rowCount(const QModelIndex &) const
{
    return itsGroups.count();
}

void CGroupList::rescan()
{
    save();
    load();
    sort(itsSortCol, itsSortOrder);
}

void CGroupList::load()
{
    time_t ts=Misc::getTimeStamp(itsFileName);

    if(!ts || ts!=itsTimeStamp)
    {
        clear();
        itsTimeStamp=ts;
        if(load(itsFileName))
            itsModified=false;
    }
}

bool CGroupList::load(const QString &file)
{
    QFile f(file);
    bool  rv(false);

    if(f.open(IO_ReadOnly))
    {
        QDomDocument doc;

        if(doc.setContent(&f))
            for(QDomNode n=doc.documentElement().firstChild(); !n.isNull(); n=n.nextSibling())
            {
                QDomElement e=n.toElement();

                if(GROUP_TAG==e.tagName() && e.hasAttribute(NAME_ATTR))
                {
                    QString name(e.attribute(NAME_ATTR));

                    CGroupListItem *item=find(name);

                    if(!item)
                    {
                        item=new CGroupListItem(name);
                        itsGroups.append(item);
                        rv=true;
                    }

                    if(item->addFamilies(e))
                        rv=true;
                }
            }
    }
    return rv;
}

bool CGroupList::save()
{
    if(itsModified && save(itsFileName, NULL))
    {
        itsTimeStamp=Misc::getTimeStamp(itsFileName);
        return true;
    }
    return false;
}

bool CGroupList::save(const QString &fileName, CGroupListItem *grp)
{
    KSaveFile file(fileName);

    if(file.open())
    {
        QTextStream str(&file);

        str << "<"GROUPS_DOC">" << endl;

        if(grp)
            grp->save(str);
        else
        {
            QList<CGroupListItem *>::Iterator it(itsGroups.begin()),
                                              end(itsGroups.end());

            for(; it!=end; ++it)
                if((*it)->isStandard())
                    (*it)->save(str);
        }
        str << "</"GROUPS_DOC">" << endl;
        itsModified=false;
        return file.finalize();
    }

    return false;
}

void CGroupList::merge(const QString &file)
{
    if(load(file))
    {
        itsModified=true;
        sort(itsSortCol, itsSortOrder);
    }
}

void CGroupList::clear()
{
    beginRemoveRows(QModelIndex(), 0, itsGroups.count());
    endRemoveRows();
    itsGroups.removeFirst(); // Remove all
    if(itsSpecialGroups[CGroupListItem::SYSTEM])
    {
        itsGroups.removeFirst(); // Remove personal
        itsGroups.removeFirst(); // Remove system
    }
    itsGroups.removeFirst(); // Remove unclassif...
    qDeleteAll(itsGroups);
    itsGroups.clear();
    itsGroups.append(itsSpecialGroups[CGroupListItem::ALL]);
    if(itsSpecialGroups[CGroupListItem::SYSTEM])
    {
        itsGroups.append(itsSpecialGroups[CGroupListItem::PERSONAL]);
        itsGroups.append(itsSpecialGroups[CGroupListItem::SYSTEM]);
    }
    itsGroups.append(itsSpecialGroups[CGroupListItem::UNCLASSIFIED]);
}

QModelIndex CGroupList::index(CGroupListItem::EType t)
{
    return createIndex(0, 0, itsSpecialGroups[t]);
}

void CGroupList::createGroup(const QString &name)
{
    if(!exists(name))
    {
        itsGroups.append(new CGroupListItem(name));
        itsModified=true;
        save();
        sort(itsSortCol, itsSortOrder);
    }
}

void CGroupList::renameGroup(const QModelIndex &idx, const QString &name)
{
    if(idx.isValid())
    {
        CGroupListItem *grp=static_cast<CGroupListItem *>(idx.internalPointer());

        if(grp && grp->isStandard() && grp->name()!=name && !exists(name))
        {
            grp->setName(name);
            itsModified=true;
            save();
            sort(itsSortCol, itsSortOrder);
        }
    }
}

bool CGroupList::removeGroup(const QModelIndex &idx)
{
    if(idx.isValid())
    {
        CGroupListItem *grp=static_cast<CGroupListItem *>(idx.internalPointer());

        if(grp && grp->isStandard() &&
           KMessageBox::Yes==KMessageBox::warningYesNo(itsParent,
                                          i18n("<p>Do you really want to remove \'<b>%1</b>\'?</p>"
                                               "<p><i>This will only remove the group, and not "
                                               "the actual fonts.</i></p>", grp->name()),
                                          i18n("Remove Group"), KGuiItem(i18n("Remove"), "list-remove",
                                          i18n("Remove group"))))
        {
            itsModified=true;
            itsGroups.remove(grp);
            delete grp;
            save();
            sort(itsSortCol, itsSortOrder);
            return true;
        }
    }

    return false;
}

void CGroupList::removeFromGroup(const QModelIndex &group, const QSet<QString> &families)
{
    if(group.isValid())
    {
        CGroupListItem *grp=static_cast<CGroupListItem *>(group.internalPointer());

        if(grp && grp->isStandard())
        {
            QSet<QString>::ConstIterator it(families.begin()),
                                         end(families.end());
            bool                         update(false);

            for(; it!=end; ++it)
                if(removeFromGroup(grp, *it))
                    update=true;

            if(update)
                emit refresh();
        }
    }
}

void CGroupList::addToGroup(const QModelIndex &group, const QSet<QString> &families)
{
    if(group.isValid())
    {
        CGroupListItem *grp=static_cast<CGroupListItem *>(group.internalPointer());

        if(grp && grp->isStandard())
        {
            QSet<QString>::ConstIterator it(families.begin()),
                                         end(families.end());
            bool                         update(false);

            for(; it!=end; ++it)
                if(!grp->hasFamily(*it))
                {
                    grp->addFamily(*it);
                    update=true;
                    itsModified=true;
                }

            if(update)
                emit refresh();
        }
    }
}

void CGroupList::removeFamily(const QString &family)
{
    QList<CGroupListItem *>::ConstIterator it(itsGroups.begin()),
                                           end(itsGroups.end());

    for(; it!=end; ++it)
        removeFromGroup(*it, family);
}

bool CGroupList::removeFromGroup(CGroupListItem *grp, const QString &family)
{
    if(grp && grp->isStandard() && grp->hasFamily(family))
    {
        grp->removeFamily(family);
        itsModified=true;
        return true;
    }

    return false;
}

static bool groupNameLessThan(const CGroupListItem *f1, const CGroupListItem *f2)
{
    return f1 && f2 && (f1->type()<f2->type() ||
                       (f1->type()==f2->type() && QString::localeAwareCompare(f1->name(), f2->name())<0));
}

static bool groupNameGreaterThan(const CGroupListItem *f1, const CGroupListItem *f2)
{
    return f1 && f2 && (f1->type()<f2->type() ||
                       (f1->type()==f2->type() && QString::localeAwareCompare(f1->name(), f2->name())>0));
}

static bool groupStatusLessThan(const CGroupListItem *f1, const CGroupListItem *f2)
{
    return f1 && f2 && (f1->type()<f2->type() ||
                       (f1->type()==f2->type() && (f1->status()<f2->status() ||
                       (f1->status()==f2->status() && QString::localeAwareCompare(f1->name(), f2->name())<0))));
}

static bool groupStatusGreaterThan(const CGroupListItem *f1, const CGroupListItem *f2)
{
    return f1 && f2 && (f1->type()<f2->type() ||
                       (f1->type()==f2->type() && (f1->status()>f2->status() ||
                       (f1->status()==f2->status() && QString::localeAwareCompare(f1->name(), f2->name())>0))));
}

void CGroupList::sort(int column, Qt::SortOrder order)
{
    itsSortOrder=order;
    itsSortCol=column;

    qSort(itsGroups.begin(), itsGroups.end(),
          COL_GROUP_NAME==column
            ? Qt::AscendingOrder==order ? groupNameLessThan : groupNameGreaterThan
            : Qt::AscendingOrder==order ? groupStatusLessThan : groupStatusGreaterThan);

    emit layoutChanged();
}

Qt::DropActions CGroupList::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

QStringList CGroupList::mimeTypes() const
{
    QStringList types;
    types << KFI_FONT_DRAG_MIME;
    return types;
}

CGroupListItem * CGroupList::find(const QString &name)
{
    QList<CGroupListItem *>::ConstIterator it(itsGroups.begin()),
                                           end(itsGroups.end());

    for(; it!=end; ++it)
        if((*it)->name()==name)
            return (*it);

    return NULL;
}

bool CGroupList::exists(const QString &name)
{
    if(NULL!=find(name))
    {
        KMessageBox::error(itsParent, i18n("<qt>A group named <b>\'%1\'</b> already "
                                           "exists!</qt>", name));
        return true;
    }

    return false;
}

CGroupListView::CGroupListView(QWidget *parent, CGroupList *model)
              : QTreeView(parent)
{
    setModel(model);
    sortByColumn(COL_GROUP_NAME, Qt::AscendingOrder);
    resizeColumnToContents(COL_GROUP_NAME);
    setColumnWidth(COL_GROUP_STATUS, SmallIcon("folder").size().width()+8);
    setSortingEnabled(true);
    setAllColumnsShowFocus(true);
    setAlternatingRowColors(true);
    setAcceptDrops(true);
    setDragDropMode(QAbstractItemView::DropOnly);
    setDropIndicatorShown(true);
    setDragEnabled(false);
    header()->setSortIndicatorShown(true);
    setRootIsDecorated(false);
    itsMenu=new QMenu(this);

    itsDeleteAct=itsMenu->addAction(KIcon("list-remove"), i18n("Remove..."),
                                    this, SIGNAL(del()));
    itsEnableAct=itsMenu->addAction(KIcon("enablefont"), i18n("Enable..."),
                                    this, SIGNAL(enable()));
    itsDisableAct=itsMenu->addAction(KIcon("disablefont"), i18n("Disable..."),
                                     this, SIGNAL(disable()));
    itsMenu->addSeparator();
    itsRenameAct=itsMenu->addAction(i18n("Rename..."), this, SLOT(rename()));
    itsMenu->addSeparator();
    itsPrintAct=itsMenu->addAction(KIcon("document-print"), i18n("Print..."),
                                   this, SIGNAL(print()));

    setWhatsThis(i18n("<p>This list shows font groups.</p>"));

    connect(this, SIGNAL(addFamilies(const QModelIndex &,  const QSet<QString> &)),
            model, SLOT(addToGroup(const QModelIndex &,  const QSet<QString> &)));
    connect(this, SIGNAL(removeFamilies(const QModelIndex &,  const QSet<QString> &)),
            model, SLOT(removeFromGroup(const QModelIndex &,  const QSet<QString> &)));
}

CGroupListItem::EType CGroupListView::getType()
{
    QModelIndexList selectedItems(selectedIndexes());

    if(selectedItems.count() && selectedItems.last().isValid())
    {
        CGroupListItem *grp=static_cast<CGroupListItem *>(selectedItems.last().internalPointer());

        return grp->type();
    }

    return CGroupListItem::ALL;
}

void CGroupListView::controlMenu(bool del, bool en, bool dis, bool p)
{
    itsDeleteAct->setEnabled(del);
    itsRenameAct->setEnabled(del);
    itsEnableAct->setEnabled(en);
    itsDisableAct->setEnabled(dis);
    itsPrintAct->setEnabled(p);
}

void CGroupListView::selectionChanged(const QItemSelection &selected,
                                      const QItemSelection &deselected)
{
    QAbstractItemView::selectionChanged(selected, deselected);

    QModelIndexList selectedItems(selectedIndexes());

    emit itemSelected(selectedItems.count()
                        ? selectedItems.last()
                        : QModelIndex());
}

void CGroupListView::rename()
{
    QModelIndex index(currentIndex());

    if(index.isValid())
    {
        CGroupListItem *grp=static_cast<CGroupListItem *>(index.internalPointer());

        if(grp && grp->isStandard())
        {
            bool    ok;
            QString name(KInputDialog::getText(i18n("Rename Group"),
                                               i18n("Please enter a new name for group:"),
                                               grp->name(), &ok, this));

            if(ok && !name.isEmpty() && name!=grp->name())
                ((CGroupList *)model())->renameGroup(index, name);
        }
    }
}

void CGroupListView::contextMenuEvent(QContextMenuEvent *ev)
{
    if(indexAt(ev->pos()).isValid())
        itsMenu->popup(ev->globalPos());
}

void CGroupListView::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->provides(KFI_FONT_DRAG_MIME))
        event->acceptProposedAction();
}

void CGroupListView::dragMoveEvent(QDragMoveEvent *event)
{
    if(event->provides(KFI_FONT_DRAG_MIME))
    {
        QModelIndex index(indexAt(event->pos()));

        if(index.isValid())
        {
            if(COL_GROUP_NAME!=index.column())
                index=((CGroupList *)model())->createIdx(index.row(), COL_GROUP_NAME, index.internalPointer());

            CGroupListItem *dest=static_cast<CGroupListItem *>(index.internalPointer());

            if(dest &&
               (dest->isStandard() || (isStandard() && dest->isAll())) &&
               !selectedIndexes().contains(index))
            {
                drawHighlighter(index);
                event->acceptProposedAction();
                return;
            }
        }
        event->ignore();
        drawHighlighter(QModelIndex());
    }
}

void CGroupListView::dragLeaveEvent(QDragLeaveEvent *)
{
    drawHighlighter(QModelIndex());
}

void CGroupListView::dropEvent(QDropEvent *event)
{
    drawHighlighter(QModelIndex());
    if(event->provides(KFI_FONT_DRAG_MIME))
    {
        event->acceptProposedAction();

        QSet<QString> families;
        QByteArray    encodedData(event->mimeData()->data(KFI_FONT_DRAG_MIME));
        QDataStream   ds(&encodedData, QIODevice::ReadOnly);
        QModelIndex   from(selectedIndexes().last()),
                      to(indexAt(event->pos()));

        ds >> families;
        // Are we removing a font from the current group?
        if(to.isValid() && from.isValid() &&
           (static_cast<CGroupListItem *>(from.internalPointer()))->isStandard() &&
           !(static_cast<CGroupListItem *>(to.internalPointer()))->isStandard())
            emit removeFamilies(from, families);
        else
            emit addFamilies(to, families);

        if(isUnclassified())
            emit unclassifiedChanged();
    }
}

void CGroupListView::drawHighlighter(const QModelIndex &idx)
{
    if(itsCurrentDropItem!=idx)
    {
        ((CGroupList *)model())->update(itsCurrentDropItem, idx);
        itsCurrentDropItem=idx;
    }
}

}

#include "GroupList.moc"
