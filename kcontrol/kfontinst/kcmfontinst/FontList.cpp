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

#include "FontList.h"
#include <kglobal.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kicon.h>
#include <kiconloader.h>
#include <kde_file.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <QFont>
#include <QMap>
#include <QFile>
#include <QImage>
#include <QDir>
#include <QFileInfo>
#include <QPixmap>
#include <QDropEvent>
#include <QDateTime>
#include <QX11Info>
#include <QHeaderView>
#include <QItemDelegate>
#include <QPainter>
#include <QMenu>
#include <QTimer>
#include <QApplication>
#include <stdlib.h>
#include <unistd.h>
#include <utime.h>
#include "FcEngine.h"
#include "KfiConstants.h"
#include "FcQuery.h"
#include "GroupList.h"
#include "config.h"

//#define KFI_FONTLIST_DEBUG

#ifdef KFI_FONTLIST_DEBUG
#include <kdebug.h>
#define KFI_DBUG kDebug() << "[" << (int)(getpid()) << "] CFontList "
#endif

// #define to control whether generated inline preview thumbs should be
// cached to disk.
#define KFI_SAVE_PIXMAPS

namespace KFI
{

int CFontList::theirPreviewSize=CFontList::constDefaultPreviewSize;

static void decompose(const QString &name, QString &family, QString &style)
{
    int commaPos=name.lastIndexOf(',');

    family=-1==commaPos ? name : name.left(commaPos);
    style=-1==commaPos ? KFI_WEIGHT_REGULAR : name.mid(commaPos+2);
}

static void addFont(CFontItem *font, CJobRunner::ItemList &urls, QStringList &fontNames,
                    QSet<Misc::TFont> *fonts,  bool *hasSys, QSet<CFontItem *> &usedFonts,
                    bool getEnabled, bool getDisabled)
{
    if(!usedFonts.contains(font) &&
        ( (getEnabled && font->isEnabled()) ||
          (getDisabled && !font->isEnabled()) ) )
    {
        urls.append(CJobRunner::Item(font->url(), font->name()));
        fontNames.append(font->name());
        usedFonts.insert(font);
        if(fonts)
            fonts->insert(Misc::TFont(font->family(), font->styleInfo()));
        if(hasSys && !(*hasSys) && font->isSystem())
            *hasSys=true;
    }
}

static QString getName(const KFileItem *item)
{
    return item
        ? item->entry().stringValue(KIO::UDS_NAME)
        : QString();
}

static QString replaceEnvVar(const QString &text)
{
    QString mod(text);
    int     endPos(text.indexOf('/'));

    if(endPos==-1)
        endPos=text.length()-1;
    else
        endPos--;

    if(endPos>0)
    {
        QString envVar(text.mid(1, endPos));

        const char *val=getenv(envVar.toLatin1().constData());

        if(val)
            mod=Misc::fileSyntax(QFile::decodeName(val)+mod.mid(endPos+1));
    }

    return mod;
}

//
// Convert from list such as:
//
//    Arial
//    Arial, Bold
//    Courier
//    Times
//    Times, Italic
//
// To:
//
//    Arial (Regular, Bold)
//    Coutier
//    Times (Regular, Italic)
QStringList CFontList::compact(const QStringList &files)
{
    QString                    lastFamily,
                               entry;
    QStringList::ConstIterator it(files.begin()),
                               end(files.end());
    QStringList                compacted;
    QSet<QString>              usedStyles;

    for(; it!=end; ++it)
    {
        QString family,
                style;

        decompose(*it, family, style);

        if(family!=lastFamily)
        {
            usedStyles.clear();
            if(entry.length())
            {
                entry+=')';
                compacted.append(entry);
            }
            entry=QString(family+" (");
            lastFamily=family;
        }
        if(!usedStyles.contains(style))
        {
            usedStyles.clear();
            if(entry.length() && '('!=entry[entry.length()-1])
                entry+=", ";
            entry+=style;
            usedStyles.insert(style);
        }
    }

    if(entry.length())
    {
        entry+=')';
        compacted.append(entry);
    }

    return compacted;
}

class CPreviewCache
{
    public:

    CPreviewCache();
#ifdef KFI_SAVE_PIXMAPS
    ~CPreviewCache() { clearOld(); }
#endif

    static QString thumbKey(const QString &family, unsigned long style, int height);
    QPixmap * getPixmap(const QString &family, const QString &name, const QString &fileName,
                        int height, unsigned long stlye, bool force=false);
#ifdef KFI_SAVE_PIXMAPS
    void clearOld();
#endif
    void empty();

    private:

