////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CFontListWidget
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 20/04/2001
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
// (C) Craig Drummond, 2001
////////////////////////////////////////////////////////////////////////////////

#include "FontListWidget.h"
#include "FontEngine.h"
#include "Config.h"
#include "XConfig.h"
#include "KfiGlobal.h"
#include "KfiCmModule.h"
#include "ErrorDialog.h"
#include "StarOfficeConfig.h"
#include "Ttf.h"
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klistview.h>
#include <kurl.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <klineeditdlg.h>
#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qbitmap.h>
#include <qpainter.h>
#include <qheader.h>
#include <qpopupmenu.h>
#include <qvalidator.h>
#include <qmime.h>
#include <qdragobject.h>
#include <qcursor.h>

#include <iostream>

static const QString constDisabledSubDir(".disabled");

static bool contains(QListViewItem *first, const QString &file)
{
    QListViewItem *item=first;

    while(item!=NULL)
    {
        if(item->text(0)==file)
            return true;

        item=item->nextSibling();
    }

    return false;
}

static bool contains(QPtrList<CFontListWidget::TItem> &list, const QString &source, const QString &dest, const QString &file)
{
    CFontListWidget::TItem *item;

    for(item=list.first(); item; item=list.next())
        if(item->source==source && item->dest==dest && item->file==file)
            return true;

    return false;
}

static bool remove(QPtrList<CFontListWidget::TItem> &list, const QString &f)
{
    CFontListWidget::TItem *item;

    for(item=list.first(); item; item=list.next())
        if(QString(CMisc::dirSyntax(item->source)+item->file)==f)
        {
            list.remove(item);
            return true;
        }

    return false;
}

static bool isRequiredDir(QListViewItem *item)
{
    CFontListWidget::CListViewItem *citem=(CFontListWidget::CListViewItem *)item;
    QString                        fName=citem->fullName();
    
    return CFontListWidget::CListViewItem::DIR==citem->getType() &&
           (fName==CKfiGlobal::cfg().getFontsDir() ||
            fName==(CKfiGlobal::cfg().getFontsDir()+CKfiGlobal::cfg().getTTSubDir()) ||
            fName==(CKfiGlobal::cfg().getFontsDir()+CKfiGlobal::cfg().getT1SubDir()));
}

static void setDisplay(QListViewItem *item)
{
    while(NULL!=item)
    {
        ((CFontListWidget::CListViewItem *)item)->setupDisplay();
        if(CFontListWidget::CListViewItem::DIR==((CFontListWidget::CListViewItem *)item)->getType())
            setDisplay(item->firstChild());
        item=item->nextSibling();
    }
}

static bool dirInList(const QString &dir, QStringList &list)
{
    //
    // Look for /a/b/c/d/e/ in
    //    /a/b/c/d/e/f/g/, /a/b/c/, /.....
    // ... /a/b/c/ matches
    
    QStringList::Iterator it;

    for(it=list.begin(); it!=list.end(); ++it)
        if(!CFontEngine::isAFont(QFile::encodeName(*it)) && 0==dir.find(CMisc::dirSyntax(*it)))
            return true;

    return false;
}

CFontListWidget::CListViewItem::CListViewItem(QListView *parent, const QString &name, EType type, bool isNew, bool enabled)
                              : QListViewItem(parent, name),
                                itsType(type)
{
    itsAvailable = true;
    itsAvailableOrig = !isNew;
    itsEnabled = enabled;
    itsEnabledOrig = enabled;
}

CFontListWidget::CListViewItem::CListViewItem(QListViewItem *parent, const QString &name, EType type, bool isNew, bool enabled)
                              : QListViewItem(parent, name),
                                itsType(type)
{
    itsAvailable = true;
    itsAvailableOrig = !isNew;
    itsEnabled = enabled;
    itsEnabledOrig = enabled;
}

void CFontListWidget::CListViewItem::reset()
{
    itsAvailable=itsAvailableOrig;
    itsEnabled=itsEnabledOrig;;
    setupDisplay();
}

void CFontListWidget::CListViewItem::saved(bool toggleState)
{
    itsAvailable=true;
    itsAvailableOrig=true;
    if(toggleState)
        itsEnabled=!itsEnabled;
    itsEnabledOrig=itsEnabled;
    setupDisplay();
}

void CFontListWidget::CListViewItem::setAvailable(bool available)
{
    itsAvailable = available;
    //if(available && parent())
    //{
    //    CListViewItem *item = (CListViewItem *) parent();
    //    if(!item->available())
    //        item->setAvailable(true);
    //}
    setupDisplay();

    CFontListWidget *lw=(CFontListWidget *)listView();
    int             idx=lw->getDelItems().findIndex(fullName());

    if(available)
    {
        if(-1!=idx)
            lw->getDelItems().remove(fullName());
    }
    else
    {
        if(-1==idx)
            lw->getDelItems().append(fullName());
    }
}

void CFontListWidget::CListViewItem::setEnabled(bool enabled)
{
    itsEnabled=enabled;
    setupDisplay();
}

class CDirectoryItem : public CFontListWidget::CListViewItem
{
    public:

    CDirectoryItem(CFontListWidget *listWidget, QListView *parent, const QString &dir,
                   const QString &name, const QString &icon, bool isNew)
        : CFontListWidget::CListViewItem(parent, name, CFontListWidget::CListViewItem::DIR, isNew, true),
          itsName(CMisc::dirSyntax(dir)),
          itsParentDir(NULL),
          itsListWidget(listWidget)
    {
        itsEnabled=itsEnabledOrig=CKfiGlobal::xcfg().inPath(fullName());

        if(QString::null!=icon)
            setPixmap(0, KGlobal::iconLoader()->loadIcon(icon, KIcon::Small));

        listView()->setUpdatesEnabled(false);
        setOpen(true);
        setupDisplay();
        listView()->setUpdatesEnabled(true);
    }

    CDirectoryItem(CFontListWidget *listWidget, CDirectoryItem *parent, const QString &name, bool isNew)
        : CFontListWidget::CListViewItem(parent, name, CListViewItem::DIR, isNew, true),
          itsName(CMisc::dirSyntax(name)),
          itsParentDir(parent),
          itsListWidget(listWidget)
    {
        bool readable=isNew || QDir(fullName()).isReadable();

        itsEnabled=itsEnabledOrig=CKfiGlobal::xcfg().inPath(fullName());
        setPixmap(0, KGlobal::iconLoader()->loadIcon(readable ? "folder" : "folder_locked" , KIcon::Small));
        setupDisplay();
    }

    virtual ~CDirectoryItem()
    {
    }

    void open()
    {
        if(QDir(fullName()).isReadable() && -1!=itsListWidget->getAdvancedOpenDirs().findIndex(fullName()))
            setOpen(true);
    }

