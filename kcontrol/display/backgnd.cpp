/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kcmdisplay.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 * 
 * Based on old backgnd.cpp:
 *
 * Copyright (c)  Martin R. Jones 1996
 * Converted to a kcc module by Matthias Hoelzer 1997
 * Gradient backgrounds by Mark Donohoe 1997
 * Pattern backgrounds by Stephan Kulow 1998
 * Randomizing & dnd & new display modes by Matej Koss 1998
 * 
 * You can Freely distribute this program under the GNU General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */

#include <qobject.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qgroupbox.h>
#include <qcombobox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qdragobject.h>
#include <qhbox.h>
#include <qevent.h>

#include <kapp.h>
#include <kconfig.h>
#include <kwm.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kiconloader.h>
#include <kcolorbtn.h>
#include <klocale.h>
#include <kipc.h>
#include <kfiledialog.h>
#include <kpixmap.h>

#include <bgdefaults.h>
#include <bgrender.h>
#include <bgdialogs.h>
#include <backgnd.h>


/**** KBGMonitor ****/

void KBGMonitor::dropEvent(QDropEvent *e)
{
    if (!QUriDrag::canDecode(e))
	return;

    QStringList uris;
    if (QUriDrag::decodeLocalFiles(e, uris) && (uris.count() > 0)) {
	QString uri = *uris.begin();
	uri.prepend('/');
	emit imageDropped(uri);
    }
}
				        
					 
void KBGMonitor::dragEnterEvent(QDragEnterEvent *e)
{
    e->accept(QImageDrag::canDecode(e)|| QUriDrag::canDecode(e));
}                                                                                                             


/**** KGlobalBackgroundSettings ****/

KGlobalBackgroundSettings::KGlobalBackgroundSettings()
{
    dirty = false;

    readSettings();
}

QString KGlobalBackgroundSettings::deskName(int desk)
{
    if (desk < _maxDesktops)
	return m_Names[desk];
    return QString();
}


void KGlobalBackgroundSettings::setDeskName(int desk, QString name)
{
    if ((desk >= _maxDesktops) || (name == m_Names[desk]))
	return;
    dirty = true;
    m_Names[desk] = name;
}


void KGlobalBackgroundSettings::setCacheSize(int size)
{
    if (size == m_CacheSize)
	return;
    dirty = true;
    m_CacheSize = size;
}


void KGlobalBackgroundSettings::setLimitCache(bool limit)
{
    if (limit == m_bLimitCache)
	return;
    dirty = true;
    m_bLimitCache = limit;
}


void KGlobalBackgroundSettings::setCommonBackground(bool common)
{
    if (common == m_bCommon)
	return;
    dirty = true;
    m_bCommon = common;
}


void KGlobalBackgroundSettings::setDockPanel(bool dock)
{
    if (dock == m_bDock)
	return;
    dirty = true;
    m_bDock = dock;
}


void KGlobalBackgroundSettings::setExportBackground(bool _export)
{
    if (_export == m_bExport)
	return;
    dirty = true;
    m_bExport = _export;
}


void KGlobalBackgroundSettings::readSettings()
{
    KConfig cfg("kdesktoprc");
    cfg.setGroup("Background Common");
    m_bCommon = cfg.readBoolEntry("CommonDesktop", _defCommon);
    m_bDock = cfg.readBoolEntry("Dock", _defDock);
    m_bExport = cfg.readBoolEntry("Export", _defExport);
    m_bLimitCache = cfg.readBoolEntry("LimitCache", _defLimitCache);
    m_CacheSize = cfg.readNumEntry("CacheSize", _defCacheSize);

    m_Names.clear();
    if (KWM::isKWMInitialized()) {
	m_bKWM = true;
	for (int i=0; i<_maxDesktops; i++)
	    m_Names.append(KWM::desktopName(i+1));
    } else {
	m_bKWM = false;
	for (int i=0; i<_maxDesktops; i++)
	    m_Names.append(i18n("Desktop %1").arg(i+1));
    }

    dirty = false;
}


void KGlobalBackgroundSettings::writeSettings()
{
    if (!dirty)
	return;

    KConfig cfg("kdesktoprc");
    cfg.setGroup("Background Common");
    cfg.writeEntry("CommonDesktop", m_bCommon);
    cfg.writeEntry("Dock", m_bDock);
    cfg.writeEntry("Export", m_bExport);
    cfg.writeEntry("LimitCache", m_bLimitCache);
    cfg.writeEntry("CacheSize", m_CacheSize);

    if (m_bKWM) {
	for (int i=0; i<_maxDesktops; i++)
	    if (m_Names[i] != KWM::desktopName(i+1))
		KWM::setDesktopName(i+1, m_Names[i]);
    }

    dirty = false;
}