    QMap<QString, QPixmap> itsMap;
#ifdef KFI_SAVE_PIXMAPS
    QString                itsPath;
#endif
};

CPreviewCache::CPreviewCache()
#ifdef KFI_SAVE_PIXMAPS
             : itsPath(KGlobal::dirs()->saveLocation("cache", "fontpreview"))
#endif
{
}

static void addAlpha(QImage &img)
{
    img=img.convertToFormat(QImage::Format_ARGB32);

    int pixelsPerLine=img.bytesPerLine()/4;

    for(int l=0; l<img.height(); ++l)
    {
        QRgb *scanLine=(QRgb *)img.scanLine(l);

        for(int pixel=0; pixel<pixelsPerLine; ++pixel)
            scanLine[pixel]=qRgba(qRed(scanLine[pixel]), qGreen(scanLine[pixel]),
                                  qBlue(scanLine[pixel]),
                                  0xFF-qRed(scanLine[pixel]));
    }
}

static QString replaceChars(const QString &in)
{
    QString rv(in);

    rv=rv.replace('/', '_');

    return rv;
}

#ifdef KFI_SAVE_PIXMAPS
static void setTimeStamp(const QString &f)
{
    QByteArray      fC(QFile::encodeName(f));
    KDE_struct_stat fStat;

    if(0==KDE_lstat(fC, &fStat))
    {
        struct utimbuf times;

        times.actime=times.modtime=time(NULL);
        utime(fC, &times);
    }
}
#endif
QString CPreviewCache::thumbKey(const QString &name, unsigned long style, int height)
{
    QString str;

    str.sprintf("--%06X--%02d", (unsigned int)style, height);

    return replaceChars(name+str)+".png";
}

QPixmap * CPreviewCache::getPixmap(const QString &family, const QString &name, const QString &fileName,
                                   int height, unsigned long style, bool force)
{
#ifdef KFI_SAVE_PIXMAPS
    static const char *constFileType="PNG";
#endif

    QString thumbName(thumbKey(family, style, height));

    if(!force && !itsMap[thumbName].isNull())
        return &(itsMap[thumbName]);

#ifdef KFI_SAVE_PIXMAPS
    QString thumbFile(itsPath+thumbName);

    if(!force && itsMap[thumbName].load(thumbFile, constFileType))
        return &(itsMap[thumbName]);
#endif

    itsMap[thumbName]=QPixmap();
    if(CFcEngine::instance()->drawPreview(fileName.isEmpty() ? name : fileName, itsMap[thumbName],
                                          height, style))  // CPD:TODO face???
    {
#ifdef KFI_SAVE_PIXMAPS
        QFile pngFile(thumbFile);

        if(pngFile.open(QIODevice::WriteOnly))
        {
#endif
            QImage thumb=itsMap[thumbName].toImage();

            addAlpha(thumb);
#ifdef KFI_SAVE_PIXMAPS
            thumb.save(&pngFile, constFileType);
            pngFile.close();
#endif
            itsMap[thumbName]=QPixmap(thumb);
            return &(itsMap[thumbName]);
#ifdef KFI_SAVE_PIXMAPS
        }
#endif
    }
    else
        itsMap[thumbName]=QPixmap(1, 1);

    return NULL;
}

#ifdef KFI_SAVE_PIXMAPS
void CPreviewCache::clearOld()
{
    //
    // Remove any files that have not been accessed for constMaxAge days.
    // ...this should be OK, as this function is called after the font list is completed,
    // so any existing fonts will already have accessed their thumbnail.

    static const int constMaxAge = 7;

    QDir d(itsPath);

    if(d.isReadable())
    {
        d.setFilter(QDir::Files);

        QFileInfoList list(d.entryInfoList());
        QDateTime     current(QDateTime::currentDateTime());

        for (int i = 0; i < list.size(); ++i)
        {
            QFileInfo fileInfo(list.at(i));
            int       diff=abs(current.daysTo(fileInfo.lastRead()));

            if(diff>constMaxAge) // More than constMaxAge days ago, so remove
                ::unlink(QFile::encodeName(fileInfo.absoluteFilePath()));
        }
    }
}
#endif

void CPreviewCache::empty()
{
#ifdef KFI_SAVE_PIXMAPS
    QDir d(itsPath);

    if(d.isReadable())
    {
        d.setFilter(QDir::Files);

        QFileInfoList list(d.entryInfoList());

        for (int i = 0; i < list.size(); ++i)
        {
            QFileInfo fileInfo(list.at(i));

            if(-1!=fileInfo.fileName().lastIndexOf(".png"))
                ::unlink(QFile::encodeName(fileInfo.absoluteFilePath()));
        }
    }
#endif
    itsMap.clear();
}

static CPreviewCache *theCache=NULL;

inline bool isSysFolder(const QString &sect)
{
    return i18n(KFI_KIO_FONTS_SYS)==sect || KFI_KIO_FONTS_SYS==sect;
}

CFontItem::CFontItem(CFontModelItem *p, const KFileItem *item, const QString &style)
         : CFontModelItem(p),
           itsItem(item),
           itsStyle(style),
           itsPixmap(NULL)
{
    const KIO::UDSEntry &udsEntry(entry());
    int                 weight,
                        width,
                        slant;

    updateStatus();
    itsName=udsEntry.stringValue(KIO::UDS_NAME);
    FC::decomposeStyleVal(FC::createStyleVal(itsName), weight, width, slant);
    itsDisplayStyleInfo=(weight<<16)+(slant<<8)+(width);
    itsFileName=udsEntry.stringValue((uint)UDS_EXTRA_FILE_NAME);
    itsStyleInfo=FC::styleValFromStr(udsEntry.stringValue((uint)UDS_EXTRA_FC_STYLE));
    itsIndex=Misc::getIntQueryVal(KUrl(udsEntry.stringValue((uint)KIO::UDS_URL)),
                                  KFI_KIO_FACE, 0);
    QString mime(mimetype());

    itsBitmap="application/x-font-pcf"==mime || "application/x-font-bdf"==mime;
    if(!Misc::root())
        setIsSystem(isSysFolder(url().path().section('/', 1, 1)));

    QString fileList=udsEntry.stringValue((uint)UDS_EXTRA_FILE_LIST);

    if(fileList.isEmpty())
        itsFiles.append(itsFileName);
    else
        itsFiles=fileList.split(KFI_FILE_LIST_SEPARATOR);
}

void CFontItem::touchThumbnail()
{
#ifdef KFI_SAVE_PIXMAPS
    // Access thumbFile, if it exists, to prevent its removal from the cache
    if(itsParent)
        setTimeStamp(CPreviewCache::thumbKey(family(), itsStyleInfo, CFontList::previewSize()));
#endif
}

const QPixmap * CFontItem::pixmap(bool force)
{
    if(parent() &&
       (!itsPixmap || itsPixmap->isNull() || force ||
        itsPixmap->height()!=CFontList::previewSize()))
        itsPixmap=theCache->getPixmap(family(), name(), isEnabled()
                                                            ? QString::null
                                                            : itsFileName,
                                      CFontList::previewSize(), itsStyleInfo, force);

    return itsPixmap;
}

CFamilyItem::CFamilyItem(CFontList &p, const QString &n)
           : CFontModelItem(NULL),
             itsName(n),
             itsStatus(ENABLED),
             itsRealStatus(ENABLED),
             itsRegularFont(NULL),
             itsParent(p)
{
}

CFamilyItem::~CFamilyItem()
{
    qDeleteAll(itsFonts);
    itsFonts.clear();
}

void CFamilyItem::touchThumbnail()
{
    if(itsRegularFont)
        itsRegularFont->touchThumbnail();
}

CFontItem * CFamilyItem::findFont(const KFileItem *i)
{
    QList<CFontItem *>::ConstIterator fIt(itsFonts.begin()),
                                      fEnd(itsFonts.end());

    for(; fIt!=fEnd; ++fIt)
        if((*(*fIt)).item()==i)
            return (*fIt);

    return NULL;
}

bool CFamilyItem::usable(const CFontItem *font, bool root)
{
    return (!font->isHidden() || itsParent.allowDisabled()) &&
            ( root ||
                (font->isSystem() && itsParent.allowSys()) ||
                (!font->isSystem() && itsParent.allowUser()));
}

void CFamilyItem::addFont(CFontItem *font)
{
    itsFonts.append(font);
    updateStatus();
    updateRegularFont(font);
}

void CFamilyItem::removeFont(CFontItem *font)
{
    itsFonts.remove(font);
    updateStatus();
    if(itsRegularFont==font)
    {
        itsRegularFont=NULL;
        updateRegularFont(NULL);
    }
    delete font;
}

void CFamilyItem::refresh()
{
    updateStatus();
    itsRegularFont=NULL;
    updateRegularFont(NULL);
    touchThumbnail();
}

bool CFamilyItem::updateStatus()
{
    bool                              root(Misc::root());
    QString                           oldIcon(itsIcon);
    EStatus                           oldStatus(itsStatus);
    QList<CFontItem *>::ConstIterator it(itsFonts.begin()),
                                      end(itsFonts.end());
    int                               en(0), dis(0), allEn(0), allDis(0);
    bool                              oldSys(isSystem()),
                                      sys(false);
    QStringList                       mimeTypes;

    itsFontCount=0;
    for(; it!=end; ++it)
        if(usable(*it, root))
        {
            QString mime((*it)->mimetype());

            if((*it)->isEnabled())
                en++;
            else
                dis++;
            if(!mimeTypes.contains(mime))
                mimeTypes.append(mime);
            if(!sys)
                sys=(*it)->isSystem();
            itsFontCount++;
        }
        else
            if((*it)->isEnabled())
                allEn++;
            else
                allDis++;

    allEn+=en;
    allDis+=dis;

    itsStatus=en && dis
                ? PARTIAL
                : en
                    ? ENABLED
                    : DISABLED;

    itsRealStatus=allEn && allDis
                ? PARTIAL
                : allEn
                    ? ENABLED
                    : DISABLED;

    itsIcon=1==mimeTypes.count()
                ? KMimeType::mimeType(mimeTypes[0])->iconName()
                : "font";

    if(!root)
        setIsSystem(sys);

    return itsStatus!=oldStatus || itsIcon!=oldIcon || isSystem()!=oldSys;
}

bool CFamilyItem::updateRegularFont(CFontItem *font)
{
    static const int constRegular=(FC_WEIGHT_REGULAR<<16)+(FC_SLANT_ROMAN<<8)+KFI_FC_WIDTH_NORMAL;

    CFontItem *oldFont(itsRegularFont);
    bool       root(Misc::root());

    if(font && usable(font, root))
    {
        if(itsRegularFont)
        {
            int regDiff=abs(itsRegularFont->displayStyleInfo()-constRegular),
                fontDiff=abs(font->displayStyleInfo()-constRegular);

            if(fontDiff<regDiff)
                itsRegularFont=font;
        }
        else
            itsRegularFont=font;
    }
    else // This case happens when the regular font is deleted...
    {
        QList<CFontItem *>::ConstIterator it(itsFonts.begin()),
                                          end(itsFonts.end());
        int                               current=0x0FFFFFFF;

        for(; it!=end; ++it)
            if(usable(*it, root))
            {
                int diff=abs((*it)->displayStyleInfo()-constRegular);
                if(diff<current)
                {
                    itsRegularFont=(*it);
                    current=diff;
                }
            }
    }

    return oldFont!=itsRegularFont;
}

CFontList::CFontList(QWidget *parent)
         : QAbstractItemModel(parent),
           itsAllowSys(true),
           itsAllowUser(true)
{
    if(!theCache)
        theCache=new CPreviewCache;

    QFont font;
    int   pixelSize((int)(((font.pointSizeF()*QX11Info::appDpiY())/72.0)+0.5));

    setPreviewSize(pixelSize+12);
    itsLister=new CFontLister(this);
    connect(itsLister, SIGNAL(completed()), SLOT(listingCompleted()));
    connect(itsLister, SIGNAL(newItems(const KFileItemList &)),
            SLOT(newItems(const KFileItemList &)));
    connect(itsLister, SIGNAL(deleteItems(const KFileItemList &)),
            SLOT(deleteItems(const KFileItemList &)));
    connect(itsLister, SIGNAL(refreshItems(const KFileItemList &)),
            SLOT(refreshItems(const KFileItemList &)));
    connect(itsLister, SIGNAL(percent(int)), SIGNAL(percent(int)));
    connect(itsLister, SIGNAL(message(QString)), SIGNAL(status(QString)));
}

CFontList::~CFontList()
{
    delete theCache;
    theCache=NULL;
    delete itsLister;
    itsLister=NULL;
    qDeleteAll(itsFamilies);
    itsFamilies.clear();
    itsFonts.clear();
}

int CFontList::columnCount(const QModelIndex &) const
{
    return 3;
}

QVariant CFontList::data(const QModelIndex &, int) const
{
    return QVariant();
}

Qt::ItemFlags CFontList::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

Qt::DropActions CFontList::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

QMimeData * CFontList::mimeData(const QModelIndexList &indexes) const
{
    QMimeData                      *mimeData = new QMimeData();
    QByteArray                     encodedData;
    QModelIndexList::ConstIterator it(indexes.begin()),
                                   end(indexes.end());
    QSet<QString>                  families;
    QDataStream                    ds(&encodedData, QIODevice::WriteOnly);

    for(; it!=end; ++it)
        if((*it).isValid())
            if((static_cast<CFontModelItem *>((*it).internalPointer()))->isFont())
            {
                CFontItem *font=static_cast<CFontItem *>((*it).internalPointer());

                families.insert(font->family());
            }
            else
            {
                CFamilyItem *fam=static_cast<CFamilyItem *>((*it).internalPointer());

                families.insert(fam->name());
            }

    ds << families;
    mimeData->setData(KFI_FONT_DRAG_MIME, encodedData);
    return mimeData;
}

QStringList CFontList::mimeTypes() const
{
    QStringList types;

    types << "text/uri-list";
    return types;
}

QVariant CFontList::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal)
        switch(role)
        {
            case Qt::DisplayRole:
                switch(section)
                {
                    case COL_FONT:
                        return i18n("Font");
                    case COL_PREVIEW:
                        return i18n("Preview");
                    default:
                        break;
                }
                break;
            case Qt::DecorationRole:
                if(COL_STATUS==section)
                    return SmallIcon("enablefont");
                break;
            case Qt::TextAlignmentRole:
                return Qt::AlignLeft;
            case Qt::ToolTipRole:
                if(COL_STATUS==section)
                    return i18n("This column shows the status of the font family, and of the "
                                "individual font styles.");
            default:
                break;
        }

    return QVariant();
}