    void setup()
    {
        setExpandable(QDir(fullName()).isReadable() ? true : false);
        QListViewItem::setup();
    }

    void    setEnabled(bool enabled);
    void    setAvailable(bool available);
    bool    available() const;
    void    setOpen(bool open);
    QString fullName() const;
    void    setupDisplay();
    QString dir() const
    {
        return CMisc::dirSyntax(fullName());
    }

    private:

    QString         itsName;
    CDirectoryItem  *itsParentDir;
    CFontListWidget *itsListWidget;
};

class CFontItem : public CFontListWidget::CListViewItem
{
    public:

    CFontItem(QListView *parent, const QString &fileName, const QString &path, bool isNew, bool enabled)
        : CFontListWidget::CListViewItem(parent, fileName, CFontListWidget::CListViewItem::FONT, isNew, enabled),
          itsFileName(fileName),
          itsPath(CMisc::dirSyntax(path)),
          itsParentDir(0)
    {
        setupDisplay();
    }

    CFontItem(CDirectoryItem *parent, const QString &fileName, const QString &path, bool isNew, bool enabled)
        : CFontListWidget::CListViewItem(parent, fileName, CFontListWidget::CListViewItem::FONT, isNew, enabled),
          itsFileName(fileName),
          itsPath(CMisc::dirSyntax(path)),
          itsParentDir(parent)
    {
        setupDisplay();
    }

    virtual ~CFontItem()
    {
    }

    void setEnabled(bool enabled);
    void setupDisplay();

    CDirectoryItem * getParentDir()
    {
        return itsParentDir;
    }

    QString dir() const
    {
        return CMisc::dirSyntax(itsPath);
    }

    QString fullName() const
    {
        if (!itsAvailableOrig)
           return QString::null!=itsData ? itsData+itsFileName : itsFileName;
        if (!itsEnabledOrig)
           return QString::null!=itsPath ? itsPath+constDisabledSubDir+"/"+itsFileName : constDisabledSubDir+"/"+itsFileName;
        return QString::null!=itsPath ? itsPath+itsFileName : itsFileName;
    }

    private:

    QString        itsFileName;
    QString        itsPath;
    CDirectoryItem *itsParentDir;
};

QString CFontListWidget::CListViewItem::key(int column, bool ascending) const
{
    QString k;

    if(ascending)
        k=(itsType==DIR ? "1" : "2");
    else
        k=(itsType==DIR ? "2" : "1");

    k+=text(column);
    return k;
}

void CFontListWidget::CListViewItem::paintCell(QPainter *painter, const QColorGroup &colourGroup, int column, int width, int align)
{
    if(itsType==DIR && CKfiGlobal::xcfg().ok() && itsEnabled)
    {
        QFont f=painter->font();

        f.setBold(true);

        if(CKfiGlobal::xcfg().isUnscaled(fullName()))
            f.setItalic(true);

        painter->setFont(f);
    }

    QListViewItem::paintCell(painter, colourGroup, column, width, align);
}

void CDirectoryItem::setEnabled(bool enabled)
{
    if(CKfiGlobal::xcfg().inPath(fullName()))
        CKfiGlobal::xcfg().removePath(fullName());
    else
        CKfiGlobal::xcfg().addPath(fullName());
    CListViewItem::setEnabled(enabled);

    QListViewItem *item=firstChild();

    while(NULL!=item)
    {
        if(FONT==((CListViewItem *)item)->getType())
            ((CListViewItem *)item)->setupDisplay();
        item=item->nextSibling();
    }
}

void CDirectoryItem::setAvailable(bool available)
{
    CListViewItem::setAvailable(available);
    setDisplay(firstChild());
}

bool CDirectoryItem::available() const
{
    bool avail=true;

    if(itsAvailable && itsParentDir)
        avail=itsParentDir->available();

    return itsAvailable && avail;
}

