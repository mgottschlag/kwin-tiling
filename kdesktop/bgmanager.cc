/* vi: ts=8 sts=4 sw=4
 * kate: space-indent on; tab-width 8; indent-width 4; indent-mode cstyle;
 *
 * This file is part of the KDE project, module kdesktop.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 *
 * You can Freely distribute this program under the GNU General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */

#include <config.h>
#include "bgrender.h"
#include "bgmanager.h"
#include "bgdefaults.h"
#include "kdesktopsettings.h"
#include "backgroundadaptor.h"

#include <assert.h>

#include <QTimer>
#include <q3scrollview.h>
//Added by qt3to4:
#include <QPixmap>
#include <QVector>
//Should have been added by qt3to4:
#include <Q3PtrVector>

#include <kiconloader.h>
#include <kconfig.h>
#include <kwin.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kipc.h>
#include <kmenu.h>
#include <kwinmodule.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#ifndef None
#define None 0L
#endif

#include "pixmapserver.h"
#include <QDesktopWidget>
#include <QX11Info>


template class Q3PtrVector<KBackgroundRenderer>;
template class Q3PtrVector<KBackgroundCacheEntry>;
template class QVector<int>;

static Atom prop_root;
static bool properties_inited = false;

const char* KBackgroundManager::backgroundDBusObjectPath = "/Background";

QString pixmapName(int desk)
{
    QString pattern = QString("DESKTOP%1");
#ifdef Q_WS_X11
    int screen_number = DefaultScreen(QX11Info::display());
    if (screen_number) {
        pattern = QString("SCREEN%1-DESKTOP").arg(screen_number) + "%1";
    }
#endif
    return pattern.arg( desk );
}

/**** KBackgroundManager ****/

KBackgroundManager::KBackgroundManager(QWidget *desktop, KWinModule* kwinModule)
    : QObject()
{

    (void) new BackgroundAdaptor( this );
    QDBusConnection::sessionBus().registerObject( backgroundDBusObjectPath, this );

    if( !properties_inited )
    {
        prop_root = XInternAtom(QX11Info::display(), "_XROOTPMAP_ID", False);
        properties_inited = true;
    }
    m_bBgInitDone = false;
    m_bEnabled = true;

    m_pDesktop = desktop;
    if (desktop == 0L)
        desktop = QApplication::desktop()->screen();

    m_Renderer.resize( 1 );
    m_Cache.resize( 1 );

    m_Serial = 0; m_Hash = 0;
    m_pConfig = KGlobal::config();
    m_bExport = m_bCommon = m_bInit = false;
    m_pKwinmodule = kwinModule;
    m_pPixmapServer = new KPixmapServer();
    m_xrootpmap = None;

    for (int i=0; i<m_Renderer.size(); i++)
    {
	m_Cache[i] = new KBackgroundCacheEntry;
        m_Cache[i]->pixmap = 0L;
        m_Cache[i]->hash = 0;
        m_Cache[i]->exp_from = -1;
        m_Renderer[i] = new KVirtualBGRenderer(i,m_pConfig);
        connect(m_Renderer[i], SIGNAL(imageDone(int)), SLOT(slotImageDone(int)));
        m_Renderer[i]->enableTiling( true ); // optimize
    }

    configure();

    m_pTimer = new QTimer(this);
    connect(m_pTimer, SIGNAL(timeout()), SLOT(slotTimeout()));
    m_pTimer->start( 60000 );

    connect(m_pKwinmodule, SIGNAL(currentDesktopChanged(int)),
	    SLOT(slotChangeDesktop(int)));
    connect(m_pKwinmodule, SIGNAL(numberOfDesktopsChanged(int)),
	    SLOT(slotChangeNumberOfDesktops(int)));

    connect( kapp->desktop(), SIGNAL( resized( int )), SLOT( desktopResized())); // RANDR support
}