QModelIndex CFontList::index(int row, int column, const QModelIndex &parent) const
{
    if(parent.isValid()) // Then font...
    {
        CFamilyItem *fam=static_cast<CFamilyItem*>(parent.internalPointer());
        CFontItem   *font=fam->fonts().value(row);

        if(font)
            return createIndex(row, column, font);
    }
    else // Family....
    {
        CFamilyItem *fam=itsFamilies.value(row);

        if(fam)
            return createIndex(row, column, fam);
    }

    return QModelIndex();
}

QModelIndex CFontList::parent(const QModelIndex &index) const
{
    if(!index.isValid())
        return QModelIndex();

    CFontModelItem *mi=static_cast<CFontModelItem *>(index.internalPointer());

    if(mi->isFamily())
        return QModelIndex();
    else
    {
        CFontItem *font=static_cast<CFontItem *>(index.internalPointer());

        return createIndex(itsFamilies.indexOf(((CFamilyItem *)font->parent())), 0, font->parent());
    }
}

int CFontList::rowCount(const QModelIndex &parent) const
{
    if(parent.isValid())
    {
        CFontModelItem *mi=static_cast<CFontModelItem *>(parent.internalPointer());

        if(mi->isFont())
            return 0;

        CFamilyItem *fam=static_cast<CFamilyItem *>(parent.internalPointer());

        return fam->fonts().count();
    }
    else
        return itsFamilies.count();
}

void CFontList::forceNewPreviews()
{
    QList<CFamilyItem *>::ConstIterator it(itsFamilies.begin()),
                                        end(itsFamilies.end());

    for(; it!=end; ++it)
    {
        QList<CFontItem *>::ConstIterator fit((*it)->fonts().begin()),
                                          fend((*it)->fonts().end());

        for(; fit!=fend; ++fit)
            (*fit)->clearPixmap();
    }

    theCache->empty();
}

void CFontList::refresh(bool allowSys, bool allowUser)
{
    itsAllowSys=allowSys;
    itsAllowUser=allowUser;
    QList<CFamilyItem *>::ConstIterator it(itsFamilies.begin()),
                                        end(itsFamilies.end());

    for(; it!=end; ++it)
        (*it)->refresh();
}

void CFontList::setAllowDisabled(bool on)
{
    itsAllowDisabled=on;
    QList<CFamilyItem *>::ConstIterator it(itsFamilies.begin()),
                                        end(itsFamilies.end());

    for(; it!=end; ++it)
        (*it)->refresh();
}

void CFontList::getFamilyStats(QSet<QString> &enabled, QSet<QString> &disabled, QSet<QString> &partial)
{
    QList<CFamilyItem *>::ConstIterator it(itsFamilies.begin()),
                                        end(itsFamilies.end());

    for(; it!=end; ++it)
        switch((*it)->realStatus())
        {
            case CFamilyItem::ENABLED:
                enabled.insert((*it)->name());
                break;
            case CFamilyItem::PARTIAL:
                partial.insert((*it)->name());
                break;
            case CFamilyItem::DISABLED:
                disabled.insert((*it)->name());
                break;
        }
}

void CFontList::listingCompleted()
{
    touchThumbnails();
    emit finished();
}

void CFontList::newItems(const KFileItemList &items)
{
#ifdef KFI_FONTLIST_DEBUG
    KFI_DBUG << "************** newItems " << items.count() << endl;

    for(KFileItemList::const_iterator it(items.begin()), end(items.end()) ; it!=end ; ++it)
        KFI_DBUG << "               " << (int)(*it) << endl;
#endif

    for(KFileItemList::const_iterator it(items.begin()), end(items.end()) ; it!=end ; ++it)
        addItem(*it);

    emit layoutChanged();
}