/**** KBackground ****/

KBackground::KBackground(QWidget *parent, Mode m)
    : KDisplayModule(parent, m)
{
    if (m == Init)
	return;

    m_pConfig = new KConfig("kdesktoprc");
    m_pDirs = KGlobal::dirs();

    // Top layout
    QGridLayout *top = new QGridLayout(this, 2, 2);
    top->setSpacing(10); top->setMargin(10);
    top->setColStretch(0, 1);
    top->setColStretch(1, 2);

    // A nice button size. Translators can adapt this
    QPushButton *pbut = new QPushButton(i18n("abcdefgh"), this);
    QSize bsize = pbut->sizeHint();
    delete pbut;

    // Desktop chooser at (0, 0)
    QGroupBox *group = new QGroupBox(i18n("Desktop"), this);
    top->addWidget(group, 0, 0);
    QVBoxLayout *vbox = new QVBoxLayout(group);
    vbox->setMargin(10); 
    vbox->setSpacing(10);
    vbox->addSpacing(10);
    m_pDeskList = new QListBox(group);
    connect(m_pDeskList, SIGNAL(clicked(int)), SLOT(slotSelectDesk(int)));
    vbox->addWidget(m_pDeskList);
    m_pCBCommon = new QCheckBox(i18n("&Common Background"), group);
    vbox->addWidget(m_pCBCommon);
    connect(m_pCBCommon, SIGNAL(toggled(bool)), SLOT(slotCommonDesk(bool)));

    // Background settings
    group = new QGroupBox(i18n("Background"), this);
    top->addWidget(group, 1, 0);
    QGridLayout *grid = new QGridLayout(group, 5, 2);
    grid->setMargin(10); 
    grid->setSpacing(10);
    grid->addRowSpacing(0, 10);
    grid->setColStretch(0, 0);

    QLabel *lbl = new QLabel(i18n("&Mode:"), group);
    lbl->setFixedSize(lbl->sizeHint());
    grid->addWidget(lbl, 1, 0, Qt::AlignLeft);
    m_pBackgroundBox = new QComboBox(group);
    connect(m_pBackgroundBox, SIGNAL(activated(int)), SLOT(slotBGMode(int)));
    lbl->setBuddy(m_pBackgroundBox);
    grid->addWidget(m_pBackgroundBox, 1, 1);

    lbl = new QLabel(i18n("Color &1:"), group);
    lbl->setFixedSize(lbl->sizeHint());
    grid->addWidget(lbl, 2, 0, Qt::AlignLeft);
    m_pColor1But = new KColorButton(group);
    connect(m_pColor1But, SIGNAL(changed(const QColor &)),
	    SLOT(slotColor1(const QColor &)));
    grid->addWidget(m_pColor1But, 2, 1);
    lbl->setBuddy(m_pColor1But);

    lbl = new QLabel(i18n("Color &2:"), group);
    lbl->setFixedSize(lbl->sizeHint());
    grid->addWidget(lbl, 3, 0, Qt::AlignLeft);
    m_pColor2But = new KColorButton(group);
    connect(m_pColor2But, SIGNAL(changed(const QColor &)),
	    SLOT(slotColor2(const QColor &)));
    grid->addWidget(m_pColor2But, 3, 1);
    lbl->setBuddy(m_pColor2But);

    QHBoxLayout *hbox = new QHBoxLayout();
    grid->addLayout(hbox, 4, 1);
    m_pBGSetupBut = new QPushButton(i18n("&Setup"), group);
    m_pBGSetupBut->setFixedSize(bsize);
    connect(m_pBGSetupBut, SIGNAL(clicked()), SLOT(slotBGSetup()));
    hbox->addWidget(m_pBGSetupBut);
    hbox->addStretch();


    // Preview monitor at (0,1)
    hbox = new QHBoxLayout();
    top->addLayout(hbox, 0, 1);
    lbl = new QLabel(this);
    lbl->setPixmap(locate("data", "kcontrol/pics/monitor.png"));
    lbl->setFixedSize(lbl->sizeHint());
    hbox->addWidget(lbl);
    m_pMonitor = new KBGMonitor(lbl);
    m_pMonitor->setGeometry(20, 10, 157, 111);
    connect(m_pMonitor, SIGNAL(imageDropped(QString)), SLOT(slotImageDropped(QString)));

    // Wallpaper at (1,1)
    group = new QGroupBox(i18n("Wallpaper"), this);
    top->addWidget(group, 1, 1);
    grid = new QGridLayout(group, 6, 2);
    grid->setMargin(10); grid->setSpacing(10);
    grid->addRowSpacing(0, 10);
    grid->addRowSpacing(4, 10);

    lbl = new QLabel(i18n("M&ode:"), group);
    lbl->setFixedSize(lbl->sizeHint());
    grid->addWidget(lbl, 1, 0, Qt::AlignLeft);
    m_pArrangementBox = new QComboBox(group);
    connect(m_pArrangementBox, SIGNAL(activated(int)), SLOT(slotWPMode(int)));
    lbl->setBuddy(m_pArrangementBox);
    grid->addWidget(m_pArrangementBox, 1, 1);

    lbl = new QLabel(i18n("&Wallpaper"), group);
    lbl->setFixedSize(lbl->sizeHint());
    grid->addWidget(lbl, 2, 0, Qt::AlignLeft);
    m_pWallpaperBox = new QComboBox(group);
    lbl->setBuddy(m_pWallpaperBox);
    connect(m_pWallpaperBox, SIGNAL(activated(const QString &)), 
	    SLOT(slotWallpaper(const QString &)));
    grid->addWidget(m_pWallpaperBox, 2, 1);

    hbox = new QHBoxLayout();
    grid->addLayout(hbox, 3, 1);
    m_pBrowseBut = new QPushButton(i18n("&Browse"), group);
    m_pBrowseBut->setFixedSize(bsize);
    connect(m_pBrowseBut, SIGNAL(clicked()), SLOT(slotBrowseWallpaper()));
    hbox->addWidget(m_pBrowseBut);
    hbox->addStretch();

    m_pCBMulti = new QCheckBox(i18n("&Random:"), group);
    m_pCBMulti->setFixedSize(m_pCBMulti->sizeHint());
    connect(m_pCBMulti, SIGNAL(toggled(bool)), SLOT(slotMultiMode(bool)));
    grid->addWidget(m_pCBMulti, 4, 0);
    hbox = new QHBoxLayout();
    grid->addLayout(hbox, 4, 1);
    m_pMSetupBut = new QPushButton(i18n("S&etup"), group);
    m_pMSetupBut->setFixedSize(bsize);
    connect(m_pMSetupBut, SIGNAL(clicked()), SLOT(slotSetupMulti()));
    hbox->addWidget(m_pMSetupBut);
    hbox->addStretch();

    m_Desk = KWM::currentDesktop() - 1;
    m_Max = KWM::numberOfDesktops();
    m_pGlobals = new KGlobalBackgroundSettings();
    for (int i=0; i<m_Max; i++) {
	m_Renderer[i] = new KBackgroundRenderer(i);
	connect(m_Renderer[i], SIGNAL(imageDone(int)), SLOT(slotPreviewDone(int)));
    }

    init();
    apply();
}


