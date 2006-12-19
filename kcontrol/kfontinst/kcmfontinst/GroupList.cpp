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

#include "GroupList.h"
#include <kglobal.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kiconloader.h>
#include <kde_file.h>
#include <kmessagebox.h>
#include <kinputdialog.h>
#include <QFont>
#include <QDropEvent>
#include <QDateTime>
#include <QX11Info>
#include <QHeaderView>
#include <QItemDelegate>
#include <QPainter>
#include <QMenu>
#include <QApplication>
#include <stdlib.h>
#include <unistd.h>
#include <utime.h>
#include "FcEngine.h"
#include "Misc.h"
#include "KfiConstants.h"
#include "FontList.h"
#include "config.h"

namespace KFI
{

static QStringList createFontNames(const QList<Misc::TFont> &fonts)
{
    QStringList                       rv;
    QList<Misc::TFont>::ConstIterator it(fonts.begin()),
                                      end(fonts.end());

    for(; it!=end; ++it)
        rv.append(CFcEngine::createName((*it).family, (*it).styleInfo));
    return rv;
}

CGroupListItem::CGroupListItem(CFontInfo::TGroupList::Iterator &item)
              : itsItem(item),
                itsType(STANDARD),
                itsHighlighted(false)
{
    itsData.validated=false;
}

CGroupListItem::CGroupListItem(EType type, CGroupList *p)
              : itsType(type),
                itsHighlighted(false)
{
    itsName=ALL==itsType ? i18n("All Fonts") : i18n("Unclassified");
    itsData.parent=p;
}

bool CGroupListItem::hasFont(const CFontInfo::TFont &fnt) const
{
    switch(itsType)
    {
        case STANDARD:
            return (*itsItem).fonts.contains(fnt);
        case ALL:
            return true;
        case UNCLASSIFIED:
        {
            QList<CGroupListItem *>::ConstIterator it(itsData.parent->itsGroups.begin()),
                                                   end(itsData.parent->itsGroups.end());

            for(; it!=end; ++it)
                if((*it)->isStandard() && (*(*it)->item()).fonts.contains(fnt))
                    return false;
            return true;
        }
    }
    return false;
}

CGroupList::CGroupList(QWidget *parent)
         : QAbstractItemModel(parent),
           itsParent(parent),
           itsSysMode(false),
           itsFontGroups(NULL),
           itsSortOrder(Qt::AscendingOrder)
{
    itsAllGroup=new CGroupListItem(CGroupListItem::ALL, this);
    itsGroups.append(itsAllGroup);
    itsUnclassifiedGroup=new CGroupListItem(CGroupListItem::UNCLASSIFIED, this);
    itsGroups.append(itsUnclassifiedGroup);
}

CGroupList::~CGroupList()
{
    qDeleteAll(itsGroups);
    itsGroups.clear();
    delete itsFontGroups;
    itsFontGroups=NULL;
}

int CGroupList::columnCount(const QModelIndex &) const
{
    return 1;
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

QVariant CGroupList::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || 0!=index.column())
        return QVariant();

    CGroupListItem *grp=static_cast<CGroupListItem *>(index.internalPointer());