void CFontList::clearItems()
{
#ifdef KFI_FONTLIST_DEBUG
    KFI_DBUG << "************** clearItems" << endl;
#endif

    beginRemoveRows(QModelIndex(), 0, itsFamilies.count());
    endRemoveRows();
    qDeleteAll(itsFamilies);
    itsFamilies.clear();
    itsFonts.clear();
}

void CFontList::refreshItems(const KFileItemList &items)
{
#ifdef KFI_FONTLIST_DEBUG
    KFI_DBUG << "************** refreshItems " << items.count() << endl;

    for(KFileItemList::const_iterator it(items.begin()), end(items.end()) ; it!=end ; ++it)
        KFI_DBUG << "               " << (int)(*it) << endl;
#endif

    QSet<CFamilyItem *> families;

    for(KFileItemList::const_iterator it(items.begin()), end(items.end()) ; it!=end ; ++it)
    {
        CFontItem *font=findFont(*it);

        if(font)
        {
            font->updateStatus();
#ifdef KFI_FONTLIST_DEBUG
            KFI_DBUG << "               Found font, status now:" << font->isEnabled()
                     << " url" << (*it)->url().prettyUrl() << endl;
#endif
            families.insert(static_cast<CFamilyItem *>(font->parent()));
        }
#ifdef KFI_FONTLIST_DEBUG
        else
            KFI_DBUG << "               Could not locate font :-( " << (int)(*it) << endl;
#endif
    }

    QSet<CFamilyItem *>::ConstIterator it(families.begin()),
                                       end(families.end());

    for(; it!=end; ++it)
        (*it)->updateStatus();

    emit layoutChanged();
}

void CFontList::deleteItems(const KFileItemList &items)
{
#ifdef KFI_FONTLIST_DEBUG
    KFI_DBUG << "************** deleteItems " << items.count() << endl;
#endif

    KFileItemList::ConstIterator it(items.begin()),
                                 end(items.end());

    for(; it!=end; ++it)
    {
        CFontItem *font=findFont(*it);

        if(font)
        {
            CFamilyItem *fam=static_cast<CFamilyItem *>(font->parent());

            if(1==fam->fonts().count())
                itsFamilies.remove(fam);
            else
                fam->removeFont(font);
            itsFonts.remove(*it);
        }
    }

    emit layoutChanged();
}

void CFontList::addItem(const KFileItem *item)
{
    CFontItem *font=findFont(item);

    if(!font)
    {
        QString family,
                style;

        decompose(getName(item), family, style);

        CFamilyItem *fam=findFamily(family, true);

        if(fam)
        {
            font=new CFontItem(fam, item, style);

            fam->addFont(font);
            itsFonts.insert(item, font);
        }
#ifdef KFI_FONTLIST_DEBUG
        else
            KFI_DBUG << "                  Could not locate family!" << endl;
#endif
    }
#ifdef KFI_FONTLIST_DEBUG
    else
        KFI_DBUG << "                  Font already exists!" << endl;
#endif
}

CFamilyItem * CFontList::findFamily(const QString &familyName, bool create)
{
    CFamilyItem                        *fam=NULL;
    QList<CFamilyItem *>::ConstIterator it(itsFamilies.begin()),
                                        end(itsFamilies.end());

    for(; it!=end && !fam; ++it)
        if((*it)->name()==familyName)
            fam=(*it);

    if(!fam && create)
    {
        fam=new CFamilyItem(*this, familyName);
        itsFamilies.append(fam);
    }

    return fam;
}

CFontItem * CFontList::findFont(const KFileItem *item)
{
    return itsFonts.contains(item)
            ? itsFonts[item]
            : NULL;
}

void CFontList::touchThumbnails()
{
#ifdef KFI_SAVE_PIXMAPS
    QList<CFamilyItem *>::ConstIterator it(itsFamilies.begin()),
                                        end(itsFamilies.end());

    for(; it!=end; ++it)
        (*it)->touchThumbnail();
#endif
}

inline bool matchString(const QString &str, const QString &pattern)
{
    return pattern.isEmpty() || -1!=str.indexOf(pattern, 0, Qt::CaseInsensitive);
}

CFontListSortFilterProxy::CFontListSortFilterProxy(QObject *parent, QAbstractItemModel *model)
                        : QSortFilterProxyModel(parent),
                          itsMgtMode(false),
                          itsGroup(NULL),
                          itsFilterCriteria(CFontFilter::CRIT_FAMILY),
                          itsFcQuery(NULL)
{
    setSourceModel(model);
    setSortCaseSensitivity(Qt::CaseInsensitive);
    setFilterKeyColumn(0);
    setDynamicSortFilter(true);
    itsTimer=new QTimer(this);
    connect(itsTimer, SIGNAL(timeout()), SLOT(timeout()));
    itsTimer->setSingleShot(true);
}

QVariant CFontListSortFilterProxy::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid())
        return QVariant();

    QModelIndex    index(mapToSource(idx));
    CFontModelItem *mi=static_cast<CFontModelItem *>(index.internalPointer());

    switch(role)
    {
        case Qt::ToolTipRole:
            if(itsMgtMode)
                if(mi->isFamily())
                {
                    CFamilyItem *fam=static_cast<CFamilyItem *>(index.internalPointer());
                    QList<CFontItem *>::ConstIterator it(fam->fonts().begin()),
                                                      end(fam->fonts().end());
                    QStringList                       allFiles;
                    QString                           tip("<h3>"+fam->name()+"</h3>");
                    int                               size(0);
                    bool                              markMatch(CFontFilter::CRIT_FONTCONFIG==itsFilterCriteria);

                    tip+=i18n("<p><b>Status:</b> %1</p>", CFamilyItem::ENABLED==fam->status()
                                                            ? i18n("Enabled")
                                                            : CFamilyItem::DISABLED==fam->status()
                                                                ? i18n("Disabled")
                                                                : i18n("Partial"));
                    tip+="<p><b>";
                    tip+=i18n("All Files:");
                    tip+="</b><ul>";

                    for(; it!=end; ++it)
                    {
                        allFiles+=(*it)->files();
                        size+=(*it)->size();
                    }

                    qSort(allFiles);
                    QStringList::ConstIterator fit(allFiles.begin()),
                                               fend(allFiles.end());

                    for(; fit!=fend; ++fit)
                        if(markMatch && (*fit)==itsFilterFcText)
                            tip+="<li><b>"+(*fit)+"</b></li>";
                        else
                            tip+="<li>"+(*fit)+"</li>";

                    tip+="</ul></p>";
                    tip+=i18n("<p><b>Total File Size:</b> %1</p>", KGlobal::locale()->formatByteSize(size));
                    return tip;
                }
                else
                {
                    CFontItem   *font=static_cast<CFontItem *>(index.internalPointer());
                    QString     tip("<h3>"+font->name()+"</h3>");
                    QStringList files(font->files());
                    bool        markMatch(CFontFilter::CRIT_FONTCONFIG==itsFilterCriteria);

                    tip+=i18n("<p><b>Status:</b> %1</p>", font->isEnabled()
                                                            ? i18n("Enabled")
                                                            : i18n("Disabled"));
                    tip+="<p><b>";
                    tip+=i18n("Files:");
                    tip+="</b><ul>";

                    qSort(files);
                    QStringList::ConstIterator fit(files.begin()),
                                               fend(files.end());

                    for(; fit!=fend; ++fit)
                        if(markMatch && (*fit)==itsFilterFcText)
                            tip+="<li><b>"+(*fit)+"</b></li>";
                        else
                            tip+="<li>"+(*fit)+"</li>";

                    tip+="</ul></p>";
                    tip+=i18n("<p><b>Total File Size:</b> %1</p>", KGlobal::locale()->formatByteSize(font->size()));
                    return tip;
                }
            break;
        case Qt::FontRole:
            if(COL_FONT==index.column())
            {
                bool sys(mi->isSystem()),
                     disabled( (mi->isFont() && !(static_cast<CFontItem *>(index.internalPointer()))->isEnabled()) ||
                               (mi->isFamily() &&
                                 CFamilyItem::DISABLED==(static_cast<CFamilyItem *>(index.internalPointer()))->status()));
                if(sys||disabled)
                {
                    QFont font;
                    font.setItalic(disabled);
                    font.setBold(sys);
                    return font;
                }
            }
            break;
        case Qt::DisplayRole:
            if(COL_FONT==index.column())
                if(mi->isFamily())
                {
                    CFamilyItem *fam=static_cast<CFamilyItem *>(index.internalPointer());

                    return i18n("%1 [%2]", fam->name(), fam->fontCount());
                }
                else
                    return (static_cast<CFontItem *>(index.internalPointer()))->style();
            break;
        case Qt::DecorationRole:
            if(mi->isFamily())
            {
                CFamilyItem *fam=static_cast<CFamilyItem *>(index.internalPointer());

                switch(index.column())
                {
                    case COL_FONT:
                        return SmallIcon(fam->icon(), 0,
                                         CFamilyItem::ENABLED==fam->status()
                                            ? K3Icon::DefaultState
                                            : K3Icon::DisabledState);
                        break;
                    case COL_STATUS:
                        switch(fam->status())
                        {
                            case CFamilyItem::PARTIAL:
                                return SmallIcon("button_ok", 0, K3Icon::DisabledState);
                            case CFamilyItem::ENABLED:
                                return SmallIcon("button_ok");
                            case CFamilyItem::DISABLED:
                                return SmallIcon("button_cancel");
                        }
                        break;
                    default:
                        break;
                }
            }
            else
                if(COL_STATUS==index.column())
                    return SmallIcon( (static_cast<CFontItem *>(index.internalPointer()))->isEnabled()
                                      ? "button_ok" : "button_cancel", 10);
        default:
            break;
    }
    return QVariant();
}