/*
 * Fill all check/listboxen
 */
void KBackground::init()
{
    int i;

    // Desktop names
    for (i=0; i<KWM::numberOfDesktops(); i++)
	m_pDeskList->insertItem(m_pGlobals->deskName(i));
    
    // Background modes: make sure these match with kdesktop/bgrender.cc !!
    m_pBackgroundBox->insertItem(i18n("Flat"));
    m_pBackgroundBox->insertItem(i18n("Pattern"));
    m_pBackgroundBox->insertItem(i18n("Background Program"));
    m_pBackgroundBox->insertItem(i18n("Horizontal Gradient"));
    m_pBackgroundBox->insertItem(i18n("Vertical Gradient"));
    m_pBackgroundBox->insertItem(i18n("Pyramid Gradient"));
    m_pBackgroundBox->insertItem(i18n("Pipecross Gradient"));
    m_pBackgroundBox->insertItem(i18n("Elliptic Gradient"));

    // Wallpapers
    QStringList lst = m_pDirs->findAllResources("wallpaper", "*", false, true);
    for (i=0; i<(int)lst.count(); i++) {
	int n = lst[i].findRev('/');
	QString s = lst[i].mid(n+1);
	m_pWallpaperBox->insertItem(s);
	m_Wallpaper[s] = i;
    }

    // Wallpaper tilings: again they must match the ones from bgrender.cc
    m_pArrangementBox->insertItem(i18n("No Wallpaper"));
    m_pArrangementBox->insertItem(i18n("Centred"));
    m_pArrangementBox->insertItem(i18n("Tiled"));
    m_pArrangementBox->insertItem(i18n("Center Tiled"));
    m_pArrangementBox->insertItem(i18n("Centred Maxpect"));
    m_pArrangementBox->insertItem(i18n("Scaled"));
}


