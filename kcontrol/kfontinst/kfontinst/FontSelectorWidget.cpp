////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CFontSelectorWidget
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 18/06/2002
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
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2002
////////////////////////////////////////////////////////////////////////////////

#include "FontSelectorWidget.h"
#include "FontEngine.h"
#include "KfiGlobal.h"
#include "Misc.h"
#include "UiConfig.h"
#include <qdir.h>
#include <qpopupmenu.h>
#include <qfile.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <stdlib.h>

CFontSelectorWidget::CListViewItem::CListViewItem(CFontSelectorWidget *listWidget, const QString &name,
                                                  const QString &icon, const QString &base)
                   : QListViewItem(listWidget),
                     itsBase(base),
                     itsListWidget(listWidget)
{
    setText(0, name);
    initIcon(icon);
    setOpen(0==CKfiGlobal::uicfg().getOpenFsDirs().count() ||
            -1!=CKfiGlobal::uicfg().getOpenFsDirs().findIndex(CMisc::dirSyntax(fullName())) ? true : false);
}

CFontSelectorWidget::CListViewItem::CListViewItem(CFontSelectorWidget *listWidget, QListViewItem *parent, const QString &name)
		                  : QListViewItem(parent),
                                    itsBase(QString::null),
                                    itsListWidget(listWidget)
{
    setText(0, name);
    if(CFontEngine::isAFont(QFile::encodeName(name)))
    {
        if(CKfiGlobal::fe().openFont(fullName(), CFontEngine::NAME))
        {
            setText(1, CKfiGlobal::fe().getFullName());
            CKfiGlobal::fe().closeFont();
        }
        else
            setText(1, i18n("ERROR: Could not open font"));

        switch(CFontEngine::getType(QFile::encodeName(name)))
        {
            case CFontEngine::TRUE_TYPE:
                initIcon("font_truetype");
                break;
            case CFontEngine::TYPE_1:
                initIcon("font_type1");
                break;
            case CFontEngine::SPEEDO:
                initIcon("font_speedo");
                break;
            default:
            case CFontEngine::BITMAP:
                initIcon("font_bitmap");
                break;
        }
    }
    else
        initIcon(QDir(CMisc::dirSyntax(fullName())).isReadable() ? "folder" : "folder_locked");
}

void CFontSelectorWidget::CListViewItem::initIcon(const QString &icn)
{
    setPixmap(0, KGlobal::iconLoader()->loadIcon(icn, KIcon::Small));
}

QString CFontSelectorWidget::CListViewItem::fullName() const
{
    QString name;

    if(QString::null==itsBase)
    {
        name=((CListViewItem *)parent())->fullName();
        if("/"!=name)
            name.append("/");
        name.append(text(0));
        if(!CFontEngine::isAFont(QFile::encodeName(text(0))))
            name.append("/");
    }
    else
        name=itsBase;

    return name;
}

void CFontSelectorWidget::CListViewItem::open()
{
    if(!CFontEngine::isAFont(QFile::encodeName(text(0))) && QDir(fullName()).isReadable()
       && -1!=CKfiGlobal::uicfg().getOpenFsDirs().findIndex(CMisc::dirSyntax(fullName())))
        setOpen(true);
}

QString CFontSelectorWidget::CListViewItem::key(int column, bool ascending) const
{
    QString k;

    if(ascending)
        k=(CFontEngine::isAFont(QFile::encodeName(text(0))) ? "2" : "1");
    else
        k=(CFontEngine::isAFont(QFile::encodeName(text(0))) ? "1" : "2");

    k+=text(column);
    return k;
}

void CFontSelectorWidget::CListViewItem::setup()
{
    setExpandable(!CFontEngine::isAFont(QFile::encodeName(text(0))) && QDir(fullName()).isReadable() ? true : false);
    QListViewItem::setup();
}

void CFontSelectorWidget::CListViewItem::setOpen(bool open)
{
    bool    readable=false;
    QString dName(CMisc::dirSyntax(fullName()));


    if(QString::null==itsBase) // Then it's not a top level folder
        setPixmap(0, KGlobal::iconLoader()->loadIcon(open ? "folder_open" : "folder", KIcon::Small));

    if(open)
    {
        QDir dir(dName);

        if(dir.isReadable())
        {
            CKfiGlobal::uicfg().addOpenFsDir(dName);

            readable = true;
            const QFileInfoList *files=dir.entryInfoList();

            if(files)
            {
                QFileInfoListIterator   it(*files);
                QFileInfo               *fInfo;
                CListViewItem           *ndItem;
                QPtrList<CListViewItem> newDirs;

                itsListWidget->progressInit(i18n("Scanning folder %1:").arg(dName), files->count());
                for(; NULL!=(fInfo=it.current()); ++it)
                {
                    if("."!=fInfo->fileName() && ".."!=fInfo->fileName())
                    {
                        itsListWidget->progressShow(fInfo->fileName());
                        if(fInfo->isDir())
                            newDirs.append(new CListViewItem(itsListWidget, this, fInfo->fileName()));
                        else
                            if(CFontEngine::isAFont(QFile::encodeName(fInfo->fileName())))
                                (void) new CListViewItem(itsListWidget, this, fInfo->fileName());
                    }
                }
                itsListWidget->progressStop();

                for(ndItem=newDirs.first(); ndItem; ndItem=newDirs.next())
                    ndItem->open();
            }
        }
    }
    else // Deleteing the items allows directories to be rescanned - although this may be slow if it has lots of fonts...
    {
        readable = true;
        QListViewItem *item=firstChild();

        CKfiGlobal::uicfg().removeOpenFsDir(dName);

        while(NULL!=item)
        {
            QListViewItem *next=item->nextSibling();
            delete item;
            item=next;
        }
    }

    if(readable)
        QListViewItem::setOpen(open);
    else
        setExpandable(false);
}