bool CFontListSortFilterProxy::acceptFont(CFontItem *fnt, bool checkFontText) const
{
    if((fnt->isBitmap() && !itsMgtMode) ||
       (fnt->isHidden() && !itsMgtMode))
        return false;

    if(itsGroup && (CGroupListItem::ALL!=itsGroup->type() || (!filterText().isEmpty() && checkFontText)))
    {
        bool fontMatch(!checkFontText);

        if(!fontMatch)
            switch(itsFilterCriteria)
            {
                case CFontFilter::CRIT_FONTCONFIG:
                    fontMatch=fnt->files().contains(itsFilterFcText);
                    break;
                case CFontFilter::CRIT_STYLE:
                    fontMatch=matchString(fnt->style(), itsFilterText);
                    break;
                case CFontFilter::CRIT_FILENAME:
                {
                    QStringList::ConstIterator it(fnt->files().begin()),
                                               end(fnt->files().end());

                    for(; it!=end && !fontMatch; ++it)
                    {
                        QString file(Misc::getFile(*it));
                        int     pos(Misc::isHidden(file) ? 1 : 0);

                        if(pos==file.indexOf(itsFilterText, pos, Qt::CaseInsensitive))
                            fontMatch=true;
                    }
                    break;
                }
                case CFontFilter::CRIT_LOCATION:
                {
                    QStringList::ConstIterator it(fnt->files().begin()),
                                               end(fnt->files().end());

                    for(; it!=end && !fontMatch; ++it)
                        if(0==Misc::getDir(*it).indexOf(itsFilterText, 0, Qt::CaseInsensitive))
                            fontMatch=true;
                    break;
                }
                default:
                    break;
            }

        return fontMatch && itsGroup->hasFont(fnt);
    }

    return true;
}

bool CFontListSortFilterProxy::acceptFamily(CFamilyItem *fam) const
{
    if(CFamilyItem::DISABLED==fam->status() && !itsMgtMode)
        return false;

    QList<CFontItem *>::ConstIterator it(fam->fonts().begin()),
                                      end(fam->fonts().end());
    bool                              familyMatch(CFontFilter::CRIT_FAMILY==itsFilterCriteria &&
                                                  matchString(fam->name(), itsFilterText));

    for(; it!=end; ++it)
        if(acceptFont(*it, !familyMatch))
            return true;
    return false;
}

bool CFontListSortFilterProxy::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index(sourceModel()->index(sourceRow, 0, sourceParent));

    if(index.isValid())
    {
        CFontModelItem *mi=static_cast<CFontModelItem *>(index.internalPointer());

        if(mi->isFont())
        {
            CFontItem *font=static_cast<CFontItem *>(index.internalPointer());

            return acceptFont(font, !(CFontFilter::CRIT_FAMILY==itsFilterCriteria &&
                                      matchString(font->family(), itsFilterText)));
        }
        else
            return acceptFamily(static_cast<CFamilyItem *>(index.internalPointer()));
    }

    return false;
}

bool CFontListSortFilterProxy::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    if(left.isValid() && right.isValid())
    {
        CFontModelItem *lmi=static_cast<CFontModelItem *>(left.internalPointer()),
                       *rmi=static_cast<CFontModelItem *>(right.internalPointer());

        if(lmi->isFont()<rmi->isFont())
            return true;

        if(lmi->isFont())
        {
            CFontItem *lfi=static_cast<CFontItem *>(left.internalPointer()),
                      *rfi=static_cast<CFontItem *>(right.internalPointer());

            if(COL_STATUS==filterKeyColumn())
            {
                if(lfi->isEnabled()<rfi->isEnabled() ||
                  (lfi->isEnabled()==rfi->isEnabled() &&
                   lfi->displayStyleInfo()<rfi->displayStyleInfo()))
                    return true;
            }
            else
                if(lfi->displayStyleInfo()<rfi->displayStyleInfo())
                    return true;
        }
        else
        {
            CFamilyItem *lfi=static_cast<CFamilyItem *>(left.internalPointer()),
                        *rfi=static_cast<CFamilyItem *>(right.internalPointer());

            if(COL_STATUS==filterKeyColumn())
            {
                if(lfi->status()<rfi->status() ||
                  (lfi->status()==rfi->status() && QString::localeAwareCompare(lfi->name(), rfi->name())<0))
                    return true;
            }
            else
                if(QString::localeAwareCompare(lfi->name(), rfi->name())<0)
                    return true;
        }
    }

    return false;
}

void CFontListSortFilterProxy::setFilterGroup(CGroupListItem *grp)
{
    if(grp!=itsGroup)
    {
//        bool wasNull=!itsGroup;

        itsGroup=grp;

       // if(!(wasNull && itsGroup && CGroupListItem::ALL==itsGroup->type()))
            clear();
    }
}

void CFontListSortFilterProxy::setFilterText(const QString &text)
{
    if(text!=itsFilterText)
    {
        //
        // If we are filtering on file location, then expand ~ to /home/user, etc.
        if (CFontFilter::CRIT_LOCATION==itsFilterCriteria && !text.isEmpty() && ('~'==text[0] || '$'==text[0]))
            if('~'==text[0])
                itsFilterText=1==text.length()
                    ? QDir::homePath()
                    : QString(text).replace(0, 1, QDir::homePath());
            else
                itsFilterText=replaceEnvVar(text);
        else
            itsFilterText=text;

        if(itsFilterText.isEmpty())
        {
            itsTimer->stop();
            timeout();
        }
        else
            itsTimer->start(CFontFilter::CRIT_FONTCONFIG==itsFilterCriteria ? 750 : 400);
    }
}