void KBackground::apply()
{
    int desk = m_Desk;
    if (m_pGlobals->commonBackground())
	desk = 0;
    KBackgroundRenderer *r = m_Renderer[desk];

    // Desktop names
    if (m_pGlobals->commonBackground()) {
	m_pCBCommon->setChecked(true);
	m_pDeskList->setEnabled(false);
    } else  {
	m_pCBCommon->setChecked(false);
	m_pDeskList->setEnabled(true);
	m_pDeskList->setCurrentItem(m_Desk);
    }

    // Background mode
    m_pBackgroundBox->setCurrentItem(r->backgroundMode());
    m_pColor1But->setColor(r->colorA());
    m_pColor2But->setColor(r->colorB());
    switch (r->backgroundMode()) {
    case KBackgroundSettings::Program:
	m_pColor1But->setEnabled(false);
	m_pColor2But->setEnabled(false);
	m_pBGSetupBut->setEnabled(true);
	break;
    case KBackgroundSettings::Flat:
	m_pColor1But->setEnabled(true);
	m_pColor2But->setEnabled(false);
	m_pBGSetupBut->setEnabled(false);
	break;
    case KBackgroundSettings::Pattern:
	m_pColor1But->setEnabled(true);
	m_pColor2But->setEnabled(true);
	m_pBGSetupBut->setEnabled(true);
	break;
    default:
	m_pColor1But->setEnabled(true);
	m_pColor2But->setEnabled(true);
	m_pBGSetupBut->setEnabled(false);
	break;
    }

    // Wallpaper mode
    QString wp = r->wallpaper();
    if (wp.isEmpty())
	wp = QString(" ");
    if (!m_Wallpaper.contains(wp)) {
	int count = m_Wallpaper.count();
	m_Wallpaper[wp] = count;
	m_pWallpaperBox->insertItem(wp);
	m_pWallpaperBox->setCurrentItem(count);
    }
    m_pWallpaperBox->setCurrentItem(m_Wallpaper[wp]);
    m_pArrangementBox->setCurrentItem(r->wallpaperMode());

    // Multi mode
    if (r->multiWallpaperMode() == KBackgroundSettings::NoMulti) {
	m_pCBMulti->setChecked(false);
	m_pWallpaperBox->setEnabled(true);
	m_pBrowseBut->setEnabled(true);
	m_pMSetupBut->setEnabled(false);
    } else {
	m_pCBMulti->setChecked(true);
	m_pWallpaperBox->setEnabled(false);
	m_pBrowseBut->setEnabled(false);
	m_pMSetupBut->setEnabled(true);
    }

    // Start preview render
    r->setPreview(m_pMonitor->size());
    r->start();
}


void KBackground::loadSettings()
{
    // what to do here?
}


void KBackground::applySettings()
{
    m_pGlobals->writeSettings();
    for (int i=0; i<m_Max; i++)
	m_Renderer[i]->writeSettings();

    // notify kdesktop. kdesktop will notify all clients
    KWM::sendKWMCommand("kbgwm_reconfigure");
}


void KBackground::defaultSettings()
{
    int desk = m_Desk;
    if (m_pGlobals->commonBackground())
	desk = 0;
    KBackgroundRenderer *r = m_Renderer[desk];

    if (r->isActive())
	r->stop();
    r->setBackgroundMode(KBackgroundSettings::Flat);
    r->setColorA(_defColorA);
    r->setColorB(_defColorB);
    r->setWallpaperMode(KBackgroundSettings::NoWallpaper);
    r->setMultiWallpaperMode(KBackgroundSettings::NoMulti);
    m_pGlobals->setCommonBackground(_defCommon);
    apply();
}


void KBackground::slotSelectDesk(int desk)
{
    if (desk == m_Desk)
	return;

    if (m_Renderer[m_Desk]->isActive())
	m_Renderer[m_Desk]->stop();
    m_Desk = desk;
    apply();
}


void KBackground::slotCommonDesk(bool common)
{
    if (common == m_pGlobals->commonBackground())
	return;

    m_pGlobals->setCommonBackground(common);
    apply();
}


/*
 * Called from the "Background Mode" combobox.
 */
void KBackground::slotBGMode(int mode)
{
    int desk = m_Desk;
    if (m_pGlobals->commonBackground())
	desk = 0;
    KBackgroundRenderer *r = m_Renderer[desk];

    if (mode == r->backgroundMode())
	return;

    r->stop();
    r->setBackgroundMode(mode);
    apply();
}


/*
 * Called from the "Background Setup" pushbutton.
 */