KBackgroundManager::~KBackgroundManager()
{
    for (int i=0; i<m_Renderer.size(); i++)
        delete m_Renderer[i];

    //delete m_pConfig; Very bad idea, this is KGlobal::config !
    delete m_pPixmapServer;
    delete m_pTimer;

    // clear the Esetroot properties, as the pixmaps they refer to are going away...
    Pixmap pm = None;
    Atom type;
    int format;
    unsigned long length, after;
    unsigned char* data_root;
    if( XGetWindowProperty( QX11Info::display(), QX11Info::appRootWindow(), prop_root, 0L, 1L, False, AnyPropertyType,
	&type, &format, &length, &after, &data_root) == Success && data_root != NULL )
    {
	if (type == XA_PIXMAP)
	    pm = *((Pixmap*)data_root);
        XFree( data_root );
    }
    // only if it's our pixmap
    if( pm == m_xrootpmap )
	XDeleteProperty(QX11Info::display(), QX11Info::appRootWindow(), prop_root);
    m_xrootpmap = None;

    if (m_bExport)
        return;

    for (int i=0; i<m_Cache.size(); i++)
    {
	if (m_Cache[i]->pixmap)
	    delete m_Cache[i]->pixmap;
    }
}


void KBackgroundManager::applyExport(bool exp)
{
    if (exp == m_bExport)
	return;

    // If export mode changed from true -> false, remove all shared pixmaps.
    // If it changed false -> true force a redraw because the current screen
    // image might not have an associated pixmap in the cache.
    if (!exp)
    {
	for (int i=0; i<m_Cache.size(); i++)
	    removeCache(i);
    } else
	m_Hash = 0;

    m_bExport = exp;
}


void KBackgroundManager::applyCommon(bool common)
{
    if (common == m_bCommon)
	return;
    m_bCommon = common;

    // If common changed from false -> true, remove all cache entries, except
    // at index 0 if exports are on.
    if (m_bCommon)
    {
	if (!m_bExport)
	    removeCache(0);
	for (int i=1; i<m_Cache.size(); i++)
	    removeCache(i);
    }
}


void KBackgroundManager::applyCache(bool limit, int size)
{
    m_bLimitCache = limit;
    m_CacheLimit = size;
    freeCache(0);
}


/*
 * Call this when the configuration has changed.
 * This method is exported with DBus.
 */
void KBackgroundManager::configure()
{
    // Global settings
    m_pConfig->reparseConfiguration();
    KDesktopSettings::self()->readConfig();

    // Read individual settings
    KVirtualBGRenderer *r;
    for (int i=0; i<m_Renderer.size(); i++)
    {
        r = m_Renderer[i];
        int ohash = r->hash();
        r->load(i,false);
        if ((r->hash() != ohash))
            removeCache(i);
    }

    applyCommon(KDesktopSettings::commonDesktop());

    bool limit = KDesktopSettings::limitCache();
    int size = KDesktopSettings::cacheSize() * 1024;
    applyCache(limit, size);

    // Repaint desktop
    slotChangeDesktop(0);
}


int KBackgroundManager::realDesktop()
{
    int desk = m_pKwinmodule->currentDesktop();
    if (desk) desk--;
    return desk;
}


int KBackgroundManager::effectiveDesktop()
{
    return m_bCommon ? 0 : realDesktop();
}


/*
 * Number of desktops changed
 */
void KBackgroundManager::slotChangeNumberOfDesktops(int num)
{
    if (m_Renderer.size() ==  num)
	return;

    if (m_Renderer.size() >  num)
    {
	for (int i=num; i<m_Renderer.size(); i++)
	{
	    if (m_Renderer[i]->isActive())
		m_Renderer[i]->stop();
	    delete m_Renderer[i];
	    removeCache(i);
	}
	for (int i=num; i<m_Renderer.size(); i++)
	    delete m_Cache[i];
	m_Renderer.resize(num);
	m_Cache.resize(num);
    } else
    {
	// allocate new renderers and caches
	int oldsz = m_Renderer.size();
	m_Renderer.resize(num);
	m_Cache.resize(num);
	for (int i=oldsz; i<num; i++)
	{
	    m_Cache[i] = new KBackgroundCacheEntry;
	    m_Cache[i]->pixmap = 0L;
	    m_Cache[i]->hash = 0;
	    m_Cache[i]->exp_from = -1;
	    m_Renderer[i] = new KVirtualBGRenderer(i,m_pConfig);
	    connect(m_Renderer[i], SIGNAL(imageDone(int)), SLOT(slotImageDone(int)));
            m_Renderer[i]->enableTiling( true ); // optimize
	}
    }
}