void CFontListSortFilterProxy::setFilterCriteria(CFontFilter::ECriteria crit)
{
    if(crit!=itsFilterCriteria)
    {
        itsFilterCriteria=crit;
        itsFilterFcText=QString();
        if(CFontFilter::CRIT_LOCATION==itsFilterCriteria)
            setFilterText(itsFilterText);
        itsTimer->stop();
        timeout();
    }
}

void CFontListSortFilterProxy::setMgtMode(bool on)
{
    if(on!=itsMgtMode)
    {
        itsMgtMode=on;
        clear();
    }
}

void CFontListSortFilterProxy::timeout()
{
    if(CFontFilter::CRIT_FONTCONFIG==itsFilterCriteria)
    {
        int     commaPos=itsFilterText.indexOf(',');
        QString query(itsFilterText);

        if(-1!=commaPos)
        {
            QString style(query.mid(commaPos+1));
            query=query.left(commaPos);
            query=query.trimmed();
            query+=":style=";
            style=style.trimmed();
            query+=style;
        }
        else
            query=query.trimmed();

        itsFilterFcText=QString();

        if(!itsFcQuery)
        {
            itsFcQuery=new CFcQuery(this);
            connect(itsFcQuery, SIGNAL(finished()), SLOT(fcResults()));
        }

        itsFcQuery->run(query);
    }
    else
    {
        clear();
        emit refresh();
    }
}

void CFontListSortFilterProxy::fcResults()
{
    if(CFontFilter::CRIT_FONTCONFIG==itsFilterCriteria)
    {
        itsFilterFcText=itsFcQuery->file();
        clear();
        emit refresh();
    }
}

class CFontListViewDelegate : public QItemDelegate
{
    public:

    CFontListViewDelegate(QObject *p, QSortFilterProxyModel *px) : QItemDelegate(p), itsProxy(px) { }
    virtual ~CFontListViewDelegate() { }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &idx) const
    {
        QItemDelegate::paint(painter, option, idx);

        QModelIndex index(itsProxy->mapToSource(idx));

        if(index.isValid() && (static_cast<CFontModelItem *>(index.internalPointer()))->isFamily() &&
           COL_PREVIEW==index.column())
        {
            CFamilyItem *fam=static_cast<CFamilyItem *>(index.internalPointer());

            if(fam->regularFont())
            {
                const QPixmap *pix=fam->regularFont()->pixmap();

                if(pix)
                    if(Qt::RightToLeft==QApplication::layoutDirection())
                        painter->drawPixmap(option.rect.x()-(pix->width()-option.rect.width()),
                                            option.rect.y(), *pix);
                    else
                        painter->drawPixmap(option.rect.x(), option.rect.y(), *pix);
            }
        }
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &idx) const
    {
        QModelIndex index(itsProxy->mapToSource(idx));

        return (index.isValid() && (static_cast<CFontModelItem *>(index.internalPointer()))->isFamily())
            ? QSize(CFontList::previewSize(), CFontList::previewSize())
            : QItemDelegate::sizeHint(option, index);
    }

    private:

    QSortFilterProxyModel *itsProxy;
};

#define COL_FONT_SIZE "FontNameColSize"

CFontListView::CFontListView(QWidget *parent, CFontList *model)
             : QTreeView(parent),
               itsProxy(new CFontListSortFilterProxy(this, model)),
               itsModel(model),
               itsAllowDrops(false)
{
    setModel(itsProxy);
    setItemDelegate(new CFontListViewDelegate(this, itsProxy));
    itsModel=model;
    resizeColumnToContents(COL_STATUS);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSortingEnabled(true);
    sortByColumn(COL_FONT, Qt::AscendingOrder);
    setAllColumnsShowFocus(true);
    setAlternatingRowColors(true);
    setAcceptDrops(true);
    setDropIndicatorShown(false);
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    header()->setClickable(true);
    header()->setSortIndicatorShown(true);
    setColumnHidden(COL_STATUS, true);
    connect(this, SIGNAL(collapsed(const QModelIndex &)), SLOT(itemCollapsed(const QModelIndex &)));
    connect(header(), SIGNAL(sectionClicked(int)), SLOT(setSortColumn(int)));
    connect(itsProxy, SIGNAL(refresh()), SIGNAL(refresh()));

    setWhatsThis(i18n("<p>This list shows your installed fonts. The fonts are grouped by family, and the"
                      " number in square brackets represents the number of styles that the family is "
                      " avaiable in. e.g.</p>"
                      "<ul>"
                        "<li>Times [4]"
                          "<ul><li>Regular</li>"
                              "<li>Bold</li>"
                              "<li>Bold Italic</li>"
                              "<li>Italic</li>"
                          "</ul>"
                        "</li>"
                      "<ul>"));

    itsStdMenu=new QMenu(this);
    itsDeleteAct=itsStdMenu->addAction(KIcon("editdelete"), i18n("Delete..."),
                                       this, SIGNAL(del()));
    itsPrintAct=itsStdMenu->addAction(KIcon("fileprint"), i18n("Print..."),
                                      this, SIGNAL(print()));
    itsViewAct=itsStdMenu->addAction(KIcon("kfontview"), i18n("Open in Font Viewer..."),
                                      this, SLOT(view()));
    itsStdMenu->addSeparator();
    QAction *reloadAct=itsStdMenu->addAction(KIcon("reload"), i18n("Reload"), this, SIGNAL(reload()));

    itsMgtMenu=new QMenu(this);
    itsMgtMenu->addAction(itsDeleteAct);
    itsMgtMenu->addSeparator();
    itsEnableAct=itsMgtMenu->addAction(KIcon("enablefont"), i18n("Enable..."),
                                       this, SIGNAL(enable()));
    itsDisableAct=itsMgtMenu->addAction(KIcon("disablefont"), i18n("Disable..."),
                                        this, SIGNAL(disable()));
    itsMgtMenu->addSeparator();
    itsMgtMenu->addAction(itsPrintAct);
    itsMgtMenu->addAction(itsViewAct);
    itsMgtMenu->addSeparator();
    itsMgtMenu->addAction(reloadAct);
}

void CFontListView::readConfig(KConfig &cfg)
{
    setColumnWidth(COL_FONT, cfg.readEntry(COL_FONT_SIZE, 200));
}

void CFontListView::writeConfig(KConfig &cfg)
{
    cfg.writeEntry(COL_FONT_SIZE, columnWidth(COL_FONT));
}

void CFontListView::getFonts(CJobRunner::ItemList &urls, QStringList &fontNames, QSet<Misc::TFont> *fonts,
                             bool *hasSys, bool selected, bool getEnabled, bool getDisabled)
{
    QModelIndexList   selectedItems(selected ? selectedIndexes() : allIndexes());
    QSet<CFontItem *> usedFonts;
    QModelIndex       index;

    foreach(index, selectedItems)
        if(index.isValid())
        {
            QModelIndex realIndex(itsProxy->mapToSource(index));

            if(realIndex.isValid())
                if((static_cast<CFontModelItem *>(realIndex.internalPointer()))->isFont())
                {
                    CFontItem *font=static_cast<CFontItem *>(realIndex.internalPointer());

                    addFont(font, urls, fontNames, fonts, hasSys, usedFonts,
                            getEnabled, getDisabled);
                }
                else
                {
                    CFamilyItem *fam=static_cast<CFamilyItem *>(realIndex.internalPointer());

                    for(int ch=0; ch<fam->fontCount(); ++ch)
                    {
                        QModelIndex child(itsProxy->mapToSource(index.child(ch, 0)));

                        if(child.isValid() &&
                          (static_cast<CFontModelItem *>(child.internalPointer()))->isFont())
                        {
                            CFontItem *font=static_cast<CFontItem *>(child.internalPointer());

                            addFont(font, urls, fontNames, fonts, hasSys, usedFonts,
                                    getEnabled, getDisabled);
                        }
                    }
                }
        }

    fontNames=CFontList::compact(fontNames);
}