    if(grp)
        switch(role)
        {
            case Qt::FontRole:
                if(!grp->isStandard())
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
                    return SmallIcon(Qt::LeftToRight==QApplication::layoutDirection() ? "1rightarrow" : "1leftarrow");
            default:
                break;
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
    if (orientation == Qt::Horizontal && 0==section)
        if(Qt::DisplayRole==role)
            return i18n("Group");
        else if(Qt::TextAlignmentRole==role)
            return Qt::AlignLeft;

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

void CGroupList::setSysMode(bool sys)
{
    if(!Misc::root() && itsSysMode!=sys)
        itsSysMode=sys;

    rescan();
}

void CGroupList::rescan()
{
    clear();
    readGroupsFile();
}

void CGroupList::clear()
{
    beginRemoveRows(QModelIndex(), 0, itsGroups.count());
    endRemoveRows();
    itsGroups.removeFirst(); // Remove all
    itsGroups.removeFirst(); // Remove unclassif...
    qDeleteAll(itsGroups);
    itsGroups.clear();
    itsGroups.append(itsAllGroup);
    itsGroups.append(itsUnclassifiedGroup);
    delete itsFontGroups;
    itsFontGroups=NULL;
}

QModelIndex CGroupList::allIndex()
{
    return createIndex(0, 0, itsAllGroup);
}

void CGroupList::createGroup(const QString &name)
{
    QList<CGroupListItem *>::ConstIterator it(itsGroups.begin()),
                                           end(itsGroups.end());

    for(; it!=end; ++it)
        if((*it)->name()==name)
        {
            KMessageBox::error(itsParent, i18n("<qt>A group named <b>\'%1\'</b> already "
                                               "exists!</qt>", name));
            return;
        }

    CFontInfo::TGroupList::Iterator git=itsFontGroups->create(name);

    if(git!=itsFontGroups->items().end())
    {
        itsGroups.append(new CGroupListItem(git));
        itsFontGroups->save();
        sort(0, itsSortOrder);
    }
    else
        KMessageBox::error(itsParent, i18n("<qt>Could not create <b>\'%1\'</b>.</qt>", name));
}

void CGroupList::renameGroup(const QModelIndex &idx, const QString &name)
{
    if(idx.isValid())
    {
        CGroupListItem *grp=static_cast<CGroupListItem *>(idx.internalPointer());

        if(grp && grp->isStandard() && grp->name()!=name)
        {
            if(itsFontGroups->items().end()!=itsFontGroups->find(name))
                KMessageBox::error(itsParent, i18n("<qt>A group named <b>\'%1\'</b> already exists!</qt>", name));
            else
                itsFontGroups->setName(grp->item(), name);
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
                                          i18n("Remove Group"), KGuiItem(i18n("Remove"), "remove",
                                          i18n("Remove group"))))
        {
            itsFontGroups->remove(grp->item());
            itsGroups.remove(grp);
            delete grp;
            itsFontGroups->save();
            sort(0, itsSortOrder);
            return true;
        }
    }

    return false;
}

void CGroupList::removeFromGroup(const QModelIndex &group, const QList<Misc::TFont> &fonts)
{
    if(group.isValid())
    {
        CGroupListItem *grp=static_cast<CGroupListItem *>(group.internalPointer());

        if(grp && grp->isStandard())
        {
            QList<Misc::TFont>::ConstIterator it(fonts.begin()),
                                              end(fonts.end());

            for(; it!=end; ++it)
                itsFontGroups->removeFrom(grp->item(), *it);
            emit refresh();
        }
    }
}

void CGroupList::addToGroup(const QModelIndex &group, const QList<Misc::TFont> &fonts)
{
    if(group.isValid())
    {
        CGroupListItem *grp=static_cast<CGroupListItem *>(group.internalPointer());


        if(grp && grp->isStandard())
        {
            QStringList                       names(createFontNames(fonts)),
                                              compact(CFontList::compact(names));
            QList<Misc::TFont>::ConstIterator it(fonts.begin()),
                                              end(fonts.end());

            for(; it!=end; ++it)
                itsFontGroups->addTo(grp->item(), *it);
            emit refresh();
        }
    }
}

void CGroupList::merge(const CFontGroups &grp)
{
    itsFontGroups->merge(grp);
    rescan();
}

static bool groupLessThan(const CGroupListItem *f1, const CGroupListItem *f2)
{
    return f1 && f2 && (f1->type()<f2->type() ||
                       (f1->type()==f2->type() && QString::localeAwareCompare(f1->name(), f2->name())<0));
}

static bool groupGreaterThan(const CGroupListItem *f1, const CGroupListItem *f2)
{
    return f1 && f2 && (f1->type()<f2->type() ||
                       (f1->type()==f2->type() && QString::localeAwareCompare(f1->name(), f2->name())>0));
}

void CGroupList::readGroupsFile()
{
    if(!itsFontGroups)
    {
        //itsFontGroups=new CFontGroups(KStandardDirs::locateLocal("data",
        //                              Misc::root() || itsSysMode ? KFI_NAME"/systemgroups.xml"
        //                                                         : KFI_NAME"/personalgroups.xml"),
        //                              true, false);
        itsFontGroups=Misc::root() || !itsSysMode
                        ? new CFontGroups
                        : new CFontGroups(QString(), false, false, "systemgroups");
    }
    else
        itsFontGroups->refresh();

    CFontInfo::TGroupList::Iterator it(itsFontGroups->items().begin()),
                                    end(itsFontGroups->items().end());

    for(; it!=end; ++it)
        itsGroups.append(new CGroupListItem(it));

    sort(0, itsSortOrder);
}

void CGroupList::sort(int, Qt::SortOrder order)
{
    itsSortOrder=order;

    qSort(itsGroups.begin(), itsGroups.end(),
          Qt::AscendingOrder==order ? groupLessThan : groupGreaterThan);

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

CGroupListView::CGroupListView(QWidget *parent, CGroupList *model)
              : QTreeView(parent)
{
    setModel(model);
    sortByColumn(0, Qt::AscendingOrder);
    resizeColumnToContents(0);
    setSortingEnabled(true);
    setAllColumnsShowFocus(true);
    setAlternatingRowColors(true);
    setAcceptDrops(true);
    setDragDropMode(QAbstractItemView::DropOnly);
    setDropIndicatorShown(true);
    setDragEnabled(false);
    //header()->setClickable(false);
    header()->setSortIndicatorShown(true);
    setRootIsDecorated(false);
    itsMenu=new QMenu(this);

    itsDeleteAct=itsMenu->addAction(KIcon("remove"), i18n("Remove..."),
                                    this, SIGNAL(del()));
    itsEnableAct=itsMenu->addAction(KIcon("enablefont"), i18n("Enable..."),
                                    this, SIGNAL(enable()));
    itsDisableAct=itsMenu->addAction(KIcon("disablefont"), i18n("Disable..."),
                                     this, SIGNAL(disable()));
    itsMenu->addSeparator();
    itsRenameAct=itsMenu->addAction(i18n("Rename..."), this, SLOT(rename()));
    itsMenu->addSeparator();
    itsExportAct=itsMenu->addAction(i18n("Export..."), this, SIGNAL(exportGroup()));
    itsPrintAct=itsMenu->addAction(KIcon("fileprint"), i18n("Print..."),
                                   this, SIGNAL(print()));

    setWhatsThis(i18n("<p>This list shows font groups.</p>"));
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

void CGroupListView::controlMenu(bool del, bool en, bool dis, bool p, bool ex)
{
    itsDeleteAct->setEnabled(del);
    itsRenameAct->setEnabled(del);
    itsEnableAct->setEnabled(en);
    itsDisableAct->setEnabled(dis);
    itsPrintAct->setEnabled(p);
    itsExportAct->setEnabled(ex);
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

        QList<Misc::TFont> fonts;
        QByteArray         encodedData(event->mimeData()->data(KFI_FONT_DRAG_MIME));
        QDataStream        ds(&encodedData, QIODevice::ReadOnly);
        QModelIndex        from(selectedIndexes().last()),
                           to(indexAt(event->pos()));

        ds >> fonts;

        // Are we removing a font from the current group?
        if(to.isValid() && from.isValid() &&
           (static_cast<CGroupListItem *>(from.internalPointer()))->isStandard() &&
           !(static_cast<CGroupListItem *>(to.internalPointer()))->isStandard())
            emit removeFonts(from, fonts);
        else
            emit addFonts(to, fonts);

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