/*
 * Call this when the desktop has been changed.
 * Desk is in KWin convention: [1..desks], instead of [0..desks-1].
 * 0 repaints the current desktop.
 */
void KBackgroundManager::slotChangeDesktop(int desk)
{
    if (desk == 0)
	desk = realDesktop();
    else
	desk--;

    // Lazy initialisation of # of desktops
    if ( desk >= m_Renderer.size())
	slotChangeNumberOfDesktops( m_pKwinmodule->numberOfDesktops() );

    int edesk = effectiveDesktop();
    m_Serial++;

    // If the background is the same: do nothing
    if (m_Hash == m_Renderer[edesk]->hash())
    {
	exportBackground(m_Current, desk);
        return;
    }

    // If we have the background already rendered: set it
    for (int i=0; i<m_Cache.size(); i++)
    {
	if (!m_Cache[i]->pixmap)
	    continue;
	if (m_Cache[i]->hash != m_Renderer[edesk]->hash())
	    continue;
//        kDebug() << "slotChangeDesktop i=" << i << endl;
	setPixmap(m_Cache[i]->pixmap, m_Cache[i]->hash, i);
	m_Cache[i]->atime = m_Serial;
	exportBackground(i, desk);
	return;
    }

    // Do we have this or an identical config already running?
    for (int i=0; i<m_Renderer.size(); i++)
    {
        if ((m_Renderer[i]->hash() == m_Renderer[edesk]->hash()) &&
            (m_Renderer[i]->isActive()))
            return;
    }

    renderBackground(edesk);
}


/*
 * Share a desktop pixmap.
 */
void KBackgroundManager::exportBackground(int pixmap, int desk)
{
    if (!m_bExport || (m_Cache[desk]->exp_from == pixmap))
        return;

    m_Cache[desk]->exp_from = pixmap;
    m_pPixmapServer->add(pixmapName(desk+1),
	    m_Cache[pixmap]->pixmap);
    KIPC::sendMessageAll(KIPC::BackgroundChanged, desk+1);
}


/*
 * Paint the pixmap to the root window.
 */
void KBackgroundManager::setPixmap(QPixmap *pm, int hash, int desk)
{
    if (m_pDesktop)
    {
       Q3ScrollView* sv = dynamic_cast<Q3ScrollView*>( m_pDesktop );
       if ( sv ) {
         // Qt eats repaint events in this case :-((
         sv->viewport()->update();
       }
       QPalette pal = m_pDesktop->palette();
       pal.setBrush(QPalette::Background, QBrush(*pm));
       m_pDesktop->setPalette(pal);
      // m_pDesktop->setErasePixmap(*pm);
       //m_pDesktop->repaint();
       static bool root_cleared = false;
       if( !root_cleared )
       { // clear the root window pixmap set by kdm
          root_cleared = true;
	  QTimer::singleShot( 0, this, SLOT( clearRoot()));
          // but make the pixmap visible until m_pDesktop is visible
          QApplication::desktop()->screen()->setErasePixmap(*pm);
          QApplication::desktop()->screen()->update();
       }
    }
    else
    {
       QApplication::desktop()->screen()->setErasePixmap(*pm);
       QApplication::desktop()->screen()->update();
    }

     // and export it via Esetroot-style for gnome/GTK apps to share in the pretties
    Pixmap bgPm = pm->handle(); // fetch the actual X handle to it
    //kDebug() << "Esetroot compat:  setting pixmap to " << bgPm << endl;

    // don't set the ESETROOT_PMAP_ID property - that would result in possible XKillClient()
    // done on kdesktop

    XChangeProperty(QX11Info::display(), QX11Info::appRootWindow(), prop_root, XA_PIXMAP, 32, PropModeReplace,
                    (unsigned char *) &bgPm, 1);
    m_xrootpmap = bgPm;

    m_Hash = hash;
    m_Current = desk;
}