void CFontListView::getPrintableFonts(QSet<Misc::TFont> &items, bool selected)
{
    QModelIndexList selectedItems(selected ? selectedIndexes() : allIndexes());
    QModelIndex     index;

    foreach(index, selectedItems)
    {
        CFontItem *font=NULL;

        if(index.isValid() && 0==index.column())
        {
            QModelIndex realIndex(itsProxy->mapToSource(index));

            if(realIndex.isValid())
                if((static_cast<CFontModelItem *>(realIndex.internalPointer()))->isFont())
                    font=static_cast<CFontItem *>(realIndex.internalPointer());
                else
                {
                    CFamilyItem *fam=static_cast<CFamilyItem *>(realIndex.internalPointer());
                    font=fam->regularFont();
                }
        }

        if(font && Misc::printable(font->mimetype()) && font->isEnabled())
            items.insert(Misc::TFont(font->family(), font->styleInfo()));
    }
}

void CFontListView::setFilterGroup(CGroupListItem *grp)
{
    CGroupListItem *oldGrp(itsProxy->filterGroup());

    itsProxy->setFilterGroup(grp);
    itsAllowDrops=grp && !grp->isStandard();

    if(!Misc::root())
    {
        bool refreshStats(false);

        if(!grp || !oldGrp)
            refreshStats=true;
        else
        {
            // Check to see whether we have changed from listing all fonts,
            // listing just system or listing personal fonts.
            CGroupListItem::EType aType(CGroupListItem::STANDARD==grp->type() ||
                                        CGroupListItem::ALL==grp->type() ||
                                        CGroupListItem::UNCLASSIFIED==grp->type()
                                            ? CGroupListItem::STANDARD : grp->type()),
                                  bType(CGroupListItem::STANDARD==oldGrp->type() ||
                                        CGroupListItem::ALL==oldGrp->type() ||
                                        CGroupListItem::UNCLASSIFIED==oldGrp->type()
                                            ? CGroupListItem::STANDARD : oldGrp->type());
            refreshStats=aType!=bType;
        }

        if(refreshStats)
            itsModel->refresh(!grp || !grp->isPersonal(),
                              !grp || !grp->isSystem());
    }
}

void CFontListView::refreshFilter()
{
    itsProxy->clear();
}

void CFontListView::filterText(const QString &text)
{
    itsProxy->setFilterText(text);
}

void CFontListView::filterCriteria(int crit)
{
    itsProxy->setFilterCriteria((CFontFilter::ECriteria)crit);
}

void CFontListView::stats(int &enabled, int &disabled, int &partial)
{
    enabled=disabled=partial=0;

    for(int i=0; i<itsProxy->rowCount(); ++i)
    {
        QModelIndex idx(itsProxy->index(i, 0, QModelIndex()));

        if(!idx.isValid())
            break;

        QModelIndex sourceIdx(itsProxy->mapToSource(idx));

        if(!sourceIdx.isValid())
            break;

        if((static_cast<CFontModelItem *>(sourceIdx.internalPointer()))->isFamily())
            switch((static_cast<CFamilyItem *>(sourceIdx.internalPointer()))->status())
            {
                case CFamilyItem::ENABLED:
                    enabled++;
                    break;
                case CFamilyItem::DISABLED:
                    disabled++;
                    break;
                case CFamilyItem::PARTIAL:
                    partial++;
                    break;
            }
    }
}

void CFontListView::selectedStatus(bool &enabled, bool &disabled)
{
    QModelIndexList selected(selectedItems());
    QModelIndex     index;

    enabled=disabled=false;

    foreach(index, selected)
    {
        QModelIndex realIndex(itsProxy->mapToSource(index));

        if(realIndex.isValid())
            if((static_cast<CFontModelItem *>(realIndex.internalPointer()))->isFamily())
                switch((static_cast<CFamilyItem *>(realIndex.internalPointer()))->status())
                {
                    case CFamilyItem::ENABLED:
                        enabled=true;
                        break;
                    case CFamilyItem::DISABLED:
                        disabled=true;
                        break;
                    case CFamilyItem::PARTIAL:
                        enabled=true;
                        disabled=true;
                        break;
                }
            else
                if((static_cast<CFontItem *>(realIndex.internalPointer()))->isEnabled())
                    enabled=true;
                else
                    disabled=true;
        if(enabled && disabled)
            break;
    }
}

QModelIndexList CFontListView::allFonts()
{
    QModelIndexList rv;
    int             rowCount(itsProxy->rowCount());

    for(int i=0; i<rowCount; ++i)
    {
        QModelIndex idx(itsProxy->index(i, 0, QModelIndex()));
        int         childRowCount(itsProxy->rowCount(idx));

        for(int j=0; j<childRowCount; ++j)
        {
            QModelIndex child(itsProxy->index(j, 0, idx));

            if(child.isValid())
                rv.append(itsProxy->mapToSource(child));
        }
    }

    return rv;
}

void CFontListView::setMgtMode(bool on)
{
    if(on!=itsProxy->mgtMode())
    {
        setDragEnabled(on);
        setDragDropMode(on ? QAbstractItemView::DragDrop : QAbstractItemView::DropOnly);
        setColumnHidden(COL_STATUS, !on);

        itsModel->setAllowDisabled(on);
        itsProxy->setMgtMode(on);
    }
}

void CFontListView::selectFirstFont()
{
    if(0==selectedItems().count())
        for(int i=0; i<NUM_COLS; ++i)
        {
            QModelIndex idx(itsProxy->index(0, i, QModelIndex()));

            if(idx.isValid())
                selectionModel()->select(idx, QItemSelectionModel::Select);
        }
}

void CFontListView::setSortColumn(int col)
{
    if(col!=itsProxy->filterKeyColumn())
    {
        itsProxy->setFilterKeyColumn(col);
        itsProxy->clear();
    }
}

void CFontListView::selectionChanged(const QItemSelection &selected,
                                     const QItemSelection &deselected)
{
    QAbstractItemView::selectionChanged(selected, deselected);

    //
    // Go throgh current selection, and for any 'font' items that are selected,
    // ensure 'family' item is not...
    QModelIndexList        selectedItems(selectedIndexes()),
                           deselectList;
    QModelIndex            index;
    QSet<CFontModelItem *> selectedFamilies;
    bool                   en(false),
                           dis(false);

    foreach(index, selectedItems)
        if(index.isValid())
        {
            QModelIndex realIndex(itsProxy->mapToSource(index));

            if(realIndex.isValid())
                if((static_cast<CFontModelItem *>(realIndex.internalPointer()))->isFont())
                {
                    CFontItem *font=static_cast<CFontItem *>(realIndex.internalPointer());

                    if(font->isEnabled())
                        en=true;
                    else
                        dis=true;
                    if(!selectedFamilies.contains(font->parent()))
                    {
                        selectedFamilies.insert(font->parent());

                        for(int i=0; i<NUM_COLS; ++i)
                            deselectList.append(itsProxy->mapFromSource(
                                                      itsModel->createIndex(font->parent()->rowNumber(),
                                                                            i, font->parent())));
                    }
                }
                else
                    switch((static_cast<CFamilyItem *>(realIndex.internalPointer()))->status())
                    {
                        case CFamilyItem::ENABLED:
                            en=true;
                            break;
                        case CFamilyItem::DISABLED:
                            dis=true;
                            break;
                        case CFamilyItem::PARTIAL:
                            en=dis=true;
                            break;
                    }
        }

    if(deselectList.count())
        foreach(index, deselectList)
            selectionModel()->select(index, QItemSelectionModel::Deselect);

    emit itemSelected(selectedItems.count()
                ? itsProxy->mapToSource(selectedItems.last())
                : QModelIndex(), en, dis);
}