void CDirectoryItem::setOpen(bool open)
{
    bool readable=false;

    if(NULL!=itsParentDir) // Then it's not a top level folder
        setPixmap(0, KGlobal::iconLoader()->loadIcon(open ? "folder_open" : "folder", KIcon::Small));

    if(open)
    {
        QDir dir(fullName());

        itsListWidget->getAdvancedOpenDirs().append(fullName());

        if(dir.isReadable())
        {
            readable = true;
            const QFileInfoList *files=dir.entryInfoList();

            if(files)
            {
                QFileInfoListIterator    it(*files);
                QFileInfo                *fInfo;
                CDirectoryItem           *ndItem;
#if QT_VERSION >= 300
                QPtrList<CDirectoryItem> newDirs;
#else
                QList<CDirectoryItem>    newDirs;
#endif

                itsListWidget->progressInit(i18n("Scanning folder %1:").arg(fullName()), files->count());
                for(; NULL!=(fInfo=it.current()); ++it)
                {
                    if("."!=fInfo->fileName() && ".."!=fInfo->fileName())
                    {
                        itsListWidget->progressShow(fInfo->fileName());
                        if(fInfo->isDir())
                        {
                            CDirectoryItem *newDir=new CDirectoryItem(itsListWidget, this, fInfo->fileName(), false);
                            newDirs.append(newDir);
                        }
                        else
                        {
                            if(CFontEngine::isAFont(QFile::encodeName(fInfo->fileName())))
                                new CFontItem(this, fInfo->fileName(), this->dir(), false, true);
                        }
                    }
                }
                itsListWidget->progressStop();

                for(ndItem=newDirs.first(); ndItem; ndItem=newDirs.next())
                    ndItem->open();
            }
        }
        
        dir.setPath(fullName()+"/"+constDisabledSubDir);
        if(dir.isReadable())
        {
            readable = true;
            const QFileInfoList *files=dir.entryInfoList();

            if(files)
            {
                QFileInfoListIterator   it(*files);
                QFileInfo               *fInfo;

                itsListWidget->progressInit(i18n("Scanning folder %1:").arg(fullName()), files->count());
                for(; NULL!=(fInfo=it.current()); ++it)
                {
                    if(!fInfo->isDir())
                    {
                        itsListWidget->progressShow(fInfo->fileName());
                        if(CFontEngine::isAFont(QFile::encodeName(fInfo->fileName())))
                            new CFontItem(this, fInfo->fileName(), this->dir(), false, false);
                    }
                }
                itsListWidget->progressStop();
            }
        }

        itsListWidget->restore(this->firstChild(), false);
    }
    else // Deleteing the items allows directories to be rescanned - although this may be slow if it has lots of fonts...
    {
        readable = true;
        QListViewItem *item=firstChild();

        itsListWidget->getAdvancedOpenDirs().remove(fullName());

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

QString CDirectoryItem::fullName() const
{
    QString name;

    if(itsParentDir)
    {
        name=itsParentDir->fullName();
        name.append(itsName);
    }
    else
        name=itsName;

    return name;
}

void CDirectoryItem::setupDisplay()
{
    if(!available())
        setPixmap(1, KGlobal::iconLoader()->loadIcon("edittrash", KIcon::Small));
    else if(added())
        setPixmap(1, KGlobal::iconLoader()->loadIcon("filenew", KIcon::Small));
    else
        setPixmap(1, QPixmap());

    if(enabled() && CKfiGlobal::xcfg().inPath(fullName()))
        setPixmap(2, KGlobal::iconLoader()->loadIcon("ok", KIcon::Small));
    else
        setPixmap(2, QPixmap());
}

void CFontItem::setupDisplay()
{
    QString         font = fullName();
    CFontListWidget *lw  = (CFontListWidget *)listView();
    bool            parentNotAvailable = lw ? dirInList(dir(), lw->getDelItems()) : false;

    if(!available() || parentNotAvailable) // (itsParentDir && !itsParentDir->available()))
        setPixmap(1, KGlobal::iconLoader()->loadIcon("edittrash", KIcon::Small));
    else if(added())
        setPixmap(1, KGlobal::iconLoader()->loadIcon("filenew", KIcon::Small));
    else
        setPixmap(1, QPixmap());

    if(enabled() && CKfiGlobal::xcfg().inPath(dir()))
        setPixmap(2, KGlobal::iconLoader()->loadIcon("ok", KIcon::Small));
    else
        setPixmap(2, QPixmap());

    switch(CFontEngine::getType(QFile::encodeName(font)))
    {
        case CFontEngine::TRUE_TYPE:
            setPixmap(0, KGlobal::iconLoader()->loadIcon("font_truetype", KIcon::Small));
            break;
        case CFontEngine::TYPE_1:
            setPixmap(0, KGlobal::iconLoader()->loadIcon("font_type1", KIcon::Small));
            break;
        case CFontEngine::SPEEDO:
            setPixmap(0, KGlobal::iconLoader()->loadIcon("font_speedo", KIcon::Small));
            break;
        default:
        case CFontEngine::BITMAP:
            setPixmap(0, KGlobal::iconLoader()->loadIcon("font_bitmap", KIcon::Small));
            break;
    }

    if(CKfiGlobal::fe().openFont(font))
    {
        setText(3, CKfiGlobal::fe().getFullName().stripWhiteSpace());
        CKfiGlobal::fe().closeFont();
    }
    else
        setText(3, i18n("ERROR: Could not open font"));
}

void CFontItem::setEnabled(bool enabled)
{
    CFontListWidget *lw=(CFontListWidget *)listView();
    QString         fName(fullName());

    if(enabled)
    {
        if(!itsEnabledOrig && -1==lw->getEnabledItems().findIndex(fName))
            lw->getEnabledItems().append(fName);
        if(itsEnabledOrig && -1!=lw->getDisabledItems().findIndex(fName))
            lw->getDisabledItems().remove(fName);
    }
    else
    {
        if(!itsEnabledOrig && -1!=lw->getEnabledItems().findIndex(fName))
            lw->getEnabledItems().remove(fName);
        if(itsEnabledOrig && -1==lw->getDisabledItems().findIndex(fName))
            lw->getDisabledItems().append(fName);
    }
    CListViewItem::setEnabled(enabled);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// CPD - disable for now...
//#define ENABLE_DRAG
CFontListWidget::CFontListWidget(QWidget *parent)
               : KListView(parent),
                 itsAdvancedMode(CKfiGlobal::cfg().getAdvancedMode()),
                 itsShowingProgress(false)
{
    addColumn(i18n("Folder/File"));

    addColumn(QString::null);
    header()->setResizeEnabled(FALSE, header()->count()-1);
    setColumnWidth(1, 22);
    setColumnWidthMode(1, QListView::Manual);
    setColumnText(1, KGlobal::iconLoader()->loadIcon("edittrash", KIcon::Small), QString::null);

    addColumn(QString::null);
    header()->setResizeEnabled(FALSE, header()->count()-1);
    setColumnWidth(2, 22);
    setColumnWidthMode(2, QListView::Manual);

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
    connect(this, SIGNAL(clicked(QListViewItem *, const QPoint &, int)), SLOT(listClicked(QListViewItem *, const QPoint &, int)));

    itsFontsPopup=new QPopupMenu(this);
    itsFixTtfPsNamesME=itsFontsPopup->insertItem(i18n("Fix TTF postscript names..."), this, SLOT(fixTtfPsNames()));

    itsDirsPopup=new QPopupMenu(this);
    itsCreateDirME=itsDirsPopup->insertItem(i18n("Create new sub-folder..."), this, SLOT(createDir()));
    itsDirsPopup->insertSeparator();
    itsSetUnscaledME=itsDirsPopup->insertItem(i18n("Set unscaled"), this, SLOT(toggleUnscaled()));
    itsSetScaledME=itsDirsPopup->insertItem(i18n("Set scaled"), this, SLOT(toggleUnscaled()));

    itsAddItems.setAutoDelete(true);
#ifdef ENABLE_DRAG
    setDragEnabled(itsAdvancedMode);
    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);
    setDropHighlighter(true);
#endif
}

void CFontListWidget::reset()
{
    clearLists();

    CListViewItem *citem=(CListViewItem *)firstChild();

    while(NULL!=citem)
    {
        CListViewItem *next=(CListViewItem *)(citem->itemBelow());

        if(citem->added())
            delete citem;
        else
            citem->reset();

        citem=next;
    }
}

void CFontListWidget::clearLists()
{
    itsAdvancedOpenDirs.clear();
    itsAddItems.clear();
    itsDelItems.clear();
    itsDisabledItems.clear();
    itsEnabledItems.clear();
}

void CFontListWidget::restore(QListViewItem *item, bool checkOpen)
{
    CFontListWidget::TItem *addItem;
    for(addItem=itsAddItems.first(); addItem; addItem=itsAddItems.next())
        if(itsAdvancedMode)
        {
            if(QString::null!=addItem->file)
                addFont(addItem->source, addItem->dest, addItem->file, checkOpen);
            else
                addSubDir(addItem->source, addItem->dest);
        }
        else
            if(QString::null!=addItem->file &&
               (CFontEngine::isAType1(QFile::encodeName(addItem->file)) || CFontEngine::isATtf(QFile::encodeName(addItem->file))))
            {
                CFontItem *fI=new CFontItem(this, addItem->file, addItem->dest, true, true);

                fI->setExtraData(addItem->source);
            }

    CListViewItem *citem=(CListViewItem *)(item);

    while(NULL!=citem)
    {
        if(-1!=itsDelItems.findIndex(citem->fullName()))
            citem->setAvailable(false);
        if(-1!=itsEnabledItems.findIndex(citem->fullName()))
            citem->setEnabled(true);
        if(-1!=itsDisabledItems.findIndex(citem->fullName()))
            citem->setEnabled(false);

        citem=(CListViewItem *)(citem->nextSibling());
    }
}

void CFontListWidget::setAdvanced(bool on)
{
    if(on!=itsAdvancedMode)
    {
#ifdef ENABLE_DRAG
        setDragEnabled(on);
#endif
        itsAdvancedMode=on;
        scan();
    }
}

unsigned int CFontListWidget::getNumSelected(CListViewItem::EType type)
{
    unsigned int  num=0;
    CListViewItem *item=(CListViewItem *)(firstChild());

    while(NULL!=item)
    {
        if(item->isSelected() && item->getType()==type)
            num++;
        item=(CListViewItem *)(item->itemBelow());
    }
    return num;
}

void CFontListWidget::getNumSelected(int &numTT, int &numT1)
{
    CListViewItem *item=(CListViewItem *)firstChild();

    numTT=numT1=0;

    while(item!=NULL)
    {
        if(item->isSelected())
            if(CListViewItem::FONT==item->getType())
                if(CFontEngine::isATtf(QFile::encodeName(item->text(0))))
                    numTT++;
                else
                    if(CFontEngine::isAType1(QFile::encodeName(item->text(0))))
                        numT1++;
        item=(CListViewItem *)(item->itemBelow());
    }
}

void CFontListWidget::addFont(const QString &from, const QString &path, const QString &file, bool checkOpen)
{
    CFontItem *result = 0;
    bool      ok=false;

    if(itsAdvancedMode) // Need to find branch, and whether it's open...
    {
        CListViewItem *item=(CListViewItem *)(firstChild());

        while(NULL!=item)
        {
            if(item->getType()==CListViewItem::DIR)
                if(item->fullName()==path)
                {
                    CDirectoryItem *dirItem = (CDirectoryItem *)item;
                    if((!checkOpen || item->isOpen()) && !contains(item->firstChild(), file))
                        result = new CFontItem(dirItem, file, dirItem->dir(), true, true);
                    else
                        if(!CMisc::dExists(item->fullName()) || !CMisc::fExists(path+"/"+file))
                            ok=true;
                    break;
                }
            item=(CListViewItem *)(item->itemBelow());
        }
    }
    else
    {
        if(firstChild() && firstChild()->text(0) == i18n("No Fonts!"))
            delete firstChild();
        if(!contains(firstChild(), file))
            result = new CFontItem(this, file, path, true, true);
        setEnabled(true);
    }

    if(result || ok)
    {
        if(result)
            result->setExtraData(from);

        if(!contains(itsAddItems, from, path, file))
            itsAddItems.append(new TItem(from, path, file));
        CKfiGlobal::cfg().addModifiedDir(path); // Move to applyChanges
        updateConfig();
    }
}

void CFontListWidget::addSubDir(const QString &top, const QString &sub)
{
    if(itsAdvancedMode) // Need to find branch, and whether it's open...
    {
        CListViewItem *item=(CListViewItem *)(firstChild());

        while(NULL!=item)
        {
            if(item->getType()==CListViewItem::DIR)
                if(item->fullName()==top)
                {
                    if(item->isOpen() && !contains(item->firstChild(), sub))
                        (void) new CDirectoryItem(this, (CDirectoryItem *)item, sub, true);
                    break;
                }
            item=(CListViewItem *)(item->itemBelow());
        }
    }
}

void CFontListWidget::scan()
{
    clear();

    if(itsAdvancedMode)
    {
        setColumnWidthMode(0, QListView::Maximum);
        setColumnText(0, i18n("Folder/File"));

        (void) new CDirectoryItem(this, this, CKfiGlobal::cfg().getFontsDir(), i18n("X11 Fonts Folder"), "fonts", false);

        setEnabled(true);
        restore(firstChild());
    }
    else
    {
        setColumnText(0, i18n("File"));
        scanDir(CKfiGlobal::cfg().getFontsDir());

        restore(firstChild());

        if(childCount())
            setEnabled(true);
        else
        {
            new QListViewItem(this, i18n("No Fonts!"));
            setEnabled(false);
        }
    }
}

void CFontListWidget::scanDir(const QString &dir, int sub)
{
    QDir d(dir);

    if(d.isReadable())
    {
        const QFileInfoList *files=d.entryInfoList();

        if(files)
        {
            QFileInfoListIterator it(*files);
            QFileInfo             *fInfo;

            if(0==sub && files->count())
                progressInit(i18n("Scanning folder %1:").arg(dir), 0);

            for(; NULL!=(fInfo=it.current()); ++it)
            {
                QString file = fInfo->fileName();
                if("."!=file && ".."!=file)
                {
                    if(fInfo->isDir())
                    {
                        if(sub<CMisc::MAX_SUB_DIRS)
                            scanDir(dir+file+"/", sub+1);
                    }
                    else
                    {
                        if(CFontEngine::isAType1(QFile::encodeName(file)) ||
                           CFontEngine::isATtf(QFile::encodeName(file)))
                        {
                            progressShow(file);
                            new CFontItem(this, file, dir, false, true);
                        }
                    }
                }
            }

            if(0==sub && files->count())
                progressStop();
        }
    }
    d.setPath(dir+"/"+constDisabledSubDir);
    if(d.isReadable())
    {
        const QFileInfoList *files=d.entryInfoList();

        if(files)
        {
            QFileInfoListIterator it(*files);
            QFileInfo             *fInfo;

            if(0==sub && files->count())
                progressInit(i18n("Scanning folder %1:").arg(dir), 0);

            for(; NULL!=(fInfo=it.current()); ++it)
            {
                QString file = fInfo->fileName();
                if(!fInfo->isDir())
                {
                    if(CFontEngine::isAType1(QFile::encodeName(file)) ||
                       CFontEngine::isATtf(QFile::encodeName(file)))
                    {
                        progressShow(file);
                        new CFontItem(this, file, dir, false, false);
                    }
                }
            }

            if(0==sub && files->count())
                progressStop();
        }
    }
}

void CFontListWidget::selectionChanged()
{
    CListViewItem *cur=(CListViewItem *)(currentItem());

    if(cur && cur->isSelected())
    {
        CListViewItem *item=(CListViewItem *)(firstChild());

        switch(cur->getType())
        {
            case CListViewItem::DIR:
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
                break;
            case CListViewItem::FONT:
                // Only allows fonts in the same dir to be selected...
                while(NULL!=item)
                {
                    if(item->isSelected() && item!=cur &&
                      (item->getType()==CListViewItem::DIR || item->parent()!=cur->parent()))
                    {
                        item->setSelected(false);
                        item->repaint();
                    }
                    item=(CListViewItem *)(item->itemBelow());
                }
                break;
        }

        if(!itsShowingProgress)
            if(cur->getType()==CListViewItem::FONT)
                emit fontSelected(cur->fullName());
    }

    CListViewItem *item=getFirstSelectedItem();
    bool          enable=false;

    if(item)
    {
        enable=true;

        while(NULL!=item && enable)
        {
            if(item->isSelected())
                if(CListViewItem::DIR==item->getType())
                    enable=item->fullName()!=CKfiGlobal::cfg().getFontsDir() && CMisc::dWritable(item->fullName());
                else
                {
                    enable=CMisc::fWritable(item->fullName());

                    if(enable)
                    {
                        QString afm=CMisc::afmName(item->fullName());

                        if(CMisc::fExists(afm))
                            enable=CMisc::fWritable(afm);
                    }
                }

            item=(CListViewItem *)(item->itemBelow());
        }
    }
}

CFontListWidget::CListViewItem * CFontListWidget::getFirstSelectedItem()
{
    CListViewItem *item=(CListViewItem *)firstChild();

    while(NULL!=item)
    {
        if(item->isSelected())
            return item;

        item=(CListViewItem *)(item->itemBelow());
    }

    return NULL;
}

void CFontListWidget::progressInit(const QString &title, int numSteps)
{
    static const int constMaxSteps=25;

    if(0==numSteps || numSteps>constMaxSteps)
    {
        itsShowingProgress=true;
        emit initProgress(title, numSteps);
    }
}

void CFontListWidget::progressShow(const QString &step)
{
    if(itsShowingProgress)
        emit progress(step);
}

void CFontListWidget::progressStop()
{
    if(itsShowingProgress)
    {
        emit stopProgress();
        itsShowingProgress=false;
    }
}

QString CFontListWidget::currentDir()
{
    CFontListWidget::CListViewItem *current=(CFontListWidget::CListViewItem *)currentItem();

    if(current)
        return current->dir();
    else
        return CKfiGlobal::cfg().getFontsDir();
}

void CFontListWidget::updateConfig()
{
    if(! (CKfiGlobal::cfg().getModifiedDirs().count()<=0 && !CKfiGlobal::xcfg().madeChanges()
                                                         && !CKfiGlobal::cfg().firstTime()) )
        emit madeChanges();
}

void CFontListWidget::install()
{
    CListViewItem *citem=NULL;
    QString       selectedDir=itsAdvancedMode && (NULL!=(citem=getFirstSelectedItem())) ? citem->dir() : QString::null;

    if(itsAdvancedMode && (QString::null==selectedDir || CKfiGlobal::cfg().getFontsDir()==selectedDir))
        KMessageBox::error(this, i18n("Please select destination folder first!"), i18n("Error"));
    else
    {
        QString    filter = itsAdvancedMode ?
                            "application/x-font-ttf application/x-font-type1 application/x-font-bdf application/x-font-pcf "
                            "application/x-font-snf application/x-font-speedo" :
                            "application/x-font-ttf application/x-font-type1";
        KURL::List list   = KFileDialog::getOpenFileNames(QString::null, filter, topLevelWidget(), i18n("Install Fonts"));

        if(!list.isEmpty())
        {
            progressInit(i18n("Reading font information"), list.count());

            for(KURL::List::ConstIterator it = list.begin(); it != list.end(); ++it)
            {
                const KURL &url = *it;

                if(!url.isLocalFile())
                    continue; // Skip remote files for now.

                QString file    = url.fileName(),
                        dir     = url.directory(false),
                        destDir = itsAdvancedMode ? selectedDir : CKfiGlobal::cfg().getFontsDir();

                if(!itsAdvancedMode)
                    if(CFontEngine::isATtf(file.local8Bit()))
                        destDir+=CKfiGlobal::cfg().getTTSubDir();
                    else if(CFontEngine::isAType1(file.local8Bit()))
                        destDir+=CKfiGlobal::cfg().getT1SubDir();

                progressShow(file);
                addFont(dir, destDir, file);
            }
            progressStop();
        }
    }
}

void CFontListWidget::uninstall()
{
    bool changed = false;

    CListViewItem *item=(CListViewItem *)firstChild();

    while(item!=NULL)
    {
        if(item->isSelected() && (CListViewItem::FONT==item->getType() || !isRequiredDir(item)))
        {
            // Be carefull with iterating, current item may get deleted.
            CListViewItem *tmp=(CListViewItem *)item->itemBelow();
            item->setAvailable(false);
            changed = true;
            item = tmp;
        }
        else
            item=(CListViewItem *)item->itemBelow();
    }

    if(changed)
        emit madeChanges();
}

void CFontListWidget::disable()
{
    changeStatus(false);
}

void CFontListWidget::enable()
{
    changeStatus(true);
}

void CFontListWidget::changeStatus(bool status)
{
    bool changed = false;
    CListViewItem *item   = (CListViewItem *)firstChild();

    while(item!=NULL)
    {
        if(item->isSelected() && (CListViewItem::FONT==item->getType() || item->fullName()!=CKfiGlobal::cfg().getFontsDir()))
        {
            item->setEnabled(status);
            changed = true;
        }
        item=(CListViewItem *)item->itemBelow();
    }

    if(changed)
        emit madeChanges();
}

CFontListWidget::EStatus CFontListWidget::doDeleteDir(const QString &dir)
{
    EStatus status = PERMISSION_DENIED;

    if(CMisc::dWritable(dir))
    {
        unsigned int numItems=CMisc::getNumItems(dir);
        bool         hasFontsDir=CMisc::fExists(dir+"fonts.dir"),
                     hasFontsScale=CMisc::fExists(dir+"fonts.scale"),
                     hasEncDir=CMisc::fExists(dir+"encodings.dir"),
                     hasAlias=CMisc::fExists(dir+"fonts.alias");

        if(hasFontsDir)
            numItems--;
        if(hasEncDir)
            numItems--;

        if(numItems)
            status = NOT_EMPTY;
        else
        {
            bool error=false;

            if(hasFontsDir)
                if(!CMisc::fWritable(dir+"fonts.dir") || !CMisc::removeFile(dir+"fonts.dir"))
                {
                    status = COULD_NOT_DELETE_DIR;
                    error=true;
                }
                else
                {
                    CKfiGlobal::cfg().addModifiedDir(dir);  // Just in case fail to delete whole folder...
                    updateConfig();
                }

            if(!error && hasFontsScale)
                if(!CMisc::fWritable(dir+"fonts.scale") || !CMisc::removeFile(dir+"fonts.scale"))
                {
                    status = COULD_NOT_DELETE_DIR;
                    error=true;
                }
                else
                {
                    CKfiGlobal::cfg().addModifiedDir(dir);  // Just in case fail to delete whole folder...
                    updateConfig();
                }

            if(!error && hasAlias)
                if(!CMisc::fWritable(dir+"fonts.alias") || !CMisc::removeFile(dir+"fonts.alias"))
                {
                    status = COULD_NOT_DELETE_DIR;
                    error=true;
                }
                else
                {
                    CKfiGlobal::cfg().addModifiedDir(dir);  // Just in case fail to delete whole folder...
                    updateConfig();
                }

            if(!error && hasEncDir)
                if(!CMisc::fWritable(dir+"encodings.dir") || !CMisc::removeFile(dir+"encodings.dir"))
                {
                    status = COULD_NOT_DELETE_DIR;
                    error=true;
                }
                else
                {
                    CKfiGlobal::cfg().addModifiedDir(dir);  // Just in case fail to delete whole folder...
                    updateConfig();
                }

            if(!error)
                if(0!=CMisc::getNumItems(dir))  // Hmm it contains other items... (NOTE: This case should *not* happen!)
                    status = NOT_EMPTY;
                else
                    if(CMisc::removeDir(dir))
                    {
                         CKfiGlobal::cfg().removeModifiedDir(dir);
                         updateConfig();
                         status = SUCCESS;
                    }
                    else
                         status = COULD_NOT_DELETE_DIR;
        }
    }
    return status;
}

void CFontListWidget::applyChanges()
{
    //
    // Remove any items marked for delete from other lists...
    QStringList::Iterator it;
    bool                  removed;
    QStringList           delFromDisplay,
                          delFailures,
                          addFailures,
                          enFailures,
                          disFailures;

    do
    {
        removed=false;

        for(it=itsDelItems.begin(); it!=itsDelItems.end(); ++it)
        {
            if(-1!=itsEnabledItems.findIndex(*it))
                itsEnabledItems.remove(*it);
            if(-1!=itsDisabledItems.findIndex(*it))
                itsDisabledItems.remove(*it);
            if((removed=remove(itsAddItems, *it)))
            {
                itsDelItems.remove(*it);
                delFromDisplay.append(*it); // i.e. don't add, then delete...
                break;
            }
        }
    }
    while(removed);

    //
    // Convert 'new' items in enabled/disabled list from source paths to dest paths...
    CFontListWidget::TItem *addItem;

    for(addItem=itsAddItems.first(); addItem; addItem=itsAddItems.next())
        if(QString::null!=addItem->file)
        {
            QString src(addItem->source+"/"+addItem->file);

            if(-1!=itsEnabledItems.findIndex(src))
            {
                itsEnabledItems.remove(src);
                itsEnabledItems.append(addItem->dest+"/"+addItem->file);
            }
            if(-1!=itsDisabledItems.findIndex(src))
            {
                itsDisabledItems.remove(src);
                itsDisabledItems.append(addItem->dest+"/"+addItem->file);
            }
        }

    int count=itsDelItems.count()+itsAddItems.count()+itsEnabledItems.count()+itsDisabledItems.count();

    if(count)
    {
        progressInit(i18n("Updating:"), count);

        //
        // Do actual work...
        EStatus     status;
        int         successes=0;

        // ...adds
        for(addItem=itsAddItems.first(); addItem; addItem=itsAddItems.next())
            if(QString::null==addItem->file)
                if(SUCCESS==(status=CMisc::createDir(addItem->source+"/"+addItem->dest) ? SUCCESS : COULD_NOT_CREATE_DIR))
                    successes++;
                else
                {
                    CKfiGlobal::errorDialog().add(addItem->source+"/"+addItem->file, statusToStr(status));
                    addFailures.append(addItem->source+"/"+addItem->dest);
                    CKfiGlobal::xcfg().removePath(addItem->source+addItem->dest+"/");
                    CKfiGlobal::cfg().removeModifiedDir(addItem->source+addItem->dest+"/");
                }
        for(addItem=itsAddItems.first(); addItem; addItem=itsAddItems.next())
            if(QString::null!=addItem->file)
                if(SUCCESS==(status=install(addItem->source, addItem->dest, addItem->file)))
                    successes++;
                else
                {
                    CKfiGlobal::errorDialog().add(addItem->source+"/"+addItem->file, statusToStr(status));
                    addFailures.append(addItem->source+"/"+addItem->file);
                }

        // ...deletes
        for(it=itsDelItems.begin(); it!=itsDelItems.end(); ++it)
            if(CFontEngine::isAFont(QFile::encodeName(*it)))
                if(SUCCESS==(status=uninstall(*it, true)))
                    successes++;
                else
                {
                    delFailures.append(*it);
                    CKfiGlobal::errorDialog().add(*it, statusToStr(status));
                    delFailures.append(*it);
                }
        for(it=itsDelItems.begin(); it!=itsDelItems.end(); ++it)
            if(!CFontEngine::isAFont(QFile::encodeName(*it)))
            {
                if(CMisc::dExists((*it)+"/"+constDisabledSubDir))
                    status=doDeleteDir((*it)+"/"+constDisabledSubDir);

                if(SUCCESS==status)
                   status=doDeleteDir((*it));

                if(SUCCESS==status)
                    successes++;
                else
                {
                    delFailures.append(*it);
                    CKfiGlobal::errorDialog().add(*it, statusToStr(status));
                    delFailures.append(*it);
                }
            }

        // ...disables
        for(it=itsDisabledItems.begin(); it!=itsDisabledItems.end(); ++it)
        {
            status=move(CMisc::getDir(*it), CMisc::getDir(*it)+"/"+constDisabledSubDir, CMisc::getFile(*it));
            if(SUCCESS==status)
            {
                CKfiGlobal::cfg().addModifiedDir(CMisc::getDir(*it));
                successes++;
            }
            else
            {
                CKfiGlobal::errorDialog().add(*it, statusToStr(status));
                disFailures.append(*it);
            }
        }

        // ...enables
        for(it=itsEnabledItems.begin(); it!=itsEnabledItems.end(); ++it)
        {
            QString destDir(CMisc::getDir(*it));
            int     idx=destDir.find(constDisabledSubDir);

            if(-1!=idx)
                destDir.remove(idx, constDisabledSubDir.length());

            status=move(CMisc::getDir(*it), destDir, CMisc::getFile(*it));
            if(SUCCESS==status)
            {
                CKfiGlobal::cfg().addModifiedDir(destDir);
                successes++;
            }
            else
            {
                CKfiGlobal::errorDialog().add(*it, statusToStr(status));
                enFailures.append(*it);
            }
        }

        progressStop();

        if(delFailures.count() || addFailures.count() || enFailures.count() || disFailures.count())
            CKfiGlobal::errorDialog().open(i18n("The following items could not be updated:"));

        if(successes)
            updateConfig();
    }

    //
    // Now update display...
    CListViewItem *citem=(CListViewItem *)firstChild();

    while(NULL!=citem)
    {
        CListViewItem *next=(CListViewItem *)(citem->itemBelow());
        QString       fName(citem->fullName());

        //
        // Remove item from display if...
        // ...it was an added item that was deleted
        // ...it couldn't be added
        // ...it was a delete item that was deleted
        if(-1!=delFromDisplay.findIndex(fName) ||
           -1!=addFailures.findIndex(fName) ||
           (-1!=itsDelItems.findIndex(fName) && -1==delFailures.findIndex(fName)))
            delete citem;
        else
            // Toggle state of item if...
            // ...it was marked for disabling, but couldn't be disabled
            // ...it was marked for enabling, but couldn't be enabled
            citem->saved((-1!=itsDisabledItems.findIndex(fName) && -1!=disFailures.findIndex(fName)) ||
                         (-1!=itsEnabledItems.findIndex(fName) && -1!=enFailures.findIndex(fName)));

        citem=next;
    }

    clearLists();
}

CFontListWidget::EStatus CFontListWidget::uninstall(const QString &path, bool deleteAfm)
{
    QString dir      = CMisc::getDir(path),
            file     = CMisc::getFile(path),
            fontFile = path;
    EStatus status   = PERMISSION_DENIED;

    //
    // Check if font is actually disabled...
    if(!CMisc::fExists(fontFile) && CMisc::fExists(dir+"/"+constDisabledSubDir+"/"+file))
    {
        fontFile=dir+"/"+constDisabledSubDir+"/"+file;
        dir=dir+"/"+constDisabledSubDir+"/";
    }

    progressShow(itsAdvancedMode ? fontFile : file);

    status=CMisc::removeFile(fontFile) ? SUCCESS : PERMISSION_DENIED;

    if(SUCCESS==status && deleteAfm)
    {
        if(CMisc::fExists(CMisc::afmName(fontFile)))
            status=uninstall(dir+CMisc::afmName(file), false);
        CStarOfficeConfig::removeAfm(fontFile);
    }
    return status;
}

CFontListWidget::EStatus CFontListWidget::install(const QString &sourceDir, const QString &destDir, const QString &fname)
{
    EStatus status=PERMISSION_DENIED;

    CMisc::createDir(destDir);

    progressShow(itsAdvancedMode ? sourceDir+fname : fname);

    // Check to make sure font is not already installed...
    if(CMisc::fExists(destDir+fname))
        status=ALREADY_INSTALLED;
    else
    {
        // Only install fonts that can be opened
        // ...this is needed because OpenOffice will fail to start if an invalid font is located in one of its font directories!
        status=CKfiGlobal::fe().openFont(sourceDir+fname, CFontEngine::TEST) ? SUCCESS : INVALID_FONT;
        if(SUCCESS==status)
        {
            status=CMisc::copyFile(sourceDir, fname, destDir) ? SUCCESS : PERMISSION_DENIED;
            if(SUCCESS==status)
            {
                QString afm=CMisc::afmName(fname);

                if(CMisc::fExists(sourceDir+afm))
                    CMisc::copyFile(sourceDir, afm, destDir);

                //if(CFontEngine::isATtf(fname.local8Bit()))
                //    CKfiGlobal::ttf().fixPsNames(destDir+fname); // CPD: Is this worth doing anymore?
            }
        }
    }

    return status;
}

CFontListWidget::EStatus CFontListWidget::move(const QString &sourceDir, const QString &destDir, const QString &fname)
{
    EStatus status=PERMISSION_DENIED;

    CMisc::createDir(destDir);

    progressShow(itsAdvancedMode ? sourceDir+fname : fname);

    // Check to make sure font is not already installed...
    if(CMisc::fExists(destDir+fname))
        status=ALREADY_INSTALLED;
    else
    {
        // Only install fonts that can be opened
        // ...this is needed because OpenOffice will fail to start if an invalid font is located in one of its font directories!
        status=CKfiGlobal::fe().openFont(sourceDir+fname, CFontEngine::TEST) ? SUCCESS : INVALID_FONT;
        if(SUCCESS==status)
        {
            status=CMisc::moveFile(sourceDir+fname, destDir) ? SUCCESS : PERMISSION_DENIED;
            if(SUCCESS==status)
            {
                QString afm=CMisc::afmName(fname);

                if(CMisc::fExists(sourceDir+afm))
                    CMisc::moveFile(sourceDir+afm, destDir);

                //if(CFontEngine::isATtf(fname.local8Bit()))
                //    CKfiGlobal::ttf().fixPsNames(destDir+fname); // CPD: Is this worth doing anymore?
            }
        }
    }

    return status;
}

void CFontListWidget::popupMenu(QListViewItem *item, const QPoint &point, int)
{
    if(itsAdvancedMode && NULL!=item)
    {
        if(!item->isSelected())
            setSelected(item, true);

        if(((CListViewItem *)item)->getType()==CListViewItem::FONT)
        {
            int numTT,
                numT1;

            getNumSelected(numTT, numT1);
            if(numTT)
            {
                itsFontsPopup->setItemEnabled(itsFixTtfPsNamesME, numTT ? true : false);
                itsFontsPopup->popup(point);
            }
        }
        else
            if(CKfiGlobal::xcfg().ok() && CKfiGlobal::xcfg().writable())
            {
                bool showMenu=false;

                if(NULL==item->parent())
                {
                    itsDirsPopup->setItemEnabled(itsSetScaledME, false);
                    itsDirsPopup->setItemEnabled(itsSetUnscaledME, false);
                }
                else
                {
                    showMenu=true;

                    if(CKfiGlobal::xcfg().inPath(((CListViewItem *)item)->fullName()))
                    {
                        bool unscaled=CKfiGlobal::xcfg().isUnscaled(((CListViewItem *)item)->fullName());

                        if(CKfiGlobal::xcfg().custom())
                        {
                            itsDirsPopup->setItemEnabled(itsSetScaledME, false);
                            itsDirsPopup->setItemEnabled(itsSetUnscaledME, false);
                        }
                        else
                        {
                            itsDirsPopup->setItemEnabled(itsSetScaledME, unscaled);
                            itsDirsPopup->setItemEnabled(itsSetUnscaledME, !unscaled);
                        }
                    }
                    else
                    {
                        itsDirsPopup->setItemEnabled(itsSetScaledME, false);
                        itsDirsPopup->setItemEnabled(itsSetUnscaledME, false);
                    }
                }

                if(CMisc::dWritable(((CListViewItem *)item)->fullName()))
                {
                    itsDirsPopup->setItemEnabled(itsCreateDirME, true);
                    showMenu=true;
                }
                else
                    itsDirsPopup->setItemEnabled(itsCreateDirME, false);

                if(showMenu)
                    itsDirsPopup->popup(point);
            }
    }
}

void CFontListWidget::listClicked(QListViewItem *item, const QPoint &, int column)
{
    if(item)
    {
        CListViewItem *citem = (CListViewItem *)item;

        switch(column)
        {
            case 1:
                 if(!isRequiredDir(item))
                 {
                    citem->setAvailable(!citem->available());
                    emit madeChanges();
                 }
                 break;
            case 2:
                if(CListViewItem::FONT==citem->getType() || citem->fullName()!=CKfiGlobal::cfg().getFontsDir())
                {
                    citem->setEnabled(!citem->enabled());
                    emit madeChanges();
                }
                break;
            default:
               break;
        }
    }
}

void CFontListWidget::fixTtfPsNames()
{
    if(KMessageBox::questionYesNo(this, i18n("This will *permanently* alter the TrueType font file(s),"
                                             "\nand cannot be undone."
                                             "\n"
                                             "\nAre you sure?"), i18n("Fix TTF postscript names"))==KMessageBox::Yes)
    {
        CListViewItem *item=(CListViewItem *)firstChild();
        int           failures=0,
                      numTT,
                      numT1;

        getNumSelected(numTT, numT1);

        CKfiGlobal::errorDialog().clear();
        progressInit(i18n("Fixing:"), numTT);

        while(NULL!=item)
        {
            if(item->isSelected() && item->getType()==CListViewItem::FONT && CFontEngine::isATtf(item->text(0).local8Bit()))
            {
                CTtf::EStatus status;

                progressShow(item->fullName());
                if(CTtf::SUCCESS!=(status=CKfiGlobal::ttf().fixPsNames(item->fullName())) && CTtf::NO_REMAP_GLYPHS!=status)
                {
                    CKfiGlobal::errorDialog().add(item->text(0), CTtf::toString(status));
                    failures++;
                }
            }
            item=(CListViewItem *)(item->itemBelow());
        }
        progressStop();
        if(failures)
            CKfiGlobal::errorDialog().open(i18n("The following files could not be modified:"));
    }
}

void CFontListWidget::toggleUnscaled()
{
    CListViewItem *item=getFirstSelectedItem();

    if(NULL!=item && item->getType()==CListViewItem::DIR)
    {
        CKfiGlobal::xcfg().setUnscaled(((CListViewItem *)item)->fullName(),
                                       !CKfiGlobal::xcfg().isUnscaled(((CListViewItem *)item)->fullName()));
        item->repaint();
    }
    updateConfig();
}

class CCreateDirDialog : public KLineEditDlg
{
    class CValidator : public QValidator
    {
        public:

        CValidator(QWidget *widget) : QValidator(widget) {}
        virtual ~CValidator() {}

        State validate(QString &input, int &) const
        {
            if(input.contains(QRegExp("[¬|!\"£$%&\\*:;@'`#~<>\\?/\\\\]")))
                return Invalid;
            else
                return Acceptable;
        }
    };

    public:

    CCreateDirDialog(QWidget *parent) : KLineEditDlg(i18n("Please type name of new folder:"), "", parent)
    {
        edit->setValidator(new CValidator(edit));
    }

    virtual ~CCreateDirDialog() {}
};

void CFontListWidget::createDir()
{
    CListViewItem *item=getFirstSelectedItem();

    if(!item || !CMisc::dWritable(item->dir()))
        KMessageBox::error(this, i18n("Folder is not writable!"), i18n("Error"));
    else
    {
        CCreateDirDialog *dlg=new CCreateDirDialog(this);

        if(dlg->exec())
        {
            QString       dir=dlg->text().stripWhiteSpace();
            CListViewItem *item=getFirstSelectedItem();

            if(dir==constDisabledSubDir)
                KMessageBox::error(this, i18n("Cannot create folders named \"%1\"").arg(constDisabledSubDir));
            else if(CMisc::dExists(item->dir()+dir) || CMisc::fExists(item->dir()+dir) ||
                    contains(itsAddItems, item->dir(), dir, QString::null))
                KMessageBox::error(this, i18n("Folder already exists!"), i18n("Error"));
            else
            {
                CKfiGlobal::xcfg().addPath(item->dir()+dir+"/");
                CKfiGlobal::cfg().addModifiedDir(item->dir()+dir+"/");
                addSubDir(item->dir(), dir);
                updateConfig();
                if(!contains(itsAddItems, item->dir(), dir, QString::null))
                    itsAddItems.append(new TItem(item->dir(), dir, QString::null));
            }
        }

        delete dlg;
    }
}

void CFontListWidget::startDrag()
{
    QDragObject *drag = dragObject();

    if(drag)
    {
        drag->setPixmap(KGlobal::iconLoader()->loadIcon(getNumSelectedDirs() ? "folder" : "document2", KIcon::Small));
        if(drag->drag() && drag->target() != viewport())
            emit moved();
    }
}

void CFontListWidget::movableDropEvent(QListViewItem *parent, QListViewItem *afterme)
{
    CListViewItem *dest=(CListViewItem *)parent;

    if(dest && CKfiGlobal::cfg().getFontsDir()!=dest->fullName())
    {
        QListViewItem *item=firstChild();

        while(item)
        {
            if(item->isSelected())
            {
                // CPD: TODO - need to take into account if item is new, deted, disabled...
                std::cerr << "MOVE ITEM:" << ((CListViewItem *)item)->fullName().latin1() << " TO:"
                     << dest->fullName().latin1() << std::endl;
            }

            item=item->itemBelow();
        }
    }
}

QString CFontListWidget::statusToStr(EStatus status)
{
    switch(status)
    {
        case SUCCESS:
            return i18n("Success");
        case PERMISSION_DENIED:
            return i18n("Permission denied?");
        case ALREADY_INSTALLED:
            return i18n("Already installed");
        case HAS_SUB_DIRS:
            return i18n("Contains sub-folders");
        case COULD_NOT_CREATE_DIR:
            return i18n("Could not create folder to uninstall to");
        case COULD_NOT_DELETE_DIR:
            return i18n("Could not delete folder");
        case INVALID_FONT:
            return i18n("Invalid font - or font file corrupt");
        case NOT_EMPTY:
            return i18n("Folder is not empty");
        default:
            return i18n("Unknown");
    }
}

#include "FontListWidget.moc"