CFontSelectorWidget::CFontSelectorWidget(QWidget *parent)
                   : KListView(parent),
                     itsShowingProgress(false),
                     itsSetup(false)
{
    addColumn(i18n("Folder/File"));
    addColumn(i18n("Name"));
    setMinimumSize(QSize(0, 24));
    setAllColumnsShowFocus(TRUE);
    setShowSortIndicator(TRUE);
    setTreeStepSize(12);
    setFullWidth(true);
    setSelectionMode(QListView::Extended);

    // signals and slots connections
    connect(this, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
    connect(this, SIGNAL(currentChanged(QListViewItem *)), this, SLOT(selectionChanged()));
    connect(this, SIGNAL(rightButtonPressed(QListViewItem *, const QPoint &, int)), SLOT(popupMenu(QListViewItem *, const QPoint &, int)));

    itsPopup=new QPopupMenu(this);
    itsPopup->insertItem(i18n("Show Meta Data..."), this, SLOT(showMeta()));
    itsPopup->insertItem(i18n("Install"), this, SLOT(install()));
    setRootIsDecorated(true);
}

void CFontSelectorWidget::storeSettings()
{
    QListViewItem *item=itemAt(QPoint(0, 0));

    if(item)
        CKfiGlobal::uicfg().setFsTopItem(((CListViewItem*)item)->fullName());
}

void CFontSelectorWidget::selectionChanged()
{
    QListViewItem *cur=currentItem();

    if(cur && cur->isSelected())
    {
        QListViewItem *item=firstChild();

	if(!CFontEngine::isAFont(QFile::encodeName(cur->text(0))))
	{
            // De-select everything else...
            while(NULL!=item)
            {
                if(item->isSelected() && item!=cur)
                {
                    item->setSelected(false);
                    item->repaint();
                }

                item=(CListViewItem *)(item->itemBelow());
            }
	}
	else
        {
            // Only allows fonts in the same dir to be selected...
            while(NULL!=item)
            {
                if(item->isSelected() && item!=cur && 
                  (!CFontEngine::isAFont(QFile::encodeName(item->text(0))) || item->parent()!=cur->parent()))
                {
                    item->setSelected(false);
                    item->repaint();
                }
                item=(CListViewItem *)(item->itemBelow());
            }
	    if(!itsShowingProgress)
                emit fontSelected(((CListViewItem *)cur)->fullName());
        }
    }
}

void CFontSelectorWidget::progressInit(const QString &title, int numSteps)
{
    static const int constMaxSteps=25;

    if(0==numSteps || numSteps>constMaxSteps)
    {
        itsShowingProgress=true;
        emit initProgress(title, numSteps);
    }
}

void CFontSelectorWidget::progressShow(const QString &step)
{
    if(itsShowingProgress)
        emit progress(step);
}

void CFontSelectorWidget::progressStop()
{
    if(itsShowingProgress)
    {
        emit stopProgress();
        itsShowingProgress=false;
    }
}

void CFontSelectorWidget::showContents()
{
    if(!itsSetup)
    {
        char *home=getenv("HOME");

        if(home)
            (void) new CListViewItem(this, i18n("Home"), "folder_home", home);

        (void) new CListViewItem(this, i18n("Root"), "folder", "/");

        QListViewItem *item=firstChild();

        while(NULL!=item)
        {
            if(((CListViewItem *)item)->fullName()==CKfiGlobal::uicfg().getFsTopItem())
            {
                ensureItemVisible(item);
                break;
            }
            item=item->itemBelow();
        }

        itsSetup=true;
    }
}

KURL::List CFontSelectorWidget::getSelectedFonts()
{
    KURL::List list;

    QListViewItem *item=firstChild();

    while(item)
    {
        if(item->isSelected() && CFontEngine::isAFont(QFile::encodeName(item->text(0))))
        {
            QString file(((CListViewItem *)item)->fullName());
            KURL    url;
    
            url.setPath(CMisc::getDir(file));
            url.setFileName(CMisc::getFile(file));
            list.append(url);
        }
        item=item->itemBelow();
    }

    return list;
}

void CFontSelectorWidget::popupMenu(QListViewItem *item, const QPoint &point, int)
{
    if(NULL!=item && CFontEngine::isAFont(QFile::encodeName(item->text(0))))
    {
        if(!item->isSelected())
            setSelected(item, true);

        itsPopup->popup(point);
    }
}

void CFontSelectorWidget::install()
{
    emit installSelected();
}

void CFontSelectorWidget::showMeta()
{
    QStringList list;

    CListViewItem *item=(CListViewItem *)firstChild();

    while(NULL!=item)
    {
        if(item->isSelected())
            list.append(item->fullName());
        item=(CListViewItem *)(item->itemBelow());
    }

    emit showMetaData(list);
}

#include "FontSelectorWidget.moc"
