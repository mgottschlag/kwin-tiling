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

#include "DuplicatesDialog.h"
#include "Misc.h"
#include "Fc.h"
#include "JobRunner.h"
#include "FontList.h"
#include <kapplication.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kdesu/su.h>
#include <kfileitem.h>
#include <kpropertiesdialog.h>
#include <kimageeffect.h>
#include <QLabel>
#include <QTimer>
#include <QGridLayout>
#include <QDir>
#include <QFileInfoList>
#include <QFileInfo>
#include <QHeaderView>
#include <QDateTime>
#include <QMenu>
#include <QContextMenuEvent>
#include <QAction>
#include <QApplication>
#include <QDesktopWidget>

namespace KFI
{

enum EDialogColumns
{
    COL_FILE,
    COL_TRASH,
    COL_SIZE,
    COL_DATE,
    COL_LINK
};

CDuplicatesDialog::CDuplicatesDialog(QWidget *parent, CJobRunner *jr, CFontList *fl)
                 : CActionDialog(parent),
                   itsModifiedSys(false),
                   itsModifiedUser(false),
                   itsRunner(jr),
                   itsFontList(fl)
{
    setCaption(i18n("Duplicate Fonts"));
    setButtons(KDialog::Ok|KDialog::Cancel);
    enableButtonOk(false);

    QFrame *page = new QFrame(this);
    setMainWidget(page);

    QGridLayout *layout=new QGridLayout(page);
    layout->setMargin(0);
    layout->setSpacing(KDialog::spacingHint());
    itsLabel=new QLabel(page);
    itsView=new CFontFileListView(page);
    itsView->hide();
    layout->addWidget(itsPixmapLabel, 0, 0);
    layout->addWidget(itsLabel, 0, 1);
    itsLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    layout->addWidget(itsView, 1, 0, 1, 2);
    itsFontFileList=new CFontFileList(this);
    connect(itsFontFileList, SIGNAL(finished()), SLOT(scanFinished()));
    connect(itsView, SIGNAL(haveDeletions(bool)), SLOT(enableButtonOk(bool)));
}

int CDuplicatesDialog::exec()
{
    itsModifiedSys=itsModifiedUser=false;
    itsLabel->setText(i18n("Scanning for duplicate fonts. Please wait..."));
    itsFontFileList->start();
    return CActionDialog::exec();
}

void CDuplicatesDialog::scanFinished()
{
    stopAnimation();

    if(itsFontFileList->wasTerminated())
        reject();
    else
    {
        CFontFileList::TFontMap duplicates;

        itsFontFileList->getDuplicateFonts(duplicates);

        if(0==duplicates.count())
            itsLabel->setText(i18n("No duplicate fonts found."));
        else
        {
            QSize sizeB4(size());

            itsLabel->setText(i18n("%1 duplicate fonts found.", duplicates.count()));
            itsView->show();

            CFontFileList::TFontMap::ConstIterator it(duplicates.begin()),
                                                   end(duplicates.end());
            QFont                                  boldFont(font());

            boldFont.setBold(true);

            for(; it!=end; ++it)
            {
                QStringList details;

                details << FC::createName(it.key().family, it.key().styleInfo);

                QTreeWidgetItem *top=new QTreeWidgetItem(itsView, details);

                QStringList::ConstIterator fit((*it).begin()),
                                           fend((*it).end());
                int                        tt(0),
                                           t1(0);

                for(; fit!=fend; ++fit)
                {
                    QFileInfo info(*fit);
                    details.clear();
                    details.append(*fit);
                    details.append("");
                    details.append(KGlobal::locale()->formatByteSize(info.size()));
                    details.append(KGlobal::locale()->formatDateTime(info.created()));
                    if(info.isSymLink())
                        details.append(info.readLink());
                    new QTreeWidgetItem(top, details);
                    if(Misc::checkExt(*fit, "pfa") || Misc::checkExt(*fit, "pfb"))
                        t1++;
                    else
                        tt++;
                }
                top->setExpanded(true);
                top->setData(COL_FILE, Qt::DecorationRole,
                             QVariant(SmallIcon(t1>tt ? "font-type1" : "font-truetype")));
                top->setFont(COL_FILE, boldFont);
            }
            itsView->setSortingEnabled(true);
            itsView->header()->resizeSections(QHeaderView::ResizeToContents);

            int width=(KDialog::marginHint()+itsView->frameWidth()+8)*2;

            for(int i=0; i<itsView->header()->count(); ++i)
                width+=itsView->header()->sectionSize(i);

            width=qMin(QApplication::desktop()->screenGeometry(this).width(), width);
            resize(width, height());
            QSize sizeNow(size());
            if(sizeNow.width()>sizeB4.width())
            {
                int xmod=(sizeNow.width()-sizeB4.width())/2,
                    ymod=(sizeNow.height()-sizeB4.height())/2;

                move(pos().x()-xmod, pos().y()-ymod);
            }
        }
    }
}

enum EStatus
{
    STATUS_NO_FILES,
    STATUS_ALL_REMOVED,
    STATUS_ERROR,
    STATUS_USER_CANCELLED
};

void CDuplicatesDialog::slotButtonClicked(int button)
{
    switch(button)
    {
        case KDialog::Ok:
        {
            switch(deleteFiles())
            {
                case STATUS_NO_FILES:
                case STATUS_ALL_REMOVED:
                    accept();
                    break;
                case STATUS_ERROR:
                {
                    QList<QString> files=itsView->getMarkedFiles().toList();

                    if(1==files.count())
                        KMessageBox::error(this, i18n("Could not delete:\n%1", files.first()));
                    else
                        KMessageBox::errorList(this, i18n("Could not delete the following files:"), files);
                    break;
                }
                default:
                case STATUS_USER_CANCELLED:
                    break;
            }
            break;
        }
        case KDialog::Cancel:
            if(!itsFontFileList->wasTerminated())
            {
                if(itsFontFileList->isRunning())
                {
                    if(KMessageBox::Yes==KMessageBox::warningYesNo(this, i18n("Abort font scan?")))
                    {
                        itsLabel->setText("Aborting...");

                        if(itsFontFileList->isRunning())
                            itsFontFileList->terminate();
                        else
                            reject();
                    }
                }
                else
                    reject();
            }
            break;
        default:
            break;
    }
}

int CDuplicatesDialog::deleteFiles()
{
    QSet<QString> files(itsView->getMarkedFiles());

    if(!files.count())
        return STATUS_NO_FILES;

    if(1==files.count()
       ? KMessageBox::Yes==KMessageBox::warningYesNo(this,
                           i18n("Are you sure you wish to delete:\n%1", files.toList().first()))
       : KMessageBox::Yes==KMessageBox::warningYesNoList(this,
                           i18n("Are you sure you wish to delete:"), files.toList()))
    {
        QSet<QString> removed;

        if(!Misc::root())
        {
            QSet<QString>                sys,
                                         user;
            QSet<QString>::ConstIterator it(files.begin()),
                                         end(files.end());
            QString                      home(Misc::dirSyntax(QDir::homePath()));

            for(; it!=end; ++it)
                if(0==(*it).indexOf(home))
                    user.insert(*it);
                else
                    sys.insert(*it);

            if(user.count())
                removed=deleteFiles(user);
            if(sys.count() && itsRunner->getAdminPasswd(this))
            {
                static const int constSysDelFiles=16; // Number of files to rm -f in one go...

                QSet<QString>::ConstIterator it(files.begin()),
                                             end(files.end());
                QStringList                  delFiles;

                for(; it!=end; ++it)
                {
                    delFiles.append(*it);

                    if(constSysDelFiles==delFiles.size())
                    {
                        removed+=deleteSysFiles(delFiles);
                        delFiles.clear();
                    }
                }

                if(delFiles.count())
                    removed+=deleteSysFiles(delFiles);
            }
        }
        else
            removed=deleteFiles(files);

        itsView->removeFiles(removed);
        return 0==itsView->getMarkedFiles().count() ? STATUS_ALL_REMOVED : STATUS_ERROR;
    }
    return STATUS_USER_CANCELLED;
}

QSet<QString> CDuplicatesDialog::deleteFiles(const QSet<QString> &files)
{
    QSet<QString>                removed;
    QSet<QString>::ConstIterator it(files.begin()),
                                 end(files.end());

    for(; it!=end; ++it)
        if(0==::unlink(QFile::encodeName(*it).data()) || !Misc::fExists(*it))
            removed.insert(*it);

    if(removed.count())
        itsModifiedUser=true;

    return removed;
}

QSet<QString> CDuplicatesDialog::deleteSysFiles(const QStringList &files)
{
    QSet<QString> removed;

    if(files.count())
    {
        QByteArray cmd("rm -f");
        QStringList::ConstIterator it(files.begin()),
                                   end(files.end());

        for(; it!=end; ++it)
        {
            cmd+=' ';
            cmd+=QFile::encodeName(KProcess::quote(*it));
        }

        SuProcess proc(KFI_SYS_USER);

        proc.setCommand(cmd);
        proc.exec(itsRunner->adminPasswd().local8Bit());

        for(it=files.begin(); it!=end; ++it)
            if(!Misc::fExists(*it))
                removed.insert(*it);
    }

    if(removed.count())
        itsModifiedSys=true;
    return removed;
}

static uint qHash(const CFontFileList::TFile &key)
{
    return qHash(key.name.toLower());
}

CFontFileList::CFontFileList(CDuplicatesDialog *parent)
             : QThread(parent),
               itsTerminated(false)
{
}

void CFontFileList::start()
{
    if(!isRunning())
    {
        itsTerminated=false;
        QThread::start();
    }
}

void CFontFileList::terminate()
{
    itsTerminated=true;
}

void CFontFileList::getDuplicateFonts(TFontMap &map)
{
    map=itsMap;

    if(map.count())
    {
        TFontMap::Iterator it(map.begin()),
                           end(map.end());

        // Now re-iterate, and remove any entries that only have 1 file...
        for(it=map.begin(); it!=end; )
            if((*it).count()<2)
                it=map.erase(it);
            else
                ++it;
    }
}

void CFontFileList::run()
{
    const QList<CFamilyItem *>          &families(((CDuplicatesDialog *)parent())->fontList()->families());
    QList<CFamilyItem *>::ConstIterator it(families.begin()),
                                        end(families.end());

    for(; it!=end; ++it)
    {
        QList<CFontItem *>::ConstIterator fontIt((*it)->fonts().begin()),
                                          fontEnd((*it)->fonts().end());

        for(; fontIt!=fontEnd; ++fontIt)
            if(!(*fontIt)->isBitmap())
            {
                Misc::TFont                font((*fontIt)->family(), (*fontIt)->styleInfo());
                QStringList::ConstIterator fileIt((*fontIt)->files().begin()),
                                           fileEnd((*fontIt)->files().end());

                for(; fileIt!=fileEnd; ++fileIt)
                    if(!Misc::isMetrics(*fileIt))
                        itsMap[font].append(*fileIt);
            }
    }

    // if we have 2 fonts: /wibble/a.ttf and /wibble/a.TTF fontconfig only returns the 1st, so we
    // now iterate over fontconfig's list, and look for other matching fonts...
    if(itsMap.count() && !itsTerminated)
    {
        // Create a map of folder -> set<files>
        TFontMap::Iterator           it(itsMap.begin()),
                                     end(itsMap.end());
        QHash<QString, QSet<TFile> > folderMap;

        for(int n=0; it!=end && !itsTerminated; ++it)
        {
            QStringList           add;
            QStringList::Iterator fIt((*it).begin()),
                                  fEnd((*it).end());

            for(; fIt!=fEnd && !itsTerminated; ++fIt, ++n)
                folderMap[Misc::getDir(*fIt)].insert(TFile(Misc::getFile(*fIt), it));
        }

        // Go through our folder map, and check for file duplicates...
        QHash<QString, QSet<TFile> >::Iterator folderIt(folderMap.begin()),
                                               folderEnd(folderMap.end());

        for(; folderIt!=folderEnd && !itsTerminated; ++folderIt)
            fileDuplicates(folderIt.key(), *folderIt);
    }

    emit finished();
}

void CFontFileList::fileDuplicates(const QString &folder, const QSet<TFile> &files)
{
    QDir                       dir(folder);
    QStringList                nameFilters;
    QSet<TFile>::ConstIterator it(files.begin()),
                               end(files.end());

    // Filter the QDir to look for filenames matching (caselessly) to those in
    // files...
    for(; it!=end; ++it)
        nameFilters.append((*it).name);

    dir.setFilter(QDir::Files|QDir::Hidden);
    dir.setNameFilters(nameFilters);

    QFileInfoList list(dir.entryInfoList());

    for (int i = 0; i < list.size() && !itsTerminated; ++i)
    {
        QFileInfo fileInfo(list.at(i));

        // Check if this file is already know about - this will do a case-sensitive comparison
        if(!files.contains(TFile(fileInfo.fileName())))
        {
            // OK, not found - this means its a duplicate, but different case. So, find the
            // FontMap iterator, and update its list of files.
            QSet<TFile>::ConstIterator entry=files.find(TFile(fileInfo.fileName(), true));

            if(entry!=files.end())
                (*((*entry).it)).append(fileInfo.absoluteFilePath());
        }
    }
}

inline void markItem(QTreeWidgetItem *item)
{
    item->setData(COL_TRASH, Qt::DecorationRole, QVariant(SmallIcon("list-remove")));
}

inline void unmarkItem(QTreeWidgetItem *item)
{
    item->setData(COL_TRASH, Qt::DecorationRole, QVariant());
}

inline bool isMarked(QTreeWidgetItem *item)
{
    return item->data(COL_TRASH, Qt::DecorationRole).isValid();
}

CFontFileListView::CFontFileListView(QWidget *parent)
                 : QTreeWidget(parent)
{
    QStringList headers;
    headers.append(i18n("Font/File"));
    headers.append("");
    headers.append(i18n("Size"));
    headers.append(i18n("Date"));
    headers.append(i18n("Links To"));
    setHeaderLabels(headers);
    headerItem()->setData(COL_TRASH, Qt::DecorationRole, QVariant(SmallIcon("edit-trash")));
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setSelectionMode(ExtendedSelection);
    sortByColumn(COL_FILE, Qt::AscendingOrder);
    setSelectionBehavior(SelectRows);
    setSortingEnabled(true);
    setAllColumnsShowFocus(true);
    setAlternatingRowColors(true);

    itsMenu=new QMenu(this);
    itsMenu->addAction(KIcon("kfontview"), i18n("Open in Font Viewer..."),
                       this, SLOT(openViewer()));
    itsMenu->addAction(KIcon("document-properties"), i18n("Properties..."),
                       this, SLOT(properties()));
    itsMenu->addSeparator();
    itsUnMarkAct=itsMenu->addAction(i18n("Unmark for deletion..."),
                                    this, SLOT(unmark()));
    itsMarkAct=itsMenu->addAction(KIcon("edit-delete"), i18n("Mark for deletion..."),
                                  this, SLOT(mark()));

    connect(this, SIGNAL(itemSelectionChanged()), SLOT(selectionChanged()));
    connect(this, SIGNAL(itemClicked(QTreeWidgetItem *, int)), SLOT(clicked(QTreeWidgetItem *, int)));
}

QSet<QString> CFontFileListView::getMarkedFiles()
{
    QTreeWidgetItem *root=invisibleRootItem();
    QSet<QString>   files;

    for(int t=0; t<root->childCount(); ++t)
    {
        QList<QTreeWidgetItem *> removeFiles;
        QTreeWidgetItem          *font=root->child(t);

        for(int c=0; c<font->childCount(); ++c)
        {
            QTreeWidgetItem *file=font->child(c);

            if(isMarked(file))
                files.insert(file->text(0));
        }
    }

    return files;
}

void CFontFileListView::removeFiles(const QSet<QString> &files)
{
    QTreeWidgetItem          *root=invisibleRootItem();
    QList<QTreeWidgetItem *> removeFonts;

    for(int t=0; t<root->childCount(); ++t)
    {
        QList<QTreeWidgetItem *> removeFiles;
        QTreeWidgetItem          *font=root->child(t);

        for(int c=0; c<font->childCount(); ++c)
        {
            QTreeWidgetItem *file=font->child(c);

            if(files.contains(file->text(0)))
                removeFiles.append(file);
        }

        QList<QTreeWidgetItem *>::ConstIterator it(removeFiles.begin()),
                                                end(removeFiles.end());

        for(; it!=end; ++it)
            delete (*it);
        if(0==font->childCount())
            removeFonts.append(font);
    }

    QList<QTreeWidgetItem *>::ConstIterator it(removeFonts.begin()),
                                            end(removeFonts.end());
    for(; it!=end; ++it)
        delete (*it);
}

void CFontFileListView::openViewer()
{
    // Number of fonts user has selected, before we ask if they really want to view them all...
    static const int constMaxBeforePrompt=10;

    QList<QTreeWidgetItem *> items(selectedItems());
    QTreeWidgetItem          *item;
    QSet<QString>            files;

    foreach(item, items)
        if(item->parent()) // Then its a file, not font name :-)
            files.insert(item->text(0));

    if(files.count() &&
       (files.count()<constMaxBeforePrompt ||
        KMessageBox::Yes==KMessageBox::questionYesNo(this, i18n("Open all %1 fonts in font viewer?", files.count()))))
    {
         QSet<QString>::ConstIterator it(files.begin()),
                                      end(files.end());

        for(; it!=end; ++it)
        {
            KProcess proc;

            proc << KFI_APP << "-v" << (*it).toUtf8();
            proc.start(KProcess::DontCare);
        }
    }
}

void CFontFileListView::properties()
{
    QList<QTreeWidgetItem *> items(selectedItems());
    QTreeWidgetItem          *item;
    KFileItemList            files;

    foreach(item, items)
        if(item->parent())
            files.append(new KFileItem(KUrl::fromPath(item->text(0)),
                                       KMimeType::findByPath(item->text(0))->name(),
                                       item->text(COL_LINK).isEmpty() ? S_IFREG : S_IFLNK));

    if(files.count())
    {
        KPropertiesDialog dlg(files, this);
        dlg.exec();

        KFileItemList::ConstIterator it(files.begin()),
                                     end(files.end());

        for(; it!=end; ++it)
            delete (*it);
    }
}

void CFontFileListView::mark()
{
    QList<QTreeWidgetItem *> items(selectedItems());
    QTreeWidgetItem          *item;

    foreach(item, items)
        if(item->parent())
            markItem(item);
    checkFiles();
}

void CFontFileListView::unmark()
{
    QList<QTreeWidgetItem *> items(selectedItems());
    QTreeWidgetItem          *item;

    foreach(item, items)
        if(item->parent())
            unmarkItem(item);
    checkFiles();
}

void CFontFileListView::selectionChanged()
{
    QList<QTreeWidgetItem *> items(selectedItems());
    QTreeWidgetItem          *item;

    foreach(item, items)
        if(!item->parent() && item->isSelected())
            item->setSelected(false);
}

void CFontFileListView::clicked(QTreeWidgetItem *item, int col)
{
    if(item && COL_TRASH==col && item->parent())
    {
        if(isMarked(item))
            unmarkItem(item);
        else
            markItem(item);
        checkFiles();
    }
}

void CFontFileListView::contextMenuEvent(QContextMenuEvent *ev)
{
    QTreeWidgetItem *item(itemAt(ev->pos()));

    if(item && item->parent())
    {
        if(!item->isSelected())
            item->setSelected(true);

        bool haveUnmarked(false),
             haveMaked(false);

        QList<QTreeWidgetItem *> items(selectedItems());
        QTreeWidgetItem          *item;

        foreach(item, items)
        {
            if(item->parent() && item->isSelected())
                if(isMarked(item))
                    haveMaked=true;
                else
                    haveUnmarked=true;

            if(haveUnmarked && haveMaked)
                break;
        }

        itsMarkAct->setEnabled(haveUnmarked);
        itsUnMarkAct->setEnabled(haveMaked);
        itsMenu->popup(ev->globalPos());
    }
}

void CFontFileListView::checkFiles()
{
    // Need to check that if we mark a file that is linked to, then we also need
    // to mark the sym link.
    QSet<QString> marked(getMarkedFiles());

    if(marked.count())
    {
        QTreeWidgetItem *root=invisibleRootItem();

        for(int t=0; t<root->childCount(); ++t)
        {
            QTreeWidgetItem *font=root->child(t);

            for(int c=0; c<font->childCount(); ++c)
            {
                QTreeWidgetItem *file=font->child(c);
                QString         link(font->child(c)->text(COL_LINK));

                if(!link.isEmpty() && marked.contains(link))
                    if(!isMarked(file))
                        markItem(file);
            }
        }

        emit haveDeletions(true);
    }
    else
        emit haveDeletions(false);
}

}

#include "DuplicatesDialog.moc"