void CFontListView::itemCollapsed(const QModelIndex &idx)
{
    if(idx.isValid())
    {
        QModelIndex index(itsProxy->mapToSource(idx));

        if(index.isValid() && (static_cast<CFontModelItem *>(index.internalPointer()))->isFamily())
        {
            CFamilyItem                       *fam=static_cast<CFamilyItem *>(index.internalPointer());
            QList<CFontItem *>::ConstIterator it(fam->fonts().begin()),
                                              end(fam->fonts().end());

            for(; it!=end; ++it)
                for(int i=0; i<NUM_COLS; ++i)
                    selectionModel()->select(itsProxy->mapFromSource(itsModel->createIndex((*it)->rowNumber(),
                                                                                           i, *it)),
                                             QItemSelectionModel::Deselect);
        }
    }
}

void CFontListView::view()
{
    // Number of fonts user has selected, before we ask if they really want to view them all...
    static const int constMaxBeforePrompt=10;

    QModelIndexList   selectedItems(selectedIndexes());
    QModelIndex       index;
    QSet<CFontItem *> fonts;

    foreach(index, selectedItems)
    {
        QModelIndex realIndex(itsProxy->mapToSource(index));

        if(realIndex.isValid())
            if((static_cast<CFontModelItem *>(realIndex.internalPointer()))->isFont())
            {
                CFontItem *font(static_cast<CFontItem *>(realIndex.internalPointer()));

                fonts.insert(font);
            }
            else
            {
                CFontItem *font((static_cast<CFamilyItem *>(realIndex.internalPointer()))->regularFont());

                if(font)
                    fonts.insert(font);
            }
    }

    if(fonts.count() &&
       (fonts.count()<constMaxBeforePrompt ||
        KMessageBox::Yes==KMessageBox::questionYesNo(this, i18n("Open all %1 fonts in font viewer?", fonts.count()))))
    {
        QSet<CFontItem *>::ConstIterator it(fonts.begin()),
                                         end(fonts.end());

        for(; it!=end; ++it)
        {
            KProcess proc;

            proc << KFI_APP << "-v" << (*it)->url().prettyUrl().toUtf8();
            proc.start(KProcess::DontCare);
        }
    }
}

QModelIndexList CFontListView::allIndexes()
{
    QModelIndexList rv;
    int             rowCount(itsProxy->rowCount());

    for(int i=0; i<rowCount; ++i)
    {
        QModelIndex idx(itsProxy->index(i, 0, QModelIndex()));
        int         childRowCount(itsProxy->rowCount(idx));

        rv.append(idx);

        for(int j=0; j<childRowCount; ++j)
        {
            QModelIndex child(itsProxy->index(j, 0, idx));

            if(child.isValid())
                rv.append(child);
        }
    }

    return rv;
}

void CFontListView::startDrag(Qt::DropActions supportedActions)
{
    QModelIndexList indexes(selectedIndexes());

    if (indexes.count())
    {
        QMimeData *data = model()->mimeData(indexes);
        if (!data)
            return;

        QModelIndex index(itsProxy->mapToSource(indexes.first()));
        const char  *icon="font_bitmap";

        if(index.isValid())
        {
            CFontItem *font=(static_cast<CFontModelItem *>(index.internalPointer()))->isFont()
                                ? static_cast<CFontItem *>(index.internalPointer())
                                : (static_cast<CFamilyItem *>(index.internalPointer()))->regularFont();

            if(font && !font->isBitmap())
                if("application/x-font-type1"==font->mimetype())
                    icon="font_type1";
                else
                    icon="font_truetype";
        }

        QPoint  hotspot;
        QPixmap pix(DesktopIcon(icon, K3Icon::SizeMedium));

        hotspot.setX(0); // pix.width()/2);
        hotspot.setY(0); // pix.height()/2);

        QDrag *drag = new QDrag(this);
        drag->setPixmap(pix);
        drag->setMimeData(data);
        drag->setHotSpot(hotspot);
        drag->start(supportedActions);
    }
}

void CFontListView::dragEnterEvent(QDragEnterEvent *event)
{
    if(itsAllowDrops && event->provides("text/uri-list")) // "application/x-kde-urilist" ??
        event->acceptProposedAction();
}

void CFontListView::dropEvent(QDropEvent *event)
{
    if(itsAllowDrops && event->provides("text/uri-list"))
    {
        event->acceptProposedAction();

        QList<QUrl>                urls(event->mimeData()->urls());
        QList<QUrl>::ConstIterator it(urls.begin()),
                                   end(urls.end());
        QSet<KUrl>                 kurls;

        for(; it!=end; ++it)
        {
            KMimeType::Ptr mime=KMimeType::findByUrl(*it, 0, false, true);

            if(mime->is("application/x-font-ttf") ||
               mime->is("application/x-font-otf") ||
               mime->is("application/x-font-ttc") ||
               mime->is("application/x-font-type1") ||
               mime->is("fonts/package") ||
               (!isColumnHidden(COL_STATUS) && 
                (mime->is("application/x-font-pcf") ||
                 mime->is("application/x-font-bdf") ||
                 mime->is("fonts/group"))))
                kurls.insert(*it);
        }

        if(kurls.count())
            emit fontsDropped(kurls);
    }
}

void CFontListView::contextMenuEvent(QContextMenuEvent *ev)
{
    bool valid(indexAt(ev->pos()).isValid());

    itsDeleteAct->setEnabled(valid);

    if(isColumnHidden(COL_STATUS))
    {
        itsPrintAct->setEnabled(valid);
        itsViewAct->setEnabled(valid);
        itsStdMenu->popup(ev->globalPos());
    }
    else
    {
        bool            en(false),
                        dis(false);
        QModelIndexList selectedItems(selectedIndexes());
        QModelIndex     index;

        foreach(index, selectedItems)
        {
            QModelIndex realIndex(itsProxy->mapToSource(index));

            if(realIndex.isValid())
                if((static_cast<CFontModelItem *>(realIndex.internalPointer()))->isFont())
                {
                    if((static_cast<CFontItem *>(realIndex.internalPointer())->isEnabled()))
                        en=true;
                    else
                        dis=true;
                }
                else
                    switch((static_cast<CFamilyItem *>(realIndex.internalPointer()))->status())
                    {
                        case CFamilyItem::ENABLED:
                            en=true;
                            break;
                        case CFamilyItem::DISABLED:
                            dis=true;
                            break;
                        case CFamilyItem::PARTIAL:
                            en=dis=true;
                            break;
                    }
            if(en && dis)
                break;
        }

        itsEnableAct->setEnabled(dis);
        itsDisableAct->setEnabled(en);
        itsPrintAct->setEnabled(en|dis);
        itsViewAct->setEnabled(en|dis);
        itsMgtMenu->popup(ev->globalPos());
    }
}

}

#include "FontList.moc"