void KBackground::slotBGSetup()
{
    int desk = m_Desk;
    if (m_pGlobals->commonBackground())
	desk = 0;
    KBackgroundRenderer *r = m_Renderer[desk];

    switch (r->backgroundMode()) {
    case KBackgroundSettings::Pattern:
    {
	KPatternSelectDialog dlg;
	QString cur = r->KBackgroundPattern::name();
	dlg.setCurrent(cur);
	if ((dlg.exec() == QDialog::Accepted) && (dlg.pattern() != cur)) {
	    r->stop();
	    r->setPattern(dlg.pattern());
	    r->start();
	}
	break;
    }
    case KBackgroundSettings::Program:
    {
	KProgramSelectDialog dlg;
	QString cur = r->KBackgroundProgram::name();
	dlg.setCurrent(cur);
	if ((dlg.exec() == QDialog::Accepted) && (dlg.program() != cur)) {
	    r->stop();
	    r->setProgram(dlg.program());
	    r->start();
	}
	break;
    }
    default:
	break;
    }
}


void KBackground::slotColor1(const QColor &color)
{
    int desk = m_Desk;
    if (m_pGlobals->commonBackground())
	desk = 0;
    KBackgroundRenderer *r = m_Renderer[desk];

    if (color == r->colorA())
	return;

    r->stop();
    r->setColorA(color);
    r->start();
}
    

void KBackground::slotColor2(const QColor &color)
{
    int desk = m_Desk;
    if (m_pGlobals->commonBackground())
	desk = 0;
    KBackgroundRenderer *r = m_Renderer[desk];

    if (color == r->colorB())
	return;

    r->stop();
    r->setColorB(color);
    r->start();
}
    

void KBackground::slotImageDropped(QString uri)
{
    qDebug("image dropped: %s", uri.latin1());
}


void KBackground::slotMultiMode(bool multi)
{
    //m_Renderer[m_Desk]->stop();
    //m_Renderer[m_Desk]->setMultiWallpaperMode(mode);
    //m_Renderer[m_Desk]->start();

    if (multi) {
	m_pWallpaperBox->setEnabled(false);
	m_pBrowseBut->setEnabled(false);
	m_pMSetupBut->setEnabled(true);
    } else {
	m_pWallpaperBox->setEnabled(true);
	m_pBrowseBut->setEnabled(true);
	m_pMSetupBut->setEnabled(false);
    }
}


void KBackground::slotWallpaper(const QString &wallpaper)
{
    int desk = m_Desk;
    if (m_pGlobals->commonBackground())
	desk = 0;
    KBackgroundRenderer *r = m_Renderer[desk];

    if (wallpaper == r->wallpaper())
	return;

    r->stop();
    r->setWallpaper(wallpaper);
    r->start();
}


void KBackground::slotBrowseWallpaper()
{
    int desk = m_Desk;
    if (m_pGlobals->commonBackground())
	desk = 0;
    KBackgroundRenderer *r = m_Renderer[desk];

    QString file = KFileDialog::getOpenFileName();
    if (file.isEmpty())
	return;
    if (file == r->wallpaper())
	return;

    if (!m_Wallpaper.contains(file)) {
	int count = m_Wallpaper.count();
	m_Wallpaper[file] = count;
	m_pWallpaperBox->insertItem(file);
	m_pWallpaperBox->setCurrentItem(count);
    }

    r->stop();
    r->setWallpaper(file);
    r->start();
}


/*
 * Called from the "Wallpaper Arrangement" combobox.
 */
void KBackground::slotWPMode(int mode)
{
    int desk = m_Desk;
    if (m_pGlobals->commonBackground())
	desk = 0;
    KBackgroundRenderer *r = m_Renderer[desk];

    if (mode == r->wallpaperMode())
	return;

    r->stop();
    r->setWallpaperMode(mode);
    r->start();
}
    

void KBackground::slotSetupMulti()
{
    qDebug("slotSetupMulti");
}


void KBackground::slotPreviewDone(int desk_done)
{
    qDebug("Preview for desktop %d done", desk_done);

    int desk = m_Desk;
    if (m_pGlobals->commonBackground())
	desk = 0;
    if (desk != desk_done)
	return;
    KBackgroundRenderer *r = m_Renderer[desk];

    KPixmap pm;
    if (QPixmap::defaultDepth() < 15)
	pm.convertFromImage(*r->image(), KPixmap::LowColor);
    else
	pm.convertFromImage(*r->image());

    m_pMonitor->setBackgroundPixmap(pm);
}
    

#include "backgnd.moc"