void KBackgroundManager::clearRoot()
{
   QApplication::desktop()->screen()->setErasePixmap( QPixmap());
   QApplication::desktop()->screen()->update();
}

/*
 * Start the render of a desktop background.
 */
void KBackgroundManager::renderBackground(int desk)
{
    KVirtualBGRenderer *r = m_Renderer[desk];
    if (r->isActive())
    {
        kDebug() << "renderer " << desk << " already active" << endl;
        return;
    }

    r->start();
}


/*
 * This slot is called when a renderer is done.
 */
void KBackgroundManager::slotImageDone(int desk)
{
    QPixmap *pm = new QPixmap();
    KVirtualBGRenderer *r = m_Renderer[desk];
    bool do_cleanup = true;

    *pm = r->pixmap();
    // If current: paint it
    bool current = (r->hash() == m_Renderer[effectiveDesktop()]->hash());
    if (current)
    {
        setPixmap(pm, r->hash(), desk);
        if (!m_bBgInitDone)
        {
            m_bBgInitDone = true;
            emit initDone();
            QTimer::singleShot( 30000, this, SLOT( saveImages()));
            do_cleanup = false;
        }
    }
    if (m_bExport || !m_bCommon)
	addCache(pm, r->hash(), desk);
    else
        delete pm;

    if (current)
        exportBackground(desk, realDesktop());

    if( do_cleanup )
    {
        r->saveCacheFile();
        r->cleanup();
    }
}


void KBackgroundManager::saveImages()
{
    for (int i=0; i<m_Renderer.size(); i++)
    {
        m_Renderer[i]->saveCacheFile();
        m_Renderer[i]->cleanup();
    }
}

/*
 * Size in bytes of a QPixmap. For use in the pixmap cache.
 */
int KBackgroundManager::pixmapSize(QPixmap *pm)
{
    return (pm->width() * pm->height()) * ((pm->depth() + 7) / 8);
}


/*
 * Total size of the pixmap cache.
 */
int KBackgroundManager::cacheSize()
{
    int total = 0;
    for (int i=0; i<m_Cache.size(); i++)
    {
        if (m_Cache[i]->pixmap)
            total += pixmapSize(m_Cache[i]->pixmap);
    }
    return total;
}


/*
 * Remove an entry from the pixmap cache.
 */
void KBackgroundManager::removeCache(int desk)
{
    if (m_bExport)
	m_pPixmapServer->remove(pixmapName(desk+1));
    else
        delete m_Cache[desk]->pixmap;
    m_Cache[desk]->pixmap = 0L;
    m_Cache[desk]->hash = 0;
    m_Cache[desk]->exp_from = -1;
    m_Cache[desk]->atime = 0;

    // Remove cache entries pointing to the removed entry
    for (int i=0; i<m_Cache.size(); i++)
    {
	if (m_Cache[i]->exp_from == desk)
	{
	    assert(m_bExport);
	    m_Cache[i]->exp_from = -1;
	    m_pPixmapServer->remove(pixmapName(i+1));
	}
    }
}


/*
 * Try to free up to size bytes from the cache.
 */
bool KBackgroundManager::freeCache(int size)
{
    if (m_bExport || !m_bLimitCache)
	return true;

    // If it doesn't fit at all, return now.
    if (size > m_CacheLimit)
	return false;

    // If cache is too full, purge it (LRU)
    while (size+cacheSize() > m_CacheLimit)
    {
	int j, min;
	min = m_Serial+1; j = 0;
	for (int i=0; i<m_Cache.size(); i++)
	{
	    if (m_Cache[i]->pixmap && (m_Cache[i]->atime < min))
	    {
		min = m_Cache[i]->atime;
		j = i;
	    }
	}
	removeCache(j);
    }
    return true;
}


