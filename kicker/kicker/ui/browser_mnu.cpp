/*****************************************************************

Copyright (c) 2001 Matthias Elter <elter@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include <QDir>
#include <QDrag>
#include <QDragMoveEvent>
#include <QEvent>
#include <QDropEvent>
#include <QMenu>
#include <QMenuItem>
#include <QDragEnterEvent>
#include <QMouseEvent>
#include <QPixmap>
#include <QVector>
#include <QPainter>

#include <kapplication.h>
#include <kdebug.h>
#include <kdesktopfile.h>
#include <kdirwatch.h>
#include <kfileitem.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <kio/global.h>
#include <klocale.h>
#include <kmimetype.h>
#include <konq_operations.h>
#include <kpanelmenu.h>
#include <kprocess.h>
#include <krun.h>
#include <ksimpleconfig.h>
#include <kstringhandler.h>
#include <kauthorized.h>
#include <kworkspace.h>

#include "kickerSettings.h"

#include "browser_mnu.h"
#include "browser_mnu.moc"

#define CICON(a) (*_icons)[a]

QMap<QString, QPixmap> *PanelBrowserMenu::_icons = 0;

PanelBrowserMenu::PanelBrowserMenu(QString path, QWidget *parent, int startid)
    : KPanelMenu(path, parent)
    , _mimecheckTimer(0)
    , _startid(startid)
    , _dirty(false)
    , _filesOnly(false)
{
    _lastpress = QPoint(-1, -1);
    setAcceptDrops(true); // Should depend on permissions of path.

    // we are not interested for dirty events on files inside the
    // directory (see slotClearIfNeeded)
    connect( &_dirWatch, SIGNAL(dirty(const QString&)),
             this, SLOT(slotClearIfNeeded(const QString&)) );
    connect( &_dirWatch, SIGNAL(created(const QString&)),
             this, SLOT(slotClear()) );
    connect( &_dirWatch, SIGNAL(deleted(const QString&)),
             this, SLOT(slotClear()) );

    kDebug() << "PanelBrowserMenu Constructor " << path << endl;
}

PanelBrowserMenu::~PanelBrowserMenu()
{
    kDebug() << "PanelBrowserMenu Destructor " << path() << endl;
}

void PanelBrowserMenu::slotClearIfNeeded(const QString& p)
{
    if (p == path())
        slotClear();
}

void PanelBrowserMenu::initialize()
{
    _lastpress = QPoint(-1, -1);

    // don't change menu if already visible
    if (isVisible())
        return;

    if (_dirty) {
        // directory content changed while menu was visible
        slotClear();
        setInitialized(false);
        _dirty = false;
    }

    if (initialized()) return;
    setInitialized(true);

    // start watching if not already done
    if (!_dirWatch.contains(path()))
        _dirWatch.addDir( path() );

    // setup icon map
    initIconMap();

    // clear maps
    _filemap.clear();
    _mimemap.clear();

    QDir::Filters filter = QDir::Dirs | QDir::Files;
    if (KickerSettings::showHiddenFiles())
    {
        filter |= QDir::Hidden;
    }

    QDir dir(path(), QString(), QDir::DirsFirst | QDir::Name | QDir::IgnoreCase, filter);

    // does the directory exist?
    if (!dir.exists()) {
        insertItem(i18n("Failed to Read Folder"));
	return;
    }

    // get entry list
    QFileInfoList list = dir.entryInfoList();

    KUrl url;
    url.setPath(path());
    if (!KAuthorized::authorizeURLAction("list", KUrl(), url))
    {
        insertItem(i18n("Not Authorized to Read Folder"));
        return;
    }

    // insert file manager and terminal entries
    // only the first part menu got them
    if(_startid == 0 && !_filesOnly) {
       // FIXME: no more menu titles so NOW WHAT?!
       // insertTitle(path());
       insertItem(CICON("kfm"), i18n("Open in File Manager"), this, SLOT(slotOpenFileManager()));
        if (KAuthorized::authorizeKAction("shell_access"))
            insertItem(CICON("terminal"), i18n("Open in Terminal"), this, SLOT(slotOpenTerminal()));
    }


    bool first_entry = true;
    bool dirfile_separator = false;
    unsigned int item_count = 0;
    int run_id = _startid;

    // get list iterator
    QFileInfoList::iterator it = list.begin();

    // jump to startid
    it += _startid;

    // iterate over entry list
    for (; it != list.end(); ++it)
    {
        // bump id
        run_id++;

        QFileInfo fi = *it;
        // handle directories
        if (fi.isDir())
        {
            QString name = fi.fileName();

            // ignore . and .. entries
            if (name == "." || name == "..") continue;

            QPixmap icon;
            QString path = fi.absoluteFilePath();

            // parse .directory if it does exist
            if (QFile::exists(path + "/.directory")) {

                KSimpleConfig c(path + "/.directory", true);
                c.setDesktopGroup();
                QString iconPath = c.readEntry("Icon");

                if ( iconPath.startsWith("./") )
                    iconPath = path + '/' + iconPath.mid(2);

                icon = KGlobal::iconLoader()->loadIcon(iconPath,
                                                       K3Icon::Small, K3Icon::SizeSmall,
                                                       K3Icon::DefaultState, 0, true);
                if(icon.isNull())
                    icon = CICON("folder");
                name = c.readEntry("Name", name);
            }

            // use cached folder icon for directories without special icon
            if (icon.isNull())
                icon = CICON("folder");

            // insert separator if we are the first menu entry
            if(first_entry) {
                if (_startid == 0 && !_filesOnly)
                    insertSeparator();
                first_entry = false;
            }

            // append menu entry
            PanelBrowserMenu *submenu = new PanelBrowserMenu(path, this);
            submenu->_filesOnly = _filesOnly;
            append(icon, name, submenu);

            // bump item count
            item_count++;

            dirfile_separator = true;
        }
        // handle files
        else if(fi.isFile())
        {
            QString name = fi.fileName();
            QString title = KIO::decodeFileName(name);

            QPixmap icon;
            QString path = fi.absoluteFilePath();

            bool mimecheck = false;

            // .desktop files
            if(KDesktopFile::isDesktopFile(path))
            {
                KSimpleConfig c(path, true);
                c.setDesktopGroup();
                title = c.readEntry("Name", title);

                QString s = c.readEntry("Icon");
                if(!_icons->contains(s)) {
                    icon  = KGlobal::iconLoader()->loadIcon(s, K3Icon::Small, K3Icon::SizeSmall,
                                                            K3Icon::DefaultState, 0, true);

                    if(icon.isNull()) {
                        QString type = c.readEntry("Type", "Application");
                        if (type == "Directory")
                            icon = CICON("folder");
                        else if (type == "Mimetype")
                            icon = CICON("txt");
                        else if (type == "FSDevice")
                            icon = CICON("chardevice");
                        else
                            icon = CICON("exec");
                    }
                    else
                        _icons->insert(s, icon);
                }
                else
                    icon = CICON(s);
            }
            else {
                // set unknown icon
                icon = CICON("unknown");

                // mark for delayed mimetime check
                mimecheck = true;
            }

            // insert separator if we are the first menu entry
            if(first_entry) {
                if(_startid == 0 && !_filesOnly)
                    insertSeparator();
                first_entry = false;
            }

            // insert separator if we we first file after at least one directory
            if (dirfile_separator) {
                insertSeparator();
                dirfile_separator = false;
            }

            // append file entry
            append(icon, title, name, mimecheck);

            // bump item count
            item_count++;
        }

        if (item_count == KickerSettings::maxEntries2())
        {
            // Only insert a "More" item if there are actually more items.
            ++it;
            if( it != list.end() ) {
                insertSeparator();
                append(CICON("kdisknav"), i18n("More"), new PanelBrowserMenu(path(), this, run_id));
            }
            break;
        }
    }

#if 0
    // WABA: tear off handles don't work together with dynamically updated
    // menus. We can't update the menu while torn off, and we don't know
    // when it is torn off.
    if(KGlobalSettings::insertTearOffHandle() && item_count > 0)
        insertTearOffHandle();
#endif

    adjustSize();

    QString dirname = path();

    int maxlen = contentsRect().width() - 40;
    if(item_count == 0)
        maxlen = fontMetrics().width(dirname);

    if (fontMetrics().width(dirname) > maxlen) {
        while ((!dirname.isEmpty()) && (fontMetrics().width(dirname) > (maxlen - fontMetrics().width("..."))))
            dirname = dirname.remove(0, 1);
        dirname.prepend("...");
    }
    setWindowTitle(dirname);

    // setup and start delayed mimetype check timer
    if(_mimemap.count() > 0) {

        if(!_mimecheckTimer)
            _mimecheckTimer = new QTimer(this);

        connect(_mimecheckTimer, SIGNAL(timeout()), SLOT(slotMimeCheck()));
        _mimecheckTimer->start(0);
    }
}

void PanelBrowserMenu::append(const QPixmap &pixmap, const QString &title, const QString &file, bool mimecheck)
{
    // avoid &'s being converted to accelerators
    QString newTitle = title;
    newTitle = KStringHandler::cEmSqueeze( newTitle, fontMetrics(), 20 );
    newTitle.replace("&", "&&");

    // insert menu item
    int id = insertItem(QIcon(pixmap), newTitle);

    // insert into file map
    _filemap.insert(id, file);

    // insert into mimetype check map
    if(mimecheck)
        _mimemap.insert(id, true);
}

void PanelBrowserMenu::append(const QPixmap &pixmap, const QString &title, PanelBrowserMenu *subMenu)
{
    // avoid &'s being converted to accelerators
    QString newTitle = title;
    newTitle = KStringHandler::cEmSqueeze( newTitle, fontMetrics(), 20 );
    newTitle.replace("&", "&&");

    // insert submenu
    insertItem(pixmap, newTitle, subMenu);
    // remember submenu for later deletion
    _subMenus.append(subMenu);
}

void PanelBrowserMenu::mousePressEvent(QMouseEvent *e)
{
    QMenu::mousePressEvent(e);
    _lastpress = e->pos();
}

void PanelBrowserMenu::mouseMoveEvent(QMouseEvent *e)
{
    QMenu::mouseMoveEvent(e);

    if (!(e->buttons() & Qt::LeftButton)) return;
    if(_lastpress == QPoint(-1, -1)) return;

    // DND delay
    if((_lastpress - e->pos()).manhattanLength() < 12) return;

    // get id
    int id = static_cast<QMenuItem*>(actionAt(_lastpress))->id();
    if(!_filemap.contains(id)) return;

    // reset _lastpress
    _lastpress = QPoint(-1, -1);

    // start drag
    KUrl url;
    url.setPath(path() + '/' + _filemap[id]);
    KUrl::List files(url);
    QDrag* drag = new QDrag(this);
    QMimeData* data = new QMimeData;
    files.populateMimeData(data);
    connect(drag, SIGNAL(destroyed(QObject*)), this, SLOT(dragObjectDestroyed(QObject*)));
    drag->setPixmap(iconSet(id).pixmap());
    drag->start();
}

void PanelBrowserMenu::dragObjectDestroyed(QObject* o)
{
    QDrag* drag = dynamic_cast<QDrag*>(o);
    if (drag && drag->target() != this)
    {
        close();
    }
}

void PanelBrowserMenu::dragEnterEvent( QDragEnterEvent *ev )
{
    if (KUrl::List::canDecode(ev->mimeData()))
    {
        ev->accept();
        return;
    }
    KPanelMenu::dragEnterEvent(ev);
}

void PanelBrowserMenu::dragMoveEvent(QDragMoveEvent *ev)
{
    QMouseEvent mev(QEvent::MouseMove, ev->pos(), Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
    QMenu::mouseMoveEvent(&mev);
}

void PanelBrowserMenu::dropEvent( QDropEvent *ev )
{
    KUrl u( path() );
    KFileItem item( u, QString::fromLatin1( "inode/directory" ), KFileItem::Unknown );
    KonqOperations::doDrop( &item, u, ev, this );
    KPanelMenu::dropEvent(ev);
    // ### TODO: Update list
}

void PanelBrowserMenu::slotExec(int id)
{
    KWorkSpace::propagateSessionManager();

    if(!_filemap.contains(id)) return;

    KUrl url;
    url.setPath(path() + '/' + _filemap[id]);
    new KRun(url, 0, true); // will delete itself
    _lastpress = QPoint(-1, -1);
}

void PanelBrowserMenu::slotOpenTerminal()
{
    KConfig * config = KGlobal::config();
    config->setGroup("General");
    QString term = config->readPathEntry("TerminalApplication", "konsole");

    KProcess proc;
    proc << term;
    if (term == "konsole")
      proc << "--workdir" << path();
    else
      proc.setWorkingDirectory(path());
    proc.start(KProcess::DontCare);
}

void PanelBrowserMenu::slotOpenFileManager()
{
    new KRun(path(), this);
}

void PanelBrowserMenu::slotMimeCheck()
{
    // get the first map entry
    QMap<int, bool>::Iterator it = _mimemap.begin();

    // no mime types left to check -> stop timer
    if(it == _mimemap.end()) {
        _mimecheckTimer->stop();
        return;
    }

    int id = it.key();
    QString file = _filemap[id];

    _mimemap.erase(it);

    KUrl url;
    url.setPath( path() + '/' + file );

//    KMimeType::Ptr mt = KMimeType::findByURL(url, 0, true, false);
//    QString icon(mt->icon(url, true));
    QString icon = KMimeType::iconNameForURL( url );
//    kDebug() << url.url() << ": " << icon << endl;

    file = KStringHandler::cEmSqueeze( file, fontMetrics(), 20 );
    file.replace("&", "&&");

    if(!_icons->contains(icon)) {
        QPixmap pm = SmallIcon(icon);
        if( pm.height() > 16 )
        {
            QPixmap cropped( 16, 16 );
	    QPainter p(&cropped);
            p.drawPixmap(0, 0, pm, 0, 0, 16, 16);
            pm = cropped;
        }
        _icons->insert(icon, pm);
        changeItem(id, pm, file);
    }
    else
        changeItem(id, CICON(icon), file);
}

void PanelBrowserMenu::slotClear()
{
    // no need to watch any further
    if (_dirWatch.contains(path()))
        _dirWatch.removeDir( path() );

    // don't change menu if already visible
    if (isVisible()) {
        _dirty = true;
        return;
    }
    KPanelMenu::slotClear();

    QVector<PanelBrowserMenu*>::iterator itEnd = _subMenus.end();
    for (QVector<PanelBrowserMenu*>::iterator it = _subMenus.begin();
         it != itEnd;
         ++it)
    {
        delete *it;
    }
    _subMenus.clear(); // deletes submenus
}

void PanelBrowserMenu::initIconMap()
{
    if(_icons) return;

//    kDebug() << "PanelBrowserMenu::initIconMap" << endl;

    _icons = new QMap<QString, QPixmap>;

    _icons->insert("folder", SmallIcon("folder"));
    _icons->insert("unknown", SmallIcon("mime_empty"));
    _icons->insert("folder_open", SmallIcon("folder_open"));
    _icons->insert("kdisknav", SmallIcon("kdisknav"));
    _icons->insert("kfm", SmallIcon("kfm"));
    _icons->insert("terminal", SmallIcon("terminal"));
    _icons->insert("txt", SmallIcon("txt"));
    _icons->insert("exec", SmallIcon("exec"));
    _icons->insert("chardevice", SmallIcon("chardevice"));
}

// vim: sw=4 et