/*
 * Try to add a pixmap to the pixmap cache. We don't use QPixmapCache here
 * because if we're exporting pixmaps, this needs special care.
 */
void KBackgroundManager::addCache(QPixmap *pm, int hash, int desk)
{
    if (m_Cache[desk]->pixmap)
	removeCache(desk);

    if (m_bLimitCache && !m_bExport && !freeCache(pixmapSize(pm)))
    {
	// pixmap does not fit in cache
	delete pm;
	return;
    }

    m_Cache[desk]->pixmap = pm;
    m_Cache[desk]->hash = hash;
    m_Cache[desk]->atime = m_Serial;
    m_Cache[desk]->exp_from = -1;
    exportBackground(desk, desk);
}

/*
 * Called every minute to check if we need to rerun a background program.
 * or change a wallpaper.
 */
void KBackgroundManager::slotTimeout()
{
    QVector<int> running(m_Renderer.size());
    running.fill(0);

    int NumDesks = m_Renderer.size();
    if (m_bCommon)
       NumDesks = 1;

    int edesk = effectiveDesktop();

    for (int i=0; i<NumDesks; i++)
    {
        KVirtualBGRenderer *r = m_Renderer[i];
        bool change = false;

        if (r->needProgramUpdate())
        {
            r->programUpdate();
            change = true;
        }

        if (r->needWallpaperChange())
        {
            r->changeWallpaper();
            change = true;
        }

        if (change && (i == edesk))
	{
	    running[i] = r->hash();
            r->start();
	}
    }
}

// Return a valid desk number.
int KBackgroundManager::validateDesk(int desk)
{
    if (desk > (int)m_Renderer.size())
        slotChangeNumberOfDesktops( m_pKwinmodule->numberOfDesktops() );

    if ( (desk <= 0) || (desk > (int)m_Renderer.size()) )
        return realDesktop();

    return desk - 1;
}

// DBus exported
// Return current wallpaper for specified desk.
// 0 is for the current visible desktop.
QString KBackgroundManager::currentWallpaper(int desk)
{
    //TODO Is the behaviour of this function appropriate for multiple screens?
    KBackgroundRenderer *r = m_Renderer[validateDesk(desk)]->renderer(0);

    return r->currentWallpaper();
}

// DBus exported
void KBackgroundManager::changeWallpaper()
{
    KVirtualBGRenderer *r = m_Renderer[effectiveDesktop()];

    r->changeWallpaper();
    slotChangeDesktop(0);
}

// DBus exported
void KBackgroundManager::setExport(int _export)
{
    kDebug() << "KBackgroundManager enabling exports.\n";
    applyExport(_export);
    slotChangeDesktop(0);
}

// DBus exported
void KBackgroundManager::setCommon(int common)
{
    applyCommon(common);
    KDesktopSettings::setCommonDesktop( m_bCommon );
    KDesktopSettings::writeConfig();
    slotChangeDesktop(0);
}

// DBus exported
void KBackgroundManager::setWallpaper(QString wallpaper, int mode)
{
    //TODO Is the behaviour of this function appropriate for multiple screens?
    for (unsigned i=0; i < m_Renderer[effectiveDesktop()]->numRenderers(); ++i)
    {
        KBackgroundRenderer *r = m_Renderer[effectiveDesktop()]->renderer(i);
        r->stop();
        r->setWallpaperMode(mode);
        r->setMultiWallpaperMode(KBackgroundSettings::NoMulti);
        r->setWallpaper(wallpaper);
        r->writeSettings();
    }
    slotChangeDesktop(0);
}

void KBackgroundManager::setWallpaper(QString wallpaper)
{
    //TODO Is the behaviour of this function appropriate for multiple screens?
    KBackgroundRenderer *r = m_Renderer[effectiveDesktop()]->renderer(0);
    int mode = r->wallpaperMode();
    if (mode == KBackgroundSettings::NoWallpaper)
       mode = KBackgroundSettings::Tiled;
    setWallpaper(wallpaper, mode);
}

// DBus exported
// Returns the filenames of all wallpaper entries for specified desk
// 0 is for current visible desktop.
QStringList KBackgroundManager::wallpaperFiles(int desk)
{
    //TODO Is the behaviour of this function appropriate for multiple screens?
    KBackgroundRenderer *r = m_Renderer[validateDesk(desk)]->renderer(0);

    return r->wallpaperFiles();
}

// DBus exported
// Returns the list of wallpaper entries (viewable in background slide
// show window) for specified desk.  0 is for current visible desktop.
QStringList KBackgroundManager::wallpaperList(int desk)
{
    //TODO Is the behaviour of this function appropriate for multiple screens?
    KBackgroundRenderer *r = m_Renderer[validateDesk(desk)]->renderer(0);;

    return r->wallpaperList();
}

// DBus exported
void KBackgroundManager::setCache( int bLimit, int size )
{
    applyCache( bLimit, size*1024 );
    KDesktopSettings::setLimitCache( (bool) bLimit );
    KDesktopSettings::setCacheSize( size );
    KDesktopSettings::writeConfig();
}

// DBus exported
void KBackgroundManager::setWallpaper(int desk, QString wallpaper, int mode)
{
    int sdesk = validateDesk(desk);

    //TODO Is the behaviour of this function appropriate for multiple screens?
    for (unsigned i=0; i < m_Renderer[sdesk]->numRenderers(); ++i)
    {
        KBackgroundRenderer *r = m_Renderer[sdesk]->renderer(i);

        setCommon(false);   // Force each desktop to have it's own wallpaper

        r->stop();
        r->setWallpaperMode(mode);
        r->setMultiWallpaperMode(KBackgroundSettings::NoMulti);
        r->setWallpaper(wallpaper);
        r->writeSettings();
    }
    slotChangeDesktop(sdesk);
}

void KBackgroundManager::repaintBackground()
{
    if (m_pDesktop)
       m_pDesktop->repaint();
    else
       QApplication::desktop()->screen()->update();
}

void KBackgroundManager::desktopResized()
{
    for (int i=0; i<m_Renderer.size(); i++)
    {
        KVirtualBGRenderer * r = m_Renderer[i];
        if( r->isActive())
            r->stop();
        removeCache(i);
        // make the renderer update its desktop size
        r->desktopResized();
    }
    m_Hash = 0;
    if( m_pDesktop )
        m_pDesktop->resize( kapp->desktop()->size());
    // Repaint desktop
    slotChangeDesktop(0);
}

// DBus exported
void KBackgroundManager::setColor(const QColor & c, bool isColorA)
{
    //TODO Is the behaviour of this function appropriate for multiple screens?
    for (unsigned i=0; i < m_Renderer[effectiveDesktop()]->numRenderers(); ++i)
    {
        KBackgroundRenderer *r = m_Renderer[effectiveDesktop()]->renderer(i);
        r->stop();

        if (isColorA)
            r->setColorA(c);
        else
            r->setColorB(c);

        int mode = r->backgroundMode();
        if (mode == KBackgroundSettings::Program)
        mode = KBackgroundSettings::Flat;

        if (!isColorA && (mode == KBackgroundSettings::Flat))
        mode = KBackgroundSettings::VerticalGradient;
        r->setBackgroundMode(mode);

        r->writeSettings();
    }
    slotChangeDesktop(0);
}

void KBackgroundManager::setBackgroundEnabled( const bool enable )
{
  if (m_bEnabled == enable)
    return;

  m_bEnabled= enable;

  int NumDesks = m_Renderer.size();
  if (m_bCommon)
    NumDesks = 1;

  for (int i=0; i<NumDesks; i++)
    {
      m_Renderer[i]->setEnabled(enable);
    }
  slotChangeDesktop(0);
}

#include "bgmanager.moc"
